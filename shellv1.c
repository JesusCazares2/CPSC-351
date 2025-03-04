#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#define ARGUMENTS_SIZE   64

void parse(char* line, char** argv) { 
    while (*line != '\0') { 
        while (isspace(*line)) {
            *line++ = '\0';
        }
        *argv++ = line;
        while (*line != '\0' && !isspace(*line)) {
            line++;
        }
    }
    printf("exiting...\n");
    *argv = NULL;  
}

int main(int argc, const char* argv[]) {
    char buf[BUFSIZ]; 
    bool finished = false;

    printf("Welcome to my shell program. "
            "Type any system command or exit or cd or mkdir\n");

    while (!finished) { 
        memset(buf, 0, sizeof(buf));

        printf("> ");
        char* p = fgets(buf, BUFSIZ, stdin);
        
        if (p == NULL) {
            continue;
        }
        
        buf[strlen(buf) - 1] = '\0'; 
        printf("You entered %lu characters: %s\n", strlen(p), buf);

        if (strncmp(buf, "exit", 4) == 0) {  
            printf("Exiting shell...\n");
            exit(0);
        } 

        char* arguments[ARGUMENTS_SIZE];
        memset(arguments, 0, ARGUMENTS_SIZE * sizeof(char*));

        parse(buf, arguments);
        printf("strlen(arguments[0]) is %lu\n", strlen(arguments[0]));
        if (arguments[1] != NULL) {
            printf("strlen(arguments[1]) is %lu\n", strlen(arguments[1]));
        }
        if (arguments[2] != NULL) {
            printf("strlen(arguments[2]) is %lu\n", strlen(arguments[2]));
        }

        char** q = arguments;
        while (*q != NULL) { 
            printf("arg is: %s\n", *q);
            ++q;
        }

        int rc = fork();
        if (rc < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) {
            if (execvp(arguments[0], arguments) == -1) {
                perror("exec failed");
                exit(1);
            }
        } else {
            printf("Parent of %d (pid:%d)\n", rc, (int) getpid());
        }
    }

    return 0;
}