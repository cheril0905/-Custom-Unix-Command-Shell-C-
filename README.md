# 🖥️ Custom Unix Command Shell (C)

A lightweight **Unix-like command shell** implemented in **C**, designed to replicate core functionalities of a traditional Linux shell using low-level **POSIX system calls**.

This project demonstrates practical understanding of **process management, signal handling, command execution, and file descriptor manipulation** in a UNIX environment.

---

## ✨ Features

- Interactive shell with dynamic **current working directory prompt**
- Execution of standard Linux commands using `fork()`, `execvp()`, and `wait()`
- Built-in command support:
  - `cd <directory>` – change working directory
  - `exit` – terminate the shell gracefully
- **Parallel command execution** using `&&`
- **Sequential command execution** using `##`
- **Output redirection** using `>` operator
- Proper handling of keyboard signals:
  - `Ctrl + C` (SIGINT)
  - `Ctrl + Z` (SIGTSTP)
- Graceful handling of invalid or empty commands

---

## 🧠 Key Concepts Used

- Process creation and control
- Parent–child process synchronization
- Signal handling in interactive programs
- File descriptor manipulation (`open`, `dup2`)
- Command parsing and argument handling
- UNIX system programming using POSIX APIs

---

## 🛠️ Technologies

- **Language:** C  
- **Platform:** Linux / UNIX  
- **APIs:** POSIX system calls  

---

## 🚀 Getting Started

### Compile
```bash
gcc myshell.c -o myshell
Execute a command
ls

Parallel execution
ls && pwd && date

Sequential execution
ls ## echo Hello ## pwd

Output redirection
ls > output.txt

📈 Learning Outcomes

Gained hands-on experience with UNIX process lifecycle management

Understood how shells interact with the operating system
