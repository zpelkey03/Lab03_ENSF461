#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#include <signal.h>

// Define MAX_PATH_LENGTH using PATH_MAX
#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

pid_t backgroundProcessID = 0;

// Signal handler function to catch child process termination
void childTerminated(int signo) {
    int status;
    pid_t terminated = waitpid(backgroundProcessID, &status, WNOHANG);
    if (terminated > 0) {
        printf("Background Process %d Terminated\n", backgroundProcessID);
        printf("$ ");
        fflush(stdout); // Flush the standard output
    }
}

int main() {
    char buffer[1024];
    char* parsedinput;
    char* args[3];
    char newline;

    bool runningBackground = false;
    int status;
    

    char *path = getenv("PATH");//get the current balue of the PATH variable
    char path_backup[200];

    strcpy(path_backup, path);//copies the current path as it somehow seems to change later on
    
    printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");

    //Register handler for SIGCHLD
    signal(SIGCHLD, childTerminated);
    do {

        //Print the terminal prompt and get input
        printf("$ ");
        fflush(stdout);
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
        else{
            //check if "| exists"
            char *pipe_position = strchr(parsedinput, '|');

            //check if "&" exists
            char* background_position = strchr(parsedinput, '&');

            //remove "&" and allow this process to be run in the background
            if(background_position != NULL){
                runningBackground = true;
                *background_position = '\0';
            }

            if (pipe_position != NULL){
                //call pipe
                *pipe_position = '\0';
                char *command1 = parsedinput;
                char *command2 = pipe_position + 1;

                int pipe_fd[2];
                if (pipe(pipe_fd) == -1) {
                    perror("Pipe Creation Failed");
                    exit(EXIT_FAILURE);
                }

                // Fork the first child process
                pid_t child_pid1 = fork();
                if (child_pid1 == -1) {
                    perror("Fork Failed");
                    exit(EXIT_FAILURE);
                }

                if (child_pid1 == 0) {
                    // Child process 1 (handles the first command)
                    close(pipe_fd[0]); // Close the read end of the pipe

                    // Redirect stdout to the write end of the pipe (pipe_fd[1])
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[1]); // Close the write end of the pipe

                    // Tokenize the first command (command1)
                    char *args[3];
                    char *token = strtok(command1, " ");
                    int arg_count = 0;

                    // Set the args
                    while (token != NULL && token != "&") {
                        args[arg_count++] = token;
                        token = strtok(NULL, " ");
                    }
                    args[arg_count] = NULL;

                    int command_executed = 0; // Flag to track if the command is executed

                    // Check if the command is an absolute path
                    if (args[0][0] == '/') {
                        if (access(args[0], X_OK) == 0) {
                            // Execute the absolute path command
                            if (execve(args[0], args, NULL) == -1) {
                                fprintf(stderr, "Error running command in execve\n");
                                exit(-100); // Exit the child process with an error code
                            }
                            command_executed = 1; // Command executed successfully
                        }
                    }

                    else if (strchr(args[0], '/') != NULL) {
                        char full_path[200];
                        //Construct the full path using the current working directory
                        snprintf(full_path, sizeof(full_path), "./%s", args[0]);
                        if (execve(full_path, args, NULL) == -1) {
                            fprintf(stderr, "Error running command in execve\n");
                            return -100;
                        }
                    }

                    else{
                        // Construct the full path for the first command
                        char full_path[250];
                        char *pathtoken = strtok(path_backup, ":");
                        while (pathtoken != NULL) {
                            snprintf(full_path, sizeof(full_path), "%s/%s", pathtoken, args[0]);
                            if (access(full_path, X_OK) == 0) {
                                // Execute the first command from the directory
                                execve(full_path, args, NULL);
                                perror("Error running command 1");
                                exit(EXIT_FAILURE);
                            }
                            pathtoken = strtok(NULL, ":");
                        }
                        fprintf(stderr, "Command 1 not found: %s\n", args[0]);
                        exit(EXIT_FAILURE);
                    }
                }

                // Fork the second child process
                pid_t child_pid2 = fork();
                if (child_pid2 == -1) {
                    perror("Fork Failed");
                    exit(EXIT_FAILURE);
                }

                if (child_pid2 == 0) {
                    // Child process 2 (handles the second command)
                    close(pipe_fd[1]); // Close the write end of the pipe

                    // Redirect stdin to the read end of the pipe (pipe_fd[0])
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[0]); // Close the read end of the pipe

                    // Tokenize the second command (command2)
                    char *args[3];
                    char *token = strtok(command2, " ");
                    int arg_count = 0;

                    // Set the args
                    while (token != NULL) {
                        args[arg_count++] = token;
                        token = strtok(NULL, " ");
                    }
                    args[arg_count] = NULL;

                    int command_executed = 0; // Flag to track if the command is executed

                    // Check if the command is an absolute path
                    if (args[0][0] == '/') {
                        if (access(args[0], X_OK) == 0) {
                            // Execute the absolute path command
                            if (execve(args[0], args, NULL) == -1) {
                                fprintf(stderr, "Error running command in execve\n");
                                exit(-100); // Exit the child process with an error code
                            }
                            command_executed = 1; // Command executed successfully
                        }
                    }

                    else if (strchr(args[0], '/') != NULL) {
                        char full_path[200];
                        //Construct the full path using the current working directory
                        snprintf(full_path, sizeof(full_path), "./%s", args[0]);
                        if (execve(full_path, args, NULL) == -1) {
                            fprintf(stderr, "Error running command in execve\n");
                            return -100;
                        }
                    }

                    else{
                        // Construct the full path for the first command
                        char full_path[250];
                        char *pathtoken = strtok(path_backup, ":");
                        while (pathtoken != NULL) {
                            snprintf(full_path, sizeof(full_path), "%s/%s", pathtoken, args[0]);
                            if (access(full_path, X_OK) == 0) {
                                // Execute the first command from the directory
                                execve(full_path, args, NULL);
                                perror("Error running command 1");
                                exit(EXIT_FAILURE);
                            }
                            pathtoken = strtok(NULL, ":");
                        }
                        fprintf(stderr, "Command 1 not found: %s\n", args[0]);
                        exit(EXIT_FAILURE);
                    }
                }

                // Close both ends of the pipe in the parent process
                close(pipe_fd[0]);
                close(pipe_fd[1]);

                // Wait for both child processes to complete
                wait(NULL);
                wait(NULL);
            } else {
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
                    if(runningBackground){
                        backgroundProcessID = forkV;
                    }else{
                        waitpid(forkV, &status, 0);
                    }
                }
            }
        }

        //Remember to free any memory you allocate!
        free(parsedinput);
    } while ( 1 );

    return 0;
}
        //Remember to free any memory you allocate!
        free(parsedinput);
    } while ( 1 );

    return 0;
}
