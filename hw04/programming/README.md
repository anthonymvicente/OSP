Homework 04
----------------------
Author: Anthony Vicente

Files:

* README - this document
* avicente_tsort.h - header file
* avicente_tsort_helpers.c - c file containing helper functions for phase 03
* avicente_tsort.c - C source code for tsort for phase 03
* avicente_tsort_helpers_phase02.c - c file containing helper functions for phase 02
* avicente_tsort_phase02.c - C source code for tsort for phase 02
* makefile - makefile to compile C binary

INSTALL:

for phase 03:

    make

for phase 02:

    make phase02make

USAGE:

    ./tsort [# of threads] [data file]

DETAILS:

    tsort is a multithreaded sorting program. It can be used to sort an array of integers. Note that the number of threads is limited to 16.

UNDERSTANDING:

    This assignment helped provide insight into the process of creating a multithreaded program. The intent was to display the importance of avoiding deadlock situations with mutexes as well as avoid inefficient busy waiting through use of condition signals. This is difficult.

ISSUES:

    I was able to get phase 2 mostly working. There were issues with certain subarrays being off by when when sorting. The boundaries are all calculated correctly, so my guess is it's some error with terminating a thread too early. However I was unable to fix this and felt it was important to attempt phase 3 for more credit. Unfortunately, I ran into further issues here. It does work occasionally, roughly half the time I've found. Other times the loop used in main to monitor the other threads will not always be terminated correctly, hanging on its wait condition while no other threads are running or signaling. Once again I was unable to solve this issue. I understand the problems, I just can't find them.

OUTISDE SOURCES:

    I was having issues passing an integer pointer to my read_file method, and found the following resource useful:

    http://stackoverflow.com/questions/2486235/initializing-the-pointer-in-the-separate-function-in-c
