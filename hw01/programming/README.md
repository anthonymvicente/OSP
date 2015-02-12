Homework 01 - Pipes
----------------------
Author: Anthony Vicente

Files:

* README - this document
* makefile - makefile to compile the C program
* pmon.c - source code

INSTALL:

    make pmon

USAGE:

    ./pmon <pipe1> <pipe2>

from a second terminal

    echo [message] > <pipe1 | pipe2>

to quit

    echo '--quit--' > <pipe1 | pipe2>

DETAILS:

pmon is a C program that will read from two pipes and print the data in either pipe to STDOUT. It also displays the pipe read from, along with the number of bytes read from said pipe.

UNDERSTANDING:

This is one of my first C programs, so I learned a lot in regards to the C syntax and how it worked. I also learned how to use file descriptors as file handles when using the open() command. This was further strengthened by the use of file descriptors when using read(). Finally, using select() for I/O multiplexing was of course very new as well. The use of the built in functions (FD_SET, FD_ISSET, etc) helped greatly in working with I/O.
