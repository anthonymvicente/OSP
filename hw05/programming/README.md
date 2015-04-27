Homework 05
----------------------
Author: Anthony Vicente

Files:

* README - this document
* vmm.c - C source file
* vmm.h - header file
* a.out - executable binary
* Makefile - makefile to compile C binary

INSTALL:

    make

USAGE:

    ./a.out [backing store] [address file]

DETAILS:

    This program is used to translate a list of logical addresses into their corresponding physical address, while reading from a binary file representing a physical disk. It implements a page table and Translation Lookaside Buffer (TLB) and tracks page faults and TLB hits.

UNDERSTANDING:

    This assignment gave me a more concrete understanding of the process involved in translating a logical address to a physical address. My implementation also gave me insight into more specialized integer types (uint8_t). It also helped illustrate the effectiveness of using TLB by looking at the TLB hit metric.
