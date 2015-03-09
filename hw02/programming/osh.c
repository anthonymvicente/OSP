#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
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
            case '?': fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], opt);
                      break;
        }
    }

    int i = 0;
    for(i = 0; i < 50; i++)
    {
        // prompt
        printf("osh> ");
        fflush(stdout);

        char arg_line[MAX_IN];
        int bytes_read;

        bytes_read = read(STDIN_FILENO, arg_line, MAX_IN);

        // remove implicit newline from cl input
        arg_line[bytes_read - 1] = '\0';

        // check for exit command
        // or CTRL+d
        if(strcmp(arg_line, EXIT) == 0)
        {
            break;
        } else if(bytes_read == 0)
        {
            printf("\n");
            break;
        } else if(bytes_read == MAX_IN)
        {
            fprintf(stderr, "command entered was too long.");
        } else if(bytes_read != 1)
        {
            // get head for command structure list
            Command h_cmd;

            parse_command_line(arg_line, &h_cmd);

            if(dflag || h_cmd.parse_state == ERR_ST)
            {
                print_cmd_list(&h_cmd);
            }

            if(h_cmd.parse_state != ERR_ST)
            {
                cmd_exec(&h_cmd);
            }

        }

        //mem_clean(&h_cmd);

        //print_argv(&h_cmd);
    }

    if(i == 50)
    {
        printf("You have reached your alloted command executions (50). Please restart the shell to continue.\n");
    }

    return 0;
}

void cmd_exec(Command *h_cmd)
{
    Command *c_cmd = h_cmd;
    pid_t ch_pid;
    char **argv;
    int wait_status;
    int exec_next;
    int pipe_count = 0;

    // fd for pipe
    int *pipe_fd;
    // read end of pipe
    int *pipe_fd_r;
    // write end of pipe
    int *pipe_fd_w;

    // pointer to pipe
    // will point to the last pipe if needed
    // useful for strings of pipes
    int *l_pipe_fd;
    int *l_pipe_fd_r;
    int *l_pipe_fd_w;

    while(c_cmd != NULL)
    {
        // assume we won't be executing another command after
        exec_next = 0;

        if(c_cmd->input_mode == I_PIPE)
        {
            l_pipe_fd = pipe_fd;
            l_pipe_fd_r = &l_pipe_fd[0];
            l_pipe_fd_w = &l_pipe_fd[1];
            c_cmd->input_fd = *l_pipe_fd_r;
        }

        if(c_cmd->output_mode == O_PIPE)
        {
            pipe_fd = (int *) malloc(sizeof(int) * 2);
            if(pipe(pipe_fd) < 0)
            {
                fprintf(stderr, "error in creating pipe.\n");
                exit(1);
            }
            pipe_fd_r = &pipe_fd[0];
            pipe_fd_w = &pipe_fd[1];
            c_cmd->output_fd = *pipe_fd_w;
        }

        // build argv
        argv = build_argv(c_cmd->token, c_cmd->arg_list);

        ch_pid = fork();
        if(ch_pid < 0)
        {
            fprintf(stderr, "problem forking.\n");
            exit(1);

        // child process
        } else if (ch_pid == 0)
        {
            // check for and setup file redirects
            file_redirect(c_cmd);

            if(c_cmd->output_mode == O_PIPE)
            {
                // dup pipe fd to stdout fd
                // dup2(*pipe_fd_w, STDOUT_FILENO);
                // close(*pipe_fd_w);
                dup2((c_cmd->output_fd), STDOUT_FILENO);
                close(c_cmd->output_fd);
            }

            // for I_PIPE modes, we need to read from the last pipe
            if(c_cmd->input_mode == I_PIPE)
            {
                // dup2(*l_pipe_fd_r, STDIN_FILENO);
                dup2(c_cmd->input_fd, STDIN_FILENO);
            }

            // exec cmd
            execvp(c_cmd->token, argv);

            // if we're here, something went wrong...
            fprintf(stderr, "exec failed.\n");
            exit(1);

        } else
        {
            c_cmd->pid = ch_pid;

            // if we're not using pipes at all, we can just wait
            if(c_cmd->input_mode != I_PIPE && c_cmd->output_mode != O_PIPE)
            {
                wait(&wait_status);
            }

            // if we're getting input from a pipe
            // we can close and cleanup
            // by now we're done with the last pipe
            if(c_cmd->input_mode == I_PIPE)
            {
                // close(*l_pipe_fd_r);
                // free(l_pipe_fd);
                close(c_cmd->input_fd);
                free(l_pipe_fd);

                // keep count of the number of pipes
                pipe_count++;

                // if we haven't encountered an output pipe
                // we can wrap up the commands that have been waiting
                if(c_cmd->output_mode != O_PIPE)
                {
                    for(; pipe_count > 0; pipe_count--)
                    {
                        wait(&wait_status);
                    }
                    wait(&wait_status);
                }
            }

            // done with the arguments
            free(argv);

            // if we're outputing to a pipe, we can close it now in parent
            // the target command will close the read end
            if(c_cmd->output_mode == O_PIPE)
            {
                // close(*pipe_fd_w);
                close(c_cmd->output_fd);

                // pipes imply execute the next command
                exec_next = 1;

            // check conditions for next command execution
            } else if(c_cmd->next_command_exec_on == NEXT_ON_ANY)
            {
                exec_next = 1;

            // if here, it could be an on success or on fail execution
            } else
            {
                if(c_cmd->next_command_exec_on == NEXT_ON_SUCCESS && WEXITSTATUS(wait_status) == 0)
                {
                    exec_next = 1;
                } else if (c_cmd->next_command_exec_on == NEXT_ON_FAIL && WEXITSTATUS(wait_status) != 0)
                {
                    exec_next = 1;
                }
            }

            if(exec_next)
            {
                c_cmd = c_cmd->next;
            } else
            {
                c_cmd = NULL;
            }

        }
    }
}

