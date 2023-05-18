# Compressed MIPS-I
A research project to add compressed instructions to MIPS-I.

## History
The idea for this project was created at the beginning of 2016 but the real work
began in summer 2016, which is roughly at the start of the hype around 
RISC-V. The idea on adding compressed instructions to the MIPS architecture was 
greatly inspired by the C-extension from RISC-V. Originally it was meant to be a 
bachelor thesis but quite some things got in the way and this project got 
stalled. 

## Idea
The main idea is to add few additional instruction formats that are 16 bits long.
The original 32-bit instruction format are modified so that they include a bit
to distinguish between 16-bit and 32-bit format. As this project only concentrates
on the integer part and leaves out the floating point and privileged parts,
only some adjustment are needed.

With the help of some tools the most used instructions were selected and
transformed into a 16-bit format. Like the C-extension from RISC-V every 16-bit
instruction has a 32-bit instruction equivalent.

As a proof-of-concept a simple processor core was developed in VHDL for the
Terasic Cyclone V GX Starter Kit. This processor supports both instructions sets
and should show that the additional
effort to support both instructions sets is negligible, especially in the
decode unit. The processor core is not part of this repository. 

## Instruction format
The original MIPS-I ISA uses three instruction formats, R-, I- and J-Type.
All three instruction formats are 32-bit long and have a 6-bit opcode field in 
the top bits. The new proposed instruction format are either 16 or 32-bit long.
The 32-bit format have a 5-bit opcode field and the top bit is used to distinguish
between 32-bit and 16-bit format. In a different view the most significant bit 
of the 6-bit opcode field is used to distinguish between 32-bit and 16-bit format.

Besides the smaller opcode field, the new and the old 32-bit instruction format
are the same. Also the encodings of the instructions are the same, except for
the memory access instructions. The memory instructions have the most significant
bit of the opcode field set, which collides with the new instruction format, where
this bit indicates a 16-bit instruction format. So only for these instructions
the opcode value were changed. Note that this work does not account for floating-point
and other co-processor instructions, where further problems arise with the smaller
opcode field. 

Another difference are branch and jump instructions. These
instructions assume that all instructions are 32-bit long and therefore multiply
the offset by 4. With 16-bit instructions this assumption does not hold and the
branch offset have to be multiplied by 2. This also means that the range of the
branch instructions is halved unless another format is created that provides
an additional bit to the offset field. In practise it is rare that the branch
offset is so large.

There are three 16-bit instruction formats. All three have the top bit for the
instruction size and the 5-bit opcode field. The destination register `rd` is
also the first source register `rs`. The analysis of the toy benchmark showed
that many instructions reused a source register as a destination register and this
instruction formats make use of this fact. Also many branches have a short offset
because there are used for constructs like an if-then-else block. The offset
field is multiplied by 2 which means that the effective range is [-1024; +1022]
bytes.

```
     +----------+------------+-----------+---------+
R16: | size (1) | opcode (5) | rd/rs (5) |  rt (5) |
     +----------+------------+-----------+---------+
I16: | size (1) | opcode (5) | rd/rs (5) | imm (5) |
     +----------+------------+-----------+---------+
J16: | size (1) | opcode (5) |      offset (10)    |
     +----------+------------+-----------+---------+
```

Every instruction in a 16-bit format has an equivalent in a 32-bit format. With
this restriction every 16-bit instruction can easily be converted to a 32-bit
instruction. For processor designs this means, that they only need small
modifications to the decode stage and can reuse most parts of the decoder.

A list of instructions and how they are encoded into the 16-bit formats is in
`common/v2_instr.c`.

## Results
Compiled with `-O2` and GCC 5.2.0

