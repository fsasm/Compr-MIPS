/**
 * @file instr.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-21
 */

#include "instr.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

static uint8_t get_opcode(uint32_t instr)
{
	return instr >> 26;
}

static uint8_t get_rs(uint32_t instr)
{
	return (instr >> 21) & 0x1F;
}

static uint8_t get_rt(uint32_t instr)
{
	return (instr >> 16) & 0x1F;
}

static uint8_t get_rd(uint32_t instr)
{
	return (instr >> 11) & 0x1F;
}

static uint8_t get_shamt(uint32_t instr)
{
	return (instr >> 6) & 0x1F;
}

static uint8_t get_func(uint32_t instr)
{
	return instr & 0x3F;
}

static uint16_t get_imm(uint32_t instr)
{
	return instr & 0xFFFF;
}

static uint32_t get_addr(uint32_t instr)
{
	return instr & 0x3FFFFFF;
}

static int16_t get_simm(uint32_t instr)
{
	uint16_t imm = instr & 0xFFFF;
	if (imm > 0x7FFF) {
		imm = (~imm) + 1;
		return -imm;
	} else {
		return imm;
	}
}

static enum operation parse_special(uint32_t instr)
{
	assert(get_opcode(instr) == 0);

	uint8_t func = get_func(instr);
	switch (func) {
	case 0x00: /* SLL */
		return SLL;

	case 0x02: /* SRL */
		return SRL;

	case 0x03: /* SRA */
		return SRA;

	case 0x04: /* SLLV */
		return SLLV;

	case 0x06: /* SRLV */
		return SRLV;

	case 0x07: /* SRAV */
		return SRAV;

	case 0x08: /* JR */
		return JR;

	case 0x09: /* JALR */
		return JALR;

	case 0x0D: /* BREAK */
		fprintf(stderr, "Unsupported BREAK\n");
		return INVALID_OP;

	case 0x10: /* MFHI */
		fprintf(stderr, "Unsupported MFHI\n");
		return INVALID_OP;

	case 0x11: /* MTHI */
		fprintf(stderr, "Unsupported MTHI\n");
		return INVALID_OP;

	case 0x12: /* MFLO */
		fprintf(stderr, "Unsupported MFLO\n");
		return INVALID_OP;

	case 0x13: /* MTLO */
		fprintf(stderr, "Unsupported MTLO\n");
		return INVALID_OP;

	case 0x18: /* MULT */
		fprintf(stderr, "Unsupported MULT\n");
		return INVALID_OP;

	case 0x19: /* MULTU */
		fprintf(stderr, "Unsupported MULTU\n");
		return INVALID_OP;

	case 0x1A: /* DIV */
		fprintf(stderr, "Unsupported DIV\n");
		return INVALID_OP;

	case 0x1B: /* DIVU */
		fprintf(stderr, "Unsupported DIVU\n");
		return INVALID_OP;

	case 0x20: /* ADD */
		return ADD;

	case 0x21: /* ADDU */
		return ADDU;

	case 0x22: /* SUB */
		return SUB;

	case 0x23: /* SUBU */
		return SUBU;

	case 0x24: /* AND */
		return AND;

	case 0x25: /* OR */
		return OR;

	case 0x26: /* XOR */
		return XOR;

	case 0x27: /* NOR */
		return NOR;

	case 0x2A: /* SLT */
		return SLT;

	case 0x2B: /* SLTU */
		return SLTU;

	default:
		fprintf(stderr, "Unknown instruction: %8.8X\n", instr);
		return INVALID_OP;
	}
}

static enum operation parse_branches(uint32_t instr)
{
	assert(get_opcode(instr) == 0x01);

	uint8_t rt = get_rt(instr);
	switch (rt) {
	case 0x00: /* BLTZ */
		return BLTZ;

	case 0x01: /* BGEZ */
		return BGEZ;

	case 0x10: /* BLTZAL */
		return BLTZAL;

	case 0x11: /* BGEZAL */
		return BGEZAL;

	default:
		fprintf(stderr, "Unknown instruction: %8.8X\n", instr);
		return INVALID_OP;
	}
}

