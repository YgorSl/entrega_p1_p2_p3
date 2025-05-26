#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LINE 1024
#define MAX_TERMS 100

typedef struct {
    int sign;              // +1 for positive, 0 for negative
    int is_multiplication; // 1 for A*B term, 0 for single value
    int multiplier;
    int multiplicand;
    int value;
} ExpressionTerm;

// Removes all whitespace
void remove_whitespace(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

int main() {
    char input_line[MAX_INPUT_LINE];
    if (!fgets(input_line, sizeof(input_line), stdin)) {
        return 0;
    }

    // Remove newline
    char *newline = strchr(input_line, '\n');
    if (newline) *newline = '\0';

    // Split by '='
    char *equals = strchr(input_line, '=');
    if (!equals) return 1;

    int var_name_len = equals - input_line;
    char variable_name[MAX_INPUT_LINE];
    strncpy(variable_name, input_line, var_name_len);
    variable_name[var_name_len] = '\0';
    remove_whitespace(variable_name);

    char expression[MAX_INPUT_LINE];
    strcpy(expression, equals + 1);
    remove_whitespace(expression);

    // Parse expression
    ExpressionTerm terms[MAX_TERMS];
    int term_count = 0;
    char *pos = expression;
    int current_sign = 1;

    while (*pos && term_count < MAX_TERMS) {
        if (*pos == '+') { current_sign = 1; pos++; continue; }
        if (*pos == '-') { current_sign = 0; pos++; continue; }

        char term_str[MAX_INPUT_LINE];
        char *end = pos;
        while (*end && *end != '+' && *end != '-') end++;
        int len = end - pos;
        strncpy(term_str, pos, len);
        term_str[len] = '\0';
        remove_whitespace(term_str);

        char *mult = strchr(term_str, '*');
        if (mult) {
            *mult = '\0';
            char left[MAX_INPUT_LINE], right[MAX_INPUT_LINE];
            strcpy(left, term_str);
            strcpy(right, mult + 1);
            remove_whitespace(left);
            remove_whitespace(right);

            terms[term_count].sign = current_sign;
            terms[term_count].is_multiplication = 1;
            terms[term_count].multiplier = atoi(left);
            terms[term_count].multiplicand = atoi(right);
        } else {
            terms[term_count].sign = current_sign;
            terms[term_count].is_multiplication = 0;
            terms[term_count].value = atoi(term_str);
        }
        term_count++;
        pos = end;
    }

    // Print variable name in UTF-8 (byte by byte)
    for (size_t i = 0; i < strlen(variable_name); i++) {
        unsigned char c = (unsigned char)variable_name[i];
        printf(">");
        for (int j = 0; j < c; j++) printf("+");
        printf(".");
        printf("[-]");  // clear cell after printing
    }

    // Print '=' (ASCII 61)
    printf(">");
    for (int i = 0; i < 61; i++) printf("+");
    printf(".");
    printf("[-]");

    // Move back to cell 0
    for (size_t i = 0; i < strlen(variable_name) + 1; i++) printf("<");

    // Clear cell 0
    printf("[-]");

    // Calculate expression into cell 0
    for (int i = 0; i < term_count; i++) {
        ExpressionTerm *t = &terms[i];
        if (t->is_multiplication) {
            int m1 = t->multiplier, m2 = t->multiplicand;

            // Use cell1 for multiplicand, cell2 for loop temp
            printf(">");
            printf("[-]");
            for (int j = 0; j < m2; j++) printf("+");

            printf(">");
            printf("[-]");

            // Multiply: loop over multiplicand, add multiplier times to cell0
            printf("<[>");  // start loop on cell1
            for (int j = 0; j < m1; j++) {
                if (t->sign) printf("+"); else printf("-");
            }
            printf("<-]>");

            // Clear cell1
            printf("<[-]");
            // Back to cell0
            printf("<");
        } else {
            int val = t->value;
            if (t->sign) {
                for (int j = 0; j < val; j++) printf("+");
            } else {
                for (int j = 0; j < val; j++) printf("-");
            }
        }
    }

    // Convert number to printable digit ('0' + value)
    printf("++++++++++++++++++++++++++++++++++++++++++++++++"); // +48
    printf(".");

    printf("\n");
    return 0;
}
