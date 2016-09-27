/**
 * @file analyzer.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-19
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../common/instr.h"
#include "../common/v2_instr.h"
#include "imm_list.h"

static char *program_name = "analyzer";

static struct imm_list branch_list;

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-c] [-p] FILE\n", program_name);
	fprintf(stderr, "\t-c\tUse the compressed instruction format\n");
	fprintf(stderr, "\t-p\tConvert to pseudo instructions\n");
	exit(EXIT_FAILURE);
}

static uint16_t to_instr2(uint8_t bytes[2])
{
	uint16_t instr = bytes[0];
	instr = (instr << 8) | bytes[1];
	return instr;
}

static uint32_t to_instr4(uint8_t bytes[4])
{
	uint32_t instr = bytes[0];
	instr = (instr << 8) | bytes[1];
	instr = (instr << 8) | bytes[2];
	instr = (instr << 8) | bytes[3];
	return instr;
}

#define PRINT(instr) \
printf("%8s | %4i | %6.2f%% | %4i | %6.2f%% | %6.2f%%\n", \
	#instr, freq[instr], (100.0 * freq[instr])/total_instr, freq_comp[instr], \
	freq[instr] > 0 ? (100.0 * freq_comp[instr])/freq[instr] : 0, \
	(100.0 * freq_comp[instr]) / total_instr)

static void print_stat(int freq[NUM_INSTR], int freq_comp[NUM_INSTR], uint32_t total_instr, bool psd)
{
	printf("instr | freq | freq/total | comp | comp/freq | comp/total\n");
	PRINT(SLL);
	PRINT(SRL);
	PRINT(SRA);
	PRINT(SLLV);
	PRINT(SRLV);
	PRINT(SRAV);
	PRINT(ADD);
	PRINT(ADDU);
	PRINT(SUB);
	PRINT(SUBU);
	PRINT(AND);
	PRINT(OR);
	PRINT(XOR);
	PRINT(NOR);
	PRINT(ADDI);
	PRINT(ADDIU);
	PRINT(ANDI);
	PRINT(ORI);
	PRINT(XORI);

	PRINT(LUI);
	PRINT(LB);
	PRINT(LH);
	PRINT(LW);
	PRINT(LBU);
	PRINT(LHU);
	PRINT(SB);
	PRINT(SH);
	PRINT(SW);

	PRINT(SLT);
	PRINT(SLTU);
	PRINT(SLTI);
	PRINT(SLTIU);

	PRINT(BLTZ);
	PRINT(BGEZ);
	PRINT(BLTZAL);
	PRINT(BGEZAL);
	PRINT(BEQ);
	PRINT(BNE);
	PRINT(BLEZ);
	PRINT(BGTZ);
	PRINT(J);
	PRINT(JAL);
	PRINT(JR);
	PRINT(JALR);

	PRINT(MFC0);
	PRINT(MTC0);

	if (psd) {
		PRINT(NOP);
		PRINT(MOV);
		PRINT(CLEAR);
		PRINT(NOT);
		PRINT(NEG);
		PRINT(B);
		PRINT(BAL);
		PRINT(BEQZ);
		PRINT(BNEZ);
		PRINT(SEQZ);
		PRINT(SNEZ);
		PRINT(SLTZ);
		PRINT(LSI);
	}

	printf("Total instructions: %u\n", total_instr);
}

#undef PRINT

static bool is_compressible(struct instr *instr)
{
	assert(instr != NULL);
	//if (is_compressible_simple(instr) || is_compressible_branch(instr))
	if (is_compressible_simple(instr))
		return true;
	return false;
}

static void update_branch_stat(struct instr *instr)
{
	assert(instr != NULL);

	int16_t simm = instr->simm;

	switch(instr->op) {
	case BEQ:
	case BNE:
	case BLTZ:
	case BLTZAL:
	case BGEZ:
	case BGEZAL:
	case BEQZ:
	case BNEZ:
	case B:
	case BAL:
		imm_list_add_signed(&branch_list, simm);
	
	default:
		return;
	}
}

struct mem_stat {
	int16_t imm;
	unsigned int w_count;
	unsigned int h_count;
	unsigned int hu_count;
	unsigned int b_count;
	unsigned int bu_count;
};

static struct mem_stat *mem_stat = NULL;
static size_t num_mem_stat_entries = 0;

static void add_mem_op(enum operation op, int16_t simm)
{
	for (size_t i = 0; i < num_mem_stat_entries; i++) {
		if (mem_stat[i].imm != simm)
			continue;

		switch(op) {
		case SW:
		case LW:
			mem_stat[i].w_count++;
			break;

		case SH:
		case LH:
			mem_stat[i].h_count++;
			break;

		case LHU:
			mem_stat[i].hu_count++;
			break;

		case SB:
		case LB:
			mem_stat[i].b_count++;
			break;

		case LBU:
			mem_stat[i].bu_count++;
			break;

		default:
			assert(0);
		}
		return;
	}

	num_mem_stat_entries++;
	mem_stat = realloc(mem_stat, num_mem_stat_entries * sizeof(struct mem_stat));
	size_t i = num_mem_stat_entries - 1;
	memset(&mem_stat[i], 0, sizeof(struct mem_stat));
	mem_stat[i].imm = simm;

	switch(op) {
	case SW:
	case LW:
		mem_stat[i].w_count++;
		break;

	case SH:
	case LH:
		mem_stat[i].h_count++;
		break;

	case LHU:
		mem_stat[i].hu_count++;
		break;

	case SB:
	case LB:
		mem_stat[i].b_count++;
		break;

	case LBU:
		mem_stat[i].bu_count++;
		break;

	default:
		assert(0);
	}
}

static void update_mem_stat(struct instr *instr)
{
	assert(instr != NULL);

	int16_t simm = instr->simm;

	switch(instr->op) {
	case SW:
	case LW:
	case SH:
	case LH:
	case LHU:
	case SB:
	case LB:
	case LBU:
		if (instr->rs != 29)
			return;
		add_mem_op(instr->op, simm);
	
	default:
		return;
	}
}

static int cmp_mem_stat(const void *left, const void *right)
{
	if (abs(((struct mem_stat *)left)->imm) < 
		abs(((struct mem_stat *)right)->imm))
		return -1;
	return 1;
}


static void analyze2(FILE *in, bool psd, bool v2)
{
	uint8_t bytes[4];
	int freq[NUM_INSTR] = {0};
	int freq_comp[NUM_INSTR] = {0};
	struct instr instr;

	uint32_t total_instr = 0;

	imm_list_init(&branch_list);

	if (v2) {
		while(fread(bytes, 2, 1, in) == 1) {
			uint32_t code = to_instr2(bytes);
			code <<= 16;

			if (code < 0x80000000) {
				/* long instruction */
				fread(bytes, 2, 1, in);
				code |= to_instr2(bytes);
			}

			total_instr++;
			memset(&instr, 0, sizeof(instr));
			parse_instr_v2(code, &instr);

			if (psd)
				conv_to_pseudo(&instr);

			if (instr.op == INVALID_OP) {
				fprintf(stderr, "Invalid instruction at %u\n", total_instr * 4 - 4);
				continue;
			}

			freq[instr.op]++;
			if (is_compressible(&instr))
				freq_comp[instr.op]++;

			update_branch_stat(&instr);
			update_mem_stat(&instr);
		}
	} else {
		while(fread(bytes, sizeof(bytes), 1, in) == 1) {
			uint32_t code = to_instr4(bytes);
			total_instr++;
			parse_instr(code, &instr);
			if (psd)
				conv_to_pseudo(&instr);

			if (instr.op == INVALID_OP) {
				fprintf(stderr, "Invalid instruction at %u\n", total_instr * 4 - 4);
				continue;
			}

			freq[instr.op]++;
			if (is_compressible(&instr))
				freq_comp[instr.op]++;

			update_branch_stat(&instr);
			update_mem_stat(&instr);
		}
	}

	print_stat(freq, freq_comp, total_instr, psd);
	uint32_t num_comp = 0;
	for (int i = 0; i < NUM_INSTR; i++) 
		num_comp += freq_comp[i];
	
	uint32_t num_uncomp = total_instr - num_comp;
	uint32_t uncomp_size = total_instr * 4;
	uint32_t comp_size = num_comp * 2 + num_uncomp * 4;
	printf("Num small instructions: %u (%5.2f%%)\n", num_comp, (100.0 * num_comp) / total_instr);
	printf("Num big instructions: %u (%5.2f%%)\n", num_uncomp, (100.0 * num_uncomp) / total_instr);
	printf("Uncompressed size %u bytes\n", uncomp_size);
	printf("Estimated comp size: %u bytes\n", comp_size);
	printf("Estimated comp ratio: %5.2f%%\n", (100.0 * comp_size) / uncomp_size);

	printf("All branch distances:\n");
	imm_list_sort_signed(&branch_list);
	struct imm_entry *branch_entries = branch_list.entries;
	for (size_t i = 0; i < branch_list.num_entries; i++) {
		printf("%3u x %5i\n", branch_entries[i].num, branch_entries[i].simm);
	}

	qsort(mem_stat, num_mem_stat_entries, sizeof(struct mem_stat), cmp_mem_stat);
	printf("All stack mem_op offsets:\n");
	printf("offset   W   H  HU   B  BU\n");
	for (size_t i = 0; i < num_mem_stat_entries; i++) {
		printf("%5i: %3u %3u %3u %3u %3u\n", mem_stat[i].imm, mem_stat[i].w_count,
			mem_stat[i].h_count, mem_stat[i].hu_count, mem_stat[i].b_count,
			mem_stat[i].bu_count);
	}
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];

	bool v2 = false;
	bool pseudo = false;

	int opt = 0;

	while ((opt = getopt(argc, argv, "cp")) != -1) {
		switch (opt) {
		case 'c':
			v2 = true;
			break;

		case 'p':
			pseudo = true;
			break;

		case '?':
		default:
			usage();
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "missing binary file\n");
		usage();
	}
	
	FILE *file = fopen(argv[optind], "rb");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open file '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	/*printf("Without pseudo instructions:\n");
	analyze2(file, false);
	rewind(file);
	printf("With pseudo instructions:\n");*/
	analyze2(file, pseudo, v2);

	fclose(file);

	return 0;
}

