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