void file_redirect(Command *cmd)
{

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    if(cmd->input_mode == I_FILE)
    {
        // get fd of read file, set to close when finished
        input_fd = open(cmd->input_file, O_CLOEXEC | O_RDONLY);
        cmd->input_fd = input_fd;
    }
    if(input_fd < 0)
    {
        fprintf(stderr, "could not open file '%s' for input\n", cmd->input_file);
        exit(1);
    }
    if(input_fd != STDIN_FILENO && dup2(input_fd, STDIN_FILENO) < 0)
    {
        fprintf(stderr, "problem duping '%s' for stdin\n", cmd->input_file);
        exit(1);
    }

    // check for file output modes
    if(cmd->output_mode == O_WRITE)
    {
        output_fd = open(cmd->output_file, O_CLOEXEC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        cmd->output_fd = output_fd;
    }
    if(cmd->output_mode == O_APPND)
    {
        output_fd = open(cmd->output_file, O_CLOEXEC | O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        cmd->output_fd = output_fd;
    }
    if(output_fd < 0)
    {
        fprintf(stderr, "could not open file '%s' for output\n", cmd->output_file);
        exit(1);
    }
    if(output_fd != STDOUT_FILENO && dup2(output_fd, STDOUT_FILENO) < 0)
    {
        fprintf(stderr, "problem duping '%s' for stdout\n", cmd->output_file);
        exit(1);
    }

}

char **build_argv(char *command, Arg *arg_list)
{
    char **argv;
    int argc = 1;

    Arg *loop_arg = arg_list;

    while(loop_arg != NULL)
    {
        argc++;
        loop_arg = loop_arg->next;
    }

    argv = (char **) (malloc((sizeof(char *) * argc) + 1));

    int i = 1;
    loop_arg = arg_list;
    argv[0] = command;
    for(i = 1; i < argc; i++)
    {
        argv[i] = loop_arg->arg;
        loop_arg = loop_arg->next;
    }

    argv[i] = NULL;

    return argv;
}

// free up our commands after we're done
void mem_clean(Command *h_cmd)
{
    if(h_cmd == NULL)
    {
        return;
    }

    Command *c_cmd = h_cmd;
    Command *n_cmd;
    Arg *c_arg;
    Arg *n_arg;

    while(c_cmd->next != NULL)
    {
        n_cmd = c_cmd->next;
        c_arg = c_cmd->arg_list;

        printf("before arg free\n");
        while(c_arg != NULL)
        {
            printf("freeing args\n");
            n_arg = c_arg->next;
            free(c_arg);
            c_arg = n_arg;
        }

        printf("args freed\n");

        free(c_arg);

        free(c_cmd);
        c_cmd = n_cmd;
    }

    free(c_cmd);

    return;
}
