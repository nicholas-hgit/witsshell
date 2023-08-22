#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <stdbool.h>

void removeSubString(char *string, const char *substring);
char *isExecutable(char *command, char *path[], int pathSize);
char *trim(char *string);
int split(char *command, char *delimiter, int index, char *array[]);
bool terminate(char *command);

#endif