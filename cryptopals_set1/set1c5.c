#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Convert two ascii strings to XOR result and write it into output buffer
void repeating_xor(const char *plaintext, const char *key, char *output) {
    size_t text_len = strlen(plaintext);
    size_t key_len = strlen(key);
    for (size_t i = 0; i < text_len; ++i) {
        unsigned char xored = plaintext[i] ^ key[i % key_len];
        sprintf(output + i * 2, "%02x", xored);
    }
    output[text_len * 2] = '\0';
}

int main() {
    const char *plaintext = "Burning 'em, if you ain't quick and nimble\n"
                            "I go crazy when I hear a cymbal";
    const char *key = "ICE";
    char output[4096]; 
    repeating_xor(plaintext, key, output);

    assert(strcmp(output, "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f") == 0);
    printf("\nXOR Result: %s\n\n", output);

    return 0;
}