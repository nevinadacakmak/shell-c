# shell-c

A Unix shell implemented from scratch in C. Supports command execution via fork/execvp, pipes, I/O redirection, background processes, and built-in commands (cd, exit). Built as a systems programming project to understand process management, file descriptors, and kernel interfaces at a low level.

## Features

- Command execution via `fork`, `execvp`, and `wait`
- Pipes (`ls | grep foo`)
- I/O redirection (`echo hello > out.txt`, `cat < in.txt`)
- Background execution (`sleep 5 &`) with zombie reaping via `SIGCHLD`
- Signal handling — shell ignores `SIGINT` and `SIGTSTP`; child processes reset to default behavior before exec
- Built-in commands: `cd`, `exit`

## Build & Run

```bash
gcc main.c -o shell
./shell
```

## Usage Examples

```
ada~$ ls | grep .c
ada~$ echo hello > out.txt
ada~$ cat < out.txt
ada~$ sleep 5 &
ada~$ cd /tmp
ada~$ exit
```

## Implementation Notes

- Input parsed with `strtok` into a null-terminated `argv` array
- Pipes implemented with `pipe()` and `dup2()` — child1 wires stdout to the write end, child2 wires stdin to the read end; parent closes both ends after forking
- I/O redirection uses `open()` and `dup2()` inside the child before `execvp`
- Background processes skip `wait()`; `SIGCHLD` handler reaps zombies asynchronously with `waitpid(-1, &status, WNOHANG)`
- `SIGINT` and `SIGTSTP` ignored in shell, reset to `SIG_DFL` in each child via `sigaction`

## What I Learned

Direct experience with the Unix process model — fork/exec separation, file descriptor inheritance, pipe plumbing, and asynchronous signal handling. Built intuition for how the kernel mediates between processes through fd tables and signal disposition.

As this is a project to learn more about shells, no AI coding tools were used for code generation.
