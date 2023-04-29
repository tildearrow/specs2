#ifndef _S2SYSTEM_H
#define _S2SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"
#include "core.h"

#define S2EMU_VERSION "dev0"

typedef struct {
  s2CPU cpu;
  s2Core core;
  unsigned char* memory;
  unsigned int memCapacity;
} s2System;

void systemInit(s2System* sys, unsigned int memCapacity);
void systemAdvance(s2System* sys, unsigned int cycles);
void systemReset(s2System* sys);
void systemQuit(s2System* sys);

#ifdef __cplusplus
};
#endif

#endif
