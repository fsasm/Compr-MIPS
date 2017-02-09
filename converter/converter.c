/**
 * @file converter.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-25
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "../common/instr.h"
#include "../common/v2_instr.h"
#include "../common/print_instr.h"

/* ssize_t is from POSIX and not available here */
typedef long ssize_t;

static char *program_name = "converter";

static void parse_bin(FILE *f);

static void usage(void)
{
	fprintf(stderr, "Usage: %s IN-FILE OUT-FILE\n", program_name);
	exit(EXIT_FAILURE);
}

static uint32_t to_instr(uint8_t bytes[4])
{
	uint32_t instr = bytes[0];
	instr = (instr << 8) | bytes[1];
	instr = (instr << 8) | bytes[2];
	instr = (instr << 8) | bytes[3];
	return instr;
}

struct instr_attr {
	uint32_t new_addr;
	uint32_t jump_target;
	ssize_t target_index; /* array index to the target instruction */
};

size_t num_instr = 0;
struct instr *prog = NULL;
struct instr_attr *attr = NULL;

static void parse_bin(FILE *in)
{
	uint8_t bytes[4];
	struct instr instr;

	uint32_t new_addr = 0;

	while(fread(bytes, sizeof(bytes), 1, in) == 1) {
		uint32_t code = to_instr(bytes);
		parse_instr(code, &instr);
		conv_to_pseudo(&instr);

		if (instr.op == INVALID_OP) {
			fprintf(stderr, "Invalid instruction at %lu\n", num_instr * 4);
			exit(EXIT_FAILURE);
		}
		
		num_instr++;
		prog = realloc(prog, num_instr * sizeof(*prog));

		if (prog == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}

		attr = realloc(attr, num_instr * sizeof(*attr));

		if (attr == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}

		instr.compressed = is_compressible_simple(&instr);

		prog[num_instr - 1] = instr;
		attr[num_instr - 1] = (struct instr_attr) {
			.new_addr = new_addr,
			.jump_target = 0,
			.target_index = -1
		};

		if (instr.op == J || instr.op == JAL) {
			attr[num_instr - 1].jump_target = instr.addr;
			attr[num_instr - 1].target_index = instr.addr / 4;
		}

		if (is_branch(instr.op)) {
			attr[num_instr - 1].target_index = num_instr + (instr.simm / 4);
		}

		new_addr += instr.compressed ? 2 : 4;
	}
}

static uint32_t correct_branch_offsets(size_t num, struct instr prog[num], struct instr_attr attr[num])
{
	uint32_t num_mod_instr = 0;

	for (size_t i = 0; i < num_instr; i++) {
		if (!is_branch(prog[i].op)) {
			continue;
		}

		ssize_t target = attr[i].target_index;
		assert(0 <= target && target < (ssize_t)num_instr);
		int32_t simm = attr[target].new_addr - attr[i + 1].new_addr;

		if (prog[i].simm != simm) {
			num_mod_instr++;
		}

		prog[i].simm = simm;
	}

	return num_mod_instr;
}

static uint32_t correct_jump_targets(size_t num, struct instr prog[num], struct instr_attr attr[num])
{
	uint32_t num_mod_instr = 0;

	for (size_t i = 0; i < num_instr; i++) {
		if (prog[i].op != J && prog[i].op != JAL) {
			continue;
		}

		ssize_t target = attr[i].target_index;
		assert(0 <= target && target < (ssize_t)num_instr);
		if (attr[i].jump_target != attr[target].new_addr) {
			num_mod_instr++;
		}

		attr[i].jump_target = attr[target].new_addr;
	}

	return num_mod_instr;
}

static uint32_t calc_new_addr(size_t num, struct instr prog[num], struct instr_attr attr[num])
{
	uint32_t new_addr = 0;
	uint32_t num_mod_instr = 0;

	for (size_t i = 0; i < num_instr; i++) {
		if (attr[i].new_addr != new_addr) {
			num_mod_instr++;
		}

		attr[i].new_addr = new_addr;

		new_addr += prog[i].compressed ? 2 : 4;
	}

	return num_mod_instr;
}

static uint32_t compress_branch(size_t num, struct instr prog[num], struct instr_attr attr[num])
{
	uint32_t num_mod_instr = 0;

	correct_branch_offsets(num, prog, attr);

	for (size_t i = 0; i < num_instr; i++) {
		if (!is_branch(prog[i].op)) {
			continue;
		}

		bool compressible = prog[i].compressed;
		if (prog[i].op == B || prog[i].op == BAL) {
			/* B and BAL have the range [-1024; 1022] */
			compressible = (-1024 <= prog[i].simm) && (prog[i].simm <= 1022);
		} else if (prog[i].op == BEQZ || prog[i].op == BNEZ) {
			/* BEQZ and BNEZ have the range [-32; 30] */
			compressible = (-32 <= prog[i].simm) && (prog[i].simm <= 30);
		}

		if (compressible != prog[i].compressed) {
			num_mod_instr++;
		}

		prog[i].compressed = compressible;
	}

	calc_new_addr(num, prog, attr);
	correct_branch_offsets(num, prog, attr);
	return num_mod_instr;
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];
	
	if (argc != 3)
		usage();
	
	FILE *file = fopen(argv[1], "rb");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open file '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	parse_bin(file);
	fclose(file);
	file = NULL;

	FILE *out_file = fopen(argv[2], "wb");

	if (out_file == NULL) {
		fprintf(stderr, "Couldn't open file '%s'\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	correct_branch_offsets(num_instr, prog, attr);

	/* correct jump targets */
	// TODO convert J and JAL to B/BAL if the user requests per option
	for (size_t i = 0; i < num_instr; i++) {
		if (prog[i].op != J && prog[i].op != JAL) {
			continue;
		}

		ssize_t target = attr[i].target_index;
		assert(0 <= target && target < (ssize_t)num_instr);

		int32_t simm = attr[target].new_addr - attr[i + 1].new_addr;

		if (-1024 <= simm && simm <= 1022) {
			/* convert to B or BAL */
			prog[i].simm = simm;
			if (prog[i].op == J) {
				prog[i].op = B;
			} else {
				prog[i].op = BAL;
			}
		}

		attr[i].jump_target = attr[target].new_addr;
	}
	

	uint32_t num_mod_instr = 0;
	do {
		num_mod_instr = compress_branch(num_instr, prog, attr);
	} while (num_mod_instr != 0);

	correct_jump_targets(num_instr, prog, attr);

	for (size_t i = 0; i < num_instr; i++) {
		if (prog[i].op == J || prog[i].op == JAL) {
			prog[i].addr = attr[i].jump_target;
		}

		uint32_t instr;
		int rc = write_instr_v2(&prog[i], &instr);

		if (rc == 2) {
			uint16_t tmp = instr & 0xFFFF;
			uint8_t byte = tmp >> 8;
			fwrite(&byte, 1, 1, out_file);
			byte = tmp & 0xFF;
			fwrite(&byte, 1, 1, out_file);
		} else if (rc == 4) {
			uint8_t byte = instr >> 24;
			fwrite(&byte, 1, 1, out_file);
			byte = instr >> 16;
			fwrite(&byte, 1, 1, out_file);
			byte = instr >> 8;
			fwrite(&byte, 1, 1, out_file);
			byte = instr & 0xFF;
			fwrite(&byte, 1, 1, out_file);
		} else {
			fprintf(stderr, "Error while writing output file\n");
			return -1;
		}
	}

	fclose(out_file);

	free(prog);
	free(attr);

	return 0;
}

