#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/select.h>
#include<fcntl.h>
#include<unistd.h>

#define BUFFSIZE 255
#define EXIT "--quit--\n"

int main(int argc, char *argv[])
{
    // check for correct number of arguments
    if(argc != 3)
    {
        printf("Usage: %s <pipe1> <pipe2>\n", argv[argc - 1]);
        return 1;
    }

    // declare our variables
    struct stat fstat; // struct for checking file type
    int fdp_one; // file descriptors
    int fdp_two;
    int bytes_read;
    fd_set fdset; // struct for file descriptor set
    int maxfd; // max value for descriptor set
    char buf[BUFFSIZE];
    int retval;

    // make sure we're using pipes
    int i = 1;
    for(; i < 3; i++)
    {
        // and the file exists
        if((stat(argv[i], &fstat) == -1))
        {
            printf("%s: file does not exist\n", argv[i]);
            return 1;
        }

        if(!S_ISFIFO(fstat.st_mode))
        {
            printf("%s: not a named pipe\n", argv[i]);
            return 1;
        }
    }

    char *pipe_one_name = argv[1];
    char *pipe_two_name = argv[2];
    char *pipe_read;

    // open the pipes
    fdp_one = open(pipe_one_name, O_RDWR | O_NONBLOCK);
    fdp_two = open(pipe_two_name, O_RDWR | O_NONBLOCK);

    // get maximum value of descriptor set
    maxfd = (fdp_one > fdp_two) ? fdp_one + 1 : fdp_two + 1;

    while(1)
    {

        // zero the set, set the file descriptors in the set, and block for input
        FD_ZERO(&fdset);
        FD_SET(fdp_one, &fdset);
        FD_SET(fdp_two, &fdset);
        retval = select(maxfd, &fdset, NULL, NULL, NULL);

        if(retval == -1)
        {
            printf("An error occured while using select(), exiting...");
            return 1;
        }

        if(FD_ISSET(fdp_one, &fdset))
        {
            bytes_read = read(fdp_one, buf, BUFFSIZE - 1);
            pipe_read = pipe_one_name;
        }
        if(FD_ISSET(fdp_two, &fdset))
        {
            bytes_read = read(fdp_two, buf, BUFFSIZE - 1);
            pipe_read = pipe_two_name;
        }

        // ensure the string read has a null terminating character
        // usefull if the user attempts to overfill the buffer
        buf[bytes_read] = '\0';

        if((strcmp(buf, EXIT) == 0))
        {
            break;
        }

        printf("%s [%d]: %s", pipe_read, bytes_read, buf);

    }

    return 0;

}
