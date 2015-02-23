#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"comm.h"
#include"osh.h"

int main(int argc, char *argv[])
{

    // prompt
    //printf("osh> ");
    //fflush(stdout);

    // input buffer and read
    //char in_buf[IN_BUFFSIZE];
    //read(STDIN_FILENO, in_buf, IN_BUFFSIZE - 1);

    char *arg_line = "ls -a -l /home/";
    Command h_cmd;

    parse_command_line(arg_line, &h_cmd);

    return 0;
}

void parse_command_line(char *in_line, Command *cmd)
{
    printf("%s\n", in_line);
    // point to our current command and initialize
    Command *c_cmd = cmd;
    init_cmd(c_cmd);

    // initialize our last state
    int last_parse_state = COMM_ST;
    int active_parse_state = COMM_ST;
    c_cmd->parse_state = COMM_ST;

    // our current token we're acting on
    char *active_tok;
    active_tok = strtok(in_line, SEP);

    while(active_parse_state != COMPL_ST && active_parse_state != ERR_ST)
    {
        if(active_parse_state == COMM_ST)
        {
            parse_command(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
        } else if(active_parse_state == ARG_ST)
        {
            parse_arg(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
        } else if(active_parse_state == IN_ST)
        {
            parse_in(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
        } else if(active_parse_state == OUT_ST)
        {
            parse_out(c_cmd, active_tok);
            active_tok = strtok(NULL, SEP);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
        } else if(active_parse_state == FILE_ST)
        {
            parse_file(c_cmd, active_tok, last_parse_state);
            active_tok = strtok(NULL, SEP);
            next_state(c_cmd, active_tok);
            last_parse_state = active_parse_state;
            active_parse_state = c_cmd->parse_state;
        } else if(active_parse_state == PIPE_ST)
        {
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
            }
        } else if(active_parse_state == NEXT_ST)
        {
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
