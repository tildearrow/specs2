#ifndef _S2CHAR_H
#define _S2CHAR_H

#include "core.h"

#define CU_SYNC 0x0010
#define CU_HOLD 0x0200

struct s2CharUnit {
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

  unsigned char syncTrigger;

  unsigned char toggle;

  // character
  unsigned char textX, textY;
  unsigned char charX, charY;
  short pauseX, pauseY;

  unsigned short color[256];

  unsigned char* font;

  unsigned char* mem;
};

typedef struct s2CharUnit s2CharUnit;

void cuInit(s2CharUnit* cu);

void cuSetPalette(s2CharUnit* cu, unsigned short* pal);
void cuSetFont(s2CharUnit* cu, unsigned char* font);

void cuReset(s2CharUnit* cu, unsigned short toggle);

// outputs a color. if chroma is -16, output sync signal.
unsigned short cuClock(s2CharUnit* cu);

bool cuRead8(s2CharUnit* cu, unsigned short addr, unsigned char* out);
bool cuRead16(s2CharUnit* cu, unsigned short addr, unsigned short* out);
bool cuRead32(s2CharUnit* cu, unsigned short addr, unsigned int* out);
bool cuWrite8(s2CharUnit* cu, unsigned short addr, unsigned char val);
bool cuWrite16(s2CharUnit* cu, unsigned short addr, unsigned short val);
bool cuWrite32(s2CharUnit* cu, unsigned short addr, unsigned int val);

#endif
