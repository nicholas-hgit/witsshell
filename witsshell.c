#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include "myfuntions.h"




enum Mode
{
	INTERACTIVE,
	BATCH,
	UNSUPPORTED
};

#define SIZE 1024
#define ERR_MSG_SIZE 30


int main(int MainArgc, char *MainArgv[])
{
	enum Mode mode = UNSUPPORTED;
	
	if(MainArgc == 1) mode = INTERACTIVE;
	else if (MainArgc == 2) mode = BATCH;
	
	size_t commandLength = 0;

   
	char *path[SIZE] = {"/bin"};
	int *pathSize = malloc(sizeof(int));
	*pathSize = 1;

    //print this everytime an error occurs
	char error_message[ERR_MSG_SIZE] = "An error has occurred\n";

	if (mode == INTERACTIVE)
	{
		char *command = NULL;

		do
		{
			printf("witsshell> ");

			size_t read = getline(&command, &commandLength, stdin);

			// terminate on Ctrl + D 
			if (read == -1)
				exit(0);

			command = trim(command);

            //if command is empty, get next command
			if (command[0] == '\0')
				continue;

			executeCommand(command,path,pathSize,error_message);

		} while (true);

		free(command);
	}
	

	if(mode == BATCH){

		const char *fileName = MainArgv[1];
		FILE *file =  fopen(fileName,"r");

		if(file == NULL){
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		} 

		char *command = NULL;
		 
		while((getline(&command,&commandLength,file)) != -1){

			command = trim(command);

        
			if(command[0] == '\0') continue;
			 
            executeCommand(command,path,pathSize,error_message);

		}

		free(command);
		fclose(file);
	}

	if(mode == UNSUPPORTED){
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}

	return (0);
}


 
