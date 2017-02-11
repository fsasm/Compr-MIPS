/**
 * @file simulator.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-11
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../common/instr.h"
#include "../common/v2_instr.h"
#include "../common/print_instr.h"

#define PC_START (0x40000000)
#define DEFAULT_NUM_CYCLES (256)
#define DEFAULT_IMEM_SIZE (16 * 1024)
#define DEFAULT_DMEM_SIZE (16 * 1024)

#define UART_STATUS (0xFFFFFFF8)
#define UART_DATA (0xFFFFFFFC)

static char *program_name = "simulator";
static bool debug = false;

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-i IMEM_SIZE] [-d DMEM_SIZE] [-n CYCLES] [-c] BIN-FILE [DATA-FILE]\n", program_name);
	fprintf(stderr, "\t-i\tSize in kiB of the instruction memory\n");
	fprintf(stderr, "\t-d\tSize in kiB of the data memory\n");
	fprintf(stderr, "\t-n\tNumber of cycles to execute. Default: %d\n", DEFAULT_NUM_CYCLES);
	fprintf(stderr, "\t-c\tUse compressed instruction format\n");
	exit(EXIT_FAILURE);
}

static uint32_t str_to_uint32(const char *str)
{
	char *endptr = NULL;
	errno = 0;
	unsigned long res = strtoul(str, &endptr, 10);

	if (endptr == str) {
		fprintf(stderr, "no digits in string\n");
		usage();
	}

	if ((errno == ERANGE && res == ULONG_MAX) || (errno != 0 && res == 0)) {
		perror("strtoul");
		exit(EXIT_FAILURE);
	}

	if (res > UINT32_MAX) {
		fprintf(stderr, "value is too high\n");
		exit(EXIT_FAILURE);
	}

	return (uint32_t)res;
}

static uint64_t str_to_uint64(const char *str)
{
	char *endptr = NULL;
	errno = 0;
	unsigned long res = strtoul(str, &endptr, 10);

	if (endptr == str) {
		fprintf(stderr, "no digits in string\n");
		usage();
	}

	if ((errno == ERANGE && res == ULONG_MAX) || (errno != 0 && res == 0)) {
		perror("strtoul");
		exit(EXIT_FAILURE);
	}

	if (res > UINT64_MAX) {
		fprintf(stderr, "value is too high\n");
		exit(EXIT_FAILURE);
	}

	return (uint64_t)res;
}

uint32_t u8to32(const uint8_t from[4]) {
	uint32_t to = 0;
	for (int i = 0; i < 4; i++) {
		to = (to << 8) | from[i];
	}
	return to;
}

uint16_t u8to16(const uint8_t from[2]) {
	uint32_t to = from[0];
	to = (to << 8) | from[1];
	return to;
}

void load_file_bin(const char *path, uint8_t *data, uint32_t size)
{
	int fd = open(path, O_RDONLY);
	
	struct stat stat;
	fstat(fd, &stat);

	if (stat.st_size > size) {
		fprintf(stderr, "Not enough memory\n");
		exit(EXIT_FAILURE);
	}

	read(fd, data, stat.st_size);
	close(fd);
}

struct simulator {
	uint32_t cur_pc;
	uint32_t jump_addr;
	bool jump;
	uint32_t reg[32];
	uint8_t *dmem;
	uint8_t *imem;
	uint32_t dmem_size;
	uint32_t imem_size;
};

uint32_t sll(uint32_t rt, uint32_t rs)
{
	return rt << (rs % 32);
}

uint32_t srl(uint32_t rt, uint32_t rs)
{
	return rt >> (rs % 32);
}

uint32_t sra(uint32_t rt, uint32_t rs)
{
	uint32_t sign = rt & 0x80000000;
	uint32_t res = rt >> (rs % 32);
	for (uint8_t i = 0; i < (rs % 32); i++) {
		res |= sign;
		sign >>= 1;
	}
	return res;
}

uint32_t slt(uint32_t rs, uint32_t rt)
{
	int32_t srs;
	if (rs < 0x80000000) {
		srs = rs;
	} else {
		rs = (~rs) + 1;
		srs = -rs;
	}

	int32_t srt;
	if (rt < 0x80000000) {
		srt = rt;
	} else {
		rt = (~rt) + 1;
		srt = -rt;
	}

	return (srs < srt) ? 1 : 0;
}

uint32_t sign_b(uint8_t byte)
{
	if (byte < 0x80)
		return byte;
	return 0xFFFFFF00 | byte;
}

uint32_t sign_h(uint16_t half)
{
	if (half < 0x8000)
		return half;
	return 0xFFFF0000 | half;
}

static bool is_eof = false;

uint32_t lb(struct simulator *sim, uint32_t addr)
{
	if (addr == UART_STATUS) {
		return 0x03; /* always ready */
	}

	if (addr == UART_DATA) {
		if (is_eof)
			return 1;

		int c = getchar();
		if (c == EOF) {
			is_eof = true;
			return 0;
		}
		return sign_b(c);
	}

	/* loads outside the valid range are ignored and read a zero value */
	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Reading from address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return 0;
	}

	return sign_b(sim->dmem[addr]);
}

