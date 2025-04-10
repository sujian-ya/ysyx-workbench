/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <cpu/decode.h>
#include <memory/vaddr.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

// Forward declaration of eval.
uint32_t eval(int p, int q, bool *success);

enum {
  TK_NOTYPE = 256, /* ' ' */
  TK_EQ,           /* '==' */
	TK_NEQ,					 /* '!=' */
	TK_AND,					 /* '&&' */
	TK_DEREF,				 /* '*' as dereference */
	TK_SUBTRACT,     /* minus number */
	TK_NUMBER,			 /* decimal integer */
	TK_HEX,					 /* hexadecimal number */
	TK_REG,					 /* register */
	TK_PLUS,				 /* '+' */
	TK_MINUS,				 /* '-' */
	TK_MUL,					 /* '*' */
	TK_DIV,					 /* '/' */
	TK_LPAREN,			 /* '(' */
	TK_RPAREN,       /* ')' */
	// Newly added.

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},					// spaces
  {"==", TK_EQ},							// equal
	{"!=", TK_NEQ},							// not equal
	{"&&", TK_AND},							// logical and
	{"0x[0-9a-fA-F]+", TK_HEX}, // hexadecimal number
	{"\\$[0-9a-zA-Z]+", TK_REG},   // register
	{"[0-9]+", TK_NUMBER},			// decimal integer
  {"\\+", TK_PLUS},						// plus
	{"-", TK_MINUS},						// minus or minus number
	{"\\*", TK_MUL},						// multiply or dereference
	{"/", TK_DIV},							// divide
	{"\\(", TK_LPAREN},					// left parenthesis
	{"\\)", TK_RPAREN},					// right parenthesis
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

#define MAX_TOKENS 65536
static Token tokens[MAX_TOKENS] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        /*Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);*/

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

				if (rules[i].token_type != TK_NOTYPE) {
						if (nr_token >= MAX_TOKENS) {
								printf("Tokens overflow!\n");
								return false;
						}

						switch (rules[i].token_type) {
						case TK_NUMBER:
						case TK_EQ:
						case TK_NEQ:
						case TK_AND:
						case TK_HEX:
						case TK_REG:
						case TK_PLUS:
						case TK_MINUS:
						case TK_MUL:
						case TK_DIV:
						case TK_LPAREN:
						case TK_RPAREN:
								strncpy(tokens[nr_token].str, substr_start, substr_len);
								tokens[nr_token].str[substr_len] = '\0';
								tokens[nr_token].type = rules[i].token_type;
								break;
						default:
								printf("Unknown token type!\n");
								return false;
							}
							nr_token++;
					}
					break;
			 }
		}

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }

  }

  return true;
}

bool check_parentheses(int p, int q) {
    if (tokens[p].type != TK_LPAREN || tokens[q].type != TK_RPAREN) {
        return false;
    }

    int paren_cnt = 0;

    for (int i = p; i <= q; i++) {
        if (tokens[i].type == TK_LPAREN) {
            paren_cnt++;
        } else if (tokens[i].type == TK_RPAREN) {
            paren_cnt--;
        }

        if (paren_cnt < 0) {
            return false;
        }
        if (paren_cnt == 0 && i < q) {
            return false;
        }
    }

    return paren_cnt == 0;
}

int get_precedence(int token_type) {
	switch (token_type) {
		case TK_AND:
			return 1;
		case TK_EQ:
		case TK_NEQ:
			return 2;
		case TK_PLUS:
		case TK_MINUS:
			return 3;
		case TK_MUL:
		case TK_DIV:
			return 4;
		case TK_DEREF:
		case TK_SUBTRACT:
			return 5;
		default:
			return 0;
	}
}

int find_main_operator(int p, int q) {
	//printf("find_main_operator called with p = %d, q = %d\n", p, q);
	int op = -1;
	int min_precedence = 5;
	int paren_cnt = 0;

	for (int i = p; i <= q; i++) {
		if (tokens[i].type == TK_LPAREN) {
		 	paren_cnt++;
		} else if (tokens[i].type == TK_RPAREN) {
		 	paren_cnt--;
			// Skip parentheses.
		}
		
		//printf("Checking token at index %d: type = %d, paren_cnt = %d\n", i, tokens[i].type, paren_cnt);

	 	if (paren_cnt == 0) {
			if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS ||
					tokens[i].type == TK_MUL || tokens[i].type == TK_DIV ||
					tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ ||
					tokens[i].type == TK_AND || tokens[i].type == TK_DEREF ||
					tokens[i].type == TK_SUBTRACT) {
				// Gain the minimum precedence of the operator.
				int precedence = get_precedence(tokens[i].type);
				//printf("Operator at index %d has precedence %d\n", i, precedence);
				if (precedence <= min_precedence) {
					min_precedence = precedence;
					op = i;
				}
			}
		}
	}
	//printf("Main operator found at index %d\n", op);
	return op;
}

