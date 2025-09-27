#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 256
#define MAX_RECURSION_DEPTH 5
#define MAX_NUM 100

static char buf[BUF_SIZE] = {};
static char code_buf[BUF_SIZE + 128] = {}; // a little larger than `buf`
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";

// Generate a random number less than n.
uint32_t choose(uint32_t n) {
    return rand() % n;
}

// Append string to buffer.
void gen(const char *str) {
    size_t len = strlen(str);
    if (strlen(buf) + len < BUF_SIZE - 1) { // Check remaining space, '- 1' is to ensure '\0' at the end.
				strcat(buf, str);
    } else {
        printf("Buffer overflow!\n");
        exit(1);
    }
}


// Generate a random unsigned 32-bit integer.
void gen_num() {
    char num_str[32];
    uint32_t num = choose(MAX_NUM); // Generate a random number between 0 and 2^32-1.
    sprintf(num_str, "%u", num);
    gen(num_str);
}

// Generate a random operator.
void gen_rand_op() {
    char ops[] = {'+', '-', '*', '/'};
    char op = ops[choose(4)];
    if (op == '/') {
        uint32_t divisor;
        do {
            divisor = choose(MAX_NUM); // Generate divisor.
        } while (divisor == 0); // Ensure divisor is not equal to 0.
        char op_str[32];
        sprintf(op_str, "/%u", divisor);
        gen(op_str);
    } else {
        char op_str[2] = {op, '\0'};
        gen(op_str);
    }
}

// Record left parentheses' amount.
int open_parentheses = 0;

// Recursion depth counter.
int recursion_depth = 0;

// Generate a random expression.
static void gen_rand_expr() {
    if (recursion_depth > MAX_RECURSION_DEPTH) {
			gen_num();
      return;
    }

    recursion_depth++;
    switch (choose(3)) {
        case 0:
            gen_num();
            break;
        case 1:
						if (strlen(buf) == 0 || strchr("+-*/(", buf[strlen(buf) - 1]) != NULL) {
						} else {
							gen_rand_op();
						}
            gen("(");
            open_parentheses++;
            gen_rand_expr();
            gen(")");
            open_parentheses--;
            break;
        default:
            gen_rand_expr();
            gen_rand_op();
            gen_rand_expr();
            break;
    }
    recursion_depth--;
}

// Check if there is a number followed directly by a parenthesis.
int has_num_followed_by_parenthesis(const char *expr) {
    for (size_t i = 0; i < strlen(expr) - 1; i++) {
        if (isdigit(expr[i]) && expr[i + 1] == '(') {
            return 1;
        }
    }
    return 0;
}

// Insert a random operator after the number.
void insert_operator_after_num(char *expr) {
    char new_expr[BUF_SIZE] = {};
    size_t j = 0;
    for (size_t i = 0; i < strlen(expr); i++) {
        new_expr[j++] = expr[i];
        if (isdigit(expr[i]) && expr[i + 1] == '(') {
            char op_str[2] = {0};
            do {
                op_str[0] = "+-/"[choose(3)];
            } while (op_str[0] == '/'); // Avoid division for simplicity
            new_expr[j++] = op_str[0];
        }
    }
    new_expr[j] = '\0';
    strcpy(expr, new_expr);
}


int main(int argc, char *argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        buf[0] = '\0'; // Clear buffer
        open_parentheses = 0; // Reset parentheses counter
        recursion_depth = 0; // Reset recursion depth
        gen_rand_expr();

        // Ensure all parentheses are closed
        while (open_parentheses > 0) {
            gen(")");
            open_parentheses--;
        }

				// Check if there is a number followed directly by a parenthesis
        if (has_num_followed_by_parenthesis(buf)) {
						//printf("***\n%s\n***\n", buf);
						//printf("Found number followed by parenthesis, inserting operator...\n");
						insert_operator_after_num(buf);
        }

        sprintf(code_buf, code_format, buf);

        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0) {
            //printf("Generated code:\n%s\n", code_buf);
            continue;
        }

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        uint32_t result;
        ret = fscanf(fp, "%u", &result);
        pclose(fp);
				if (result > 0xffffffff) {
					loop ++;
					continue;
				}
        printf("%u %s\n", result, buf);
    }
    return 0;
}
