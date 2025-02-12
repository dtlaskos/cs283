#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_N0_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */

 
 // Helper function to trim whitespace from a string
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
 
 int main() {
     char cmd_buff[SH_CMD_MAX];
     command_list_t clist;
 
     while (1) {
         printf("%s", SH_PROMPT);
         if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
             printf("\n");
             break;
         }
 
         // Remove trailing newline
         cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
 
         // Trim the command line to check for exit or empty
         char *trimmed_line = trim_whitespace(cmd_buff);
 
         // Check for empty command
         if (strlen(trimmed_line) == 0) {
             printf("%s", CMD_WARN_NO_CMD);
             continue;
         }
 
         // Check for exit command
        if (strcmp(trimmed_line, EXIT_CMD) == 0) break;
 
         // Parse the command line
         int rc = build_cmd_list(cmd_buff, &clist);
 
         switch (rc) {
             case OK:
                 printf(CMD_OK_HEADER, clist.num);
                 for (int i = 0; i < clist.num; i++) {
                     command_t *cmd = &clist.commands[i];
                     printf("<%d> %s", i + 1, cmd->exe);
                     if (cmd->args[0] != '\0') {
                         printf(" [%s]", cmd->args);
                     }
                     printf("\n");
                 }
                 break;
             case ERR_TOO_MANY_COMMANDS:
                 printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                 break;
             case ERR_CMD_OR_ARGS_TOO_BIG:
                 printf("error: command or arguments too long\n");
                 break;
             default:
                 break;
         }
     }
 
     exit(EXIT_SUCCESS);
 }