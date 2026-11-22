#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 128
#define MAX_CMDS 32

void handle_sigchld(int sig) {
    (void)sig;
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        
    }
    errno = saved_errno;
}

void setup_signals() {
    struct sigaction sa_ign, sa_chld;

    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags = 0;
    sigaction(SIGINT, &sa_ign, NULL);
    sigaction(SIGTSTP, &sa_ign, NULL);

    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}

void reset_child_signals() {
    struct sigaction sa_dfl;
    sa_dfl.sa_handler = SIG_DFL;
    sigemptyset(&sa_dfl.sa_mask);
    sa_dfl.sa_flags = 0;
    sigaction(SIGINT, &sa_dfl, NULL);
    sigaction(SIGTSTP, &sa_dfl, NULL);
}

int tokenize(char *input, char **tokens) {
    int count = 0;
    char *p = input;

    while (*p != '\0' && count < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            tokens[count++] = p;
            while (*p != '\0' && *p != quote) p++;
            if (*p == quote) *p++ = '\0';
        } else {
            tokens[count++] = p;
            while (*p != '\0' && *p != ' ' && *p != '\t') p++;
            if (*p != '\0') *p++ = '\0';
        }
    }
    tokens[count] = NULL;
    return count;
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        int status = (args[1] != NULL) ? atoi(args[1]) : 0;
        exit(status);
    }

    if (strcmp(args[0], "cd") == 0) {
        const char *target = (args[1] != NULL) ? args[1] : getenv("HOME");
        if (target == NULL || chdir(target) != 0) {
            perror("cd");
        }
        return 1;
    }

    if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }

    if (strcmp(args[0], "help") == 0) {
        printf("myshell - Custom UNIX Command-Line Interpreter\n");
        printf("Built-in commands:\n");
        printf("  cd [path]     Change working directory (defaults to $HOME)\n");
        printf("  pwd           Print current working directory\n");
        printf("  help          Display this help message\n");
        printf("  exit [status] Exit myshell with optional status code\n");
        printf("Features: I/O redirection (<, >, >>), pipelines (|), background (&)\n");
        return 1;
    }

    return 0; 
}

void apply_redirection(char **args) {
    char *clean_args[MAX_ARGS];
    int j = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {
            int fd = open(args[++i], O_RDONLY);
            if (fd < 0) {
                perror("open input");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        } else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
            int fd = open(args[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (strcmp(args[i], ">>") == 0 && args[i + 1] != NULL) {
            int fd = open(args[++i], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror("open append");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else {
            clean_args[j++] = args[i];
        }
    }
    clean_args[j] = NULL;

    for (int i = 0; i <= j; i++) {
        args[i] = clean_args[i];
    }
}

/* si no estoy mal de momneto tiene  la parte del signal handelin, lo del built in comand y el  redirection, creo que esta bien 
faltaria el execution, pipas y background, se puede jajajaja igualmente mi nigga cualquier cosa escribame sin pena*/