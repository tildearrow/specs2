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

#endif
