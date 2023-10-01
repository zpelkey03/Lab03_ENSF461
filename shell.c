#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"


// Define MAX_PATH_LENGTH using PATH_MAX
#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

int main() {
    char buffer[1024];
    char* parsedinput;
    char* args[3];
    char newline;

    char *path = getenv("PATH");//get the current balue of the PATH variable
    char path_backup[200];

    strcpy(path_backup, path);//copies the current path as it somehow seems to change later on
    
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
        //Sample shell logic implementation
        if ( strcmp(parsedinput, "quit") == 0 ) {
            printf("Bye!!\n");
            return 0;
        }
        else {
            pid_t forkV = fork();
            if ( forkV == 0 ) {

                args[0] = parsedinput;// get the path/command
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

                int command_executed = 0; // Flag to track if the command is executed

                // Check if the command is an absolute path
                if (parsedinput[0] == '/') {
                    if (access(parsedinput, X_OK) == 0) {
                        // Execute the absolute path command
                        if (execve(parsedinput, args, NULL) == -1) {
                            fprintf(stderr, "Error running command in execve\n");
                            exit(-100); // Exit the child process with an error code
                        }
                        command_executed = 1; // Command executed successfully
                    }
                }

                else if (strchr(args[0], '/') != NULL) {
                    char full_path[200];
                    //Construct the full path using the current working directory
                    snprintf(full_path, sizeof(full_path), "./%s", parsedinput);
                    if (execve(full_path, args, NULL) == -1) {
                        fprintf(stderr, "Error running command in execve\n");
                        return -100;
                    }
                }

                else{

                    char *pathtoken = strtok(path_backup, ":");//Token the PATH variables

                    while (pathtoken != NULL){
                        //Construct a path to the command in the directory
                        char full_path[250];
                        snprintf(full_path, sizeof(full_path),"%s/%s", pathtoken, parsedinput);
                        //Check if the file exists and is executable
                        if (access(full_path, X_OK) == 0){
                            //Execute the command from the directory
                            if (execve(full_path, args, NULL) == -1) {
                            fprintf(stderr, "Error running command in execve\n");
                            return -100;
                            }
                            break;
                        }
                        pathtoken = strtok(NULL, ":");
                    }
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

