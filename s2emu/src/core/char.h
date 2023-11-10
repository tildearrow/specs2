#ifndef _S2CHAR_H
#define _S2CHAR_H

#include "core.h"

#define CU_SYNC 0x0010
#define CU_HOLD 0x0200

typedef struct {
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
  signed char curRow, curCol;
  short pauseX, pauseY;

  unsigned short color[256];

  unsigned char* font;

  unsigned char* mem;
} s2CharUnit;

void cuInit(s2CharUnit* cu);

void cuSetPalette(s2CharUnit* cu, unsigned short* pal);
void cuSetFont(s2CharUnit* cu, unsigned char* font);

void cuReset(s2CharUnit* cu, unsigned short toggle);

// outputs a color. if chroma is -16, output sync signal.
unsigned short cuClock(s2CharUnit* cu);

#endif
