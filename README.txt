Terezie Schaller
CS344 spring 2018
Program 3 - smallsh

Compile:

gcc -o smallsh smallsh.c

Run:

smallsh

Notes:

1: Expanding $$ does not fully work. The testscript will run just make sure there isn't already a 'testdir$$' in the home directory. I do have code that successfully expands $$, but it breaks on the last step of the grading script. This may be due to signal handing, not the $$ expansion code. However, I have left it commented out so the test script will run to completion. 

2: This implementation only supports commands up to 100 char. This is probably because I used a static array, when I should have used a dynamic one to get user input. I have really struggled with C strings in the last couple of assignments. 

3: Signal handling for CTRL-Z does not meet assignment specifications. I tried a few things, but they all broke the code badly.

4: In addition to 1-3, the code is rather messy and convoluted. I learned a lot while writing this. If I had time, I would go back and significantly improve/streamline most of it, especially the command parsing. Darn C-strings! 

5: Things that DO work: blank lines, comments, built in commands (status, cd, exit), user commands and their arguments like pwd/ls/echo/cat, etc, redirects < and >, & runs a process in the background, CTRL-C does not terminate shell and ends foreground child processes. Expanding $$ partially works; however, it is commented out in the version I am submitting. 

6: Wow! Tough assignment! This is the first time in this program that I have not been able to complete a program on time. I started right away, cleared my schedule, and did the maximum amount of work in the time allotted. It really pushed my boundaries and I learned a lot -- more than I think is reflected in the final product. 

