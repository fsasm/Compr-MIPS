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

#define CONV_PSEUDO  (0x01)
#define COMPRESSED   (0x02)
#define BRANCH_STAT  (0x04)
#define STACK_STAT   (0x08)
#define IMM_STAT     (0x10)
#define NOP_DEL_STAT (0x20)
#define REG_STAT     (0x40)

static char *program_name = "analyzer";

static struct imm_list branch_list;
static struct imm_list uimm_list;
static struct imm_list simm_list;
uint32_t rs_count[32] = {0};
uint32_t rd_count[32] = {0};

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-cpbmidr] FILE\n", program_name);
	fprintf(stderr, "\t-c\tUse the compressed instruction format\n");
	fprintf(stderr, "\t-p\tConvert to pseudo instructions\n");
	fprintf(stderr, "\t-b\tShow statistics about branch offsets\n");
	fprintf(stderr, "\t-m\tShow statistics about stack offsets\n");
	fprintf(stderr, "\t-i\tShow statistics about immediates\n");
	fprintf(stderr, "\t-d\tShow statistics about NOPs in delay slots\n");
	fprintf(stderr, "\t-r\tShow statistics about used registers\n");
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
printf("%6s | %4i |    %6.2f%% | %4i |   %6.2f%% | %6.2f%%\n", \
	#instr, freq[instr], (100.0 * freq[instr])/total_instr, freq_comp[instr], \
	freq[instr] > 0 ? (100.0 * freq_comp[instr])/freq[instr] : 0, \
	(100.0 * freq_comp[instr]) / total_instr)

