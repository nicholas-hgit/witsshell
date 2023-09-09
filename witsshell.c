#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include "myfunctions.h"

enum Mode
{
	INTERACTIVE,
	BATCH,
	UNSUPPORTED
};

#define SIZE 1024


int main(int MainArgc, char *MainArgv[])
{
	enum Mode mode = UNSUPPORTED;
	
	if(MainArgc == 1){
		mode = INTERACTIVE;
	}
	else if (MainArgc == 2){
		mode = BATCH;
	}

    //store commands i.e parallel commands
	char *listOfCommands[SIZE];

	//store current command from list of commands
	char *currentCommand[SIZE];

	//number of commands in list of commands
	//use this so not to iterate the whole list
	int numberOfCommands = 0;

	//store current command length
	//usage same as numberOfCommands
	size_t commandLength = 0;

	char *path[SIZE] = {"/bin"};
	int *pathSize = malloc(sizeof(int));
	//same usage as numberOfCommands
	*pathSize = 1;

    //print this everytime an error occurs
	char error_message[30] = "An error has occurred\n";

	

	if (mode == INTERACTIVE)
	{

		char *command = NULL;

		do
		{
			printf("witsshell> ");

			// read the command
			size_t read = getline(&command, &commandLength, stdin);

			// remove trailing and leading spaces
			command = trim(command);

			// if the command is empty continue to get the next command
			if (command[0] == '\0')
				continue;

			// terminate on Ctrl + D or when command is "exit"
			if (read == -1 || terminate(command))
				exit(0);

			// separate parallel commands into a list of commands
			numberOfCommands = split(command, "&", 0, listOfCommands);

			// for each command split it into a list of command name and arguments
			for (int number = 0; number < numberOfCommands; ++number)
			{

				char *tempCommand = trim(listOfCommands[number]);
				commandLength = split(tempCommand, " ", 0, currentCommand);
				

				// check if command is executable
				char *dir = isExecutable(currentCommand[0], path, pathSize);

				if (dir != NULL)
				{

					pid_t pid = fork();

					if (pid == 0)
					{
						char buffer[1024];
						//concatenate directory and the executable command and store it in buffer
						snprintf(buffer, sizeof(buffer), "%s/%s", dir, currentCommand[0]);

						//array to hold the arguments of the command
                        char **arguments = malloc((commandLength + 1) * sizeof(char *));

                        for (int i = 0; i < commandLength; i++) {
							
							//check if command contains the redirection symbol
							/*if the redirection symbol is last or if there is more than one argument
							  after it, it is an error
					
							*/
							if(strcmp(currentCommand[i],">") == 0){
								if(i == 0 || i == commandLength - 1 || i < commandLength - 2){
									write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}else{

									//get output file name
									char *outputFileName = currentCommand[i + 1];
									//open file for writing output
									int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

									if(file == -1){
                                       write(STDERR_FILENO, error_message, strlen(error_message));
									   exit(1);
									}
                                      //redirect output
									  dup2(file, STDOUT_FILENO);
                                      close(file);

									  arguments[i + 1] = NULL; 
						              execv(buffer, arguments);

									  exit(0);
								} 
							}
                            arguments[i] = currentCommand[i];
                        }
 
                        arguments[commandLength] = NULL; 
						execv(buffer, arguments);
						exit(0);
					}
					else if (pid > 0)
					{
					 
					}
				}
				else
				{
					if (strcmp(tempCommand, "cd") == 0)
					{
						changeToDirectory(currentCommand[1], commandLength);
					}
					else if(strcmp(tempCommand,"path") == 0){
                       changePath(path,currentCommand,commandLength,pathSize);
					}
					else
					{
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
				}
			}

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
			if(terminate(command)) exit(0);

			// separate parallel commands into a list of commands
			numberOfCommands = split(command, "&", 0, listOfCommands);

			for(int number = 0; number < numberOfCommands; ++number){

				char *tempCommand = trim(listOfCommands[number]);
				commandLength = split(tempCommand, " ", 0, currentCommand);

				// check if command is executable
				char *dir = isExecutable(currentCommand[0], path, pathSize);

				if(dir != NULL){

					pid_t pid = fork();

					if(pid == 0){

						char buffer[1024];
						//concatenate directory and the executable command and store it in buffer
						snprintf(buffer, sizeof(buffer), "%s/%s", dir, currentCommand[0]);

						//array to hold the arguments of the command
                        char **arguments = malloc((commandLength + 1) * sizeof(char *));

						for(int i = 0; i < commandLength; i++){


							//check if command contains the redirection symbol
							/*if the redirection symbol is last or if there is more than one argument
							  after it, it is an error
							*/
							if(strcmp(currentCommand[i],">") == 0){
								if(i == 0 || i == commandLength - 1 || i < commandLength - 2){
									write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}else{

									//get output file name
									char *outputFileName = currentCommand[i + 1];
									//open file for writing output
									int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

									if(file == -1){
                                       write(STDERR_FILENO, error_message, strlen(error_message));
									   exit(1);
									}
                                      //redirect output
									  dup2(file, STDOUT_FILENO);
                                      close(file);

									  arguments[i + 1] = NULL; 
						              execv(buffer, arguments);

									  
								} 
							}
							arguments[i] = currentCommand[i];
						}

                        arguments[commandLength] = NULL; 
						execv(buffer, arguments);
						
					}
					else if (pid > 0)
					{
					   wait(NULL);
					}
					
				}
				else
				{
					if (strcmp(tempCommand, "cd") == 0)
					{
						changeToDirectory(currentCommand[1], commandLength);
					}
					else if(strcmp(tempCommand,"path") == 0){
                       changePath(path,currentCommand,commandLength,pathSize);
					}
					else
					{
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
				}
			}


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


