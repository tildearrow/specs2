#ifndef _S2SYSTEM_H
#define _S2SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"
#include "core.h"
#include "video.h"
#include "sound.h"

#define S2EMU_VERSION "dev0"

typedef struct {
  s2CPU cpu;
  s2Core core;
  s2VideoUnit vu;
  s2SoundUnit su;

  unsigned char* memory;
  unsigned int memCapacity;

  unsigned short* frame;
  int framePos;
  unsigned int frameCyclesCur, frameCycles;
  short frameSyncCycles, prevFrameSyncCycles;
  unsigned short vuOutput;
  short suOutL, suOutR;
} s2System;

void systemInit(s2System* sys, unsigned int memCapacity);
void systemAdvance(s2System* sys, unsigned int cycles);
void systemReset(s2System* sys);
void systemQuit(s2System* sys);

#ifdef __cplusplus
};
#endif

#endif
