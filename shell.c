/**
 *	File: shell.c
 *	Course: POS - Advanced Operating Systems
 *	Project: 2. Shell
 *	Name: Tomas Bruckner, xbruck02@stud.fit.vutbr.cz
 *	Date: 2016-04-16
 *	Description:
 **/

//#define _POSIX_SOURCE
#define _XOPEN_SOURCE 700

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
int reading = 0, notend = 1;
pid_t pid;

void sig_handler(int signo){
    if(signo == SIGINT){
        kill(pid, SIGINT);
        print_prompt();
    }
    else if (signo == SIGCHLD){
        int status = -1;
        pid_t pid2 = waitpid(-1, &status, 0);
        printf("Process %d ended\n", pid2);
        fflush(stdout);
    }
}

void *read_thread(void *arg){
    int n = -1;
    while(notend){
        pthread_mutex_lock(&mutex);
        reading = 1;
        memset(buffer, '\0', BUFFER_SIZE);
        print_prompt();
        n = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        if(n >= BUFFER_SIZE){
            while( (n = getchar()) != EOF ) ;
            printf("Input is too long\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }
        
        reading = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while(!reading){
            pthread_cond_wait(&cond,&mutex);
        }

        pthread_mutex_unlock(&mutex);
    }

	return (void *)0;
}

void *process_thread(void *arg){
    while(notend){
        pthread_mutex_lock(&mutex);
        while(reading){
            pthread_cond_wait(&cond,&mutex);
        }

        if(strlen(buffer) == 0){
            // should not happen, print error
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        Arguments arguments;
        arguments = parse_argv();
        if(strcmp(arguments.argv[0], "exit") == 0){
            notend = 0;
        }
        else{
            pid = fork();
            if(pid == 0){
                if(arguments.inredirect){
                    int fd = open(arguments.infile, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if(arguments.outredirect){
                    int fd = creat(arguments.outfile, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                
                if(arguments.background){

                }
        
                if(execvp(arguments.argv[0], arguments.argv) == -1){
                    fprintf(stderr, "Prikaz nenalezen!\n");
                }
                _exit(0); // fail
            }
            else if(pid > 0){
                if(arguments.background){
                
                }
                else{
                    int status;
                    waitpid(-1, &status, 0); 
                }
            }
            else{
                fprintf(stderr, "Fork se nepovedl\n");
            }
        }

        free(arguments.argv);
        reading = 1;       
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

	return (void *)0;
}

Arguments parse_argv(){
    Arguments arguments; 
    arguments.argv = malloc(BUFFER_SIZE * sizeof(char*));   
    if(!arguments.argv){

    }

    char *token = NULL;
    int index = 0;

    token = strtok(buffer, TOKEN_DELIMITER);
    arguments.argv[index++] = token;

    while(token != NULL){
        token = strtok(NULL, TOKEN_DELIMITER);
        arguments.argv[index++] = token;
    }
    
    arguments.argv[index] = NULL;
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

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
	pthread_t pt, pt2;
	pthread_attr_t attr;
	void *statp;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); 

	if (pthread_create(&pt, &attr, read_thread, NULL) != 0 
        || pthread_create(&pt2, &attr, process_thread, NULL) != 0) {
		printf("pthread_create() err=%d\n", errno);
		exit(1);
	}
	if (pthread_join(pt, &statp) == -1 || pthread_join(pt, &statp) == -1) {
		printf("pthread_join() err=%d\n", errno);
		exit(1);
	}

    free_resources();

	return(0);
}

void free_resources(){
     pthread_mutex_destroy(&mutex);
}

// vim: expandtab:shiftwidth=4:tabstop=4:softtabstop=0:textwidth=120

