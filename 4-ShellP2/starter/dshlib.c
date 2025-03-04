#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dshlib.h"

int isspace(int c) {
    /* Check for standard whitespace characters:
       - space (' ')
       - form feed ('\f')
       - newline ('\n')
       - carriage return ('\r')
       - horizontal tab ('\t')
       - vertical tab ('\v') */
    return (c == ' '  || 
            c == '\f' || 
            c == '\n' || 
            c == '\r' || 
            c == '\t' || 
            c == '\v');
}

// Helper function to trim whitespace
static char *trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int parse_cmd_line(char *line, cmd_buff_t *cmd_buff) {
    char *trimmed = trim_whitespace(line);
    if (strlen(trimmed) == 0) return WARN_NO_CMDS;

    cmd_buff->_cmd_buffer = strdup(trimmed);
    if (!cmd_buff->_cmd_buffer) return ERR_CMD_OR_ARGS_TOO_BIG;

    int argc = 0;
    int in_quote = 0;
    char quote_char = 0;
    size_t len = strlen(cmd_buff->_cmd_buffer);
    char *buffer = cmd_buff->_cmd_buffer;

    cmd_buff->argv[argc++] = buffer;

    for (size_t i = 0; i < len; i++) {
        if (!in_quote && (buffer[i] == '"' || buffer[i] == '\'')) {
            in_quote = 1;
            quote_char = buffer[i];
            memmove(&buffer[i], &buffer[i+1], len - i);
            len--;
            i--;
        } else if (in_quote && buffer[i] == quote_char) {
            in_quote = 0;
            quote_char = 0;
            memmove(&buffer[i], &buffer[i+1], len - i);
            len--;
            i--;
        } else if (!in_quote && buffer[i] == ' ') {
            buffer[i] = '\0';
            while (i + 1 < len && buffer[i+1] == ' ') {
                memmove(&buffer[i+1], &buffer[i+2], len - i - 1);
                len--;
            }
            if (argc < CMD_ARGV_MAX - 1) {
                cmd_buff->argv[argc++] = &buffer[i+1];
            } else {
                free(cmd_buff->_cmd_buffer);
                return ERR_TOO_MANY_COMMANDS;
            }
        }
    }

    cmd_buff->argc = argc;
    cmd_buff->argv[argc] = NULL;

    for (int i = 0; i < argc; i++) {
        if (strlen(cmd_buff->argv[i]) >= ARG_MAX) {
            free(cmd_buff->_cmd_buffer);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }

    return OK;
}

int exec_local_cmd_loop() {
    char line[SH_CMD_MAX];
    cmd_buff_t cmd_buff = {0};

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(trimmed, EXIT_CMD) == 0) break;

        int rc = parse_cmd_line(trimmed, &cmd_buff);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) printf("%s", CMD_WARN_NO_CMD);
            else if (rc == ERR_TOO_MANY_COMMANDS) printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            else fprintf(stderr, "error: command or arguments too long\n");
            if (cmd_buff._cmd_buffer) free(cmd_buff._cmd_buffer);
            memset(&cmd_buff, 0, sizeof(cmd_buff_t));
            continue;
        }

        if (cmd_buff.argc == 0) {
            free(cmd_buff._cmd_buffer);
            continue;
        }

        if (strcmp(cmd_buff.argv[0], "cd") == 0) {
            if (cmd_buff.argc > 2) fprintf(stderr, "cd: too many arguments\n");
            else if (cmd_buff.argc == 2 && chdir(cmd_buff.argv[1]) != 0) perror("cd");
            free(cmd_buff._cmd_buffer);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            free(cmd_buff._cmd_buffer);
            continue;
        } else if (pid == 0) {
            execvp(cmd_buff.argv[0], cmd_buff.argv);
            perror("execvp failed");
            free(cmd_buff._cmd_buffer);
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, NULL, 0);
        }

        free(cmd_buff._cmd_buffer);
        memset(&cmd_buff, 0, sizeof(cmd_buff_t));
    }
}