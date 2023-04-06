# Start file for programms

.set noreorder

.extern main

.text
.balign 4
.global _start
.ent _start
.type _start, %function

_start:
	nop
	la $gp, _gp
	la $sp, __stack_start
	jal main
	nop

# special instruction for the simulator to stop
	break
	nop

.Lloop:
	j .Lloop
	nop

.end _start
.size _start, .-_start