static enum operation parse_cop0(uint32_t instr)
{
	assert(get_opcode(instr) == 0x10);
	uint8_t rs = get_rs(instr);
	switch (rs) {
	case 0x00: /* MFC0 */
		return MFC0;

	case 0x04: /* MTC0 */
		return MTC0;

	default:
		fprintf(stderr, "Unknown instruction: %8.8X\n", instr);
		return INVALID_OP;
	}
}

void parse_instr(uint32_t instr, struct instr *out)
{
	assert(out != NULL);

	out->rs = get_rs(instr);
	out->rt = get_rt(instr);
	out->rd = get_rd(instr);

	out->shamt = get_shamt(instr);
	out->imm  = get_imm(instr);
	out->simm = get_simm(instr);
	out->addr = get_addr(instr);
	out->compressed = false;

	uint8_t opcode = get_opcode(instr);

	switch (opcode) {
	case 0x00:
		out->op = parse_special(instr);
		break;

	case 0x01:
		out->simm *= 4;
		out->op = parse_branches(instr);
		break;
	
	case 0x02: /* J */
		out->addr *= 4;
		out->op = J;
		break;

	case 0x03: /* JAL */
		out->addr *= 4;
		out->op = JAL;
		break;

	case 0x04: /* BEQ */
		out->simm *= 4;
		out->op = BEQ;
		break;

	case 0x05: /* BNE */
		out->simm *= 4;
		out->op = BNE;
		break;

	case 0x06: /* BLEZ */
		out->simm *= 4;
		out->op = BLEZ;
		break;

	case 0x07: /* BGTZ */
		out->simm *= 4;
		out->op = BGTZ;
		break;

	case 0x08: /* ADDI */
		out->op = ADDI;
		break;

	case 0x09: /* ADDIU */
		out->op = ADDIU;
		break;

	case 0x0A: /* SLTI */
		out->op = SLTI;
		break;

	case 0x0B: /* SLTIU */
		out->op = SLTIU;
		break;

	case 0x0C: /* ANDI */
		out->op = ANDI;
		break;

	case 0x0D: /* ORI */
		out->op = ORI;
		break;

	case 0x0E: /* XORI */
		out->op = XORI;
		break;

	case 0x0F: /* LUI */
		out->op = LUI;
		break;

	case 0x10: 
		out->op = parse_cop0(instr);
		break;

	case 0x20: /* LB */
		out->op = LB;
		break;

	case 0x21: /* LH */
		out->op = LH;
		break;

	case 0x23: /* LW */
		out->op = LW;
		break;

	case 0x24: /* LBU */
		out->op = LBU;
		break;

	case 0x25: /* LHU */
		out->op = LHU;
		break;

	case 0x28: /* SB */
		out->op = SB;
		break;

	case 0x29: /* SH */
		out->op = SH;
		break;

	case 0x2B: /* SW */
		out->op = SW;
		break;

	default:
		fprintf(stderr, "Unknown instruction: %8.8X\n", instr);
		out->op = INVALID_OP;
	}
}

