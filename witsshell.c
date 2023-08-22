#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>

enum Mode
{
	INTERACTIVE,
	BATCH
};

bool terminate(char *command)
{
	return (strcmp(command, "exit") == 0);
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

void removeSubstring(char *str, const char *substring)
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
	char *buffer = NULL for (int i = 0; i < pathSize; i++)
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
			removeSubstring(buffer, command);
			return buffer;
		}

		free(buffer);
	}
	return NULL;
}

int main(int MainArgc, char *MainArgv[])
{
	enum Mode mode = INTERACTIVE;

	char *listOfInput[1000];
	int index = 0;

	char *path[1000] = {"/bin"};

	if (MainArgc > 1)
		mode = BATCH;

	if (mode == INTERACTIVE)
	{

		char *command = NULL;
		size_t commandLength = 0;

		do
		{
			printf("witsshell> ");
			size_t read = getline(&command, &commandLength, stdin);

			command = trim(command);

			if (command[0] == '\0')
				continue;

			if (read == -1 || terminate(command))
				exit(0);

			index = split(command, " ", 0, listOfInput);

			command = listOfInput[0];
			char *dir = isExecutable(command, path, 1);

			if (dir != NULL)
			{

				pid_t pid = fork();

				if (pid == 0)
				{

					char buffer[1024];
					snprintf(buffer, sizeof(buffer), "%s/%s", dir, command);

					char *args[] = {command, listOfInput[1], NULL};
					execv(buffer, args);
				}
				else if (pid > 0)
				{

					wait(NULL);
				}
				else
				{

					perror("fork");
				}

				free(dir);
			}
			else
			{
				printf("Command not found: %s\n", command);
			}

		} while (true);

		free(command);
	}

	return (0);
}