static void print_stat(int freq[NUM_INSTR], int freq_comp[NUM_INSTR], uint32_t total_instr, bool psd)
{
	printf(" instr | freq | freq/total | comp | comp/freq | comp/total\n");
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

	if (instr->op == B || instr->op == BAL) {
		if (-1024 <= instr->simm && instr->simm <= 1022) {
			return true;
		}
	} else if (instr->op == BEQZ || instr->op == BNEZ) {
		if (-32 <= instr->simm && instr->simm <= 30) {
			return true;
		}
	}
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

static void update_imm_stat(struct instr *instr)
{
	assert(instr != NULL);

	if (contains_imm(instr->op)) {
		imm_list_add_unsigned(&uimm_list, instr->imm);
	}

	if (contains_simm(instr->op)) {
		imm_list_add_signed(&simm_list, instr->simm);
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

static uint32_t num_branch_nops = 0;
static uint32_t num_jump_nops = 0;
static uint32_t num_load_nops = 0;

static void update_nop_stat(struct instr *instr)
{
	assert(instr != NULL);

	static bool prev_branch = false;
	static bool prev_jump = false;
	static bool prev_load = false;

	assert(!(prev_branch && prev_jump));

	struct instr temp = *instr;
	conv_to_pseudo(&temp);

	if (prev_branch) {
		prev_branch = false;

		if (temp.op == NOP) {
			num_branch_nops++;
		}
	}

	if (prev_jump) {
		prev_jump = false;

		if (temp.op == NOP) {
			num_jump_nops++;
		}
	}

	if (prev_load) {
		prev_load = false;

		if (temp.op == NOP && !(prev_branch || prev_jump)) {
			num_load_nops++;
		}
	}

	switch (instr->op) {
	case BLTZ:
	case BGEZ:
	case BLTZAL:
	case BGEZAL:
	case BEQ:
	case BNE:
	case BLEZ:
	case BGTZ:
	case B:
	case BAL: 
	case BEQZ: /* rs */
	case BNEZ: /* rs */
		prev_branch = true;
		return;

	case J:
	case JAL:
	case JR:
	case JALR:
		prev_jump = true;
		return;

	case LB:
	case LH:
	case LW:
	case LBU:
	case LHU:
		prev_load = true;
		return;

	default:
		break;
	}

}

static void update_reg_stat(struct instr *instr)
{
	assert(instr != NULL);

	switch(instr->op) {
	case SLL:
	case SRL:
	case SRA:
		rs_count[instr->rt]++;
		rd_count[instr->rd]++;
		break;

	case SLLV:
	case SRLV:
	case SRAV:
	case ADD:
	case ADDU:
	case SUB:
	case SUBU:
	case AND:
	case OR:
	case XOR:
	case NOR:
	case SLT:
	case SLTU:
		rs_count[instr->rs]++;
		rs_count[instr->rt]++;
		rd_count[instr->rd]++;
		break;

	case ADDI:
	case ADDIU:
	case ANDI:
	case ORI:
	case XORI:
	case LB:
	case LH:
	case LW:
	case LBU:
	case LHU:
	case SLTI:
	case SLTIU:
		rs_count[instr->rs]++;
		rd_count[instr->rt]++;
		break;

	case LUI:
		rd_count[instr->rt]++;
		break;

	case SB:
	case SH:
	case SW:
	case BEQ:
	case BNE:
		rs_count[instr->rs]++;
		rs_count[instr->rt]++;
		break;


	case BLTZ:
	case BGEZ:
	case BLEZ:
	case BGTZ:
	case JR:
		rs_count[instr->rs]++;
		break;

	case BLTZAL:
	case BGEZAL:
		rs_count[instr->rs]++;
		rd_count[31]++;
		break;


	case JAL:
	case BAL: 
		rd_count[31]++;
		break;

	case JALR:
		rs_count[instr->rs]++;
		rd_count[instr->rd]++;
		break;

	case MFC0:
	case MTC0:
	case J:
	case NOP:
	case B:
		break;

	case MOV: /* rd = rt */
	case NOT: /* rd = ~rt */
	case NEG: /* rd = -rt */
		rs_count[instr->rt]++;
		rd_count[instr->rd]++;
		break;

	case CLEAR: /* rd = 0 */
		rd_count[instr->rd]++;
		break;

	case BEQZ: /* rs */
	case BNEZ: /* rs */
		rs_count[instr->rs]++;
		break;

	case SEQZ: /* rd = rt == 0 */
		rs_count[instr->rs]++;
		rd_count[instr->rt]++;
		break;

	case SNEZ: /* rd = rt != 0 */
		rs_count[instr->rt]++;
		rd_count[instr->rd]++;
		break;

	case SLTZ: /* rd = rt <  0 */
		rs_count[instr->rs]++;
		rd_count[instr->rt]++;
		break;

	case LSI: /* load small immediate */
		rd_count[instr->rd]++;
		break;
	
	default:
		return;
	}
}


static void analyze2(FILE *in, uint32_t flags)
{
	uint8_t bytes[4];
	int freq[NUM_INSTR] = {0};
	int freq_comp[NUM_INSTR] = {0};
	struct instr instr;

	bool psd           = (flags & CONV_PSEUDO)  != 0;
	bool v2            = (flags & COMPRESSED)   != 0;
	bool branch_stat   = (flags & BRANCH_STAT)  != 0;
	bool show_mem_stat = (flags & STACK_STAT)   != 0;
	bool imm_stat      = (flags & IMM_STAT)     != 0;
	bool nop_stat      = (flags & NOP_DEL_STAT) != 0;
	bool reg_stat      = (flags & REG_STAT)     != 0;

	uint32_t total_instr = 0;

	imm_list_init(&branch_list);
	imm_list_init(&uimm_list);
	imm_list_init(&simm_list);

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
			update_imm_stat(&instr);
			update_nop_stat(&instr);
			update_reg_stat(&instr);
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
			update_imm_stat(&instr);
			update_nop_stat(&instr);
			update_reg_stat(&instr);
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

	if (branch_stat) {
		printf("All branch distances:\n");
		imm_list_sort_signed(&branch_list);
		struct imm_entry *branch_entries = branch_list.entries;
		for (size_t i = 0; i < branch_list.num_entries; i++) {
			printf("%3u x %5i\n", branch_entries[i].num, branch_entries[i].simm);
		}
	}

	if (show_mem_stat) {
		qsort(mem_stat, num_mem_stat_entries, sizeof(struct mem_stat), cmp_mem_stat);
		printf("All stack mem_op offsets:\n");
		printf("offset   W   H  HU   B  BU\n");
		for (size_t i = 0; i < num_mem_stat_entries; i++) {
			printf("%5i: %3u %3u %3u %3u %3u\n", mem_stat[i].imm, mem_stat[i].w_count,
				mem_stat[i].h_count, mem_stat[i].hu_count, mem_stat[i].b_count,
				mem_stat[i].bu_count);
		}
	}

	if (imm_stat) {
		imm_list_sort_unsigned(&uimm_list);
		imm_list_sort_signed(&simm_list);

		printf("All immediates:\n");
		printf("imm   |  U    S | total\n");

		struct imm_entry *uimm_entries = uimm_list.entries;
		struct imm_entry *simm_entries = simm_list.entries;

		/*
		for (size_t i = 0; i < uimm_list.num_entries; i++) {
			printf("%5i | %u %u | \n", uimm_entries[i].imm, uimm_entries[i].num, 0);
		}

		for (size_t i = 0; i < simm_list.num_entries; i++) {
			printf("%5i | %u %u | \n", simm_entries[i].simm, 0, simm_entries[i].num);
		}
		*/

		size_t u_iter = 0;
		size_t s_iter = 0;

		do {
			if (u_iter < uimm_list.num_entries && s_iter == simm_list.num_entries) {
				uint32_t times = uimm_entries[u_iter].num;
				int32_t uimm = uimm_entries[u_iter].imm;

				printf("%5i | %3u   0 | %3u\n", uimm, times, times);

				u_iter++;
			} else if (u_iter == uimm_list.num_entries && s_iter < simm_list.num_entries) {
				uint32_t times = simm_entries[s_iter].num;
				int32_t simm = simm_entries[s_iter].simm;

				printf("%5i |   0 %3u | %3u\n", simm, times, times);

				s_iter++;
			} else {
				int32_t uimm = uimm_entries[u_iter].imm;
				int32_t simm = simm_entries[s_iter].simm;

				uint32_t stimes = simm_entries[s_iter].num;
				uint32_t utimes = uimm_entries[u_iter].num;
				
				if (uimm < simm) {
					printf("%5i | %3u   0 | %3u\n", uimm, utimes, utimes);
					u_iter++;
				} else if (simm < uimm) {
					printf("%5i |   0 %3u | %3u\n", simm, stimes, stimes);
					s_iter++;
				} else {
					assert(simm == uimm);
					printf("%5i | %3u %3u | %3u\n", simm, utimes, stimes, utimes + stimes);
					s_iter++;
					u_iter++;
				}
			}

		} while (u_iter < uimm_list.num_entries || s_iter < simm_list.num_entries);
	}

	if (nop_stat) {
		printf("%u NOPs in Branch delay slots\n", num_branch_nops);
		printf("%u NOPs in Jump delay slots\n", num_jump_nops);
		printf("%u NOPs in Load delay slots\n", num_load_nops);
		printf("Total %u NOPs in delay slots\n", num_branch_nops + num_jump_nops + num_load_nops);
	}

	if (reg_stat) {
		
		printf("reg |  src |  dst\n");
		for (uint32_t i = 0; i < 32; i++) {
			printf("r%-2u | %4u | %4u\n", i, rs_count[i], rd_count[i]);
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];

	uint32_t flags = 0;

	int opt = 0;

	while ((opt = getopt(argc, argv, "mcbpird")) != -1) {
		switch (opt) {
		case 'c':
			flags |= COMPRESSED;
			break;

		case 'b':
			flags |= BRANCH_STAT;
			break;

		case 'm':
			flags |= STACK_STAT;
			break;

		case 'p':
			flags |= CONV_PSEUDO;
			break;

		case 'i':
			flags |= IMM_STAT;
			break;

		case 'r':
			flags |= REG_STAT;
			break;

		case 'd':
			flags |= NOP_DEL_STAT;
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
	analyze2(file, flags); 

	fclose(file);

	return 0;
}

