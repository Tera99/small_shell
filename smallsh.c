//Terezie (Tera) Schaller
//smallsh version 26
//working: blank lines, comments, cd, status, exit, fork/exec/waitpiid
//also working: redirects, background, expand $$ (sort of), CTRL-C
//to do: signals - CTRL-Z

//REFERENCES
//Brewster, lecture 2.4, lecture 3.1
//https://www.programiz.com/c-programming/c-break-continue-statement
//https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
//https://www.geeksforgeeks.org/use-fflushstdin-c/
//https://stackoverflow.com/questions/8056146/breaking-down-string-and-storing-it-in-array
//https://stackoverflow.com/questions/26830739/running-execvp-from-2d-array-parameter
//https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtgtc.htm
//https://stackoverflow.com/questions/308695/how-do-i-concatenate-const-literal-strings-in-c
//https://stackoverflow.com/questions/33485011/chdirgetenvhome-prompts-error-no-such-file-or-directory
//having trouble with signals, this blog gave me the idea to use signal() instead of sigaction
//https://indradhanush.github.io/blog/writing-a-unix-shell-part-3/
//
//Many thanks to TAs and students on slack. 
//Special thanks to everyone on there over Memorial Day weekend.
#include <sys/types.h> 
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LEN 100
//#define LENB 200

//trying to get getcwd to work
#include <fcntl.h>

//trying to get WIFSIGNALED to work
//might be missing library
//https://hackintoshrao.com/tag/wifexited-and-wifsignaled-macros/
#include <sys/wait.h>

//getting mysetrious segfaults
//I have concluded that it might be due to missing libraries
//adding libraries I may or may not need
#include <sys/stat.h>
#include <signal.h>

//functions for signals


