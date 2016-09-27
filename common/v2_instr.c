/**
 * @file v2_instr.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-09-26
 */

#include "../common/v2_instr.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* 5 bit opcodes for the small instructions (without the high bit set) */
#define C_MOV   (0x00)
#define C_ADDU  (0x01)
#define C_SUBU  (0x02)
#define C_OR    (0x03)
#define C_XOR   (0x04)
#define C_NEG   (0x05)
#define C_NOT   (0x06)
#define C_SLTU  (0x07)
#define C_ADDIU (0x08)
#define C_ANDI  (0x09)
#define C_SLL   (0x0A)
#define C_SRL   (0x0B)
#define C_SRA   (0x0C)
#define C_LSI   (0x0D)
#define C_B     (0x0E)
#define C_BAL   (0x0F)
#define C_BEQZ  (0x10)
#define C_BNEZ  (0x11)
#define C_JALR  (0x12)
#define C_LWS   (0x13)
#define C_SWS   (0x14)

int parse_instr_v2(uint32_t instr, struct instr *out)
{
	/* converts back to native instructions, to minimize the changes in the simulator */

	/* small instructions are always in the top 16 bits */
	if ((instr & 0x80000000) == 0) {
		/* large instructions */
		/* The only instructions that have the highest opcode bit set are load
		 * and store instructions. Using the second highest opcode bit for load
		 * and store instructions only collides with Coprocessor instructions.
		 * All other opcodes stay the same.
		 */
		uint8_t opcode = instr >> 26;
		out->rs = (instr >> 21) & 0x1F;
		out->rt = (instr >> 15) & 0x1F;
		out->compressed = false;

		uint16_t imm = instr & 0xFFFF;
		int16_t simm;
		if (imm > 0x7FFF) {
			imm = (~imm) + 1;
			simm = -imm;
		} else {
			simm = imm;
		}
		out->simm = simm;
		out->addr = instr & 0x3FFFFFF;

		assert(opcode < 0x20);

		switch(opcode) {
		case 0x01:
			out->simm *= 2;
			switch (out->rt) {
			case 0x00: /* BLTZ */
				out->op = BLTZ;
				break;
		
			case 0x01: /* BGEZ */
				out->op = BGEZ;
				break;
		
			case 0x10: /* BLTZAL */
				out->op = BLTZAL;
				break;
		
			case 0x11: /* BGEZAL */
				out->op = BGEZAL;
				break;
		
			default:
				fprintf(stderr, "Unknown instruction: %8.8X\n", instr);
				out->op = INVALID_OP;
			}
			break;

		case 0x02: /* J */
			out->addr *= 2;
			out->op = J;
			break;
	
		case 0x03: /* JAL */
			out->addr *= 2;
			out->op = JAL;
			break;
	
		case 0x04: /* BEQ */
			out->simm *= 2;
			out->op = BEQ;
			break;
	
		case 0x05: /* BNE */
			out->simm *= 2;
			out->op = BNE;
			break;
	
		case 0x06: /* BLEZ */
			out->simm *= 2;
			out->op = BLEZ;
			break;
	
		case 0x07: /* BGTZ */
			out->simm *= 2;
			out->op = BGTZ;
			break;
	
		case 0x18: /* LB */
			out->op = LB;
			break;

		case 0x19: /* LH */
			out->op = LH;
			break;

		case 0x1A: /* LW */
			out->op = LW;
			break;

		case 0x1B: /* LBU */
			out->op = LBU;
			break;

		case 0x1C: /* LHU */
			out->op = LHU;
			break;

		case 0x1D: /* SB */
			out->op = SB;
			break;

		case 0x1E: /* SH */
			out->op = SH;
			break;

		case 0x1F: /* SW */
			out->op = SW;
			break;

		default:
			parse_instr(instr, out);
		}
		assert(out->op < NOP);
		return 4;
	} else {
		/* small instructions */
		instr >>= 16;
		uint8_t opcode = (instr >> 10) & 0x1F;
		uint8_t rds    = (instr >>  5) & 0x1F;
		uint8_t rt     = instr & 0x1F;
		int16_t offset = instr & 0x03FF;
		if (offset > 0x01FF) {
			offset = -(((~offset) + 1) & 0x03FF);
		}
		assert(-512 <= offset && offset <= 511);
		
		uint8_t uimm = instr & 0x1F;
		int8_t  simm;
		if (uimm > 0x0F) {
			simm = -(((~uimm) + 1) & 0x1F);
		} else {
			simm = uimm;
		}

		assert(-16 <= simm && simm <= 15);

		out->compressed = true;
		switch(opcode) {
		case C_MOV:
			out->op = ADDU;
			out->rd = rds;
			out->rs = 0;
			out->rt = rt;
			break;

		case C_ADDU:
			out->op = ADDU;
			out->rd = rds;
			out->rs = rds;
			out->rt = rt;
			break;

		case C_SUBU:
			out->op = SUBU;
			out->rd = rds;
			out->rs = rds;
			out->rt = rt;
			break;

		case C_OR:
			out->op = OR;
			out->rd = rds;
			out->rs = rds;
			out->rt = rt;
			break;

		case C_XOR:
			out->op = XOR;
			out->rd = rds;
			out->rs = rds;
			out->rt = rt;
			break;

		case C_NEG:
			out->op = SUBU;
			out->rd = rds;
			out->rs = 0;
			out->rt = rt;
			break;

		case C_NOT:
			out->op = NOR;
			out->rd = rds;
			out->rs = 0;
			out->rt = rt;
			break;

		case C_SLTU:
			out->op = SLTU;
			out->rd = rds;
			out->rs = rds;
			out->rt = rt;
			break;

		case C_ADDIU:
			out->op = ADDIU;
			out->rt = rds;
			out->rs = rds;
			out->simm = simm;
			break;

		case C_ANDI:
			out->op = ANDI;
			out->rt = rds;
			out->rs = rds;
			out->imm = uimm;
			break;

		case C_SLL:
			out->op = SLL;
			out->rd = rds;
			out->rt = rds;
			out->shamt = uimm;
			break;

		case C_SRL:
			out->op = SRL;
			out->rd = rds;
			out->rt = rds;
			out->shamt = uimm;
			break;

		case C_SRA:
			out->op = SRA;
			out->rd = rds;
			out->rt = rds;
			out->shamt = uimm;
			break;

		case C_LSI:
			out->op = ADDIU;
			out->rt = rds;
			out->rs = 0;
			out->simm = simm;
			break;

		case C_B:
			out->op = BGEZ;
			out->rs = 0;
			out->simm = offset * 2;
			break;

		case C_BAL:
			out->op = BGEZAL;
			out->rs = 0;
			out->simm = offset * 2;
			break;

		case C_BEQZ:
			out->op = BEQ;
			out->rs = 0;
			out->rt = rds;
			out->simm = simm * 2;
			break;

		case C_BNEZ:
			out->op = BNE;
			out->rs = 0;
			out->rt = rds;
			out->simm = simm * 2;
			break;

		case C_JALR:
			if (rds == 0) {
				out->op = JR;
				out->rs = rt;
			} else {
				out->op = JALR;
				out->rs = rt;
				out->rd = rds;
			}
			break;

		case C_LWS:
			out->op = LW;
			out->rs = 29;
			out->rt = rds;
			out->simm = uimm * 4;
			break;

		case C_SWS:
			out->op = SW;
			out->rs = 29;
			out->rt = rds;
			out->simm = uimm * 4;
			break;

		default:
			fprintf(stderr, "unknown opcode\n");
			return 0;
		}

		assert(out->op < NOP);
		return 2;
	}
}

