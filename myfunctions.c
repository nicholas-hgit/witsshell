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

char *isExecutable(char *command, char *path[], int pathSize)
{
    char *buffer = NULL;
    for (int i = 0; i < pathSize; i++)
    {

        buffer = malloc(strlen(path[i]) + strlen(command) + 2);
        if (buffer == NULL)
        {
            perror("malloc");
            exit(1);
        }
        snprintf(buffer, strlen(path[i]) + strlen(command) + 2, "%s/%s", path[i], command);

        if (access(buffer, X_OK) == 0)
        {
            removeSubString(buffer, command);
            return buffer;
        }

        free(buffer);
    }
    return NULL;
}

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

int split(char *command, char *delimiter, int index, char *array[])
{
    char *string;
    while (true)
    {
        string = strsep(&command, delimiter);
        array[index] = string;

        if (string == NULL)
            break;
        ++index;
    }

    return index;
}

bool terminate(char *command)
{
    return (strcmp(command, "exit") == 0);
}