uint32_t lbu(struct simulator *sim, uint32_t addr)
{
	if (addr == UART_STATUS) {
		return 0x03; /* always ready */
	}

	if (addr == UART_DATA) {
		if (is_eof)
			return 1;

		int c = getchar();
		if (c == EOF) {
			is_eof = true;
			return 0;
		}
		return c;
	}

	/* loads outside the valid range are ignored and read a zero value */
	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Reading from address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return 0;
	}

	return sim->dmem[addr];
}

uint32_t lh(struct simulator *sim, uint32_t addr)
{
	if (addr == UART_STATUS) {
		return 0x03; /* always ready */
	}

	if (addr == UART_DATA) {
		if (is_eof)
			return 1;

		int c = getchar();
		if (c == EOF) {
			is_eof = true;
			return 0;
		}
		return c;
	}

	/* loads outside the valid range are ignored and read a zero value */
	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Reading from address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return 0;
	}

	uint16_t b0 = sim->dmem[addr + 1];
	uint16_t b1 = sim->dmem[addr];
	b0 |= b1 << 8;
	return sign_h(b0);
}

uint32_t lhu(struct simulator *sim, uint32_t addr)
{
	if (addr == UART_STATUS) {
		return 0x03; /* always ready */
	}

	if (addr == UART_DATA) {
		if (is_eof)
			return 1;

		int c = getchar();
		if (c == EOF) {
			is_eof = true;
			return 0;
		}
		return c;
	}

	/* loads outside the valid range are ignored and read a zero value */
	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Reading from address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return 0;
	}
	
	uint16_t b0 = sim->dmem[addr + 1];
	uint16_t b1 = sim->dmem[addr];
	b0 |= b1 << 8;
	return b0;
}

uint32_t lw(struct simulator *sim, uint32_t addr)
{
	if (addr == UART_STATUS) {
		return 0x03; /* always ready */
	}

	if (addr == UART_DATA) {
		if (is_eof)
			return 1;

		int c = getchar();
		if (c == EOF) {
			is_eof = true;
			return 0;
		}
		return c;
	}

	/* loads outside the valid range are ignored and read a zero value */
	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Reading from address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return 0;
	}
	
	uint32_t b0 = sim->dmem[addr];
	uint32_t b1 = sim->dmem[addr + 1];
	uint32_t b2 = sim->dmem[addr + 2];
	uint32_t b3 = sim->dmem[addr + 3];
	b1 |= b0 << 8;
	b3 |= b2 << 8;
	return (b1 << 16) | b3;
}

void sb(struct simulator *sim, uint32_t addr, uint32_t value)
{
	if (addr == UART_DATA) {
		printf("%c", value & 0xFF);
		fflush(stdout);
		return;
	}

	if (addr == UART_STATUS) {
		return; /* ignored */
	}

	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Writing to address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return;
	}
	
	sim->dmem[addr] = value & 0xFF;
}

void sh(struct simulator *sim, uint32_t addr, uint32_t value)
{
	if (addr == UART_DATA) {
		printf("%c", value & 0xFF);
		fflush(stdout);
		return;
	}

	if (addr == UART_STATUS) {
		return; /* ignored */
	}

	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Writing to address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return;
	}
	
	sim->dmem[addr + 1] = value & 0xFF;
	sim->dmem[addr + 0] = (value >> 8) & 0xFF;
}