void conv_to_pseudo(struct instr *out)
{
	assert(out != NULL);

	uint8_t rs = out->rs;
	uint8_t rt = out->rt;
	uint8_t rd = out->rd;
	uint8_t shamt = out->shamt;
	uint16_t imm = out->imm;
	int16_t simm = out->simm;

	switch (out->op) {
	case SLL:
		if (rd == 0) {
			out->op = NOP;
		} else if (shamt == 0) {
			if (rt == 0) {
				out->op = CLEAR;
			} else {
				out->op = MOV;
			}
		}
		break;

	case SRL:
		if (rd == 0) {
			out->op = NOP;
		} else if (shamt == 0) {
			out->op = MOV;
		} else if (shamt == 31) {
			out->op = SLTZ;
			out->rs = rt;
		}
		break;

	case SRA:
		if (rd == 0) {
			out->op = NOP;
		} else if (shamt == 0) {
			out->op = MOV;
		}
		break;

	case SLLV:
		if (rd == 0) {
			out->op = NOP;
		} else if(rs == 0) {
			out->op = CLEAR;
		} if (rt == 0) {
			out->op = MOV;
		}
		break;

	case SRLV:
		if (rd == 0) {
			out->op = NOP;
		} else if(rs == 0) {
			out->op = CLEAR;
		} if (rt == 0) {
			out->op = MOV;
		}
		break;

	case SRAV:
		if (rd == 0) {
			out->op = NOP;
		} else if(rs == 0) {
			out->op = CLEAR;
		} if (rt == 0) {
			out->op = MOV;
		}
		break;

	case ADDU:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rt == 0) {
			out->op = MOV;
			if (rt == 0) {
				out->rt = rs;
			}
		}
		break;

	case SUBU:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0) {
			out->op = NEG;
		} else if (rt == 0) {
			out->op = MOV;
			out->rt = rs;
		}
		break;

	case AND:
		if (rd == 0) {
			out->op = NOP;
		} else if (rt == 0 || rs == 0) {
			out->op = CLEAR;
		}
		break;

	case OR:
		if (rd == 0) {
			out->op = NOP;
		} else if (rt == 0 && rs == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rt == 0) {
			out->op = MOV;
			if (rt == 0) {
				out->rt = rs;
			}
		}
		break;

	case XOR:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rt == 0) {
			out->op = MOV;
			if (rt == 0) {
				out->rt = rs;
			}
		}
		break;

	case NOR:
		if (rd == 0) {
			out->op = NOP;
		} else if ((rs == 0 && rt != 0) || (rs != 0 && rt == 0)) {
			out->op = NOT;
			if (rt == 0) {
				out->rt = rs;
			}
		}
		break;

	case ADDIU:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && simm == 0) {
			out->op = CLEAR;
			out->rd = rt;
		} else if (simm == 0) {
			out->op = MOV;
			out->rd = rt;
			out->rt = rs;
		} else if (rs == 0 && -16 <= simm && simm <= 15) {
			out->op = LSI;
			out->rd = rt;
		}
		break;

	case ANDI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 || imm == 0) {
			out->op = CLEAR;
			out->rd = rt;
		}
		break;

	case ORI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
			out->rd = rt;
		} else if (imm == 0) {
			out->op = MOV;
			out->rd = rt;
			out->rt = rs;
		}
		break;

	case XORI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
			out->rd = rt;
		} else if (imm == 0) {
			out->op = MOV;
			out->rd = rt;
			out->rt = rs;
		}
		break;

	case LUI:
		if (imm == 0) {
			out->op = CLEAR;
			out->rd = rt;
		}
		break;

	case SLT:
		if (rt == 0) {
			out->op = SLTZ;
		}
		break;
	
	case SLTU:
		if (rs == 0) {
			out->op = SNEZ;
		}
		break;
	
	case SLTI:
		if (out->simm == 0) {
			out->op = SLTZ;
			out->rd = rt;
		}
		break;

	case SLTIU:
		if (imm == 1) {
			out->op = SEQZ;
		}
		break;
	
	case BGEZ:
		if (rs == 0) {
			out->op = B;
		}
		break;
	
	case BGEZAL:
		if (rs == 0) {
			out->op = BAL;
		}
		break;

	case BEQ:
		if (rs == rt) {
			out->op = B;
		} else if (rs == 0 || rt == 0) {
			out->op = BEQZ;
			if (rs == 0) {
				out->rs = rt;
			}
		}
		break;

	case BNE:
		if ((rs == 0 && rt != 0) || (rs != 0 && rt == 0)) {
			out->op = BNEZ;
			if (rs == 0) {
				out->rs = rt;
			}
		}
		break;

	case BLEZ:
		if (rs == 0) {
			out->op = B;
		}
		break;

	case JALR:
		if (rd == 0) {
			out->op = JR;
		}
		break;
	
	default:
		break;
		/* ignore */
	}
}

bool is_branch(enum operation op)
{
	switch(op) {
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
		case BEQZ:
		case BNEZ:
			return true;

		default:
			return false;
	}
}

