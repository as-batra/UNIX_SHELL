# Unix Shell — mysh

A POSIX-style shell implemented in C from scratch, demonstrating 
core operating system concepts including process management, 
inter-process communication, and file descriptor manipulation.

## Features

- **Command execution** — runs any system command via `fork()` + `execvp()`
- **Single pipe** — connects two commands via `pipe()` + `dup2()`
- **Output redirection** — redirects stdout to a file using `open()` + `dup2()`
- **Built-in commands** — `cd` (runs in parent process) and `exit`

## Syscalls Used

| Syscall | Purpose |
|---|---|
| `fork()` | Creates a child process to execute commands |
| `execvp()` | Replaces child process image with the target program |
| `wait()` | Parent waits for child to finish before re-prompting |
| `pipe()` | Creates a unidirectional communication channel between processes |
| `dup2()` | Rewires file descriptors for pipe and redirection |
| `open()` | Opens output file for redirection |
| `chdir()` | Changes working directory for cd built-in |

## Build & Run

```bash
gcc -o myshell shell.c
./myshell
```

## Usage

```bash
# Basic commands
mysh> ls
mysh> pwd
mysh> echo hello

# Output redirection
mysh> ls > output.txt

# Pipe
mysh> ls | grep .c

# Built-ins
mysh> cd ..
mysh> exit
```

## How It Works

**Command execution:** The shell forks a child process and calls 
`execvp()` to replace it with the requested program. The parent 
calls `wait()` to block until the child finishes.

**Pipe:** Two child processes are forked. The first child's stdout 
is wired to the pipe write-end via `dup2()`. The second child's 
stdin is wired to the pipe read-end. Both are then exec'd.

**Redirection:** Inside the child process, stdout is replaced with 
an open file descriptor using `dup2()` before calling `execvp()` — 
the program writes to the file without knowing it.

**cd built-in:** `cd` must run in the parent process via `chdir()`. 
Running it in a child would change the child's directory, which exits 
immediately — the parent shell would be unaffected.
