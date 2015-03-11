Homework 02 - Phase I
----------------------
Author: Anthony Vicente

Files:

* README - this document
* comm.h - header file defining Command structure
* osh.h - header file defining functions found in osh.c and other macros
* osh.c - C source code for osh
* makefile - makefile to compile C binary

INSTALL:

    make

USAGE:

    ./osh [OPTIONS]

type commands into given prompt.

to quit type "exit" or use the ctrl+d combination.

DETAILS:

osh is a very basic unix shell which allows a user to execute commands. It is bare bones, in that it does not have any change directory functionality, or other conveniences afforded by most shells. That being said, it should be suitable for running standard unix commands found in a users PATH variable.

UNDERSTANDING:

This assignment helped to strengthen an understanding of the fork() and wait() commands, along with the working behind file descriptor duplication (using dup2), how file descriptors are stored in memory for use by a process and its children, and the use of non-named pipes. Personally, one of the more interesting things I learned was how dup2 worked in 'redirecting' a programs standard input or output to whatever file descriptor you may need to use. Because these memory spaces are shared amongst a processes children, it became possible to setup files, pipes, etc. as needed before forking a process to execute the given command. Another issue that was tricky to handle was piping large amounts of data between consecutively running processes. This required multiple forking, along with counting pipes used to ensure the shell waited for all processes to complete.

COLLABORATION:

As a disclaimer, please know that I helped/was helped by the following people through a high level discussion of the issues. We were careful to not disclose direct code, but ideas on how to approach certain problems, especially piping, was shared.

    Gregg Atkin
    Diksha Ahuja
    Matt Lickei
