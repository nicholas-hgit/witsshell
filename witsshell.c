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
void terminate(int commandLength, char error_message[]);
void changeToDirectory(char *directory, int numberOfArguments, char error_message[]);
void changePath(char *path[], char *arguments[],int commandLength, int *pathSize);
void executeCommand(char *command, char *path[], int *pathSize, char error_message[]);
void executeBuiltInCommand(char *command[],char error_message[],int commandLength, int *pathSize,char *path[]);
void resetPath(char *path[],int *pathSize);
void setPath(char *path[],char *arguments[],int commandLength, int *pathSize);
void normalRedirectionHandler(char *arguments[],char *command[],char executable[],char error_message[],int index);
void compactRedirectionHandler(char *arguments[],char *command[],char executable[],char error_message[],int index);

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

    //initialize path variable to bin
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

            //if command is empty, get next command
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


 
void removeSubString(char *string, const char *substring)
{
    char *sub = strstr(string, substring);

    if (sub != NULL)
    {
        size_t remainingLength = strlen(sub + strlen(substring));
        memmove(sub, sub + strlen(substring), remainingLength + 1);
    }
}

//loops through the path variable to check if command is executable
char *isExecutable(char *command, char *path[], int *pathSize)
{
    char *directory = NULL;
    for (int i = 0; i < *pathSize; i++)
    {
        size_t directorySize = strlen(path[i]) + strlen(command) + 2;
        directory = malloc(directorySize);

        snprintf(directory,directorySize, "%s/%s", path[i], command);

        if (access(directory, X_OK) == 0)
        {
			//remove command from path and return path
            removeSubString(directory, command);
            return directory;
        }

        free(directory);
    }

    return NULL;
}

