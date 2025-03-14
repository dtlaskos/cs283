#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dshlib.h"

static int split_commands(char *input, char **parts, int max_parts);
static void free_command_parts(char **parts, int count);

static int isspace(int c) {
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
    cmd_buff_t command_buffers[CMD_MAX];
    char *command_parts[CMD_MAX];
    int num_commands = 0;

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

        num_commands = split_commands(trimmed, command_parts, CMD_MAX);
        if (num_commands < 0) {
            fprintf(stderr, "error: command line too complex or too many commands\n");
            continue;
        } else if (num_commands == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        int has_cd = 0;
        for (int i = 0; i < num_commands; i++) {
            if (parse_cmd_line(command_parts[i], &command_buffers[i]) != OK) {
                fprintf(stderr, "error: failed to parse command %d\n", i);
                has_cd = -1;
                break;
            }
            if (strcmp(command_buffers[i].argv[0], "cd") == 0) {
                has_cd = 1;
            }
        }

        if (has_cd == -1) {
            for (int i = 0; i < num_commands; i++) {
                if (command_buffers[i]._cmd_buffer) {
                    free(command_buffers[i]._cmd_buffer);
                    memset(&command_buffers[i], 0, sizeof(cmd_buff_t));
                }
            }
            free_command_parts(command_parts, num_commands);
            continue;
        }

        if (has_cd) {
            if (num_commands > 1) {
                fprintf(stderr, "cd: can't be part of a pipeline\n");
            } else {
                if (command_buffers[0].argc > 2) {
                    fprintf(stderr, "cd: too many arguments\n");
                } else {
                    char *path = command_buffers[0].argc == 1 ? getenv("HOME") : command_buffers[0].argv[1];
                    if (chdir(path) != 0) {
                        perror("cd");
                    }
                }
            }
            for (int i = 0; i < num_commands; i++) {
                free(command_buffers[i]._cmd_buffer);
            }
            free_command_parts(command_parts, num_commands);
            continue;
        }

        int prev_pipe_read = -1;
        int pipefds[2];
        pid_t pids[CMD_MAX];
        int num_pids = 0;

        for (int i = 0; i < num_commands; i++) {
            if (i < num_commands - 1) {
                if (pipe(pipefds) == -1) {
                    perror("pipe");
                    break;
                }
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                break;
            }

            if (pid == 0) {
                if (i != 0) {
                    dup2(prev_pipe_read, STDIN_FILENO);
                    close(prev_pipe_read);
                }

                if (i < num_commands - 1) {
                    dup2(pipefds[1], STDOUT_FILENO);
                    close(pipefds[0]);
                    close(pipefds[1]);
                }

                execvp(command_buffers[i].argv[0], command_buffers[i].argv);
                perror("execvp failed");
                _exit(EXIT_FAILURE);
            } else {
                if (i != 0) {
                    close(prev_pipe_read);
                }
                if (i < num_commands - 1) {
                    prev_pipe_read = pipefds[0];
                    close(pipefds[1]);
                }
                pids[num_pids++] = pid;
            }
        }

        if (prev_pipe_read != -1) {
            close(prev_pipe_read);
        }

        for (int i = 0; i < num_pids; i++) {
            waitpid(pids[i], NULL, 0);
        }

        for (int i = 0; i < num_commands; i++) {
            free(command_buffers[i]._cmd_buffer);
        }
        free_command_parts(command_parts, num_commands);
        memset(command_buffers, 0, sizeof(command_buffers));
        memset(command_parts, 0, sizeof(command_parts));
        num_commands = 0;
    }

    return 0;
}

static int split_commands(char *input, char **parts, int max_parts) {
    int in_quote = 0;
    char quote_char = 0;
    int part_start = 0;
    int part_count = 0;
    size_t len = strlen(input);

    for (size_t i = 0; i < len; i++) {
        if (input[i] == '"' || input[i] == '\'') {
            if (!in_quote) {
                in_quote = 1;
                quote_char = input[i];
            } else if (input[i] == quote_char) {
                in_quote = 0;
                quote_char = 0;
            }
        } else if (input[i] == '|' && !in_quote) {
            if (part_count >= max_parts) return -1;

            size_t part_len = i - part_start;
            parts[part_count] = malloc(part_len + 1);
            if (!parts[part_count]) return -1;
            strncpy(parts[part_count], input + part_start, part_len);
            parts[part_count][part_len] = '\0';
            parts[part_count] = trim_whitespace(parts[part_count]);
            if (strlen(parts[part_count]) == 0) {
                free(parts[part_count]);
                return -1;
            }
            part_count++;
            part_start = i + 1;
            while (part_start < len && isspace(input[part_start])) part_start++;
            i = part_start - 1;
        }
    }

    if (part_count >= max_parts) return -1;
    parts[part_count] = strdup(input + part_start);
    if (!parts[part_count]) return -1;
    parts[part_count] = trim_whitespace(parts[part_count]);
    if (strlen(parts[part_count]) == 0) {
        free(parts[part_count]);
        return part_count ? part_count : -1;
    }
    part_count++;
    return part_count;
}

static void free_command_parts(char **parts, int count) {
    for (int i = 0; i < count; i++) {
        free(parts[i]);
    }
}