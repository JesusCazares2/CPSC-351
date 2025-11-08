#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_LINE_LEN 512
#define MAX_PLAINTEXT_LEN 512

char input_filename[] = "_set1c4_input.txt";

// English frequency table (rough estimates)
double english_freq[256] = { 0 };

void init_english_freq() {
    english_freq['a'] = 0.0651738; english_freq['b'] = 0.0124248;
    english_freq['c'] = 0.0217339; english_freq['d'] = 0.0349835;
    english_freq['e'] = 0.1041442; english_freq['f'] = 0.0197881;
    english_freq['g'] = 0.0158610; english_freq['h'] = 0.0492888;
    english_freq['i'] = 0.0558094; english_freq['j'] = 0.0009033;
    english_freq['k'] = 0.0050529; english_freq['l'] = 0.0331490;
    english_freq['m'] = 0.0202124; english_freq['n'] = 0.0564513;
    english_freq['o'] = 0.0596302; english_freq['p'] = 0.0137645;
    english_freq['q'] = 0.0008606; english_freq['r'] = 0.0497563;
    english_freq['s'] = 0.0515760; english_freq['t'] = 0.0729357;
    english_freq['u'] = 0.0225134; english_freq['v'] = 0.0082903;
    english_freq['w'] = 0.0171272; english_freq['x'] = 0.0013692;
    english_freq['y'] = 0.0145984; english_freq['z'] = 0.0007836;
    english_freq[' '] = 0.1918182;
}

double score_english(const unsigned char *text, size_t len) {
    double score = 0.0;
    for (size_t i = 0; i < len; ++i) {
        score += english_freq[tolower(text[i])];
    }
    return score;
}

size_t hex_to_bytes(const char *hexstr, unsigned char *output) {
    size_t len = strlen(hexstr);
    size_t out_len = 0;

    for (size_t i = 0; i < len; i += 2) {
        sscanf(&hexstr[i], "%2hhx", &output[out_len++]);
    }
    return out_len;
}

void singlechar_xor_brute_force(const unsigned char *input, size_t len,
                                 unsigned char *best_output, double *best_score) {
    unsigned char output[MAX_PLAINTEXT_LEN];
    double max_score = 0.0;

    for (int key = 0; key < 256; ++key) {
        for (size_t i = 0; i < len; ++i) {
            output[i] = input[i] ^ key;
        }
        double score = score_english(output, len);
        if (score > max_score) {
            memcpy(best_output, output, len);
            *best_score = score;
            best_output[len] = '\0';
            max_score = score;
        }
    }
}

int main() {
    init_english_freq();

    FILE *fp = fopen(input_filename, "r");
    if (!fp) {
        perror("Failed to open input file");
        return 1;
    }

    char line[MAX_LINE_LEN];
    unsigned char hexbuf[MAX_LINE_LEN];
    unsigned char best_result[MAX_PLAINTEXT_LEN] = {0};
    double highest_score = 0.0;

    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (line[len - 1] == '\n') line[len - 1] = '\0';

        size_t binlen = hex_to_bytes(line, hexbuf);

        unsigned char candidate[MAX_PLAINTEXT_LEN] = {0};
        double score = 0.0;
        singlechar_xor_brute_force(hexbuf, binlen, candidate, &score);

        if (score > highest_score) {
            highest_score = score;
            memcpy(best_result, candidate, binlen);
            best_result[binlen] = '\0';
        }
    }

    fclose(fp);

    printf("\nBest candidate:\n%s\n", best_result);
    int result = strncmp((char*)best_result, "Now that the party is jumping\n", 30);
    printf("Assertion %s\n\n", result == 0 ? "PASSED." : "failed.");

    return result;
}
