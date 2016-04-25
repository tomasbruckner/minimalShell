#define BUFFER_SIZE 513
#define TOKEN_DELIMITER " \t\n\v\f\r"
#define TRUE 1
#define FALSE 0


typedef struct{
    int inredirect;
    int outredirect;
    int background;
    char *infile;
    char *outfile;	
    char **argv;
} Arguments;

void *read_thread(void *arg);

void *process_thread(void *arg);

void print_prompt();

Arguments parse_argv();

void free_resources();

int check_char_index(char ** argv, const char * c);

// vim: expandtab:shiftwidth=4:tabstop=4:softtabstop=0:textwidth=120

