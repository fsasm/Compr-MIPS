/**
 * @file print_instr.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-07-12
 */

#include "print_instr.h"
#include <stdio.h>
#include <stdarg.h>

#define PRINT(...) fprintf(stdout, __VA_ARGS__)
#define PRINT_R(op)  PRINT("%s r%d, r%d, r%d\n", op, instr->rd, instr->rs, instr->rt)
#define PRINT_R2(op)  PRINT("%s r%d, r%d\n", op, instr->rd, instr->rt)
#define PRINT_R1(op)  PRINT("%s r%d\n", op, instr->rd)
#define PRINT_R_REV(op) PRINT("%s r%d, r%d, r%d\n", op, instr->rd, instr->rt, instr->rs)
#define PRINT_S(op)  PRINT("%s r%d, r%d, %d\n", op, instr->rd, instr->rt, instr->shamt)
#define PRINT_I(op)  PRINT("%s r%d, r%d, %d\n", op, instr->rt, instr->rs, instr->imm)
#define PRINT_I2(op) PRINT("%s r%d, %d\n", op, instr->rs, instr->simm)
#define PRINT_Is(op) PRINT("%s r%d, r%d, %d\n", op, instr->rt, instr->rs, instr->simm)
#define PRINT_LS(op) PRINT("%s r%d, %d(r%d)\n", op, instr->rt, instr->simm, instr->rs)

void print_instr(struct instr *instr)
{
	switch (instr->op) {
	case SLL:
		PRINT_S("sll");
		break;
	
	case SRL:
		PRINT_S("srl");
		break;
	
	case SRA:
		PRINT_S("sra");
		break;
	
	case SLLV:
		PRINT_R_REV("sllv");
		break;

	case SRLV:
		PRINT_R_REV("srlv");
		break;

	case SRAV:
		PRINT_R_REV("srav");
		break;
	
	case ADD:
		PRINT_R("add");
		break;
	
	case ADDU:
		PRINT_R("addu");
		break;
	
	case SUB:
		PRINT_R("sub");
		break;
	
	case SUBU:
		PRINT_R("subu");
		break;
	
	case AND:
		PRINT_R("and");
		break;
	
	case OR:
		PRINT_R("or");
		break;
	
	case XOR:
		PRINT_R("xor");
		break;
	
	case NOR:
		PRINT_R("nor");
		break;
	
	case ADDI:
		PRINT_Is("addi");
		break;
	
	case ADDIU:
		PRINT_Is("addiu");
		break;
	
	case ANDI:
		PRINT_I("andi");
		break;
	
	case ORI:
		PRINT_I("ori");
		break;
	
	case XORI:
		PRINT_I("xori");
		break;

	case LUI:
		PRINT("lui r%d, 0x%X\n", instr->rt, instr->imm);
		break;

	case LB:
		PRINT_LS("lb");
		break;
	
	case LH:
		PRINT_LS("lh");
		break;
	
	case LW:
		PRINT_LS("lw");
		break;
	
	case LBU:
		PRINT_LS("lbu");
		break;
	
	case LHU:
		PRINT_LS("lhu");
		break;
	
	case SB:
		PRINT_LS("sb");
		break;
	
	case SH:
		PRINT_LS("sh");
		break;
	
	case SW:
		PRINT_LS("sw");
		break;

	case SLT:
		PRINT_R("slt");
		break;
	
	case SLTU:
		PRINT_R("sltu");
		break;
	
	case SLTI:
		PRINT_Is("slti");
		break;
	
	case SLTIU:
		PRINT_I("sltiu");
		break;
	
	case BLTZ:
		PRINT_I2("bltz");
		break;
	
	case BGEZ:
		PRINT_I2("bgez");
		break;
	
	case BLTZAL:
		PRINT_I2("bltzal");
		break;
	
	case BGEZAL:
		PRINT_I2("bgezal");
		break;
	
	case BEQ:
		PRINT_Is("beq");
		break;
	
	case BNE:
		PRINT_Is("bne");
		break;
	
	case BLEZ:
		PRINT_I2("blez");
		break;
	
	case BGTZ:
		PRINT_I2("bgtz");
		break;
	
	case J:
		PRINT("j 0x%X\n", instr->addr);
		break;
	
	case JAL:
		PRINT("jal 0x%X\n", instr->addr);
		break;
	
	case JR:
		PRINT("jr r%d\n", instr->rs);
		break;
	
	case JALR:
		PRINT("jalr r%d, r%d\n", instr->rd, instr->rs);
		break;

	/* pseudo instructions */
	case NOP:
		PRINT("nop\n");
		break;
	
	case MOV:
		PRINT_R2("mov");
		break;
	
	case CLEAR:
		PRINT_R1("clear");
		break;

	case NOT:
		PRINT_R2("not");
		break;

	case NEG:
		PRINT_R2("neg");
		break;

	case B:
		PRINT("b %d\n", instr->simm);
		break;

	case BAL:
		PRINT("bal %d\n", instr->simm);
		break;
	
	case BEQZ:
		PRINT("beqz r%d, %d\n", instr->rs, instr->simm);
		break;

	case BNEZ:
		PRINT("bnez r%d, %d\n", instr->rs, instr->simm);
		break;

	case SEQZ:
		PRINT("seqz r%d, r%d\n", instr->rd, instr->rt);
		break;

	case SNEZ:
		PRINT("snez r%d, r%d\n", instr->rd, instr->rt);
		break;

	case SLTZ:
		PRINT("sltz r%d, r%d\n", instr->rd, instr->rt);
		break;

	case LSI:
		PRINT("lsi r%d, %d\n", instr->rd, instr->simm);
		break;

	default:
		fprintf(stderr, "unknown op\n");
	}
}
