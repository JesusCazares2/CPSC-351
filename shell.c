#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>  
#include <sys/wait.h>
#include <fcntl.h>

#define ARGUMENTS_SIZE 64

char last_command[1024] = "";

void parse(char* line, char** argv) {
    while (*line != '\0') {
        while (isspace(*line)) {
            *line++ = '\0';
        }

        if (*line == '<' || *line == '>' || *line == '|') {
            *argv++ = line;  
            *line++;  
            *argv = NULL;    
            continue;
        }

        *argv++ = line; 

        while (*line != '\0' && !isspace(*line) && *line != '<' && *line != '>' && *line != '|') {
            line++;  
        }
    }
    *argv = NULL;  
}

void echo(char** arguments){
    int i = 0;
            while (arguments[i] != NULL && strcmp(arguments[i], "<") != 0 && strcmp(arguments[i], ">") != 0 && strcmp(arguments[i], "|") != 0) { //print arguments until a special character is encountered
                    printf("%s ", arguments[i]);
                    i++;
            }
            printf("\n");
}

void IO(char** arguments, int last_index){ // if last word in input in IO
    if(strcmp(arguments[last_index - 1], "<") == 0){ // print LT + text file name 
        printf("LT %s\n", arguments[last_index]);
    }
    else if(strcmp(arguments[last_index - 1], ">") == 0){   // pribt GT + text file name
        printf("GT %s\n", arguments[last_index]);
    }
}

void pipes(char** arguments){
    int k = 0;  // if the last word in the input is PIPE
    while (arguments[k] != NULL) {
        if (strcmp(arguments[k], "|") == 0) {
            printf("PIPE ");
            printf("%s ", arguments[k + 1]);
        }
        k++;
    }
    printf("\n"); 
}

void builtin_commands(char** arguments, bool* finished){    
    if(strcmp(arguments[0], "help") == 0){  // help - prints help screen with all built in commands listed
        printf("\nHELP SCREEN\n\n");
        printf("cd <path>: Changes directory\n");
        printf("mkdir <directory>: Creates a new directory\n");
        printf("exit: Exits the shell\n");
        printf("!!: Repeats last command\n\n");
    } else if(strcmp(arguments[0], "cd") == 0){ // cd will change the file directory, ex. cd /mnt/c/CSUF/
        if(chdir(arguments[1]) != 0){
            perror("cd error");
        }
    } else if(strcmp(arguments[0], "mkdir") == 0){  // mkdir will create a new file directory, must be inputted similary to cd to avoid error
        if(mkdir(arguments[1], 0755) != 0) {
            perror("mkdir");
        }
    } else if(strcmp(arguments[0], "exit") == 0){ // terminates the program
        *finished = true;
    } else if(strcmp(arguments[0], "!!") == 0){ // repeats the previous built-in command
        if (strlen(last_command) == 0){ // if nothing has been stored 
            printf("No previous command\n");
        } else{
            printf("Repeating: %s\n", last_command);

            // we need to parse once more because we are searching for the last command 
            char* last_arguments[ARGUMENTS_SIZE];
            memset(last_arguments, 0, sizeof(last_arguments));
            parse(last_command, last_arguments);

        if (strcmp(last_arguments[0], "help") == 0 || strcmp(last_arguments[0], "cd") == 0 || strcmp(last_arguments[0], "mkdir") == 0 || strcmp(last_arguments[0], "exit") == 0) {
            builtin_commands(last_arguments, finished); // run the function once more based off the command that is stored 
        }
    }
}
}

