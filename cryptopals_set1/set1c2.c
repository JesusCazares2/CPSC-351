#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper function to convert a hex character to its integer value
int hex_char_to_int(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Convert two hex strings to XOR result and write it into output buffer
void hex_xor(const char *hex1, const char *hex2, char *output) {
    size_t len = strlen(hex1);
    for (size_t i = 0; i < len; i += 2) {
        int high1 = hex_char_to_int(hex1[i]);
        int low1  = hex_char_to_int(hex1[i + 1]);
        int high2 = hex_char_to_int(hex2[i]);
        int low2  = hex_char_to_int(hex2[i + 1]);

        int byte1 = (high1 << 4) | low1;
        int byte2 = (high2 << 4) | low2;
        int xored = byte1 ^ byte2;

        sprintf(output + i, "%02x", xored);
    }
    output[len] = '\0'; // null-terminate the result
}

int main() {
    const char *input1 = "1c0111001f010100061a024b53535009181c";
    const char *input2 = "686974207468652062756c6c277320657965";
    char output[100]; // ensure it's large enough

    hex_xor(input1, input2, output);

    assert(strcmp(output, "746865206b696420646f6e277420706c6179") == 0);
    printf("\nTest passed: %s\n\n", output);

    return 0;
}