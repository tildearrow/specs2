#ifndef _S2CORE_H
#define _S2CORE_H

#include <stdbool.h>

typedef struct s2CharUnit s2CharUnit;
typedef struct s2VideoUnit s2VideoUnit;
typedef struct s2SoundUnit s2SoundUnit;

typedef struct {
  s2CharUnit* charUnit;
  s2VideoUnit* videoUnit;
  s2SoundUnit* soundUnit;
  unsigned char* mem;
  unsigned int memCapacity;
  unsigned char* bios;
  unsigned short biosLen;
  unsigned char irq;
  unsigned char ex;
  unsigned char cpuClock;
  unsigned char suBusClock;
  unsigned char suClock;
  bool clockCPU;
  bool clockSU;

  // state
  unsigned short biosPos;
  signed char curPage;
  unsigned char suAddr;
  unsigned char suData;
  // 0: nothing
  // 1: write
  // 2: read
  unsigned char suNextOp;

  unsigned char isl[256];
  unsigned short pageTable[4][4096];
} s2Core;

void coreInit(s2Core* core);

void coreReset(s2Core* core);
void coreSetMemory(s2Core* core, unsigned char* memory, unsigned int capacity);
void coreSetBIOS(s2Core* core, unsigned char* src, unsigned short len);
void coreBind(s2Core* core, s2CharUnit* cu, s2VideoUnit* vu, s2SoundUnit* su);

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
