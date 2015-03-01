#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
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

    int i = 0;
    for(i = 0; i < 50; i++)
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

        cmd_exec(&h_cmd);

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
    // fd for pipe
    int pipe_fd[2];
    // read end of pipe
    int *pipe_fd_r = &pipe_fd[0];
    // write end of pipe
    int *pipe_fd_w = &pipe_fd[1];


    int inf_loop = 0;

    while(c_cmd != NULL)
    {
        if(c_cmd->output_mode == O_PIPE)
        {
            if(pipe(pipe_fd) < 0)
            {
                printf("error in creating pipe.\n");
                exit(1);
            }
        }
        ch_pid = fork();
        if(ch_pid < 0)
        {
            printf("problem forking.\n");
            exit(1);

        // child process
        } else if (ch_pid == 0)
        {
            // build argv
            argv = build_argv(c_cmd->token, c_cmd->arg_list);

            // check for and setup file redirects
            file_redirect(c_cmd);

            if(c_cmd->output_mode == O_PIPE)
            {
                // dup pipe fd to stdout fd
                dup2(*pipe_fd_w, STDOUT_FILENO);
                close(*pipe_fd_w);
            }
            if(c_cmd->input_mode == I_PIPE)
            {
                dup2(*pipe_fd_r, STDIN_FILENO);
            }

            // exec cmd
            execvp(c_cmd->token, argv);

            // if we're here, something went wrong...
            printf("exec failed.\n");
            exit(1);
        } else
        {
            wait(&wait_status);

            // if we're outputing to a pipe, we can close it now in parent
            if(c_cmd->output_mode == O_PIPE)
            {
                close(*pipe_fd_w);
            }

            c_cmd = c_cmd->next;
        }
        inf_loop++;
        if(inf_loop == 20)
        {
            printf("infinite loop?\n");
            break;
        }
    }
}

void file_redirect(Command *cmd)
{

    int input_fd;
    int output_fd;

    if(cmd->input_mode == I_FILE)
    {
        // get fd of read file, set to close when finished
        input_fd = open(cmd->input_file, O_CLOEXEC | O_RDONLY);
    }
    if(input_fd < 0)
    {
        printf("could not open file '%s' for input\n", cmd->input_file);
        exit(1);
    }
    if(dup2(input_fd, STDIN_FILENO) < 0)
    {
        printf("problem duping '%s' for stdin\n", cmd->input_file);
        exit(1);
    } else
    {
        cmd->input_fd = input_fd;
    }

    // check for file output modes
    if(cmd->output_mode == O_WRITE)
    {
        output_fd = open(cmd->output_file, O_CLOEXEC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    }
    if(cmd->output_mode == O_APPND)
    {
        output_fd = open(cmd->output_file, O_CLOEXEC | O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    }
    if(output_fd < 0)
    {
        printf("could not open file '%s' for output\n", cmd->output_file);
        exit(1);
    }
    if(dup2(output_fd, STDOUT_FILENO) < 0)
    {
        printf("problem duping '%s' for stdout\n", cmd->output_file);
        exit(1);
    } else
    {
        cmd->output_fd = output_fd;
    }

}
