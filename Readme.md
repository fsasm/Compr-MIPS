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

## Results
Compiled with `-O2` and GCC 5.2.0

| test       | u c | compr. rate | compr. size | uncompr. size | compr. bw | uncompr. bw | bw rate |
|------------|-----|-------------|-------------|---------------|-----------|-------------|---------|
| hello      | s s |      64.1 % |         100 |           156 |       454 |         672 |  67.6 % |
| md5        | s s |      73.7 % |        1454 |          1972 |    129760 |      185572 |  69.9 % |
| md5_u      | s s |      70.2 % |        4328 |          6164 |     96634 |      136732 |  70.7 % |
| qsort      | s s |      72.1 % |         600 |           832 |    211198 |      278072 |  76.0 % |
| sha256     | s s |      79.3 % |        1444 |          1820 |    207080 |      273740 |  75.6 % |
| sha512     | s s |      82.5 % |        2290 |          2776 |    561662 |      714172 |  78.6 % |
| calc       | s s |      72.2 % |        2416 |          3348 |     70244 |       95804 |  73.3 % |
| echo       | s s |      64.7 % |         282 |           436 |    559752 |      851904 |  65.7 % |
| lz4_comp   | s s |      73.8 % |        6314 |          8552 |  15592456 |    21969488 |  71.0 % |
| lz4_dec    | s s |      72.6 % |        2322 |          3200 |   8075722 |    11645656 |  69.3 % |

Compiled with `-Os` and GCC 5.2.0

| test       | u c | compr. rate | compr. size | uncompr. size | compr. bw | uncompr. bw | bw rate |
|------------|-----|-------------|-------------|---------------|-----------|-------------|---------|
| hello      | s s |      62.3 % |         132 |           212 |       664 |        1080 |  61.5 % |
| md5        | s s |      70.0 % |        1220 |          1744 |    141506 |      211104 |  67.0 % |
| md5_u      | s s |      69.5 % |        3082 |          4432 |     72260 |      104756 |  69.0 % |
| qsort      | s s |      72.4 % |         614 |           848 |    200992 |      268176 |  74.9 % |
| sha256     | s s |      76.2 % |        1326 |          1740 |    218034 |      291476 |  74.8 % |
| sha512     | s s |      80.2 % |        2086 |          2600 |    538878 |      715344 |  75.3 % |
| calc       | s s |      68.7 % |        2178 |          3172 |     78726 |      112252 |  70.1 % |
| echo       | s s |      63.3 % |         276 |           436 |    523528 |      827940 |  63.2 % |
| lz4_comp   | s s |      73.3 % |        5070 |          6920 |  18961100 |    27808396 |  68.2 % |
| lz4_dec    | s s |      73.7 % |        2180 |          2956 |   9077186 |    13339732 |  68.0 % |

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