static uint16_t write_r(uint8_t opcode, uint8_t rds, uint8_t rt)
{
	assert(opcode < 32);
	assert(rds < 32);
	assert(rt < 32);

	uint16_t instr = 1;
	instr = (instr << 5) | opcode;
	instr = (instr << 5) | rds;
	instr = (instr << 5) | rt;

	return instr;
}

static uint16_t write_ui(uint8_t opcode, uint8_t rds, uint8_t imm)
{
	assert(opcode < 32);
	assert(rds < 32);
	assert(imm < 32);

	uint16_t instr = 1;
	instr = (instr << 5) | opcode;
	instr = (instr << 5) | rds;
	instr = (instr << 5) | imm;

	return instr;
}

static uint32_t write_si(uint8_t opcode, uint8_t rds, int8_t simm)
{
	assert(opcode < 32);
	assert(rds < 32);
	assert(-16 <= simm && simm <= 15);

	uint8_t imm;
	if (simm < 0) {
		imm = (~(-simm) + 1);
	} else {
		imm = simm;
	}

	uint16_t instr = 1;
	instr = (instr << 5) | opcode;
	instr = (instr << 5) | rds;
	instr = (instr << 5) | imm;

	return instr;
}

static uint32_t write_b(uint8_t opcode, int16_t offset)
{
	assert(opcode < 32);
	assert(-512 <= offset && offset <= 511);

	uint16_t imm;
	if (offset < 0) {
		imm = (~(-offset) + 1);
	} else {
		imm = offset;
	}

	uint16_t instr = 1;
	instr = (instr << 5)  | opcode;
	instr = (instr << 10) | imm;

	return instr;
}

static uint32_t write_l_si(uint8_t opcode, uint8_t rs, uint8_t rt, int16_t simm)
{
	assert(opcode < 64);
	assert(rs < 32);
	assert(rt < 32);

	uint16_t imm;
	if (simm < 0) {
		imm = -simm;
		imm = (~imm) + 1;
	} else {
		imm = simm;
	}

	uint32_t instr = opcode;
	instr = (instr << 5) | rs;
	instr = (instr << 5) | rt;
	instr = (instr << 16) | imm;

	return instr;
}

static uint32_t write_l_j(uint8_t opcode, uint32_t target)
{
	assert(opcode < 64);
	assert(target < 0x4000000);
	uint32_t instr = opcode;
	instr = (instr << 26) | target;

	return instr;
}