bool contains_imm(enum operation op)
{
	switch(op) {
		case ANDI:
		case LUI: /* it's neither signed or unsigned on 32bit but signed on 64bit */
		case ORI:
		case XORI:
			return true;

		default:
			return false;
	}
}

bool contains_simm(enum operation op)
{
	switch(op) {
		case ADDI:
		case ADDIU:
		case SLTI:
		case SLTIU:
			return true;

		default:
			return false;
	}
}


bool is_compressible_simple(struct instr *instr)
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
	case LSI:
	case JR:
	case JALR:
		return true;

	case SLL:
	case SRL:
	case SRA:
		if (rd == rt)
			return true;
		return false;

	case ADDU:
	case OR:
	case XOR:
		if (rd == rs || rd == rt)
			return true;
		return false;
	
	case SUBU:
	case SLTU:
		if (rd == rs)
			return true;
		return false;

	case ADDIU:
		if (rs == rt && -16 <= simm && simm <= 15)
			return true;
		return false;

	case ANDI:
		if (rs == rt && imm <= 31)
			return true;
		return false;

		/*
	case LUI:
		if (-16 <= simm && simm <= 15)
			return true;
		return false;
		*/
	
	case SW:
	case LW:
		/* Stack pointer is $29 */
		if (rs == 29 && imm % 4 == 0 && imm < 128)
			return true;
		return false;
	
	default:
		return false;
	}
}

void conv_to_native(struct instr *out)
{
	assert(out != NULL);

	switch(out->op) {
	case NOP:
		out->op = ADDU;
		out->rd = 0;
		out->rs = 0;
		out->rt = 0;
		break;

	case MOV:
		out->op = ADDU;
		out->rs = 0;
		break;

	case CLEAR:
		out->op = ADDU;
		out->rs = 0;
		out->rt = 0;
		break;

	case NOT:
		out->op = NOR;
		break;

	case NEG:
		out->op = SUBU;
		out->rs = 0;
		break;

	case B:
		out->op = BGEZ;
		out->rs = 0;
		out->rt = 0;
		break;

	case BAL: 
		out->op = BGEZAL;
		out->rs = 0;
		out->rt = 0;
		break;

	case BEQZ:
		out->op = BEQ;
		break;

	case BNEZ:
		out->op = BNE;
		break;

	case SEQZ:
		out->op = SLTIU;
		out->simm = 1;
		out->imm = 1;
		break;

	case SNEZ:
		out->op = SLTU;
		out->rs = 0;
		break;

	case SLTZ:
		out->op = SLT;
		out->rt = 0;
		break;

	case LSI:
		out->op = ADDIU;
		out->rs = 0;
		break;
	
	default:
		break;
	}

}

static uint32_t write_r(uint8_t opcode, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t func)
{
	assert(opcode < 64);
	assert(rs < 32);
	assert(rt < 32);
	assert(rd < 32);
	assert(shamt < 32);
	assert(func < 64);

	uint32_t instr = opcode;
	instr = (instr << 5) | rs;
	instr = (instr << 5) | rt;
	instr = (instr << 5) | rd;
	instr = (instr << 5) | shamt;
	instr = (instr << 6) | func;

	return instr;
}

static uint32_t write_si(uint8_t opcode, uint8_t rs, uint8_t rt, int16_t simm)
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

static uint32_t write_ui(uint8_t opcode, uint8_t rs, uint8_t rt, uint16_t uimm)
{
	assert(opcode < 64);
	assert(rs < 32);
	assert(rt < 32);

	uint32_t instr = opcode;
	instr = (instr << 5) | rs;
	instr = (instr << 5) | rt;
	instr = (instr << 16) | uimm;

	return instr;
}

static uint32_t write_j(uint8_t opcode, uint32_t target)
{
	assert(opcode < 64);
	assert(target < 0x4000000);
	uint32_t instr = opcode;
	instr = (instr << 26) | target;

	return instr;
}

