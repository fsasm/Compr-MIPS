/**
 * @file calc.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-07-29
 * A simple calculator that reads mathematical integer terms and prints the result.
 */

#include <stdint.h>
#include <stdio.h>
#include "mul.h"
#include "div.h"
#include "fix16.h"
#include <stdbool.h>

enum operation {
	OP_NUMBER,
	OP_PLUS,
	OP_MINUS,
	OP_MULT,
	OP_DIV,
	OP_LPAR,
	OP_RPAR,
	OP_END,
	OP_INVALID
};

struct token {
	enum operation op;
	//int32_t number;
	fix16_t number;
};

static struct token parse_token(void)
{
	do {
		char c = getchar();
	
		switch(c) {
		case '-':
			return (struct token) {OP_MINUS, 0};
	
		case '+':
			return (struct token) {OP_PLUS, 0};

		case '*':
			return (struct token) {OP_MULT, 0};
	
		case '/':
			return (struct token) {OP_DIV, 0};
	
		case '\n':
		case '\r':
			return (struct token) {OP_END, 0};

		case '(':
			return (struct token) {OP_LPAR, 0};

		case ')':
			return (struct token) {OP_RPAR, 0};
	
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			/*
			number = c - '0';
			do {
				c = getchar();
				if (c < '0' || c > '9') {
					ungetchar(c);
					return (struct token) {OP_NUMBER, number};
				}
				number = (int32_t)mul32(number, 10) + (c - '0');
			} while (true);
			*/
			ungetchar(c);
			return (struct token) {OP_NUMBER, fix_parse()};
			break;
	
		case ' ':
			/* read until a token can be parsed */
			break;

		/* EOF */
		case '\0':
		case -1:
			return (struct token) {OP_INVALID, 0};
	
		default:
			puts("Invalid token");
			return (struct token) {OP_INVALID, 0};
		}
	} while (true);
}

#define STACK_SIZE (255)
#define QUEUE_SIZE (255)

static struct token stack[STACK_SIZE];
static struct token queue[QUEUE_SIZE];

static uint8_t num_stack   = 0;
static uint8_t queue_start = 0;
static uint8_t queue_size  = 0;

void print_token(struct token t)
{
	switch(t.op) {
	case OP_NUMBER:
		puts("NUMBER ");
		fix_print(t.number);
		putchar('\n');
		break;
	
	case OP_PLUS:
		puts("PLUS");
		break;
	
	case OP_MINUS:
		puts("MINUS");
		break;
	
	case OP_MULT:
		puts("MULT");
		break;
	
	case OP_DIV:
		puts("DIV");
		break;

	case OP_LPAR:
		puts("LPAR");
		break;
	
	case OP_RPAR:
		puts("RPAR");
		break;
	
	case OP_END:
		puts("END");
		break;
	
	case OP_INVALID:
		puts("INVALID");
		break;
	}
}

struct token pop_stack(void)
{
	if (num_stack == 0) {
		#ifdef DEBUG
		printf("pop_stack: INVALID\n");
		#endif
		return (struct token) {OP_INVALID, 0};
	}

	num_stack--;

	#ifdef DEBUG
	printf("pop_stack: ");
	print_token(stack[num_stack]);
	#endif

	return stack[num_stack];
}

void push_stack(struct token t)
{
	if (num_stack == STACK_SIZE) {
		puts("Stack already full");
		return;
	}

	#ifdef DEBUG
	printf("push_stack: ");
	print_token(t);
	#endif

	stack[num_stack] = t;
	num_stack++;
}

void clear_stack(void)
{
	num_stack = 0;
}

struct token pop_queue(void)
{
	if (queue_size == 0) {
		#ifdef DEBUG
		printf("pop_queue: INVALID\n");
		#endif
		return (struct token) {OP_INVALID, 0};
	}

	struct token t = queue[queue_start];

	#ifdef DEBUG
	printf("pop_queue: ");
	print_token(t);
	#endif 

	queue_start++;
	queue_size--;
	return t;
}

