/**
 *	File: shell.h
 *	Course: POS - Advanced Operating Systems
 *	Project: 2. Shell
 *	Name: Tomas Bruckner, xbruck02@stud.fit.vutbr.cz
 *	Date: 2016-04-16
 *	Description:
 **/

#define BUFFER_SIZE 513
#define TOKEN_DELIMITER " \t\n\v\f\r"
#define TRUE 1
#define FALSE 0

typedef struct{
    int inredirect;     // true/false if input is redirected
    int outredirect;    // true/false if output is redirected
    int background;     // true/false if program should be run in background
    char *infile;       // input file
    char *outfile;	    // output file
    char **argv;        // program arguments
} Arguments;

/*
 * Reading stdin.
 */
void *read_thread(void *arg);

/*
 *  Processing command from user.
 */
void *process_thread(void *arg);

/*
 * SIGINT and SIGCHLD handler.
 */
void sig_handler(int signo);

/*
 * Prints prompt.
 */
void print_prompt();

/*
 * Parsing program arguments.
 */
Arguments parse_argv();

/*
 * Frees resource at the end of shell.
 */
void free_resources();

// vim: expandtab:shiftwidth=4:tabstop=4:softtabstop=0:textwidth=120