int write_instr_v2(struct instr *instr, uint32_t *out)
{
	assert(instr != NULL);
	assert(out   != NULL);

	if (!instr->compressed) {
		// convert pseudo ops to native ops
		struct instr inst = *instr;
		conv_to_native(&inst);
		assert(inst.op < NOP);

		switch(inst.op) {
		case LB:
			*out = write_l_si(0x20, inst.rs, inst.rt, inst.simm);
			break;

		case LH:
			*out = write_l_si(0x21, inst.rs, inst.rt, inst.simm);
			break;

		case LW:
			*out = write_l_si(0x23, inst.rs, inst.rt, inst.simm);
			break;

		case LBU:
			*out = write_l_si(0x24, inst.rs, inst.rt, inst.simm);
			break;

		case LHU:
			*out = write_l_si(0x25, inst.rs, inst.rt, inst.simm);
			break;

		case SB:
			*out = write_l_si(0x28, inst.rs, inst.rt, inst.simm);
			break;

		case SH:
			*out = write_l_si(0x29, inst.rs, inst.rt, inst.simm);
			break;

		case SW:
			*out = write_l_si(0x2B, inst.rs, inst.rt, inst.simm);
			break;

		case BLTZ:
			*out = write_l_si(0x01, instr->rs, 0x00, instr->simm / 2);
			break;
	
		case BGEZ:
			*out = write_l_si(0x01, instr->rs, 0x01, instr->simm / 2);
			break;
	
		case BLTZAL:
			*out = write_l_si(0x01, instr->rs, 0x10, instr->simm / 2);
			break;
	
		case BGEZAL:
			*out = write_l_si(0x01, instr->rs, 0x11, instr->simm / 2);
			break;
	
		case BEQ:
			*out = write_l_si(0x04, instr->rs, instr->rt, instr->simm / 2);
			break;
	
		case BNE:
			*out = write_l_si(0x05, instr->rs, instr->rt, instr->simm / 2);
			break;
	
		case BLEZ:
			*out = write_l_si(0x06, instr->rs, 0x00, instr->simm / 2);
			break;
	
		case BGTZ:
			*out = write_l_si(0x07, instr->rs, 0x00, instr->simm / 2);
			break;
	
		case J:
			*out = write_l_j(0x02, instr->addr / 2);
			break;
	
		case JAL:
			*out = write_l_j(0x03, instr->addr / 2);
			break;

		default:
			*out = write_instr(&inst);
		}

		return 4;
	}

	switch(instr->op) {
	case NOP:
		*out = write_r(C_MOV, 0, 0);
		break;

	case MOV:
		*out = write_r(C_MOV, instr->rd, instr->rt);
		break;

	case CLEAR:
		*out = write_r(C_MOV, instr->rd, 0);
		break;

	case ADDU:
		*out = write_r(C_ADDU, instr->rd, instr->rt);
		break;

	case SUBU:
		*out = write_r(C_SUBU, instr->rd, instr->rt);
		break;

	case OR:
		*out = write_r(C_OR, instr->rd, instr->rt);
		break;

	case XOR:
		*out = write_r(C_XOR, instr->rd, instr->rt);
		break;

	case NEG:
		*out = write_r(C_NEG, instr->rd, instr->rt);
		break;

	case NOT:
		*out = write_r(C_NOT, instr->rd, instr->rt);
		break;

	case SLTU:
		*out = write_r(C_SLTU, instr->rd, instr->rt);
		break;

	case ADDIU:
		*out = write_si(C_ADDIU, instr->rd, instr->simm);
		break;

	case ANDI:
		*out = write_ui(C_ANDI, instr->rd, instr->imm);
		break;

	case SLL:
		*out = write_ui(C_SLL, instr->rd, instr->shamt);
		break;

	case SRL:
		*out = write_ui(C_SRL, instr->rd, instr->shamt);
		break;

	case SRA:
		*out = write_ui(C_ANDI, instr->rd, instr->shamt);
		break;

	case LSI:
		*out = write_si(C_LSI, instr->rd, instr->simm);
		break;

	case B:
		*out = write_b(C_B, instr->simm / 2);
		break;

	case BAL: 
		*out = write_b(C_BAL, instr->simm / 2);
		break;

	case BEQZ:
		*out = write_si(C_BEQZ, instr->rs, instr->simm / 2);
		break;

	case BNEZ:
		*out = write_si(C_BNEZ, instr->rs, instr->simm / 2);
		break;

	case JR:
		*out = write_r(C_JALR, 0, instr->rs);
		break;

	case JALR:
		*out = write_r(C_JALR, instr->rd, instr->rs);
		break;

	case SW:
		*out = write_ui(C_SWS, instr->rd, instr->imm / 4);
		break;

	case LW:
		*out = write_ui(C_LWS, instr->rd, instr->imm / 4);
		break;

	default:
		fprintf(stderr, "Invalid compressed instruction\n");
		exit(EXIT_FAILURE);
		break;
	}
	return 2;
}
