/**
 * @file instr.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-21
 */

#include "instr.h"
#include <stdio.h>
#include <assert.h>

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

	uint8_t opcode = get_opcode(instr);

	switch (opcode) {
	case 0x00:
		out->op = parse_special(instr);
		break;

	case 0x01:
		out->op = parse_branches(instr);
		break;
	
	case 0x02: /* J */
		out->op = J;
		break;

	case 0x03: /* JAL */
		out->op = JAL;
		break;

	case 0x04: /* BEQ */
		out->op = BEQ;
		break;

	case 0x05: /* BNE */
		out->op = BNE;
		break;

	case 0x06: /* BLEZ */
		out->op = BLEZ;
		break;

	case 0x07: /* BGTZ */
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

void conv_pseudo(struct instr *out)
{
	assert(out != NULL);

	uint8_t rs = out->rs;
	uint8_t rt = out->rt;
	uint8_t rd = out->rd;
	uint8_t shamt = out->shamt;
	uint16_t imm = out->imm;

	switch (out->op) {
	case SLL:
		if (rd == 0) {
			out->op = NOP;
		} else if (shamt == 0) {
			out->op = MOV;
		}
		break;

	case SRL:
		if (rd == 0) {
			out->op = NOP;
		} else if (shamt == 0) {
			out->op = MOV;
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

	case ADD:
		if (rd == 0) {
			out->op = NOP; /* this still could generate an overflow exception. */
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rs == 0) {
			out->op = MOV;
		}
		break;

	case ADDU:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rs == 0) {
			out->op = MOV;
		}
		break;

	case SUB:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0) {
			/* In this case a negation can trigger a overflow exception. */
			out->op = NEG;
		} else if (rt == 0) {
			out->op = MOV;
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
		}
		break;

	case XOR:
		if (rd == 0) {
			out->op = NOP;
		} else if (rs == 0 && rt == 0) {
			out->op = CLEAR;
		} else if (rs == 0 || rt == 0) {
			out->op = MOV;
		}
		break;

	case NOR:
		if (rd == 0) {
			out->op = NOP;
		} else if ((rs == 0 && rt != 0) || (rs != 0 && rt == 0)) {
			out->op = NOT;
		}
		break;

	case ADDI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
		} else if (imm == 0) {
			out->op = MOV;
		}
		break;

	case ADDIU:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
		} else if (imm == 0) {
			out->op = MOV;
		}
		break;

	case ANDI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 || imm == 0) {
			out->op = CLEAR;
		}
		break;

	case ORI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
		} else if (imm == 0) {
			out->op = MOV;
		}
		break;

	case XORI:
		if (rt == 0) {
			out->op = NOP;
		} else if (rs == 0 && imm == 0) {
			out->op = CLEAR;
		} else if (imm == 0) {
			out->op = MOV;
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
		}
		break;

	case BNE:
		if ((rs == 0 && rt != 0) || (rs != 0 && rt == 0)) {
			out->op = BNEZ;
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
