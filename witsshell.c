#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>


void removeSubString(char *string, const char *substring);
char *isExecutable(char *command, char *path[], int *pathSize);
char *trim(char *string);
int split(char *command, char *delimiter, int index, char *array[]);
bool terminate(char *command);
void changeToDirectory(char *directory, int numberOfArguments);
void changePath(char *path[], char *arguments[],int commandLength, int *pathSize);
bool includes(const char *argument, char target);
 

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
							}else if(includes(currentCommand[i],'>')){

								char *redirectionArry[SIZE];
								size_t numberOfArguments = 0;

								numberOfArguments = split(currentCommand[i],">",numberOfArguments,redirectionArry);

								if(numberOfArguments = 0 || numberOfArguments > 2 || i == 0){
									write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}

								//get output file name
								char *outputFileName = redirectionArry[1];
								//open file for writing output
								int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

								if(file == -1){
                                    write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}
                                 //redirect output
								dup2(file, STDOUT_FILENO);
                                close(file);

								arguments[i] = redirectionArry[0];
								arguments[i + 1] = NULL; 
						        execv(buffer, arguments);
								exit(0);
							}
                            arguments[i] = currentCommand[i];
                        }
 
                        arguments[commandLength] = NULL; 
						execv(buffer, arguments);
						exit(0);
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
			
			for(int i = 0 ; i < numberOfCommands; ++i){
				wait(NULL);
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
                                      exit(0);
									  
								} 
							}else if(includes(currentCommand[i],'>')){

								char *redirectionArry[SIZE];
								size_t numberOfArguments = 0;

								numberOfArguments = split(currentCommand[i],">",numberOfArguments,redirectionArry);

								if(numberOfArguments = 0 || numberOfArguments > 2 || i == 0){
									write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}

								//get output file name
								char *outputFileName = redirectionArry[1];
								//open file for writing output
								int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

								if(file == -1){
                                    write(STDERR_FILENO, error_message, strlen(error_message));
									exit(1);
								}
                                 //redirect output
								dup2(file, STDOUT_FILENO);
                                close(file);

								arguments[i] = redirectionArry[0];
								arguments[i + 1] = NULL; 
						        execv(buffer, arguments);
								exit(0);
							}
							arguments[i] = currentCommand[i];
						}

                        arguments[commandLength] = NULL; 
						execv(buffer, arguments);
						
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

			for(int i = 0 ; i < numberOfCommands; ++i){
				wait(NULL);
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


void removeSubString(char *str, const char *substring)
{

    char *sub = strstr(str, substring);

    if (sub != NULL)
    {

        size_t remainingLength = strlen(sub + strlen(substring));

        memmove(sub, sub + strlen(substring), remainingLength + 1);
    }
}

// check is the command is executable
// if it is not an executable return null otherwise return directory/
char *isExecutable(char *command, char *path[], int *pathSize)
{
    char *directory = NULL;
    for (int i = 0; i < *pathSize; i++)
    {

        directory = malloc(strlen(path[i]) + strlen(command) + 2);
        snprintf(directory, strlen(path[i]) + strlen(command) + 2, "%s/%s", path[i], command);

        if (access(directory, X_OK) == 0)
        {
            removeSubString(directory, command);
            return directory;
        }

        free(directory);
    }

    return NULL;
}

// remove leading and trailing spaces
char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if (str == NULL)
    {
        return NULL;
    }
    if (str[0] == '\0')
    {
        return str;
    }

    len = strlen(str);
    endp = str + len;

    while (isspace((unsigned char)*frontp))
    {
        ++frontp;
    }
    if (endp != frontp)
    {
        while (isspace((unsigned char)*(--endp)) && endp != frontp)
        {
        }
    }

    if (frontp != str && endp == frontp)
        *str = '\0';
    else if (str + len - 1 != endp)
        *(endp + 1) = '\0';

    endp = str;
    if (frontp != str)
    {
        while (*frontp)
        {
            *endp++ = *frontp++;
        }
        *endp = '\0';
    }

    return str;
}

// split the string according to a delimeter
int split(char *command, char *delimiter, int index, char *array[])
{
    char *string;
    while (true)
    {
        // split and store individual strings in the array
        string = strsep(&command, delimiter);
        if (string == NULL)
            break;

        if(strlen(string) == 0) continue;
        array[index] = string;
        ++index;
    }

    // return the number of strings in the array
    return index;
}

// terminate when receive the exit command
bool terminate(char *command)
{
    return (strcmp(command, "exit") == 0);
}

//change directory when the command is cd
void changeToDirectory(char *directory, int numberOfArguments)
{
    if (numberOfArguments != 2 || chdir(directory) == -1)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

//update the path variable
//every path command overides the path
//TODO: find a way to optimize instead of removing /
void changePath(char *path[], char *arguments[],int commandLength, int *pathSize){
    
     bool isAbsolutePath = false;
     
     if(commandLength == 1){
        for(int index = 0; index < *pathSize; ++index){
            path[index] = NULL;
        }
        *pathSize = 0;
        return;
     }

     *pathSize = 0;
     char currentWorkingDirectory[1024];
     getcwd(currentWorkingDirectory,sizeof(currentWorkingDirectory));

     for(int index = 0; index < commandLength - 1; ++index){

        char *argument = arguments[index + 1];
         
         // if there is a leading "/" then treat it as an absolute path
        if (argument[0] == '/') {
           isAbsolutePath = true;
        }

        // Remove trailing '/'
        size_t argumentLength = strlen(argument);
        if (argumentLength > 0 && argument[argumentLength - 1] == '/') {
            argument[argumentLength - 1] = '\0';
        }

        
        size_t directoryLength;
        char * directory;

        if(!isAbsolutePath){
            // If it's a relative path, append it to the current working directory
            directoryLength = strlen(currentWorkingDirectory) + 1 + strlen(argument) + 1;
            directory = (char *)malloc(directoryLength);
            snprintf(directory, directoryLength, "%s/%s", currentWorkingDirectory, argument);
        }else{
             // It's an absolute path
            directoryLength = strlen(argument) + 1;
            directory = (char *)malloc(directoryLength);
            strcpy(directory, argument);
        }

         

        snprintf(directory,directoryLength, "%s/", argument);

        path[index] = directory;
        ++ *pathSize;
     }
}

bool includes(const char *argument, char target) {
    while (*argument) {
        if (*argument == target) return true;
        
        argument++;
    }
    return false;
}


