#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
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
     // Trim leading space
     while (isspace((unsigned char)*str)) str++;
 
     if (*str == 0) return str;
 
     // Trim trailing space
     char *end = str + strlen(str) - 1;
     while (end > str && isspace((unsigned char)*end)) end--;
     end[1] = '\0';
 
     return str;
 }
 
 int build_cmd_list(char *cmd_line, command_list_t *clist) {
     memset(clist, 0, sizeof(command_list_t));
 
     char *cmd_line_copy = strdup(cmd_line);
     if (!cmd_line_copy) {
         return ERR_CMD_OR_ARGS_TOO_BIG;
     }
 
     char *saveptr_pipe;
     char *part = strtok_r(cmd_line_copy, PIPE_STRING, &saveptr_pipe);
     while (part != NULL) {
         char *trimmed_part = trim_whitespace(part);
         if (strlen(trimmed_part) == 0) {
             free(cmd_line_copy);
             return ERR_CMD_OR_ARGS_TOO_BIG;
         }
 
         if (clist->num >= CMD_MAX) {
             free(cmd_line_copy);
             return ERR_TOO_MANY_COMMANDS;
         }
 
         command_t *cmd = &clist->commands[clist->num];
         memset(cmd, 0, sizeof(command_t));
 
         // Split into exe and args
         char *saveptr;
         char *exe = strtok_r(trimmed_part, " \t", &saveptr);
         if (exe == NULL) {
             free(cmd_line_copy);
             return ERR_CMD_OR_ARGS_TOO_BIG;
         }
 
         if (strlen(exe) >= EXE_MAX) {
             free(cmd_line_copy);
             return ERR_CMD_OR_ARGS_TOO_BIG;
         }
         strncpy(cmd->exe, exe, EXE_MAX - 1);
         cmd->exe[EXE_MAX - 1] = '\0';
 
         // Process args
         char args_buf[ARG_MAX] = {0};
         char *token;
         int first_arg = 1;
         size_t args_len = 0;
 
         while(token = strtok_r(NULL, " \t", &saveptr)) {
             size_t token_len = strlen(token);
             if (args_len + token_len + (first_arg ? 0 : 1) >= ARG_MAX) {
                 free(cmd_line_copy);
                 return ERR_CMD_OR_ARGS_TOO_BIG;
             }
             if (!first_arg) {
                 strcat(args_buf, " ");
                 args_len++;
             }
             strcat(args_buf, token);
             args_len += token_len;
             first_arg = 0;
         }
 
         strncpy(cmd->args, args_buf, ARG_MAX - 1);
         cmd->args[ARG_MAX - 1] = '\0';
 
         clist->num++;
         part = strtok_r(NULL, PIPE_STRING, &saveptr_pipe);
     }
 
     free(cmd_line_copy);
     return OK;
 }