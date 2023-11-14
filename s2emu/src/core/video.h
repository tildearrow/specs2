#ifndef _S2VIDEO_H
#define _S2VIDEO_H

#include "core.h"

#define VU_SYNC 0x0010
#define VU_HOLD 0x0200

typedef struct {
  unsigned short count;
  unsigned char width;
  unsigned char y;
} ReadySprite;

struct s2VideoUnit {
  // internal counters
  unsigned int initAddr;

  unsigned short vcount;
  unsigned short hcount;

  unsigned short vpos;
  unsigned short hpos;

  unsigned char vmode;
  unsigned char hmode;
  unsigned char vslatch;

  unsigned char divider;

  unsigned char bitmap;
  unsigned char syncTrigger;

  // tile mode
  unsigned int tileX[3];
  unsigned int tileY[3];

  ReadySprite sprite[8];

  // bitmap mode
  unsigned int bptr;
  unsigned int bvptr;
  unsigned char bshift;
  unsigned char bfmt;

  unsigned char* mem;
  unsigned int memCapacity;
  unsigned int memMask;

  unsigned short toggle;
};

typedef struct s2VideoUnit s2VideoUnit;

void vuInit(s2VideoUnit* vu, unsigned int memCapacity);

void vuReset(s2VideoUnit* vu, unsigned short toggle);

// outputs a color. if chroma is -16, output sync signal.
unsigned short vuClock(s2VideoUnit* vu);

bool vuRead8(s2VideoUnit* vu, unsigned int addr, unsigned char* out);
bool vuRead16(s2VideoUnit* vu, unsigned int addr, unsigned short* out);
bool vuRead32(s2VideoUnit* vu, unsigned int addr, unsigned int* out);
bool vuWrite8(s2VideoUnit* vu, unsigned int addr, unsigned char val);
bool vuWrite16(s2VideoUnit* vu, unsigned int addr, unsigned short val);
bool vuWrite32(s2VideoUnit* vu, unsigned int addr, unsigned int val);

#endif
