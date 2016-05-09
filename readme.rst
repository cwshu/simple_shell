HW Requirement
~~~~~~~~~~~~~~

1. [basic] Execute a single command. (parser + fork-exec)
2. [basic] Properly block or unblock signals. (sigaction)
3. [basic] Replace standard input/output of a process using the redirection operators (< and >).
4. [basic] Setup foreground process group and background process groups.
5. [basic] Create pipeline for commands separated by the pipe operator (|), and put the commands into the same process group.

6. [optional] Manipulate environment variables: provide commands like export and unset to add and remove environment variables, respectively.
7. [optional] Expand of the * and ? operators.
8. [optional] Job control: support process suspension using Ctrl-Z, and jobs, fg and bg command.
