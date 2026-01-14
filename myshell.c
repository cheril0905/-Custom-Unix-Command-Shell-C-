#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

/* Parse input into argv-style array (tokens separated by spaces) */
char **parseInput(char *input) {
    // We allocate a fixed-size argv buffer for simplicity in this assignment.
    char **args = malloc(64 * sizeof(char *));
    int idx = 0;

    // strtok splits by spaces; suitable for the assignment’s basic parsing needs.
    char *tok = strtok(input, " ");
    while (tok != NULL) {
        args[idx++] = tok;
        tok = strtok(NULL, " ");
    }
    args[idx] = NULL; // argv must be NULL-terminated
    return args;
}

/* Utility: check if a specific symbol (e.g., "&&", "##", ">") exists in args */
int contains(char **args, const char *sym) {
    for (int i = 0; args[i]; i++)
        if (strcmp(args[i], sym) == 0) return 1;
    return 0;
}

/* Run a single command (handles built-ins `cd`, `exit`; else forks & execs) */
void executeCommand(char **args) {
    if (!args[0]) return; // empty command, nothing to do

    // Built-in: cd <path>
    // We print the required error message when path is missing or chdir fails.
    if (strcmp(args[0], "cd") == 0) {
        if (!args[1] || chdir(args[1]) == -1)
            printf("Shell: Incorrect command\n");
        return;
    }

    // Built-in: exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // External command: fork a child, exec in child, wait in parent
    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed — print required error message
        printf("Shell: Incorrect command\n");
    } else if (pid == 0) {
        // Child: restore default Ctrl+C / Ctrl+Z so external programs behave normally
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Replace child with the requested program
        if (execvp(args[0], args) < 0)
            printf("Shell: Incorrect command\n");
        // If exec fails, make sure child exits
        exit(0);
    } else {
        // Parent waits for child to finish
        wait(NULL);
    }
}

/* Execute commands in parallel separated by "&&" (start all, then wait for all) */
void executeParallelCommands(char **args) {
    // We split the incoming argv into sub-argvs per "&&"
    char *cmds[20][50]; // up to 20 commands, each up to 50 tokens (fits assignment scope)
    int cIdx = 0, aIdx = 0, total = 1;

    // Partition args by "&&" boundaries
    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], "&&") == 0) {
            cmds[cIdx][aIdx] = NULL; // finish current sub-argv
            cIdx++; aIdx = 0; total++;
        } else {
            cmds[cIdx][aIdx++] = args[i];
        }
    }
    cmds[cIdx][aIdx] = NULL; // terminate last sub-argv

    // Launch all commands concurrently
    pid_t pids[20];
    for (int j = 0; j < total; j++) {
        pids[j] = fork();
        if (pids[j] == 0) {
            // Child: restore default signals, then exec
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if (execvp(cmds[j][0], cmds[j]) < 0)
                printf("Shell: Incorrect command\n");
            exit(0);
        }
    }

    // Parent: wait for all children to complete
    for (int j = 0; j < total; j++) wait(NULL);
}

/* Execute commands sequentially separated by "##" (run one after another) */
void executeSequentialCommands(char **args) {
    // Build one sub-argv at a time and run immediately when we hit "##"
    char *cmd[50];
    int idx = 0;

    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], "##") == 0) {
            cmd[idx] = NULL;        // close current argv
            executeCommand(cmd);    // run it
            idx = 0;                // reset for next command
        } else {
            cmd[idx++] = args[i];
        }
    }

    // Run the last pending command (if any tokens were captured)
    cmd[idx] = NULL;
    if (idx > 0) executeCommand(cmd);
}

/* Handle output redirection using ">" (send child’s stdout to a file) */
void executeCommandRedirection(char **args) {
    // Find ">" and split command from output filename
    char *outfile = NULL;
    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;          // terminate command argv before ">"
            outfile = args[i+1];     // capture filename (expected immediately after '>')
            break;
        }
    }

    // Validate presence of a filename after ">"
    if (!outfile) {
        printf("Shell: Incorrect command\n");
        return;
    }

    // Open (or create) target file for writing; truncate if it exists
    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("Shell: Incorrect command\n");
        return;
    }

    // Fork and redirect stdout in the child
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Duplicate fd onto stdout so all normal prints go to file
        dup2(fd, STDOUT_FILENO);
        close(fd);

        if (execvp(args[0], args) < 0)
            printf("Shell: Incorrect command\n");
        exit(0);
    } else {
        // Parent: close our copy and wait
        close(fd);
        wait(NULL);
    }
}

/* Main REPL: show <cwd>$ prompt, read input, route to the proper executor */
int main() {
    // Make the shell resilient: ignore Ctrl+C / Ctrl+Z in the shell process itself
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while (1) {
        // Build and print prompt with current working directory
        char cwd[128];
        getcwd(cwd, sizeof(cwd));
        printf("%s$", cwd);

        // Read a full line (getline allocates/reallocates as needed)
        char *line = NULL;
        size_t cap = 0;
        ssize_t got = getline(&line, &cap, stdin);

        // Handle EOF (e.g., Ctrl+D) gracefully
        if (got == -1) {
            free(line);
            printf("\nExiting shell...\n");
            break;
        }

        // Trim trailing newline for easier parsing
        if (line[0] != '\0' && line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = '\0';

        // If the user just pressed Enter, re-prompt
        if (strlen(line) == 0) { free(line); continue; }

        // Special-case: if input is exactly "exit", print required message and quit
        if (strcmp(line, "exit") == 0) {
            printf("Exiting shell...\n");
            free(line);
            break;
        }

        // Convert raw line into argv-like array for downstream logic
        char **args = parseInput(line);

        // Dispatch based on operators required by the assignment
        if (contains(args, "&&")) {
            executeParallelCommands(args);
        } else if (contains(args, "##")) {
            executeSequentialCommands(args);
        } else if (contains(args, ">")) {
            executeCommandRedirection(args);
        } else {
            executeCommand(args);
        }

        // Clean up per-iteration allocations
        free(args);
        free(line);
    }
    return 0;
}
