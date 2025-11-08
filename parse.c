#include <stdio.h>
#include <string.h>
#include <ctype.h>

void parse_command(char *command) {
    char *token;
    int len = strlen(command);
    char result[1024] = "";  // To store the final result

    // Remove the trailing newline, if any
    if (command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }

    // Split the command into words
    token = strtok(command, " ");

    while (token != NULL) {
        // Check if the token is not one of the special characters '<', '>', '|'
        if (strcmp(token, "<") != 0 && strcmp(token, ">") != 0 && strcmp(token, "|") != 0) {
            // Add the token to the result
            strcat(result, token);
            strcat(result, " ");
        }
        token = strtok(NULL, " ");
    }

    // Remove the trailing space if there's any
    if (strlen(result) > 0 && result[strlen(result) - 1] == ' ') {
        result[strlen(result) - 1] = '\0';
    }

    // Check if the last word is "ECHO"
    char *last_word = strrchr(result, ' ');
    if (last_word != NULL) {
        last_word[0] = '\0';  // Remove "ECHO" from the result
    }

    // Print the final result
    printf("%s\n", result);
}

int main() {
    char command[1024];

    // Read the command from the user
    printf("Enter a command: ");
    fgets(command, sizeof(command), stdin);

    // Parse and echo the command if it ends with "ECHO"
    parse_command(command);

    return 0;
}
