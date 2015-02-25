#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"comm.h"
#include"osh.h"

int dflag = 0;

int main(int argc, char *argv[])
{

    // check for options
    int opt;
    while((opt = getopt(argc, argv, "d")) != -1)
    {
        switch(opt)
        {
            // debug
            case 'd': dflag = 1;
                      printf("WARNING: debug flag found, output will be very verbose\n");
                      break;
            case '?': printf("%s: invalid option -- '%c'\n", argv[0], opt);
                      break;
        }
    }

    while(1)
    {
        // prompt
        printf("osh> ");
        fflush(stdout);

        char arg_line[MAX_IN];
        int bytes_read;

        bytes_read = read(STDIN_FILENO, arg_line, MAX_IN - 1);

        // remove implicit newline from cl input
        arg_line[bytes_read - 1] = '\0';

        // check for exit command
        if(strcmp(arg_line, EXIT) == 0)
        {
            break;
        }

        // get head for command structure list
        Command h_cmd;

        parse_command_line(arg_line, &h_cmd);

        print_cmd_list(&h_cmd);
    }

    return 0;
}

void parse_command_line(char *in_line, Command *cmd)
{
    // point to our current command and initialize
    Command *c_cmd = cmd;
    init_cmd(c_cmd);

    // initialize our states
    int last_parse_state = COMM_ST;
    int active_parse_state = COMM_ST;
    c_cmd->parse_state = COMM_ST;

    char *active_tok;

    // need a mutable string to tokenize
    char *in_copy = (char *) malloc(MAX_IN);
    strcpy(in_copy, in_line);

    active_tok = strtok(in_copy,SEP);

    // begin state machine
    while(active_parse_state != COMPL_ST && active_parse_state != ERR_ST)
    {
        // COMM_ST is our initial state for all commands
        if(active_parse_state == COMM_ST)
        {
            print_debug("COMMAND");

            parse_command(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

            print_debug("EXIT COMMAND");
            print_debug(c_cmd->token);

        } else if(active_parse_state == ARG_ST)
        {
            print_debug("ARG");

            parse_arg(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

            print_debug("EXITING ARG");

        } else if(active_parse_state == IN_ST)
        {
            print_debug("IN_ST");

            parse_in(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

            print_debug("EXITING IN_ST");

        } else if(active_parse_state == OUT_ST)
        {
            print_debug("OUT_ST");

            parse_out(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

            print_debug("EXITING OUT_ST");

        } else if(active_parse_state == FILE_ST)
        {
            print_debug("FILE_ST");

            parse_file(c_cmd, active_tok, last_parse_state);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

            print_debug("EXITING FILE_ST");

        } else if(active_parse_state == PIPE_ST)
        {
            print_debug("PIPE_ST");

            active_tok = strtok(NULL, SEP);

            if(valid_pipe(c_cmd, active_tok))
            {
                // if we've reached a pipe, the current command is complete
                c_cmd->parse_state = COMPL_ST;

                // link the new command to the completed one
                c_cmd->next = (Command *) malloc(sizeof(Command));

                // make new command active
                c_cmd = c_cmd->next;
                init_cmd(c_cmd);

                // initialize new command state and input mode
                c_cmd->parse_state = COMM_ST;
                c_cmd->input_mode = I_PIPE;

                print_debug("EXITING PIPE_ST VALID");

            }

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

        } else if(active_parse_state == NEXT_ST)
        {
            print_debug("NEXT_ST");

            active_tok = strtok(NULL, SEP);

            if(!contains_special_char(active_tok))
            {
                // if here we've run into one of the next command tokens, '||', '&&', or ';'
                c_cmd->parse_state = COMPL_ST;
                c_cmd->next = (Command *) malloc(sizeof(Command));
                c_cmd = c_cmd->next;
                init_cmd(c_cmd);
                c_cmd->parse_state = COMM_ST;

                print_debug("EXITING NEXT_ST VALID");
            }

            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;

        }
        if(c_cmd->parse_state == ERR_ST)
        {
            cmd->parse_state = ERR_ST;
            cmd->err_msg = c_cmd->err_msg;
        }
    }
}

// ensures command has no special characters,
// then sets token value of command struct
void parse_command(Command *cmd, char *token)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Command contains special character.";
    } else
    {
        // assume valid command
        cmd->token = token;
    }
}

// checks token for special characters
// then builds argument link list
void parse_arg(Command *cmd, char *token)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Argument contains special character.";
    } else
    {
        // valid arg, creating or appending cmd's arg list

        Arg *new_arg = (Arg *) malloc(sizeof(Arg));
        new_arg->arg = token;

        if(cmd->arg_list == NULL)
        {
            // first argument, creating list
            cmd->arg_list = new_arg;
            cmd->last_arg = new_arg;
        } else
        {
            cmd->last_arg->next = new_arg;
            cmd->last_arg = new_arg;
        }

    }
}