uint32_t eval(int p, int q, bool* success) {
	*success = true;
	//printf("Call eval function with p = %d, q = %d\n", p, q);
	// success is set to error handling
	if (p > q) {
		/* Bad expression*/
		printf("Error: Bad expression (p>q).\n");
		*success = false;
		return 0;
	}
	else if (p == q) {
		Token *token = &tokens[p];
		switch (token->type) {
			case TK_NUMBER:	{
				char *end;
				uint32_t val = strtoul(token->str, &end, 10);
				if (*end != '\0') {
					printf("Invalid number: %s\n", token->str);
					*success = false;
					return 0;
				}
				*success = true;
				return val;
			}
											
			case TK_HEX: {
				char *end;
				uint32_t val = strtoul(token->str, &end, 16);
				if (*end != '\0') {
					printf("Invalid hexdecimal: %s\n", token->str);
					*success = false;
					return 0;
				}
				*success = true;
				return val;
			}

			case TK_REG: {
				const char *reg_name = token->str + 1; // Skip '$'.
				if (strcmp(reg_name, "pc") == 0) {
					*success = true;
					return (uint32_t)cpu.pc;
				}
				else {
					bool reg_success;
					uint32_t val = (uint32_t)isa_reg_str2val(reg_name, &reg_success);
					if (!reg_success) {
						printf("Invalid registr: %s\n", reg_name);
						*success = false;
						return 0;
					}
					*success = true;
					return val;
				}
			}

			default:
				printf("Unexpected single token: %d\n", token->type);
				*success = false;
				return 0;
		}
	}
	else if (check_parentheses(p, q)) {
		/* The expression is surrouded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p + 1, q - 1, success);
	}
	else {
		// Handle unary operators first.
		int op = find_main_operator(p, q);
		//printf("op: %s, token_type: %d\n", tokens[op].str, tokens[op].type);
		if (op != -1) {
			// give priority to handing unary operators.
			if (op == p) {
				bool unary_success = true;
				uint32_t val = eval(op + 1, q, &unary_success);
				//printf("Unary operation result: %u\n", val);
				if (!unary_success) {
					printf("Can't not calculate in the unary\n");
					*success = false;
					return 0;
				}
				if (tokens[op].type == TK_DEREF) {
					*success = true;
					return vaddr_read(val, 4);
				}
				if (tokens[op].type == TK_SUBTRACT){
					*success = true;
					return -val;
				}
			}
			else {
				bool success1, success2;
				int32_t val1 = eval(p, op - 1, &success1);
				int32_t val2 = eval(op + 1, q, &success2);
				//printf("Left sub-expression result: %u, Right sub-expression result: %u\n", val1, val2);
				if (!success1) {
					printf("Error evaluating left sub-expression (p = %d, op - 1 = %d)\n", p, op - 1);
				}
				if (!success2) {
					printf("Error evaluating right sub-expression (op + 1 = %d, q = %d)\n", op + 1, q);
				}
				if (!success1 || !success2) {
					*success = false;
					return 0;
				}

				switch (tokens[op].type) {
					case TK_PLUS:  return (uint32_t)(val1 + val2);
					case TK_MINUS: return (uint32_t)(val1 - val2);
					case TK_MUL:   return (uint32_t)(val1 * val2);
					case TK_DIV: {
						if (val2 == 0) {
							printf("Division by zero\n");
							*success = false;
							return 0;
						}
						return (uint32_t)(val1 / val2);
					}
					case TK_EQ:    return val1 == val2 ? 1 : 0;
					case TK_NEQ:   return val1 != val2 ? 1 : 0;
					case TK_AND:   return val1 && val2 ? 1 : 0;
					default:
						printf("Unkown operator: %d\n", tokens[op].type);
						*success = false;
						return 0;
				}
			}
		} else { // op == -1
			printf("Error: No main operator found between %d and %d\n", p, q);
			*success = false;
			return 0;
		}
	}
	// add a default return statement to ensure the function always a value
	*success = false;
	return 0;
}

void is_deref_or_minus(int index){
	bool is_deref_or_minus = false;
	if (index == 0) {
		is_deref_or_minus = true; 
	} else {
		int prev_type = tokens[index - 1].type;
		if (prev_type == TK_PLUS || prev_type == TK_MINUS ||
				prev_type == TK_MUL || prev_type == TK_DIV ||
				prev_type == TK_EQ || prev_type == TK_NEQ ||
				prev_type == TK_AND || prev_type == TK_LPAREN) {
			is_deref_or_minus = true;
		}
	}
	if (is_deref_or_minus && tokens[index].type == TK_MUL) {
		tokens[index].type = TK_DEREF;
	}
	if (is_deref_or_minus && tokens[index].type == TK_MINUS) {
		tokens[index].type = TK_SUBTRACT;
	}
}

uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        printf("Error: Lexical analysis failed.\n");
        *success = false;
        return 0;
    }
		for (int i = 0; i < nr_token; i++) {
			if (tokens[i].type == TK_MUL || tokens[i].type == TK_MINUS)
				is_deref_or_minus(i);
		}
    uint32_t result = eval(0, nr_token - 1, success);
    return result;
}
