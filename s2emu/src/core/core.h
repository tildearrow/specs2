#ifndef _S2CORE_H
#define _S2CORE_H

#include <stdbool.h>

typedef struct {
  unsigned char* mem;
  unsigned int memCapacity;
  unsigned char* bios;
  unsigned short biosLen;
  unsigned char irq;
  unsigned char ex;
  unsigned char cpuClock;
  unsigned char suClock;

  unsigned char isl[256];
  unsigned short pageTable[16384];

  // state
  unsigned short biosPos;

  bool clockCPU;
  bool clockSU;
} s2Core;

void coreInit(s2Core* core);

void coreReset(s2Core* core);
void coreSetMemory(s2Core* core, unsigned char* memory, unsigned int capacity);
void coreSetBIOS(s2Core* core, unsigned char* src, unsigned short len);

void coreClock(s2Core* core);

bool coreReadCtrl(s2Core* core, unsigned int addr, unsigned char* out);
bool coreWriteCtrl(s2Core* core, unsigned int addr, unsigned char val);

bool coreRead8(s2Core* core, unsigned int addr, unsigned char* out);
bool coreRead16(s2Core* core, unsigned int addr, unsigned short* out);
bool coreRead32(s2Core* core, unsigned int addr, unsigned int* out);
bool coreReadIns(s2Core* core, unsigned int addr, unsigned char* out);

bool coreWrite8(s2Core* core, unsigned int addr, unsigned char val);
bool coreWrite16(s2Core* core, unsigned int addr, unsigned short val);
bool coreWrite32(s2Core* core, unsigned int addr, unsigned int val);

unsigned char coreGetIRQ(s2Core* core);
unsigned char coreGetException(s2Core* core);

unsigned char coreRaiseIRQ(s2Core* core, unsigned char irq);

#endif
