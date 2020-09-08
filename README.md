Course:		CMSC 421
Instructor:	Lawrence Sebald
Author:		Sasha Arsenyuk
Date:		2/23/2019

Homework 1 -- Simple Shell

For this homework, I was tasked with designing a simple shell in C programming
language, which would support the following functionalities:

a) if run with no arguments (argc == 1), shell prompts the user to enter
commands, until user enters "exit" with no arguments. Then it exits with
a success code.

b) if run with one argument (argc == 2), shell treats the argument as
a filename, opens it, and executes commands line-by-line from it. Shell 
terminates immediately (skipping the rest of the commands) and exits
with a failure code if an invalid command is encountered. If no such command 
was encountered, shell exits with a success code.

I chose to make the child process exit with 1 when an invalid command was
encountered (execvp() did not return 0). In the parent process, I checked
the value that WEXITSTATUS() macro evaluated to (i.e. the exit status of
the child process) and acted accordingly. If the exit status was 1, parent
process called exit(1) as well.

***Note: My processes do not free all of the memory allocated before exiting.
I know this is not a good practice BUT: (1) by the time I realized this was 
happenning, it was to late to modify my shell to accommodate for this and 
(2) it is not too bad of an issue (hopefully) because the virtual memory
allocated for the process gets flushed as soon as it exits.

c) if run with two or more arguments (argc > 2), shell exits with 1.
(It prints a (meaningful) error message to stderr and exits.)

d) There was to be no limit on command length, so I implemented all buffers
dynamically with realloc(), doubling the size of the buffer every time (as
needed).

e) Parsing user commands: I took advantage of the provided functions: 
first_unquoted_space() and unescape(). The first was used to locate spaces
at which to split the buffer. For example, echo hello world returned indexes
4 and 10, while echo "hello world" only returned 4. The second function was
used to "clean up" the resulting arguments by removing quotes, adjusting
escape sequences, etc.
