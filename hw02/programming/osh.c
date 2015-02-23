#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"comm.h"
#include"osh.h"

int debug = 1;

int main(int argc, char *argv[])
{

    // prompt
    //printf("osh> ");
    //fflush(stdout);

    // input buffer and read
    //char in_buf[IN_BUFFSIZE];
    //read(STDIN_FILENO, in_buf, IN_BUFFSIZE - 1);

    char *arg_line = "ls -a -l /home/ | cat test.txt ; ls > out.txt";
    Command h_cmd;

    parse_command_line(arg_line, &h_cmd);

    print_cmd_list(&h_cmd);

    return 0;
}

void parse_command_line(char *in_line, Command *cmd)
{
    print_debug(31, in_line);
    // point to our current command and initialize
    Command *c_cmd = cmd;
    init_cmd(c_cmd);

    // initialize our last state
    int last_parse_state = COMM_ST;
    int active_parse_state = COMM_ST;
    c_cmd->parse_state = COMM_ST;

    char *active_tok;

    // need a mutable string to tokenize
    char *in_copy = (char *) malloc(MAX_IN);
    strcpy(in_copy, in_line);
    active_tok = strtok(in_copy,SEP);

    while(active_parse_state != COMPL_ST && active_parse_state != ERR_ST)
    {
        if(active_parse_state == COMM_ST)
        {
            print_debug(54, "COMMAND");
            parse_command(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
            print_debug(60, "EXIT COMMAND");
            print_debug(60, c_cmd->token);
        } else if(active_parse_state == ARG_ST)
        {
            print_debug(66, "ARG");
            parse_arg(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
            print_debug(72, "EXITING ARG");
        } else if(active_parse_state == IN_ST)
        {
            print_debug(77, "IN_ST");
            parse_in(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
            print_debug(76, "EXITING IN_ST");
        } else if(active_parse_state == OUT_ST)
        {
            print_debug(77, "OUT_ST");
            parse_out(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
            print_debug(84, "EXITING OUT_ST");
        } else if(active_parse_state == FILE_ST)
        {
            print_debug(87, "FILE_ST");
            parse_file(c_cmd, active_tok, last_parse_state);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
            print_debug(93, "EXITING FILE_ST");
        } else if(active_parse_state == PIPE_ST)
        {
            print_debug(96, "PIPE_ST");
            active_tok = strtok(NULL, SEP);
            if(valid_pipe(c_cmd, active_tok))
            {
                c_cmd->parse_state = COMPL_ST;
                c_cmd->next = (Command *) malloc(sizeof(Command));
                c_cmd = c_cmd->next;
                init_cmd(c_cmd);
                c_cmd->parse_state = COMM_ST;
                c_cmd->input_mode = I_PIPE;
                last_parse_state = active_parse_state;
                active_parse_state = c_cmd->parse_state;
                print_debug(108, "EXITING PIPE_ST VALID");
            }
        } else if(active_parse_state == NEXT_ST)
        {
            print_debug(112, "NEXT_ST");
            active_tok = strtok(NULL, SEP);
            if(!contains_special_char(active_tok))
            {
                c_cmd->parse_state = COMPL_ST;
                c_cmd->next = (Command *) malloc(sizeof(Command));
                c_cmd = c_cmd->next;
                init_cmd(c_cmd);
                c_cmd->parse_state = COMM_ST;
                last_parse_state = active_parse_state;
                active_parse_state = c_cmd->parse_state;
                print_debug(123, "EXITING NEXT_ST VALID");
            }
        }
    }


}

void parse_command(Command *cmd, char *token)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
    } else
    {
        // assume valid command
        cmd->token = token;
    }
}

void parse_arg(Command *cmd, char *token)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
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
    if(cmd->input_mode != 0 || strlen(token) != 1 || token[0] != IN_RE)
    {
        cmd->parse_state = ERR_ST;
        return;
    } else
    {
        cmd->input_mode = I_FILE;
    }

    cmd->parse_state = FILE_ST;
}

void parse_out(Command *cmd, char *token)
{
    int tok_length = strlen(token);

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
    }
}

void parse_file(Command *cmd, char *token, int last_parse_state)
{
    if(contains_special_char(token))
    {
        cmd->parse_state = ERR_ST;
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
        }
    }
}

int valid_pipe(Command *cmd, char * token)
{
    cmd->output_mode = O_PIPE;
    if(token == NULL)
    {
        cmd->parse_state = ERR_ST;
        return 0;
    }

    return 1;
}

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

void init_cmd(Command *cmd)
{
    cmd->parse_state = 0;
    cmd->token = NULL;
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

    while(c_cmd != NULL)
    {
        print_cmd(c_cmd);
        c_cmd = c_cmd->next;
    }

}

void print_cmd(Command *c_cmd)
{
    printf("parse_state: %s\n", state_to_str(c_cmd->parse_state));
    printf("command: %s\n", c_cmd->token);
    printf("arguments: \n");
    Arg *c_arg = c_cmd->arg_list;
    while(c_arg != NULL)
    {
        printf("\t%s\n", c_arg->arg);
        c_arg = c_arg->next;
    }
    printf("input_mode: %s\n", input_to_str(c_cmd->input_mode));
    printf("input_file: %s\n", c_cmd->input_file);
    printf("output_mode: %s\n", output_to_str(c_cmd->output_mode));
    printf("output_file: %s\n", c_cmd->output_file);
    printf("next_command_exec_on: %s\n", exec_to_str(c_cmd->next_command_exec_on));
}

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
        default: state_str = "Problem determining state";
                 break;
    }

    return state_str;
}

char *input_to_str(int in_mode)
{
    char *in_string;

    switch(in_mode)
    {
        case I_FILE: in_string = "I_FILE";
                     break;
        case I_PIPE: in_string = "I_PIPE";
                     break;
        default: in_string = "Problem determining input_mode";
                 break;
    }

    return in_string;
}

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
        default: out_string = "Problem determing output_mode";
                 break;
    }

    return out_string;
}

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
        default: exec_string = "Problem determing next_command_exec_on";
                 break;
    }

    return exec_string;
}

void print_debug(int line_num, char *line)
{
    if(debug)
    printf("%d: %s\n", line_num, line);
}