//remove leading and trailing spaces from command
char *trim(char *string) {
    if (string == NULL || string[0] == '\0') return string;

    while (isspace((unsigned char)*string)) {
        string++;
    }

    char *end = string + strlen(string) - 1;
    while (end > string && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';

    return string;
}

 
int split(char *command, char *delimiter, int index, char *array[])
{
    char *string;
    while (true)
    {
        // split and store individual strings in the array
        string = strsep(&command, delimiter);
        if (string == NULL)
            break;
        
		//check not to store empty strings in the array
        if(strlen(string) == 0) continue;
        array[index] = string;
        ++index;
    }
 
    return index;
}

//built in command "exit"
void terminate(int commandLength, char error_message[]){
	if(commandLength > 1) write(STDERR_FILENO, error_message, strlen(error_message));
	else exit(0);
}

//built in command "cd"
void changeToDirectory(char *directory, int numberOfArguments, char error_message[]){
    if (numberOfArguments != 2 || chdir(directory) == -1){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

//built in command "path"
//every path command overides the path variable
void changePath(char *path[], char *arguments[],int commandLength, int *pathSize){

	//if there is no argument, reset the path variable to null i.e only built in commands should work
    if(commandLength == 1){
        resetPath(path,pathSize);
        return;
    }

    //override the path variable
	setPath(path,arguments,commandLength,pathSize);  
}

void executeCommand(char *command, char *path[], int *pathSize, char error_message[]){

	 
	char *listOfCommands[SIZE];
	char *currentCommand[SIZE];
	int numberOfCommands = 0;

	//store current command length
	size_t commandLength = 0;

	// separate parallel commands into a list of commands
	numberOfCommands = split(command, "&", 0, listOfCommands);

	// for each command split it into a list of command name and arguments
	for (int number = 0; number < numberOfCommands; ++number)
	{
		char *tempCommand = trim(listOfCommands[number]);
		commandLength = split(tempCommand, " ", 0, currentCommand);
				
		char *dir = isExecutable(currentCommand[0], path, pathSize);

		if (dir != NULL)
		{
			pid_t pid = fork();

			if (pid == 0)
			{
				char executable[SIZE];
				snprintf(executable, sizeof(executable), "%s/%s", dir, currentCommand[0]);

                char **arguments = malloc((commandLength + 1) * sizeof(char *));

                for (int i = 0; i < commandLength; i++) {
							
				   
					//check for redirection symbols that leaves space between them and arguments
					//e.g echo hello > outputfile
					if(strcmp(currentCommand[i],">") == 0){
						if(i == 0 || i == commandLength - 1 || i < commandLength - 2){
							write(STDERR_FILENO, error_message, strlen(error_message));
							exit(1);
						}else{

							normalRedirectionHandler(arguments,currentCommand,executable,error_message,i);
						} 

					//check for redirection symbols that leaves no space between them and arguments
					//e.g echo hello>outputfile
				    }else if(strstr(currentCommand[i],">") != NULL){

                   		 compactRedirectionHandler(arguments,currentCommand,executable,error_message,i);
					}
                        arguments[i] = currentCommand[i];
                }
 
                    arguments[commandLength] = NULL; 
					execv(executable, arguments);
					exit(0);
			}
			 
		}
		else
		{
			executeBuiltInCommand(currentCommand,error_message,commandLength,pathSize,path);
		}
	}
    
	for(int i = 0 ; i < numberOfCommands; ++i){
		wait(NULL);
	}
}

void executeBuiltInCommand(char *command[],char error_message[],int commandLength, int *pathSize,char *path[]){

	if (strcmp(command[0], "cd") == 0)
	{
		changeToDirectory(command[1], commandLength,error_message);
	}
	else if(strcmp(command[0],"path") == 0){
        changePath(path,command,commandLength,pathSize);
	}
	else if(strcmp(command[0], "exit") == 0){
		terminate(commandLength,error_message);
	}
	else
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
	}

}

//set path variable to nothing
void resetPath(char *path[],int *pathSize){
	for(int index = 0; index < *pathSize; ++index){
        path[index] = NULL;
    }
    *pathSize = 0;
}

//overrides the path variable with path arguments
void setPath(char *path[],char *arguments[],int commandLength, int *pathSize){

	//check if the path is absolute or relative to the current working directory
    bool isAbsolutePath = false;

	*pathSize = 0;
    char currentWorkingDirectory[SIZE];
    getcwd(currentWorkingDirectory,sizeof(currentWorkingDirectory));

    //iterate the arguments and add them to the path variable
	for(int index = 0; index < commandLength - 1; ++index){

        char *argument = arguments[index + 1];
         
         // if there is a leading "/" then treat argument as an absolute path
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
           
            directoryLength = strlen(argument) + 1;
            directory = (char *)malloc(directoryLength);
            strcpy(directory, argument);
        }

         

        snprintf(directory,directoryLength, "%s/", argument);

        path[index] = directory;
        ++ *pathSize;
    }
}

//handles redirection when redirection symbols leaves space between them and arguments
//e.g echo hello > outputfile
void normalRedirectionHandler(char *arguments[],char *command[],char executable[],char error_message[],int index){

 
	char *outputFileName = command[index + 1];
	 
    int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if(file == -1){
        write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}

    //redirect output to file
	dup2(file, STDOUT_FILENO);
    close(file);

	arguments[index + 1] = NULL; 
	execv(executable, arguments);
	exit(0);
}

//handles redirection when redirection symbols leaves no space between them and arguments
//e.g echo hello>outputfile
void compactRedirectionHandler(char *arguments[],char *command[],char executable[],char error_message[],int index){

	char *redirectionArry[SIZE];
	size_t numberOfArguments = 0;

	numberOfArguments = split(command[index],">",numberOfArguments,redirectionArry);

	if(numberOfArguments = 0 || numberOfArguments > 2 || index == 0){
		write(STDERR_FILENO, error_message, strlen(error_message));
	    exit(1);
	}

	char *outputFileName = redirectionArry[1];
	int file = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if(file == -1){
        write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
						
    //redirect output to file
	dup2(file, STDOUT_FILENO);
    close(file);

	arguments[index] = redirectionArry[0];
	arguments[index + 1] = NULL; 
	execv(executable, arguments);
	exit(0);
					
}