uint32_t write_instr(struct instr *instr)
{
	assert(instr != NULL);
	assert(instr->op < NOP);

	switch(instr->op) {
	/* ALU */
	case SLL:
		return write_r(0x00, 0x00, instr->rt, instr->rd, instr->shamt, 0x00);
		break;

	case SRL:
		return write_r(0x00, 0x00, instr->rt, instr->rd, instr->shamt, 0x02);
		break;

	case SRA:
		return write_r(0x00, 0x00, instr->rt, instr->rd, instr->shamt, 0x03);
		break;

	case SLLV:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x04);
		break;

	case SRLV:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x06);
		break;

	case SRAV:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x07);
		break;

	case ADD:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x20);
		break;

	case ADDU:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x21);
		break;

	case SUB:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x22);
		break;

	case SUBU:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x23);
		break;

	case AND:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x24);
		break;
		
	case OR:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x25);
		break;
		
	case XOR:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x26);
		break;
		
	case NOR:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x27);
		break;

	/* ALU imm */
	case ADDI:
		return write_si(0x08, instr->rs, instr->rt, instr->simm);
		break;

	case ADDIU:
		return write_si(0x09, instr->rs, instr->rt, instr->simm);
		break;

	case ANDI:
		return write_ui(0x0C, instr->rs, instr->rt, instr->imm);
		break;

	case ORI:
		return write_ui(0x0D, instr->rs, instr->rt, instr->imm);
		break;

	case XORI:
		return write_ui(0x0E, instr->rs, instr->rt, instr->imm);
		break;

	case LUI:
		return write_ui(0x0F, 0x00, instr->rt, instr->imm);
		break;

	/* load & store */
	case LB:
		return write_si(0x20, instr->rs, instr->rt, instr->simm);
		break;

	case LH:
		return write_si(0x21, instr->rs, instr->rt, instr->simm);
		break;

	case LW:
		return write_si(0x23, instr->rs, instr->rt, instr->simm);
		break;

	case LBU:
		return write_si(0x24, instr->rs, instr->rt, instr->simm);
		break;

	case LHU:
		return write_si(0x25, instr->rs, instr->rt, instr->simm);
		break;

	case SB:
		return write_si(0x28, instr->rs, instr->rt, instr->simm);
		break;

	case SH:
		return write_si(0x29, instr->rs, instr->rt, instr->simm);
		break;

	case SW:
		return write_si(0x2B, instr->rs, instr->rt, instr->simm);
		break;

	/* compare */
	case SLT:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x2A);
		break;

	case SLTU:
		return write_r(0x00, instr->rs, instr->rt, instr->rd, 0x00, 0x2B);
		break;

	case SLTI:
		return write_si(0x0A, instr->rs, instr->rt, instr->simm);
		break;

	case SLTIU:
		return write_si(0x0B, instr->rs, instr->rt, instr->simm);
		break;

	/* branches */
	case BLTZ:
		return write_si(0x01, instr->rs, 0x00, instr->simm / 4);
		break;

	case BGEZ:
		return write_si(0x01, instr->rs, 0x01, instr->simm / 4);
		break;

	case BLTZAL:
		return write_si(0x01, instr->rs, 0x10, instr->simm / 4);
		break;

	case BGEZAL:
		return write_si(0x01, instr->rs, 0x11, instr->simm / 4);
		break;

	case BEQ:
		return write_si(0x04, instr->rs, instr->rt, instr->simm / 4);
		break;

	case BNE:
		return write_si(0x05, instr->rs, instr->rt, instr->simm / 4);
		break;

	case BLEZ:
		return write_si(0x06, instr->rs, 0x00, instr->simm / 4);
		break;

	case BGTZ:
		return write_si(0x07, instr->rs, 0x00, instr->simm / 4);
		break;

	case J:
		return write_j(0x02, instr->addr / 4);
		break;

	case JAL:
		return write_j(0x03, instr->addr / 4);
		break;

	case JR:
		return write_r(0x00, instr->rs, 0x00, 0x00, 0x00, 0x08);
		break;

	case JALR:
		return write_r(0x00, instr->rs, 0x00, instr->rd, 0x00, 0x09);
		break;

	default:
		fprintf(stderr, "Unimplemented instruction\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

