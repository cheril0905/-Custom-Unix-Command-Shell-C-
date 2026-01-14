🖥️ Custom Unix Command Shell (C)

A lightweight Unix-like command shell implemented in C, supporting core shell functionalities using low-level POSIX system calls.

The shell provides an interactive prompt displaying the current working directory and executes user commands through process creation and management primitives. It supports built-in commands, parallel and sequential execution, output redirection, and robust signal handling, closely mimicking the behavior of a standard Linux shell.

✨ Features

Interactive shell loop with dynamic <cwd>$ prompt

Execution of Linux commands using fork(), execvp(), and wait()

Built-in command support (cd, exit)

Parallel command execution using &&

Sequential command execution using ##

Output redirection using >

Proper handling of Ctrl+C and Ctrl+Z signals

Graceful handling of invalid commands and edge cases

🛠️ Tech Stack

Language: C

Platform: Linux / UNIX

System Calls: fork, execvp, wait, chdir, open, dup2, signal

🚀 Learning Outcomes

Practical understanding of process lifecycle management

Signal handling in interactive programs

File descriptor manipulation and I/O redirection

Building real system-level software using POSIX APIs
