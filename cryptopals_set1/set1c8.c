#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BLOCK_SIZE 32  // 16 bytes = 32 hex characters
#define MAX_LINE_LENGTH 1024
#define MAX_BLOCKS 100

// Function to count repeated blocks in a hex-encoded ciphertext line
int count_aes_ecb_repetitions(const char *line) {
    int num_blocks = strlen(line) / BLOCK_SIZE;
    const char *blocks[MAX_BLOCKS];
    int repeats = 0;

    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = line + i * BLOCK_SIZE;
    }

    // Compare each block with every other block
    for (int i = 0; i < num_blocks; i++) {
        for (int j = i + 1; j < num_blocks; j++) {
            if (strncmp(blocks[i], blocks[j], BLOCK_SIZE) == 0) {
                repeats++;
            }
        }
    }

    return repeats;
}

int main() {
    FILE *file = fopen("set1c8_input.txt", "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int most_repeats = 0;
    int line_number = 0;
    int ecb_line = -1;

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = 0;

        int repeats = count_aes_ecb_repetitions(line);
        if (repeats > most_repeats) {
            most_repeats = repeats;
            ecb_line = line_number;
        }

        line_number++;
    }

    fclose(file);

    assert(ecb_line == 132);
    printf("Encrypted line: %d\n", ecb_line);

    return 0;
}
