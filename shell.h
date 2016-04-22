typedef struct{
    int inredirect;
    int outredirect;
    int background;
    char * infile;
    char * outfile;	
    char ** argv;
} Arguments;

void *read_thread(void *arg);

void *process_thread(void *arg);

void print_prompt();

Arguments parse_argv();

void free_resources();

// vim: expandtab:shiftwidth=4:tabstop=4:softtabstop=0:textwidth=120

