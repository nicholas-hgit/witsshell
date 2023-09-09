#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <stdbool.h>
#include <stddef.h>

void removeSubString(char *string, const char *substring);
char *isExecutable(char *command, char *path[], int *pathSize);
char *trim(char *string);
int split(char *command, char *delimiter, int index, char *array[]);
bool terminate(char *command);
void changeToDirectory(char *directory, int numberOfArguments);
void changePath(char *path[], char *arguments[],int commandLength, int *pathSize);
char* execute_system_command(const char* command);

#endif