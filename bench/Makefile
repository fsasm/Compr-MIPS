# Makefile for cross-compiling test programs 
# Fabjan Sukalia <fsukalia@gmail.com>
# 2016-03-21

PREFIX=mips-unknown-elf-

CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AS=$(PREFIX)as
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy

TOOLCHAIN_LIB=../../mips-unknown-elf/lib/gcc/mips-unknown-elf/5.2.0/

OPT?=-O2
CFLAGS=$(OPT) -mips1 -msoft-float -EB -ffreestanding -I./common/ -ffunction-sections -fdata-sections -Wall -Wextra -fno-jump-tables
LFLAGS=-N -T ./common/linker.ld --gc-sections -L $(TOOLCHAIN_LIB)

.PHONY: all_mif all_u_mif all_bin all_u_bin clean

all_bin: md5.bin md5.data.bin sha256.bin sha256.data.bin sha512.bin sha512.data.bin mandelbrot.bin mandelbrot.data.bin hello.bin hello.data.bin echo.bin echo.data.bin qsort.bin qsort.data.bin calc.bin calc.data.bin lz4_dec.bin lz4_dec.data.bin lz4_comp.bin lz4_comp.data.bin

# unrolled
all_u_bin: md5_u.bin md5_u.data.bin sha256_u.bin sha256_u.data.bin sha512_u.bin sha512_u.data.bin 

all_mif: md5.imem.mif md5.dmem.mif sha256.imem.mif sha256.dmem.mif sha512.imem.mif sha512.dmem.mif mandelbrot.imem.mif mandelbrot.dmem.mif hello.imem.mif hello.dmem.mif echo.imem.mif echo.dmem.mif qsort.imem.mif qsort.dmem.mif calc.imem.mif calc.dmem.mif lz4_dec.imem.mif lz4_dec.dmem.mif lz4_comp.imem.mif lz4_comp.dmem.mif

# unrolled
all_u_mif: md5_u.imem.mif md5_u.dmem.mif sha256_u.imem.mif sha256_u.dmem.mif sha512_u.imem.mif sha512_u.dmem.mif 

md5.elf: md5.o md5_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

md5_test.o: hash_test.c
	$(CC) $(CFLAGS) -DHASH_MD5 -c -o $@ $<

md5_u.elf: md5_u.o md5_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

md5_u.o: md5.c
	$(CC) $(CFLAGS) -DUNROLLED -c -o $@ $<

sha256.elf: sha256.o sha256_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

sha256_test.o: hash_test.c
	$(CC) $(CFLAGS) -DHASH_SHA256 -c -o $@ $<

sha256_u.elf: sha256_u.o sha256_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

sha256_u.o: sha256.c
	$(CC) $(CFLAGS) -DUNROLLED -c -o $@ $<

sha512.elf: sha512.o sha512_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

sha512_test.o: hash_test.c
	$(CC) $(CFLAGS) -DHASH_SHA512 -c -o $@ $<

sha512_u.elf: sha512_u.o sha512_test.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

sha512_u.o: sha512.c
	$(CC) $(CFLAGS) -DUNROLLED -c -o $@ $<

lz4_comp.elf: lz4.o lz4_comp.o libc.a
	$(LD) $(LFLAGS) -o $@ $^ -lgcc

lz4_dec.elf: lz4.o lz4_dec.o libc.a
	$(LD) $(LFLAGS) -o $@ $^ -lgcc

%.mif: %.hex
	./hex2mif.pl < $< > $@

%.imem.hex: %.elf
	$(OBJCOPY) -j .text -O ihex $< $@

%.dmem.hex: %.elf
	$(OBJCOPY) -R .text -R .MIPS.abiflags -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -O binary $< $@

%.data.bin: %.elf
	$(OBJCOPY) -R .text -R .MIPS.abiflags -O binary $< $@

%.elf: %.o libc.a
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.s
	$(AS) -msoft-float -o $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

libc.a: ./common/crt0.o ./common/stdio.o ./common/string.o ./common/mul.o ./common/div.o ./common/fix16.o
	$(AR) rc $@ $^

clean:
	rm -f *.o *.bin *.data.bin *.elf libc.a
	rm -f ./common/*.o
	rm -f *.imem.mif *.dmem.mif
