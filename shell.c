#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 100

// ---------------- PARSE INPUT ----------------
char** parse_input(char *line) {
    char **args = malloc(MAX_ARGS * sizeof(char*));
    int i = 0;
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

// ---------------- HANDLE REDIRECTION ----------------
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            return;
        }
    }
}

// ---------------- HANDLE PIPE ----------------
int handle_pipe(char **args) {
    int pipe_index = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }
    if (pipe_index == -1) return 0;

    args[pipe_index] = NULL;
    char **left  = args;
    char **right = &args[pipe_index + 1];

    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(left[0], left);
        perror("exec failed");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(right[0], right);
        perror("exec failed");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
    return 1;
}

// ---------------- MAIN ----------------
int main() {
    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (getline(&line, &len, stdin) == -1) {
            printf("\n");
            break;
        }

        char **args = parse_input(line);

        if (args[0] == NULL) {
            free(args);
            continue;
        }

        // -------- built-in: exit --------
        if (strcmp(args[0], "exit") == 0) {
            free(args);
            exit(0);
        }

        // -------- built-in: cd --------
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                if (chdir(args[1]) != 0)
                    perror("cd failed");
            } else {
                printf("cd: missing argument\n");
            }
            free(args);
            continue;
        }

        // -------- handle pipe --------
        if (handle_pipe(args)) {
            free(args);
            continue;
        }

        // -------- normal execution --------
        pid_t pid = fork();
        if (pid == 0) {
            handle_redirection(args);
            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork failed");
        }

        free(args);
    }

    free(line);
    return 0;
}