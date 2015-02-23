#ifndef	DEFS_H
#define DEFS_H

/*
 * Input Modes
 */
#define I_FILE  1
#define I_PIPE  2

/*
 * Output Modes
 */
#define O_WRITE        1
#define O_APPND        2
#define O_PIPE         3

/*
 * Run Next Command Modes
 */
#define NEXT_ON_ANY        1
#define NEXT_ON_SUCCESS    2
#define NEXT_ON_FAIL       3


/*
 * Parse State
 */
#define NEED_ANY_TOKEN 0
#define NEED_NEW_COMMAND 1
#define NEED_IN_PATH 2
#define NEED_OUT_PATH 3

extern int cerror;


/* A C sytle linked list to parse and build the argv structure */
typedef struct ArgX{
        char *arg;
        struct ArgX *next;
} Arg;


/* A C sytle linked list to parse the input line */
typedef struct CommandX{
        int  parse_state;
        // holds actual command
        char *token;

        Arg  *arg_list;
        Arg  *last_arg;

        char *input_file;
        int  input_mode;
        int  input_fd;

        char *output_file;
        int  output_mode;
        int  output_fd;

        int  next_command_exec_on;
        pid_t pid;
        struct CommandX *next;
} Command;

#endif
