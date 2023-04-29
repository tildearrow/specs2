all: bios
bios: bios.o
	s2ld -o bios.bin bios.o
bios.o:
	s2as -o bios.o bios.s
