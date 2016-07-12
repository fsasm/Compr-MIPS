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

#include "../common/instr.h"

static char *program_name = "analyzer";

static void usage(void)
{
	fprintf(stderr, "Usage: %s FILE\n", program_name);
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
	}

	printf("Total instructions: %u\n", total_instr);
}

#undef PRINT

static bool is_compressible(struct instr *instr)
{
	assert(instr != NULL);

	uint8_t rs = instr->rs;
	uint8_t rt = instr->rt;
	uint8_t rd = instr->rd;
	uint16_t imm = instr->imm;
	int16_t simm = instr->simm;

	switch(instr->op) {
	case NOP:
	case MOV:
	case CLEAR:
	case NOT:
	case NEG:
	case JR:
	case JALR:
		return true;

	case SLL:
	case SRL:
	case SRA:
	case SLLV:
	case SRLV:
	case SRAV:
		if (rd == rt)
			return true;
		return false;

	case ADD:
	case ADDU:
	case AND:
	case OR:
	case XOR:
	case NOR:
	case SLT:
	case SLTU:
		if (rd == rs || rd == rt)
			return true;
		return false;
	
	case SUB:
	case SUBU:
		if (rd == rs)
			return true;
		return false;

	case ADDI:
	case ADDIU:
	case ANDI:
	case ORI:
	case XORI:
	case SLTI:
	case SLTIU:
		if (rs == rt && -16 <= simm && simm <= 15)
			return true;
		return false;

	case LUI:
	case BLTZ:
	case BLTZAL:
	case BGEZ:
	case BGEZAL:
	case BEQZ:
	case BNEZ:
		if (-16 <= simm && simm <= 15)
			return true;
		return false;
	
	case B:
	case BAL:
		/* 10 bit immediate */
		if (-512 <= simm && simm <= 511)
			return true;
		return false;
	
	case SW:
	case LW:
		/* Stack pointer is $29 */
		if (rs == 29 && imm % 4 == 0 && imm <= 128)
			return true;
		return false;
			
	
	default:
		return false;
	}
}

static void analyze2(FILE *in, bool psd)
{
	uint8_t bytes[4];
	int freq[NUM_INSTR] = {0};
	int freq_comp[NUM_INSTR] = {0};
	struct instr instr;

	uint32_t total_instr = 0;

	while(fread(bytes, sizeof(bytes), 1, in) == 1) {
		uint32_t code = to_instr(bytes);
		total_instr++;
		parse_instr(code, &instr);
		if (psd)
			conv_pseudo(&instr);

		if (instr.op == INVALID_OP) {
			fprintf(stderr, "Invalid instruction at %u\n", total_instr * 4 - 4);
			continue;
		}

		freq[instr.op]++;
		if (is_compressible(&instr))
			freq_comp[instr.op]++;
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
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];
	
	if (argc != 2)
		usage();
	
	FILE *file = fopen(argv[1], "rb");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open file '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	/*printf("Without pseudo instructions:\n");
	analyze2(file, false);
	rewind(file);
	printf("With pseudo instructions:\n");*/
	analyze2(file, true);

	fclose(file);

	return 0;
}

