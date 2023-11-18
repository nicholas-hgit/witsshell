# Basic Shell - witsshell
In this university of witswatersrand assignment, I implemented a command line interpreter (CLI) or, as it is more commonly known, a shell.
This basic shell, called witsshell, is basically an interactive loop: it repeatedly prints a prompt "witsshell> ". It then parses the input, execute the command
specified on that line of input, and wait for the command to finish. This is repeated until the user types exit. The shell can only be invoked with either no argument
or a single argument, everything else is an error.

## Modes
The shell supports two running modes, namely, interactive mode and batch mode

### Interactive mode
In interactive mode, the shell is ran with no arguments, and allows the user to enter the commands directly. Here is the no arguments way
```shell
prompt> ./witsshell
witsshell> ls 
```
### Batch mode
The shell also supports batch mode, which instead reads the commands from a batch file. Here is how you run the shell with a batch file named batch.txt
```
prompt> ./witsshell  batch.txt
```

## Built-in commands
When the shell receives a command, it checks if the command is a built-in command or not. If it is built-in, it invokes the implementation of the built-in command.
For example, the built-in command exit simply calls exit(0), which will then exit the shell.
The shell implements the commands <b>exit</b>,<b>cd</b>, and <b>path</b>.

### exit
The <b>exit</b> command takes 0 arguments. Executing this command simply closes the shell
### cd
The <b>cd</b> command always takes 1 argument, which is the directory that you want to change into.
### path
The <b>path</b> command takes 0 or more arguments, with each argument separated by a whitespace from the others. A typical usage would be like this
```
witsshell> path /bin/ /usr/bin/
```
which would add /bin/ and /usr/bin/ to the search path of the shell. If path is executed with 0 arguments,
then the shell should not be able to run any programs (except built-in commands).
The path command always overwrites the old path with the newly specified path.

## Redirections
The shell also supports redirection of standard input. This is achieved by the use of a special symbol ">". Fo example, the command
```
witsshell> ls -la /tmp > output
```
will print nothing on the screen. Instead, the standard output of the ls command will be rerouted to the "output" file. If the "output" file already exist,
it will be overwritten. Built-in commands do not support redirection. Multiple redirection operators or multiple files to the right of the redirection symbols
are not supported.

## Parallel commands
The shell also support the use of parallel commands. This is accomplished with the ampersand operator as follows.
```
witsshell> cmd1 & cmd2 args1 args2 & cmd3 args1
```

## Errors
The one and only error message. The shell prints this one and only error message when it encounters an error of any type
```
An error has occurred
```
Although there is a difference between the errors that the shell catches and those that the program catches. The shell only catch the syntax errors. It does not
worry about any program-related errors such as invalid arguments to ls when it is executed, in this case, the program will print its own error and exit.
