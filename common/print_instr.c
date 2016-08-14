/**
 * @file print_instr.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-07-12
 */

#include "print_instr.h"
#include <stdio.h>

#define PRINT_R(op) fprintf(stderr, "%s r%d, r%d, r%d\n", op, instr->rd, instr->rs, instr->rt)
#define PRINT_R2(op) fprintf(stderr, "%s r%d, r%d, r%d\n", op, instr->rd, instr->rt, instr->rs)
#define PRINT_S(op) fprintf(stderr, "%s r%d, r%d, %d\n", op, instr->rd, instr->rt, instr->shamt)
#define PRINT_I(op) fprintf(stderr, "%s r%d, r%d, %d\n", op, instr->rt, instr->rs, instr->imm)
#define PRINT_I2(op) fprintf(stderr, "%s r%d, r%d, %d\n", op, instr->rt, instr->rs, instr->simm)
#define PRINT_Is(op) fprintf(stderr, "%s r%d, r%d, %d\n", op, instr->rt, instr->rs, instr->simm)
#define PRINT_LS(op) fprintf(stderr, "%s r%d, %d(r%d)\n", op, instr->rt, instr->simm, instr->rs)

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
		PRINT_R2("sllv");
		break;

	case SRLV:
		PRINT_R2("srlv");
		break;

	case SRAV:
		PRINT_R2("srav");
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
		fprintf(stderr, "lui r%d, 0x%X\n", instr->rt, instr->imm);
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
		fprintf(stderr, "j 0x%X\n", instr->addr);
		break;
	
	case JAL:
		fprintf(stderr, "jal 0x%X\n", instr->addr);
		break;
	
	case JR:
		fprintf(stderr, "jr r%d\n", instr->rs);
		break;
	
	case JALR:
		fprintf(stderr, "jalr r%d, r%d\n", instr->rd, instr->rs);
		break;

	default:
		fprintf(stderr, "unkown op\n");
	}
}
