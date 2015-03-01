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

        print_argv(&h_cmd);
    }

    if(i == 50)
    {
        printf("You have reached your alloted command executions (50). Please restart the shell to continue.\n");
    }

    return 0;
}

