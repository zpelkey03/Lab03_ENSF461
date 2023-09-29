#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

int main() {
    char buffer[1024];
    char* parsedinput;
    char* args[3];
    char newline;

    printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");
    do {
        //Print the terminal prompt and get input
        printf("$ ");
        char *input = fgets(buffer, sizeof(buffer), stdin);
        if(!input)
        {
            fprintf(stderr, "Error reading input\n");
            return -1;
        }
        
        //Clean and parse the input string
        parsedinput = (char*) malloc(BUFLEN * sizeof(char));
        
        size_t parselength = trimstring(parsedinput, input, BUFLEN);
        printf("parsedinput: %s\n", parsedinput);
        //Sample shell logic implementation
        if ( strcmp(parsedinput, "quit") == 0 ) {
            printf("Bye!!\n");
            return 0;
        }
        else {
            pid_t forkV = fork();
            if ( forkV == 0 ) {

                args[0] = parsedinput;
                char *token = strtok(parsedinput, " ");
                
                int arg_count = 1;  // Start from index 1 for arguments

                // Store the remaining arguments in args
                while ((token = strtok(NULL, " ")) != NULL) {
                            if (token[0] == '"') {
                                memmove(token, token + 1, strlen(token)); // Remove opening quote
                                token[strlen(token) - 1] = '\0'; // Remove closing quote
                            }
                    args[arg_count++] = token;
                }

                args[arg_count] = NULL;  // Null-terminate the arguments array

                if(execve(args[0], args, NULL) == -1)
                {
                    fprintf(stderr, "Error running command in execve\n");
                    return -100;
                }
                
            } else{
                wait(NULL);
            }
        }

        //Remember to free any memory you allocate!
        free(parsedinput);
    } while ( 1 );

    return 0;
}