void parse_in(Command *cmd, char *token)
{
    // ensures command structure doesn't already have an input mode (AMBIGUOUS INPUT)
    // also checks to make sure the token is an input redirect (Should already be done, but just in case)
    if(cmd->input_mode != 0 || strlen(token) != 1 || token[0] != IN_RE)
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Ambiguous input redirect.";
        return;
    } else
    {
        // if we have an input redirect, we will need a file for input
        cmd->input_mode = I_FILE;
        cmd->parse_state = FILE_ST;
    }

}

void parse_out(Command *cmd, char *token)
{
    int tok_length = strlen(token);

    // ensures command structure doesn't already have an output mode (AMBIGUOUS OUTPUT)
    // determines whether we write or append to the file
    if(cmd->output_mode == 0 && tok_length == 1 && token[0] == OUT_RE)
    {
        cmd->output_mode = O_WRITE;
        cmd->parse_state = FILE_ST;
    } else if(cmd->output_mode == 0 && tok_length == 2 && (strcmp(token, APP_RE) == 0))
    {
        cmd->output_mode = O_APPND;
        cmd->parse_state = FILE_ST;
    } else
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Ambiguous output redirect.";
    }
}

// places file in input/output file element of command structure, depending on the current state
void parse_file(Command *cmd, char *token, int last_parse_state)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Filename contains special character.";
    } else
    {
        if(last_parse_state == IN_ST)
        {
            cmd->input_file = token;
        } else if(last_parse_state == OUT_ST)
        {
            cmd->output_file = token;
        } else
        {
            cmd->parse_state = ERR_ST;
            cmd->err_msg = "Attempted file parse without valid state.";
        }
    }
}

int valid_pipe(Command *cmd, char * token)
{
    // ensure output mode isn't already set (AMBIGUOUS OUTPUT)
    if(cmd->output_mode != 0)
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Ambiguous output redirect.";
        return 0;
    }

    cmd->output_mode = O_PIPE;
    cmd->next_command_exec_on = NEXT_ON_ANY;

    // if it's a pipe, there has to be a command after
    if(token == NULL)
    {
        cmd->parse_state = ERR_ST;
        cmd->err_msg = "Expected command following pipe.";
        return 0;
    }

    return 1;
}

// determines next state to move to from the following states:
// COMM_ST
// ARG_ST
// FILE_ST
void next_state(Command *cmd, char *token)
{
    // no next token, finally done
    if(token == NULL)
    {
        cmd->parse_state = COMPL_ST;
        return;
    }

    // assume next state is arg, then check
    cmd->parse_state = ARG_ST;

    int tok_length = strlen(token);

    // if token is length 1, could be a
    // pipe '|'
    // redirect '<' '>'
    // exec ';'
    if(tok_length == 1)
    {
        if(token[0] == OUT_RE)
        {
            cmd->parse_state = OUT_ST;
        } else if(token[0] == IN_RE)
        {
            cmd->parse_state = IN_ST;
        } else if(token[0] == PIPE)
        {
            cmd->parse_state = PIPE_ST;
        } else if(token[0] == EXEC)
        {
            cmd->parse_state = NEXT_ST;
            cmd->next_command_exec_on = NEXT_ON_ANY;
        }

    // if token is length 2, could be a
    // append '>>'
    // exec on success '&&'
    // exec on fail '||'
    } else if(tok_length == 2)
    {
        if(strcmp(token, APP_RE) == 0)
        {
            cmd->parse_state = OUT_ST;
        } else if(strcmp(token, SUCC_EX) == 0)
        {
            cmd->parse_state = NEXT_ST;
            cmd->next_command_exec_on = NEXT_ON_SUCCESS;
        } else if( strcmp(token, FAIL_EX) == 0)
        {
            cmd->parse_state = NEXT_ST;
            cmd->next_command_exec_on = NEXT_ON_FAIL;
        }
    }
}

