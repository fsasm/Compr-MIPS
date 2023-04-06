
MEMORY
{
	ram (!rx) : ORIGIN = 0x0004, LENGTH = 0x3FFC
	rom (rx) : ORIGIN = 0x40000000, LENGTH = 0x10000
}

OUTPUT_ARCH(mips)

ENTRY(_start)
SEARCH_DIR(./common)
/* STARTUP(crt0.o) */

SECTIONS
{
	/* force that the first data byte is at address 4 */
	. = 0x04;
	.data . : 
	{
		*(.data*)
		*(.rodata*)
		*(.bss*)
		*(.sbss*)
	} >ram

	_gp = .;
	__stack_start = 0x4000;

	/*. = 0x40000000;*/
	.text : 
	{
		crt0.o(.text) 
		*(.text*)
	} >rom
}

