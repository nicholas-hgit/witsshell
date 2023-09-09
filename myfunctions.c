#include "myfunctions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <ctype.h>


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