void sw(struct simulator *sim, uint32_t addr, uint32_t value)
{
	if (addr == UART_DATA) {
		printf("%c", value & 0xFF);
		fflush(stdout);
		return;
	}

	if (addr == UART_STATUS) {
		return; /* ignored */
	}

	if (addr >= sim->dmem_size) {
		fprintf(stderr, "Warning: Writing to address 0x%X (max. addr. 0x%X)\n", 
				addr, sim->dmem_size);
		return;
	}

	sim->dmem[addr + 3] = value & 0xFF;
	sim->dmem[addr + 2] = (value >> 8) & 0xFF;
	sim->dmem[addr + 1] = (value >> 16) & 0xFF;
	sim->dmem[addr + 0] = (value >> 24) & 0xFF;
}

void simulator_run(struct simulator *sim, uint64_t num_steps, bool v2)
{
	for (uint64_t i = 0; i < num_steps; i++) {
		uint32_t pc = sim->cur_pc;
		if (pc < PC_START) {
			fprintf(stdout, "Invalid pc(0x%X). Must be >= 0x%X\n", pc, PC_START);
			return;
		}
		uint32_t instr_code = u8to32(&sim->imem[pc - PC_START]);
		struct instr instr;
		memset(&instr, 0, sizeof(instr));


		/* cur_pc points to the next instruction */
		if (sim->jump) {
			sim->cur_pc = sim->jump_addr;
			sim->jump = false;
			if (v2) {
				parse_instr_v2(instr_code, &instr);
			} else {
				parse_instr(instr_code, &instr);
			}
		} else {
			if (v2) {
				if (instr_code < 0x80000000) {
					/* long instruction */
					sim->cur_pc += 4;
				} else {
					sim->cur_pc += 2;
				}
				parse_instr_v2(instr_code, &instr);
			} else {
				sim->cur_pc += 4;
				parse_instr(instr_code, &instr);
			}
		}

		int size_next_instr = 4; 
		if (v2 && sim->imem[sim->cur_pc - PC_START] >= 0x80) {
			size_next_instr = 2;
		}

		assert(instr.op < NOP);
	
		if (debug) {
			//fprintf(stdout, "%8.8X: (%8.8X) ", pc, instr_code);
			struct instr i2 = instr;
			conv_to_pseudo(&i2);
			print_instr(&i2); 
		}

		uint32_t rt = sim->reg[instr.rt];
		uint32_t rs = sim->reg[instr.rs];
		uint32_t imm = instr.imm;
		int32_t simm = instr.simm;

		if (instr.rt == 0)
			rt = 0;
	
		if (instr.rs == 0)
			rs = 0;

		switch(instr.op) {
		case SLL:
			sim->reg[instr.rd] = sll(rt, instr.shamt);
			break;

		case SRL:
			sim->reg[instr.rd] = srl(rt, instr.shamt);
			break;

		case SRA:
			sim->reg[instr.rd] = sra(rt, instr.shamt);
			break;

		case SLLV:
			sim->reg[instr.rd] = sll(rt, rs);
			break;

		case SRLV:
			sim->reg[instr.rd] = srl(rt, rs);
			break;

		case SRAV:
			sim->reg[instr.rd] = sra(rt, rs);
			break;

		case ADD: /* TODO check for overflow */
			sim->reg[instr.rd] = rt + rs;
			fprintf(stderr, "ADD: overflow not implemented\n");
			break;

		case ADDU:
			sim->reg[instr.rd] = rt + rs;
			break;

		case SUB: /* TODO check for overflow */
			sim->reg[instr.rd] = rs - rt;
			fprintf(stderr, "SUB: overflow not implemented\n");
			break;
		
		case SUBU: 
			sim->reg[instr.rd] = rs - rt;
			break;

		case AND:
			sim->reg[instr.rd] = rs & rt;
			break;

		case OR:
			sim->reg[instr.rd] = rs | rt;
			break;

		case XOR:
			sim->reg[instr.rd] = rs ^ rt;
			break;

		case NOR:
			sim->reg[instr.rd] = ~(rs | rt);
			break;

		case ADDI: /* TODO overflow */
			sim->reg[instr.rt] = rs + simm;
			fprintf(stderr, "ADDI: overflow not implemented\n");
			break;

		case ADDIU:
			sim->reg[instr.rt] = rs + simm;
			break;

		case ANDI:
			sim->reg[instr.rt] = rs & imm;
			break;

		case ORI:
			sim->reg[instr.rt] = rs | imm;
			break;

		case XORI:
			sim->reg[instr.rt] = rs ^ imm;
			break;

		case LUI:
			sim->reg[instr.rt] = imm << 16;
			break;

		case LB:
			sim->reg[instr.rt] = lb(sim, rs + simm);
			break;

		case LH:
			sim->reg[instr.rt] = lh(sim, rs + simm);
			break;

		case LW:
			sim->reg[instr.rt] = lw(sim, rs + simm);
			break;

		case LBU:
			sim->reg[instr.rt] = lbu(sim, rs + simm);
			break;

		case LHU:
			sim->reg[instr.rt] = lhu(sim, rs + simm);
			break;

		case SB:
			sb(sim, rs + simm, rt);
			break;

		case SH:
			sh(sim, rs + simm, rt);
			break;

		case SW:
			sw(sim, rs + simm, rt);
			break;

		case SLT:
			sim->reg[instr.rd] = slt(rs, rt);
			break;

		case SLTU:
			sim->reg[instr.rd] = (rs < rt) ? 1 : 0;
			break;

		case SLTI:
			sim->reg[instr.rt] = slt(rs, simm);
			break;

		case SLTIU:
			sim->reg[instr.rt] = (rs < imm) ? 1 : 0;
			break;

		case BLTZ:
			if (rs >= 0x80000000) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BGEZ:
			if (rs < 0x80000000) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BLTZAL:
			sim->reg[31] = sim->cur_pc + size_next_instr;
			if (rs >= 0x80000000) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BGEZAL:
			sim->reg[31] = sim->cur_pc + size_next_instr;
			if (rs < 0x80000000) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BEQ:
			if (rs == rt) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BNE:
			if (rs != rt) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BLEZ:
			if (rs >= 0x80000000 || rs == 0) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case BGTZ:
			if (rs < 0x80000000 && rs > 0) {
				sim->jump_addr = sim->cur_pc + simm;
				sim->jump = true;
			}
			break;

		case J:
			sim->jump_addr = (sim->cur_pc & 0xF0000000) | (instr.addr & 0x0FFFFFFF);
			sim->jump = true;
			break;

		case JAL:
			sim->jump_addr = (sim->cur_pc & 0xF0000000) | (instr.addr & 0x0FFFFFFF);
			sim->reg[31] = sim->cur_pc + size_next_instr;
			sim->jump = true;
			break;

		case JR:
			sim->jump_addr = rs;
			sim->jump = true;
			break;

		case JALR:
			sim->jump_addr = rs;
			sim->reg[instr.rd] = sim->cur_pc + size_next_instr;
			sim->jump = true;
			break;

		default:
			assert(0);
		}

		sim->reg[0] = 0; /* register 0 must always be zero */
	}
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];
	
	uint32_t imem_size  = DEFAULT_IMEM_SIZE;
	uint32_t dmem_size  = DEFAULT_DMEM_SIZE;
	uint64_t num_cycles = DEFAULT_NUM_CYCLES;

	bool v2 = false;

	const char *bin_file_path = NULL;
	const char *data_file_path = NULL;

	int opt = 0;

	while ((opt = getopt(argc, argv, "i:d:cn:x")) != -1) {
		switch (opt) {
		case 'i':
			imem_size = 1024 * str_to_uint32(optarg);
			break;

		case 'd':
			dmem_size = 1024 * str_to_uint32(optarg);
			break;

		case 'n':
			num_cycles = str_to_uint64(optarg);
			break;

		case 'x':
			debug = true;
			break;

		case 'c':
			v2 = true;
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

	if (dmem_size > PC_START) {
		fprintf(stderr, "size of data memory is too big.\n");
		exit(EXIT_FAILURE);
	}

	bin_file_path = argv[optind];

	if (optind + 1 < argc)
		data_file_path = argv[optind + 1];
	
	struct simulator sim;

	memset(&sim, 0, sizeof(sim));
	sim.cur_pc = PC_START;

	sim.imem = calloc(1, imem_size);
	sim.dmem = calloc(1, dmem_size);

	sim.imem_size = imem_size;
	sim.dmem_size = dmem_size;

	load_file_bin(bin_file_path, sim.imem, sim.imem_size);
	if (data_file_path != NULL)
		load_file_bin(data_file_path, sim.dmem + 4, sim.dmem_size - 4);
	
	simulator_run(&sim, num_cycles, v2);

	free(sim.imem);
	free(sim.dmem);

	return 0;
}

