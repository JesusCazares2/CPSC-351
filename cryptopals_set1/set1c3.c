#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CIPHERTEXT_HEX "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736"

typedef struct {
    unsigned char key;
    double score;
    char *plaintext;
} Candidate;

// Character frequencies in English language
double get_char_freq(char c) {
    switch (tolower(c)) {
        case 'a': return 0.0651738; case 'b': return 0.0124248; case 'c': return 0.0217339;
        case 'd': return 0.0349835; case 'e': return 0.1041442; case 'f': return 0.0197881;
        case 'g': return 0.0158610; case 'h': return 0.0492888; case 'i': return 0.0558094;
        case 'j': return 0.0009033; case 'k': return 0.0050529; case 'l': return 0.0331490;
        case 'm': return 0.0202124; case 'n': return 0.0564513; case 'o': return 0.0596302;
        case 'p': return 0.0137645; case 'q': return 0.0008606; case 'r': return 0.0497563;
        case 's': return 0.0515760; case 't': return 0.0729357; case 'u': return 0.0225134;
        case 'v': return 0.0082903; case 'w': return 0.0171272; case 'x': return 0.0013692;
        case 'y': return 0.0145984; case 'z': return 0.0007836; case ' ': return 0.1918182;
        default: return 0.0;
    }
}

double get_english_score(const char *text, size_t len) {
    double score = 0.0;
    for (size_t i = 0; i < len; ++i) {
        score += get_char_freq(text[i]);
    }
    return score;
}

void hex_to_bytes(const char *hex, unsigned char *out, size_t *out_len) {
    size_t len = strlen(hex);
    *out_len = len / 2;
    for (size_t i = 0; i < *out_len; ++i) {
        sscanf(hex + 2 * i, "%2hhx", &out[i]);
    }
}

Candidate singlechar_xor_brute_force(const unsigned char *ciphertext, size_t len) {
    Candidate best = {0, 0.0, NULL};

    for (int key = 0; key < 256; ++key) {
        char *decoded = malloc(len + 1);
        for (size_t i = 0; i < len; ++i) {
            decoded[i] = ciphertext[i] ^ key;
        }
        decoded[len] = '\0';

        double score = get_english_score(decoded, len);
        if (score > best.score) {
            if (best.plaintext) free(best.plaintext);
            best.key = key;
            best.score = score;
            best.plaintext = decoded;
        } else {
            free(decoded);
        }
    }

    return best;
}

int main() {
    unsigned char ciphertext[256];
    size_t len;

    hex_to_bytes(CIPHERTEXT_HEX, ciphertext, &len);

    Candidate result = singlechar_xor_brute_force(ciphertext, len);

    printf("\n\n%s\tScore: %.2f\tKey: %c\n", result.plaintext, result.score, result.key);

    // Check result
    if (strncmp(result.plaintext, "Cooking MC's like a pound of bacon", len) == 0) {
        printf("✅ Correct decryption!\n\n");
    } else {
        printf("❌ Incorrect result.\n");
    }

    free(result.plaintext);
    return 0;
}