void push_queue(struct token t)
{
	if (queue_size == QUEUE_SIZE) {
		puts("Queue already full");
		return;
	}

	#ifdef DEBUG
	printf("push_queue: ");
	print_token(t);
	#endif

	uint16_t index = (uint16_t)queue_start + (uint16_t)queue_size;
	if (index >= QUEUE_SIZE) {
		index -= QUEUE_SIZE;
	}

	queue[index] = t;
	queue_size++;
}

void clear_queue(void)
{
	queue_size = 0;
	queue_start = 0;
}

static bool is_operator(enum operation op)
{
	switch(op) {
	case OP_PLUS:
	case OP_MINUS:
	case OP_MULT:
	case OP_DIV:
		return true;

	case OP_NUMBER:
	case OP_LPAR:
	case OP_RPAR:
	case OP_END:
	case OP_INVALID:
		return false;
	}
	return false;
}

static bool is_left_assoc(enum operation op)
{
	(void)op;
	return true;
}

static int get_precedence(enum operation op)
{
	if (op == OP_PLUS || op == OP_MINUS)
		return 1;

	if (op == OP_MULT || op == OP_DIV)
		return 2;

	return 0;
}

bool parse_term(void)
{
	struct token t;
	do {
		t = parse_token();

		if (t.op == OP_NUMBER) {
			push_queue(t);
			continue;
		}

		if (t.op == OP_LPAR) {
			push_stack(t);
			continue;
		}

		if (t.op == OP_RPAR) {
			do {
				struct token t2 = pop_stack();

				if (t2.op == OP_LPAR) {
					break;
				}

				if (t2.op == OP_INVALID) {
					puts("Mismatched parentheses");
					return false;
				}

				push_queue(t2);
			} while(true);
			continue;
		}

		if (is_operator(t.op)) {
			int op1_prec = get_precedence(t.op);

			do {
				struct token t2 = pop_stack();
				if (t2.op == OP_INVALID) {
					break;
				}

				if (!is_operator(t2.op)) {
					push_stack(t2);
					break;
				}

				if (is_left_assoc(t.op)) {
					if (op1_prec <= get_precedence(t2.op)) {
						push_queue(t2);
					} else {
						push_stack(t2);
						break;
					}
				} else {
					if (op1_prec < get_precedence(t2.op)) {
						push_queue(t2);
					} else {
						push_stack(t2);
						break;
					}
				}
			} while(true);
			push_stack(t);
		}
	} while(t.op != OP_INVALID && t.op != OP_END);

	if (t.op == OP_INVALID)
		return false;

	do {
		t = pop_stack();
		push_queue(t);
	} while(t.op != OP_INVALID && t.op != OP_END);
	return true;
}

void calc_term(void)
{
	struct token t;
	do {
		t = pop_queue();

		if (t.op == OP_INVALID) {
			break;
		}

		if (t.op == OP_NUMBER) {
			push_stack(t);
			continue;
		}

		struct token right = pop_stack();
		struct token left  = pop_stack();

		if (left.op != OP_NUMBER || right.op != OP_NUMBER) {
			puts("Numbers expected");
			return;
		}

		if (t.op == OP_PLUS) {
			//push_stack((struct token) {OP_NUMBER, left.number + right.number});
			push_stack((struct token) {OP_NUMBER, fix_add(left.number, right.number)});
		} else if (t.op == OP_MINUS) {
			//push_stack((struct token) {OP_NUMBER, left.number - right.number});
			push_stack((struct token) {OP_NUMBER, fix_sub(left.number, right.number)});
		} else if (t.op == OP_MULT) {
			//push_stack((struct token) {OP_NUMBER, mul32(left.number, right.number)});
			push_stack((struct token) {OP_NUMBER, fix_mul(left.number, right.number)});
		} else if (t.op == OP_DIV) {
			//push_stack((struct token) {OP_NUMBER, div32(left.number, right.number).quot});
			push_stack((struct token) {OP_NUMBER, fix_div(left.number, right.number)});
		} else {
			puts("Invalid token");
			return;
		}
	} while(t.op != OP_INVALID && t.op != OP_END);

	t = pop_stack();

	if (t.op != OP_NUMBER) {
		puts("Result expected");
	} else {
		fix_print(t.number);
		putchar('\n');
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	while (true) {
		if (!parse_term()) {
			return 0;
		}
		calc_term();
		clear_stack();
		clear_queue();
	}

	return 0;
}