void pipe_redirect(char **arguments) {
    int i = 0;
    int fd[2];
    int in_fd = 0; // Store previous pipe's read end
    pid_t pid;

    while (arguments[i] != NULL) {
        char *cmd[ARGUMENTS_SIZE]; // Store command
        int j = 0;
        char *output_file = NULL;

        // Extract command until pipe '|' or redirection '>'
        while (arguments[i] != NULL && strcmp(arguments[i], "|") != 0 && strcmp(arguments[i], ">") != 0) {
            cmd[j++] = arguments[i++];
        }
        cmd[j] = NULL; // Null-terminate the command

        if (arguments[i] != NULL && strcmp(arguments[i], ">") == 0) { // Handle output redirection
            if (arguments[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: no output file specified\n");
                return;
            }
            output_file = arguments[i + 1];
            arguments[i] = NULL; // Terminate command at '>'
            i += 2; // Skip '>' and filename
        }

        if (arguments[i] != NULL && strcmp(arguments[i], "|") == 0) { // Pipe found
            if (pipe(fd) == -1) {
                perror("pipe failed");
                exit(1);
            }
        }

        pid = fork();
        if (pid == 0) { // Child process
            if (in_fd != 0) { // If there's a previous pipe
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (arguments[i] != NULL && strcmp(arguments[i], "|") == 0) { // If there's a next pipe
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
            }
            if (output_file) { // Handle output redirection
                int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("open failed");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            execvp(cmd[0], cmd);
            perror("execvp failed");
            exit(1);
        }

        // Parent process
        if (in_fd != 0) close(in_fd); // Close previous pipe read end
        if (arguments[i] != NULL && strcmp(arguments[i], "|") == 0) { // If there's a pipe
            close(fd[1]);  // Close write end
            in_fd = fd[0]; // Update input for the next command
            i++; // Move past the pipe symbol
        } else {
            break; // No more pipes
        }
    }

    while (wait(NULL) > 0); // Wait for all child processes to finish
}



int main(int argc, const char* argv[]) {
    char buf[BUFSIZ];
    bool finished = false;

    while (!finished) {
        memset(buf, 0, sizeof(buf));
        printf("> ");
        char* p = fgets(buf, BUFSIZ, stdin);
        if (p == NULL) {
            continue;
        }
        buf[strcspn(buf, "\n")] = '\0';  

        char* arguments[ARGUMENTS_SIZE];
        memset(arguments, 0, ARGUMENTS_SIZE * sizeof(char*));
        parse(buf, arguments);

        //Inputs containing ECHO or IO
        int i = 0;
        int last_index = -1;
        bool echo_found = false;
        bool IO_found = false;

        while (arguments[i] != NULL) {  //locate ECHO or IO in input
            if (strcasecmp(arguments[i], "ECHO") == 0) {
                echo_found = true;
                break;
            } else if(strcasecmp(arguments[i], "IO") == 0){
                IO_found = true;
                break;
            }
            last_index = i; // this is needed for file name
            i++;
        }

        if (echo_found) {   //if ECHO found, print everything before the special characters '>', '<', '|'
            echo(arguments);
            continue;
        }

        if(IO_found){   //if IO is found, print either GT for '>' or LT for '<'
            IO(arguments, last_index);
            continue;
        }

        //Inputs containing PIPES 
        bool pipe_found = false;
        int k = 0;

        while(arguments[k] != NULL){    //search for PIPE in input
            if(strcmp(arguments[k], "PIPE") == 0){
                pipe_found = true;
                break;
            }
            k++;
        }  
        
        if (pipe_found) {   //for every '|' in the input, print 'PIPE' and the file it is piping to
            pipes(arguments);
            continue;
        }

        //check if the input is a built-in command
        if(strcmp(arguments[0], "help") == 0 || strcmp(arguments[0], "cd") == 0 || strcmp(arguments[0], "mkdir") == 0 || strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "!!") == 0){
            if (strcmp(arguments[0], "!!") != 0) {  //if input is a built-in we need to store it in case we wish to call it back
                strncpy(last_command, buf, sizeof(last_command) - 1);
            }

            builtin_commands(arguments, &finished);
        
            continue; // this is needed to avoid built-in commands from triggering fork/exec
        }

         // if none of the previous loops are triggered, assume that we either want to pipe or redirect content 
        pipe_redirect(arguments);
    }
    return 0;
}
