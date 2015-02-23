// maximum input size
#define MAX_IN 80
#define SEP " "

// state of parse
#define COMPL_ST    0
#define COMM_ST     1
#define ARG_ST      2
#define IN_ST       3
#define OUT_ST      4
#define FILE_ST     5
#define PIPE_ST     6
#define NEXT_ST     7
#define ERR_ST      -1

#define OUT_RE  '>'
#define APP_RE  ">>"
#define IN_RE   '<'
#define PIPE    '|'
#define SUCC_EX "&&"
#define AMP     '&'
#define FAIL_EX "||"
#define EXEC    ';'

void parse_command_line(char *cl_input, Command *);

void parse_command(Command *, char *);
void parse_arg(Command *, char *);
void parse_in(Command *, char *);
void parse_out(Command *, char *);
void parse_file(Command *, char *, int);
int valid_pipe(Command *, char *);
void next_state(Command *, char *);
void init_cmd(Command *);
int contains_special_char(char *);
