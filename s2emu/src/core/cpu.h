#ifndef _S2CPU_H
#define _S2CPU_H

#include "core.h"

typedef union {
  unsigned char c;
  unsigned short s;
  unsigned int i;
} s2CPURegister;

typedef struct {
  s2Core* core;
  unsigned char curIns[8];
  unsigned char nextIns[8];

  s2CPURegister reg[8];

  unsigned int pc; // program counter
  unsigned int sp; // stack pointer
  unsigned int st; // stack top
  unsigned int sb; // stack bottom
  unsigned int cycleTotal;
  unsigned int cc; // cycle counter
  unsigned int ev; // exception vector
  unsigned int iv; // interrupt vector table pointer
  unsigned char flags, ex, ir, initFetch, halted, debug;

  // division state
  int divN; // numerator
  int divD; // denominator
  int divQ; // result
  int divR; // remainder
  // TODO: add division state registers
  unsigned char divBusy;

  unsigned char cycles;

  struct {
    s2CPURegister reg[8];
    unsigned int sp, st, sb;
    unsigned char flags;
  } ctx[5];
} s2CPU;

void cpuInit(s2CPU* cpu, s2Core* core);

void cpuReset(s2CPU* cpu);

int cpuDisAsm(const unsigned char* ptr, char* str, unsigned int len, unsigned int pc);
void cpuDumpState(s2CPU* cpu);
void cpuClock(s2CPU* cpu);

#endif