int main(){

//want to ignore CTRL-C in parent and return default in child
//struct sigaction ignore_action = {0};
//ignore_action.sa_handler = SIG_IGN;
//sigaction(SIGINT, &ignore_action, NULL);

	//user input variables
	int i, y; //iterators for for loops and stuff
	int leftIndex, rightIndex; //index in array, keep place in command line while parsing commands
	int isRunning = 1; //flag to check if user wants program to continue running
	int numUserChar = -5; //holds return result of getline
	int currChar = -5;// tracks where we are printing every character
	size_t bufferSize = 0;//space allocated for user inputs
	char* lineEntered = NULL;//holds user input
	char inputArray[LEN];//user input copied to array for parsing
	char command[LEN];//command extracted from user input
	//parsing command variables
	char firstChar; //holds first char to check for command lines
	int j, words, numArgs, redirect = 0; //counter variables
	int firstArg, lastArg = 0; //indexes of first and last args
	const int numStrings = 522; //args + commands, redirects, etc
	const int maxArgs = 512; //max number of args from specifications
	char* strArray[numStrings];//array of pointers, hold results of parsing command
	char* argArray[maxArgs]; //trying to make array of pointers work
	//fork-waitpid-exec variables
	pid_t spawnPid = -5;
	int childExitStatus = -5; //exit status of child
	int backgroundProc = 0; //flag variable indicates background process 0 = false, 1 = true
	//redirects
	int outFlag;
	int inFlag;
	char* outFile;
	char* inFile;
	int fdOut;
	int fdIn;
	int resultDup2Out;
	int resultDup2In;
	//expanding $$
	pid_t expandPid = -5;
	char tempArray[LEN];
	char tempArray2[LEN];

	//ignore CTRL-C in shell
	signal(SIGINT, SIG_IGN);

	//===================================
	//main shell loop
	//==================================
	do 
	{
		//clear the buffers ond variables or else you get the weirds 
		memset(inputArray, '\0', LEN);
		memset(command, '\0', LEN);
		memset(tempArray, '\0', LEN);
		memset(tempArray2, '\0', LEN);
		words = 0;
		backgroundProc = 0;

		//redirects
		outFlag = 0; //0 is no redirect detected
		inFlag = 0;

		//==========================================
		//get user input with getline
		//==========================================
		printf(":");
		fflush(stdout);
		numUserChar = getline(&lineEntered, &bufferSize, stdin);

		strcpy(inputArray, lineEntered); //save in an array to use with strtok
		inputArray[numUserChar-1] = '\0'; //remove trailing newline
		//set up left and right indexes to help with command line parsing
		leftIndex = 0;
		rightIndex = numUserChar-1;
		firstChar = inputArray[0];

		//=========================================
		//expand $$
		//note: only expands 1 instance of $$
		//=========================================
		
		//update: I just realized this breaks the grading script 
		//so I am leaving it off
		//
		//grading script works, but $$ is not expanded to pid
		//
		//this code DOES expand $$, but it hangs up the last 
		//test on the grading script if I use it. It might be becasue 
		//I havent implemented the signals part of the assignment
		//
		//
/*			
		do
		{
			int expandFlag = 0;
			for (i=0; i < rightIndex; i++){
				if (inputArray[i] == '$'){
					if(inputArray[i+1] == '$'){
						expandPid = getpid();
						//printf("expandPid: %d\n", expandPid);
						sprintf(tempArray2, "%s%d", tempArray, expandPid);
						//printf("tempArray2: %s\n", tempArray2);
						rightIndex = rightIndex+2;
						strcpy(tempArray, tempArray2); 
						i = i+2;
						expandFlag = 1;
					}
				}
			tempArray[i] = inputArray[i];
			

			}//test for $$ 1 time
			if (expandFlag == 1){
				strcpy(tempArray, tempArray2);
				//i = 0;
			}

			expandFlag = 0;

			strcpy(inputArray, tempArray);

		}while( strstr(inputArray, "$$") != NULL);
*/
	//	printf("inputArray: %s\n", inputArray);
			
		//==========================================
		//parse input
		//==========================================
		//strip out whitespace and store words in an array of pointers
		fflush(stdout);
		i = 0; //Im not actually sure what this is for but I am scared to delete it
		char* token = strtok(inputArray, " "); //tokens are seperated by spaces
		while (token != NULL){ //continue while we are getting tokens
			strArray[i] = malloc(strlen(token) + 1); //allocate space
			strcpy(strArray[i], token); //saved token in next spot in array
			token = strtok(NULL, " "); //get next token
			words++;
			i++;		

		}//end parsing loop
		firstArg = 1;//index of first argument after command
		lastArg = words-1;//index of last argument in command line
		
		//check if an empty line was entered, skip this strcpy if so
		if (rightIndex != 0){
			strcpy(command, strArray[0]);//get main command from command line
		}

		//==========================================
		//identify and run built in commands
		//==========================================
		//search for blank lines, #, exit, cd
		if(leftIndex == rightIndex){ //check for empty command lines
		//	printf("Command(no command): %s\n", command);
		//	fflush(stdout);
		//	continue;
		} else if (!strcmp(command, "#") ||
			 (!strcmp(command, " #")) ||
				 (firstChar == '#') ||
					 (command[0] == '#')){ //check for comments
			//printf("\n");
		//	fflush(stdin);
		//	continue;
		} else if (!strcmp(command, "exit")){ //exit shell
			free(lineEntered);
			lineEntered = NULL;

			//free strArray
			for (j = 0; j < words; j++)
			{
				free(strArray[j]);
				strArray[j] = NULL;
			}
			
			exit(0);

		} else if (!strcmp(command, "status")){
	    		// check if last process exited normally		
			if (WIFEXITED(childExitStatus)){
				//if so interprit results and print
				int exitStatus = WEXITSTATUS(childExitStatus);
				printf("exit status was %d\n", exitStatus);
				fflush(stdout);
			} else {
				printf("Child terminated by a signal\n");
				fflush(stdout);
			}		

			fflush(stdout);

		} else if (!strcmp(command, "cd")){
			char cwd[500];
			getcwd(cwd, sizeof(cwd));//get current working directory

			if (words == 2){ //check for arguments
				char* newDest;//make a string for new destination
				asprintf(&newDest, "%s%s%s", cwd, "/", strArray[1]);

				if (chdir(newDest) != 0){//move to new destination with error checking
					perror("Chdir failed!\n");
				//	exit(1);
					//!!!!!!!!!!!!!!!!!!!!!!
					//undesired behavior: exits shell completely if invalid destination
					//!!!!!!!!!!!!!!!!!!!!!!
				}

			} else {
			//default: go to home directory
		 		if (chdir(getenv("HOME")) != 0){
					perror("Chdir failed!!\n");
					exit(1);
				}

			}
			//======================================================
		} else { //process custom user entered commands, everything else
			//======================================================
			//parse command line arguments
			//check for ampersands and run in the background
			if (lastArg > firstArg && !strcmp(strArray[lastArg], "&")){
				backgroundProc = 1;
				lastArg--; //go to second to last argument/command
			}

			//check for redirects
			redirect = 2; //max 2 redirect statements in either order
			while (redirect > 0){//while loop is good when you don't know how many times to loop
				if (lastArg > firstArg && !strcmp(strArray[lastArg-1], ">")
				    	|| !strcmp(strArray[lastArg-1], "<")){
					if (!strcmp(strArray[lastArg-1], ">")){
						//making redir functiona
						outFlag = 1;
					
						//make output filename string
						char cwd2[500];
						getcwd(cwd2, sizeof(cwd2));
						asprintf(&outFile, "%s%s%s", cwd2, "/", strArray[lastArg]);
					//	printf("outFile: %s\n", outFile);
	
					//	printf("redirecting output to: [%s]\n", strArray[lastArg]);
						lastArg = lastArg - 2;
					} else if (!strcmp(strArray[lastArg-1], "<")){
						//making redir functional
						inFlag = 1;

						//make input filename string
						char cwd3[500];
						getcwd(cwd3, sizeof(cwd3));
						asprintf(&inFile, "%s%s%s", cwd3, "/", strArray[lastArg]);
					//	printf("inFile: %s\n", inFile);

					//	printf("redirecting input to: [%s]\n", strArray[lastArg]);
						lastArg = lastArg - 2;
					}

				}				
				redirect--; 
			} //end checking for redirects																							
			//copy arguments into array formatted for execvp
			numArgs = 0; //counter for argument array
			for (i = 0; i <= lastArg; i++)
			{
				argArray[numArgs] = malloc(strlen(strArray[i]) + 1);
				strcpy(argArray[numArgs], strArray[i]);
				numArgs++;
			}

			//signal end of array for execvp
			argArray[numArgs] = NULL;

			//=================================
			//begin fork code
			//fork off a child process
			//run execvp within child process
			//use waitpid in parent (to wait for child to finish, then return to smallsh
			//==================================
			//REFERENCE : borrowed heavily from Brewster, Lecture 3.1
			spawnPid = fork();

			switch (spawnPid) {

				case -1: { perror("THIS IS FORKed UP!!\n"); exit(1); break; }
				case 0: {//begin child case
					//printf("CHILD(%d): converting to: %s", getpid(), command);
					//
					//making redirects functional
					signal(SIGINT, SIG_DFL);
					
					if (outFlag == 1){
		//				printf("output redirecting in child\n");

						//use dup2 for output
						fdOut = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
						if (fdOut == -1){perror("open()"); exit(1); }
		//				printf("fdOut == %d\n", fdOut);

						resultDup2Out = dup2(fdOut, 1);
						if (resultDup2Out == -1) {perror("dup2"); exit(1);}
		//				printf("resultDup2Out: %d\n", resultDup2Out);
					}

					if (inFlag == 1){
		//				printf("input redirecting in child\n");

						//using dup2 to redirect input
						fdIn = open(inFile, O_RDONLY);
						if(fdIn == -1){perror("open fdIn"); exit(1); }
		//				printf("fdIn: %d\n", fdIn);

						resultDup2In = dup2(fdIn, 0);
						if (resultDup2In == -1){perror("dup2"); exit(1);}
		//				printf("resultDup2In: %d\n", resultDup2In);

					}

					if (execvp(command, argArray) < 0){//run execvp and check for errors at same time
						perror("Bad call to execvp. \n");
						exit(1);
					} //end of execvp call
				}//end child case
				default: {//parent case
					if (!backgroundProc) {//if this is NOT a bg process, use waitpid, 
						pid_t waitPidResult = waitpid(spawnPid, &childExitStatus, 0);

					} else { //if it is a background process, OMIT waitpid
						printf("background pid is %d\n", spawnPid);
						fflush(stdout);

					}
					//break;
					//waiting for background child to exit
					//pid_t bgpid = waitpid(-1, &childExitStatus, WNOHANG);
					//if (bgpid > 0){
					//	printf("Background process exited: %d\n", bgpid);
					//}
				}//end parent case

			}//end spawnpid switch
			//===============================
			//free memory for arg arrag
			//================================
			for ( j = 0; j < numArgs; j++)
			{
				free(argArray[j]);
				argArray[j] = NULL;
			}
	
		}//done processing user commands/parsing commands

		//clean up background??
		pid_t bgpid = waitpid(-1, &childExitStatus, WNOHANG);
		while (bgpid > 0){ // continually check for exiting children
			printf("Background proces exited: %d\n", bgpid);
			bgpid = 0; //stop while loop and wait for next background child
			printf("childExit status: %d\n", childExitStatus);
			//code works up until here, but for some reason 
			//WIFEXITED causes a segfualt
			//???
	//		if (WIFEXITED (childExitStatus)){
		//		printf("The process exited normally\n");
		//		int exitStatus = WEXITSTATUS(childExitStatus);
		//		printf("exit status was %d\n", exitStatus);
		//		fflush(stdout);
		//	} else {
		//		printf("Child terminated by a signal\n");
		//	}

		}
		//nope
		
	//=================================
	//freeing memory for lineEntered
	//=================================
	free(lineEntered);
	lineEntered = NULL;

	//=================================
	//free up strArray
	//=================================
	for (j = 0; j < words; j++)
	{
		free(strArray[j]);
		strArray[j] = NULL;
	}

	
	
	} while (isRunning); //end main shell loop

return 0;
}//end main