// initialize command structure to avoid unknown behavior of memory pointers
void init_cmd(Command *cmd)
{
    cmd->parse_state = 0;
    cmd->token = NULL;
    cmd->err_msg = NULL;
    cmd->arg_list = NULL;
    cmd->last_arg = NULL;
    cmd->input_file = NULL;
    cmd->input_mode = 0;
    cmd->output_file = NULL;
    cmd->output_mode = 0;
    cmd->next = NULL;
    cmd->next_command_exec_on = 0;
    cmd->pid = 0;
}

int contains_special_char(char *token)
{
    int i = 0;
    while(token[i] != '\0')
    {
        if(     token[i] == OUT_RE ||
                token[i] == IN_RE ||
                token[i] == PIPE ||
                token[i] == AMP ||
                token[i] == EXEC
          )
        {
            return 1;
        }
        i++;
    }

    return 0;
}

void print_cmd_list(Command *h_cmd)
{
    Command *c_cmd = h_cmd;

    if(c_cmd->parse_state == ERR_ST)
    {
        printf("%s\n", c_cmd->err_msg);
    } else
    {

        while(c_cmd != NULL)
        {
            printf("-------------------------\n");
            print_cmd(c_cmd);
            c_cmd = c_cmd->next;
        }

        printf("-------------------------\n");
    }

}

void print_cmd(Command *c_cmd)
{
    printf("Cmd: %s\n", c_cmd->token);
    Arg *c_arg = c_cmd->arg_list;
    while(c_arg != NULL)
    {
        printf("Arg: %s\n", c_arg->arg);
        c_arg = c_arg->next;
    }
    printf("input file: %s\n", c_cmd->input_file);
    printf("Input mode: %s\n", input_to_str(c_cmd->input_mode));
    printf("output file: %s\n", c_cmd->output_file);
    printf("Output mode: %s\n", output_to_str(c_cmd->output_mode));
    printf("Command combine mode: %s\n", exec_to_str(c_cmd->next_command_exec_on));
}

// translates parse state to string
char *state_to_str(int state)
{
    char *state_str;

    switch(state)
    {
        case COMPL_ST: state_str = "COMPL_ST";
                       break;
        case COMM_ST: state_str = "COMM_ST";
                      break;
        case ARG_ST: state_str = "ARG_ST";
                     break;
        case IN_ST: state_str = "IN_ST";
                    break;
        case OUT_ST: state_str = "OUT_ST";
                    break;
        case FILE_ST: state_str = "FILE_ST";
                      break;
        case PIPE_ST: state_str = "PIPE_ST";
                      break;
        case NEXT_ST: state_str = "NEXT_ST";
                      break;
        case ERR_ST: state_str = "ERR_ST";
                     break;
        default: state_str = "-";
                 break;
    }

    return state_str;
}

// translates input_mode to string
char *input_to_str(int in_mode)
{
    char *in_string;

    switch(in_mode)
    {
        case I_FILE: in_string = "I_FILE";
                     break;
        case I_PIPE: in_string = "I_PIPE";
                     break;
        default: in_string = "-";
                 break;
    }

    return in_string;
}

// translates output_mode to string
char *output_to_str(int out_mode)
{
    char *out_string;

    switch(out_mode)
    {
        case O_WRITE: out_string = "O_WRITE";
                      break;
        case O_APPND: out_string = "O_APPND";
                      break;
        case O_PIPE: out_string = "O_PIPE";
                     break;
        default: out_string = "-";
                 break;
    }

    return out_string;
}

// translates exec mode to string
char *exec_to_str(int exec_mode)
{
    char *exec_string;

    switch(exec_mode)
    {
        case NEXT_ON_ANY: exec_string = "NEXT_ON_ANY";
                          break;
        case NEXT_ON_SUCCESS: exec_string = "NEXT_ON_SUCCESS";
                              break;
        case NEXT_ON_FAIL: exec_string = "NEXT_ON_FAIL";
                           break;
        default: exec_string = "-";
                 break;
    }

    return exec_string;
}

void print_debug(char *line)
{
    if(dflag)
        printf("DEBUG: %s\n", line);
}
