#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    if (len > BUFFER_SZ) {
        // The user-supplied string is too large
        return -1;
    }

    char *src = user_str; // Pointer to traverse the user-supplied string
    char *dest = buff;    // Pointer to populate the internal buffer
    int user_str_len = 0; // Tracks the length of the user-supplied string
    int prev_was_space = 0; // Tracks if the previous character was whitespace

    // Process the user-supplied string
    while (*src != '\0') {
        if (*src == ' ' || *src == '\t') {
            // Handle whitespace
            if (!prev_was_space) {
                *dest = ' ';      // Add a single space to the buffer
                dest++;
                user_str_len++;
                if (user_str_len > len) {
                    // The user-supplied string exceeded buffer size
                    return -1;
                }
            }
            prev_was_space = 1; // Mark that the current char is whitespace
        } else {
            // Copy non-whitespace character
            *dest = *src;
            dest++;
            user_str_len++;
            if (user_str_len > len) {
                // The user-supplied string exceeded buffer size
                return -1;
            }
            prev_was_space = 0; // Reset whitespace tracker
        }
        src++; // Move to the next character in the user string
    }

    // Fill the remainder of the buffer with '.'
    while (user_str_len < BUFFER_SZ) {
        *dest = '.';
        dest++;
        user_str_len++;
    }

    // Return the length of the user-supplied string
    return (dest - buff);
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    return 0;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

void reverse_string(char *buff, int str_len) {
    printf("Reversed String: ");
    for (int i = str_len - 1; i >= 0; i--) {
        putchar(buff[i]);
    }
    putchar('\n');
}

void word_print(char *buff, int str_len) {
    printf("Word Print\n----------\n");
    int word_index = 1;
    int word_len = 0;
    int start = 0;
    for (int i = 0; i <= str_len; i++) {
        if (buff[i] == ' ' || buff[i] == '\0') {
            if (word_len > 0) {
                printf("%d. ", word_index);
                for (int j = start; j < start + word_len; j++) {
                    putchar(buff[j]);
                }
                printf(" (%d)\n", word_len);
                word_index++;
            }
            word_len = 0;
        } else {
            if (word_len == 0) {
                start = i;
            }
            word_len++;
        }
    }
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      The || operator uses short-circuit evaluation. If argc < 2 is true,
    //      the second condition (*argv[1] != '-') is not evaluated, avoiding 
    //      an invalid memory access.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      If there are < 3 arguments, print proper usage and exit
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = malloc(BUFFER_SZ);
    if(buff == NULL){
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff, user_str_len);
            break;

        case 'w':
            word_print(buff, user_str_len);
            break;

        case 'x':
            if (argc < 5) {
                printf("Error: Missing arguments for -x option.\n");
                exit(2);
            }
            printf("Not implemented!\n");
            exit(3);

        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          By explicitly passing the length, the function can check bounds and
//          prevent buffer overflows, making the code safer and less prone to 
//          vulnerabilities