/**
 *	File: shell.c
 *	Course: POS - Advanced Operating Systems
 *	Project: 2. Shell
 *	Name: Tomas Bruckner, xbruck02@stud.fit.vutbr.cz
 *	Date: 2016-04-16
 *	Description:
 **/

#define _POSIX_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"

#define BUFFER_SIZE 513
#define TOKEN_DELIMITER " \t\n\v\f\r"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char buffer[BUFFER_SIZE];
int reading = 0, notend = 1;

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

        char **argv = NULL;
        argv = parse_argv();
        if(strcmp(argv[0], "exit") == 0){
            notend = 0;
        }
        else{
            pid_t pid = fork();
            if(pid == 0){
            
                if(execvp(argv[0], argv) == -1){

                }
                _exit(0);
            }
            else if(pid > 0){
                int status = -1;
                waitpid(-1, &status, 0);
            }
            else{
            }
        }

        free(argv);
        reading = 1;       
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

	return (void *)0;
}

char **parse_argv(){
    char **tokens = malloc(BUFFER_SIZE * sizeof(char*));   
    if(!tokens){

    }

    char *token = NULL;
    int index = 0;
    token = strtok(buffer, TOKEN_DELIMITER);
    tokens[index++] = token;

    while(token != NULL){
        token = strtok(NULL, TOKEN_DELIMITER);
        tokens[index++] = token;
    }
    
    tokens[index] = NULL;
    return tokens;
}

void print_prompt(){
    printf("$ ");
    fflush(stdout);
}

int main(int argc, char **argv)
{
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

