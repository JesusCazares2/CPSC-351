#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ctype.h>

int atoi_radix(char c, int radix) {
    int value;

    if (radix < 2 || radix > 36) return -1; // Invalid radix

    if (isdigit(c))
        value = c - '0';
    else if (isalpha(c))
        value = toupper(c) - 'A' + 10;
    else
        return -1; // Invalid character

    return (value < radix) ? value : -1; // Return -1 if out of range
}

char b64_encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijk"
                     "lmnopqrstuvwxyz0123456789+/";

enum Status { START_NEW = 0, TAKE_2 = 1, TAKE_4 = 2 };

void hex_to_base64(const char* hexdata, char* b64data, size_t length) {
    memset(b64data, 0, length);                

    int sixbits = 0;                 
    enum Status status = START_NEW;   

    while (*hexdata != '\0') { 
        char hexchar = *hexdata;
        int dec = atoi_radix(hexchar, 16);   

        if (status == START_NEW) {
            sixbits = dec;
            status = TAKE_2;
        }
        else if (status == TAKE_2) {
            sixbits = (sixbits << 2) | (dec >> 2);
            *b64data++ = b64_encoding_table[sixbits];
            sixbits = (dec & 0x3);   
            status = TAKE_4;
        }
        else if (status == TAKE_4) {
            sixbits = (sixbits << 4) | dec;
            *b64data++ += b64_encoding_table[sixbits];
            status = START_NEW;
        }
        hexdata++;
    }

    if (status == TAKE_2) {
        sixbits <<= 2;
        *b64data++ += b64_encoding_table[sixbits];
        *b64data++ += '=';
    }
    else if (status == TAKE_4) {
        sixbits <<= 4;
        *b64data++ += b64_encoding_table[sixbits];
        *b64data++ += '=';
        *b64data++ += '=';
    }
}

int main(int argc, const char* argv[]) {
    char base64[1000];  
    
    hex_to_base64("49276d206b696c6c696e6720796f757220627261696e206c696b"
                                      "65206120706f69736f6e6f7573206d757368726f6f6d", base64, sizeof(base64));
    bool correct = strcmp(base64, "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t") == 0;
    printf("\nbase64 is...%s\nEncoding is correct? %s\n\n", base64, correct ? "YES" : "no");
    return 0;
}