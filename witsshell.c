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
	BATCH
};

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
