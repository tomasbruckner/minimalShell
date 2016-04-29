/**
 *	File: shell.c
 *	Course: POS - Advanced Operating Systems
 *	Project: 2. Shell
 *	Name: Tomas Bruckner, xbruck02@stud.fit.vutbr.cz
 *	Date: 2016-04-16
 *	Description:
 **/

#define _POSIX_C_SOURCE 199506L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "shell.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

char buffer[BUFFER_SIZE];
int reading = 1,
    notend = 1;
pid_t pid = -1; // actual foreground process

int main(int argc, char **argv){
    // stops buffering
    setbuf(stdout, NULL);
	pthread_t pt, pt2;
	pthread_attr_t attr;
	void *statp;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // starts threads
	if (pthread_create(&pt, &attr, read_thread, NULL) != 0
        || pthread_create(&pt2, &attr, process_thread, NULL) != 0) {
		printf("pthread_create() err=%d\n", errno);
		exit(1);
	}

    // waits for shell exit
	if (pthread_join(pt, &statp) == -1 || pthread_join(pt, &statp) == -1) {
		printf("pthread_join() err=%d\n", errno);
		exit(1);
	}

    free_resources();

	return(0);
}

void *read_thread(void *arg){
    int n = -1; // number of bytes returned from read(3)

    // reading thread doesn't handle signals
    sigset_t sig_mask;
    sigfillset(&sig_mask);
    pthread_sigmask(SIG_SETMASK, &sig_mask, NULL);

    while(notend){
        pthread_mutex_lock(&mutex);

        // clears buffer
        memset(buffer, '\0', BUFFER_SIZE);
        print_prompt();
        n = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        // handles only <= 512 bytes of input
        if(n >= BUFFER_SIZE){
            while( (n = getchar()) != EOF ) ;
            printf("Prilis velky vstup!\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }

        // stops reading, starts processing
        reading = 0;
        pthread_cond_signal(&cond);
        while(!reading){
            pthread_cond_wait(&cond,&mutex);
        }

        pthread_mutex_unlock(&mutex);
    }

	return (void *)0;
}

void *process_thread(void *arg){
    // handling signals in process thread
    struct sigaction sig_a;
    sigfillset(&sig_a.sa_mask);
    sig_a.sa_handler = sig_handler;
    sig_a.sa_flags = 0;
    sigaction(SIGINT, &sig_a, NULL);
    sigaction(SIGCHLD, &sig_a, NULL);

    while(notend){
        pthread_mutex_lock(&mutex);
        while(reading){
            pthread_cond_wait(&cond,&mutex);
        }

        Arguments arguments;
        arguments = parse_argv();

        // invalid input
        if(arguments.argv[0] == NULL){
            printf("Nevalidni vstup!\n");
            reading = 1;
            free(arguments.argv);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        // exit
        if(strcmp(arguments.argv[0], "exit") == 0){
            notend = 0;
        }
        // executing command
        else{
            pid = fork();
            if(pid == 0){
                // default handling in child
                sig_a.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sig_a, NULL);
                sigaction(SIGCHLD, &sig_a, NULL);

                // input redirect
                if(arguments.inredirect){
                    int fd = open(arguments.infile, O_RDONLY);
                    if(fd == -1){
                        fprintf(stderr, "Chyba otevreni vstupniho souboru %s.\n", arguments.infile);
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // output redirect
                if(arguments.outredirect){
                    int fd = creat(arguments.outfile, 0644);
                    if(fd == -1){
                        fprintf(stderr, "Chyba vyvoreni souboru %s.\n", arguments.outfile);
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                // background process blocks interrupt
                if(arguments.background){
                    sigset_t mask;
                    sigemptyset(&mask);
                    sigaddset(&mask, SIGINT);
                    sigprocmask(SIG_SETMASK, &mask, NULL);
                }

                if(execvp(arguments.argv[0], arguments.argv) == -1){
                    fprintf(stderr, "Prikaz nenalezen!\n");
                }

                _exit(1); // fail
            }
            else if(pid > 0){
                // don't save background process pid
                if(arguments.background){
                    pid = -1;
                }
                // foreground
                else{
                    // waits for SIGCHLD
                    sigset_t mask;
                    sigemptyset(&mask);

                    while(pid > 0){
                        sigsuspend(&mask);
                    }
                }
            }
            // fork error
            else{
                fprintf(stderr, "Fork se nepovedl\n");
            }
        }

        free(arguments.argv);
        // stops processing, starts reading
        reading = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

	return (void *)0;
}

void sig_handler(int signo){
    // interrupt foreground process
    if(signo == SIGINT && pid > 0){
        kill(pid, SIGINT);
        printf("Process %d interrupted!\n", pid);
    }
    // process ended
    else if (signo == SIGCHLD){
        int status = -1;
        pid_t pid2 = waitpid(-1, &status, 0);

        // foreground process
        if(pid == pid2){
            pid = -1;
        }
        // background process
        else{
            printf("Background process %d ended!\n", pid2);
            print_prompt();
        }
    }
}

Arguments parse_argv(){
    Arguments arguments;
    arguments.argv = NULL;
    arguments.inredirect = FALSE;
    arguments.outredirect = FALSE;
    arguments.background = FALSE;

    arguments.argv = malloc(BUFFER_SIZE * sizeof(char*));
    if(!arguments.argv){
        fprintf(stderr, "Selhal malloc!\n");
        return arguments;
    }

    char *token = NULL;
    int index = 0;
    token = strtok(buffer, TOKEN_DELIMITER);

    // NULL is checked outside function
    arguments.argv[index++] = token;

    while(token != NULL){
        token = strtok(NULL, TOKEN_DELIMITER);
        arguments.argv[index++] = token;
    }

    // last argument must be NULL
    arguments.argv[index] = NULL;

    // check for redirects or background proccesing
    for(int i = 1; i < index -1; i++){
        if(arguments.argv[i][0] == '<'){
            arguments.inredirect = TRUE;
            arguments.infile = arguments.argv[i] + 1;
            arguments.argv[i] = NULL;
        }
        else if(arguments.argv[i][0] == '>'){
            arguments.outredirect = TRUE;
            arguments.outfile = arguments.argv[i] + 1;
            arguments.argv[i] = NULL;
        }
        else if(arguments.argv[i][0] == '&'){
            arguments.background = TRUE;
            arguments.argv[i] = NULL;
        }
    }

    return arguments;
}

void print_prompt(){
    fflush(stdout);
    printf("$ ");
    fflush(stdout);
}

void free_resources(){
     pthread_mutex_destroy(&mutex);
}

// vim: expandtab:shiftwidth=4:tabstop=4:softtabstop=0:textwidth=120
