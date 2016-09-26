/**
 * @file instr.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-19
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef INSTR_H
#define INSTR_H

/* This enum contains all real and pseudo instructions */
enum operation {
	/* ALU - 20 instr. */
	SLL,
	SRL,
	SRA,
	SLLV,
	SRLV,
	SRAV,
	ADD,
	ADDU,
	SUB,
	SUBU,
	AND,
	OR,
	XOR,
	NOR,
	
	ADDI,
	ADDIU,
	ANDI,
	ORI,
	XORI,
	LUI,

	/* Load & Store - 8 instr. */
	LB,
	LH,
	LW,
	LBU,
	LHU,

	SB,
	SH,
	SW,

	/* Compare - 4 instr. */
	SLT,
	SLTU,
	SLTI,
	SLTIU,

	/* Branch & Jump - 12 instr. */
	BLTZ,
	BGEZ,
	BLTZAL,
	BGEZAL,
	BEQ,
	BNE,
	BLEZ,
	BGTZ,
	J,
	JAL,
	JR,
	JALR,

	/* COP0 instructions */
	MFC0,
	MTC0,

	/* Pseudo instructions */
	NOP,
	MOV,
	CLEAR,
	NOT,
	NEG,
	B,
	BAL, 
	BEQZ,
	BNEZ,
	SEQZ,
	SNEZ,
	SLTZ,
	LSI, /* load small immediate */

	NUM_INSTR,
	INVALID_OP
};

struct instr {
	enum operation op;
	uint8_t rs;
	uint8_t rt;
	uint8_t rd;
	uint8_t shamt;
	uint32_t imm;
	int32_t simm;
	uint32_t addr;
	bool compressed;
};

void parse_instr(uint32_t instr, struct instr *out);
uint32_t write_instr(struct instr *instr);
void conv_to_pseudo(struct instr *out);
void conv_to_native(struct instr *out);

bool is_branch(enum operation op);

bool is_compressible_simple(struct instr *instr);

#endif