| test     | compr. size | uncompr. size | compr. rate | compr. bw | uncompr. bw | bw rate |
|----------|-------------|---------------|-------------|-----------|-------------|---------|
| hello    |         100 |           156 |      64.1 % |       454 |         672 |  67.6 % |
| md5      |        1454 |          1972 |      73.7 % |    129760 |      185572 |  69.9 % |
| md5_u    |        4328 |          6164 |      70.2 % |     96634 |      136732 |  70.7 % |
| qsort    |         600 |           832 |      72.1 % |    211198 |      278072 |  76.0 % |
| sha256   |        1444 |          1820 |      79.3 % |    207080 |      273740 |  75.6 % |
| sha512   |        2290 |          2776 |      82.5 % |    561662 |      714172 |  78.6 % |
| calc     |        2416 |          3348 |      72.2 % |     70244 |       95804 |  73.3 % |
| echo     |         282 |           436 |      64.7 % |    559752 |      851904 |  65.7 % |
| lz4_comp |        6314 |          8552 |      73.8 % |  15592456 |    21969488 |  71.0 % |
| lz4_dec  |        2322 |          3200 |      72.6 % |   8075722 |    11645656 |  69.3 % |

Compiled with `-Os` and GCC 5.2.0

| test     | compr. size | uncompr. size | compr. rate | compr. bw | uncompr. bw | bw rate |
|----------|-------------|---------------|-------------|-----------|-------------|---------|
| hello    |         132 |           212 |      62.3 % |       664 |        1080 |  61.5 % |
| md5      |        1220 |          1744 |      70.0 % |    141506 |      211104 |  67.0 % |
| md5_u    |        3082 |          4432 |      69.5 % |     72260 |      104756 |  69.0 % |
| qsort    |         614 |           848 |      72.4 % |    200992 |      268176 |  74.9 % |
| sha256   |        1326 |          1740 |      76.2 % |    218034 |      291476 |  74.8 % |
| sha512   |        2086 |          2600 |      80.2 % |    538878 |      715344 |  75.3 % |
| calc     |        2178 |          3172 |      68.7 % |     78726 |      112252 |  70.1 % |
| echo     |         276 |           436 |      63.3 % |    523528 |      827940 |  63.2 % |
| lz4_comp |        5070 |          6920 |      73.3 % |  18961100 |    27808396 |  68.2 % |
| lz4_dec  |        2180 |          2956 |      73.7 % |   9077186 |    13339732 |  68.0 % |

## Structure
* `analyzer/`: analyzes program code and prints statistics the instruction distribution.
* `bench/`: toy benchmark programs that can run on bare-metal CPUs
* `common/`: collection of functions and data structures that are used by multiple tools
* `converter/`: converts program code that uses the old uncompressed format into program code that used the new compressed format
* `disas/`: simple disassembler that can be helpfull during debugging
* `simulator/`: simulator for both instructions format
* `uart_escape/`: encodes binary data so that it does not interfere with control characters

The toy benchmark consists of the following programs:
* `calc`: calculator for simple mathematical terms. It contains fixed point arithmetic and parsing workload.
* `echo`: copies the input to the output
* `hello`: prints "Hello World!"
* `lz4_comp`, `lz4_dec`: compressor and decompressor that use the lz4 compression algorithm
* `mandelbrot`: calculates the mandelbrot fractal. It contains heavy fixed point arithmetic workload.
* `qsort`: quick sort 
* `md5`, `sha1`, `sha256`, `sha512`: calculates the cryptographic hash value of the data block. `sha512` uses 64-bit arithmetic

## How to compile
For the tools only a C99 compiler with POSIX extension is needed. To compile
just execute `make` in every tool folder. The toy benchmark needs a cross-compiler
that targets bare-metal MIPS-I and not some hosted target. GCC will have the
`mips-unknown-elf` prefix when it targets bare-metal MIPS-I. The toy benchmark
was tested with GCC 5.2.0 from crosstool-NG and probably newer version will also
work. Please read the documentation of crosstool-NG to learn on how to create 
the correct cross-compiler. 

## Drawbacks
During research and development some problems were found. First the opcode
space is more than half full which means that shrinking the opcode field by
one bit makes problems. Mostly the co-processor instructions make problems
and they would need a redesign.

Second, the compilers assume that all instructions are 32-bit long and create
constructs that build upon this assumption. One such construct is the jump table
that is used for switch statements. GCC has a flag that prevents the generation
of jump tables. 
