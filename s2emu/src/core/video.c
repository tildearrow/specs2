#include "video.h"
#include <stdlib.h>
#include <string.h>

enum s2VideoReg {
  v_STATUS  =0x00,
  v_VPOS    =0x02,
  v_HPOS    =0x04,
  v_VBLCNT  =0x06,
  v_INTC    =0x07,
  v_INTH    =0x08,
  v_INTV    =0x0a,
  v_CMD     =0x0c,
  v_VCACNT  =0x10,
  v_VCAHBL  =0x12,
  v_VCAHBP  =0x13,
  v_VCABLU  =0x14,
  v_VCABLL  =0x15,
  v_VCAACT  =0x16,
  v_VCHSY0  =0x18,
  v_VCHSY1  =0x19,
  v_VCHSZ0  =0x1a,
  v_VCHSZ1  =0x1c,
  v_VCTRL   =0x1e,
  v_EXTSYN  =0x1f,
  v_SCRX0   =0x20,
  v_SCRY0   =0x22,
  v_SCRX1   =0x24,
  v_SCRY1   =0x26,
  v_SCRX2   =0x28,
  v_SCRY2   =0x2a,
  v_BMPFMT  =0x2c,
  v_BMPPTR  =0x2e,
  v_TILSET  =0x30,
  v_TILCOL  =0x31,
  v_TILSIZ  =0x32,
  v_TILP0   =0x34,
  v_TILP1   =0x35,
  v_TILP2   =0x36,
  v_TILNEG  =0x38,
  v_COLBK   =0x3a,
  v_SETADD  =0x3d,
  v_PRGCNT  =0x3e,
  v_COLS0   =0x40,
  v_COLS1   =0x50,
  v_COLS2   =0x60,
  v_COLS3   =0x70,
  v_COLS4   =0x80,
  v_COLS5   =0x90,
  v_COLS6   =0xa0,
  v_COLS7   =0xb0,
  v_COL4    =0xc0,
  v_COL2    =0xe0,
  v_TILHX0  =0xf0,
  v_TILHY0  =0xf1,
  v_TILVX0  =0xf2,
  v_TILVY0  =0xf3,
  v_TILHX1  =0xf4,
  v_TILHY1  =0xf5,
  v_TILVX1  =0xf6,
  v_TILVY1  =0xf7,
  v_TILHX2  =0xf8,
  v_TILHY2  =0xf9,
  v_TILVX2  =0xfa,
  v_TILVY2  =0xfb,
  v_TILSH0  =0xfc,
  v_TILSH1  =0xfd,
  v_TILSH2  =0xfe,
  v_MASTER  =0x100,
  v_COLT    =0x300,
  v_SPRITE  =0x400,
};

// format: width pre-h|post-h up|down height sy0|sy1 sz0 sz1 flags
unsigned short videoParams[2][2][8]={
  // PAL
  {
    {800, 58|(24<<8), 23|(10<<8), 576, 70|(140<<8), 882, 812, 0x00},
    {800, 58|(24<<8), 13|(4<<8), 288, 70|(140<<8), 882, 812, 0x0c},
  },
  // NTSC
  {
    {800, 58|(24<<8), 23|(6<<8), 480, 100|(200<<8), 882, 782, 0x00},
    {800, 58|(24<<8), 13|(2<<8), 240, 100|(200<<8), 882, 782, 0x0c},
  }
};

unsigned char shiftTable[4][8]={
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 4, 0, 4, 0, 4, 0, 4},
  {0, 2, 4, 6, 0, 2, 4, 6},
  {0, 1, 2, 3, 4, 5, 6, 7},
};

unsigned char shiftAndTable[4]={
  255, 15, 3, 1
};

void vuInit(s2VideoUnit* vu, unsigned int memCapacity) {
  vu->memCapacity=memCapacity;
  vu->memMask=memCapacity-1;
  vu->mem=malloc(memCapacity);

  vuReset(vu,0);
}

// toggles:
// 1: TV mode (15KHz interlaced)
// 0: NTSC/PAL (NTSC if on)
void vuReset(s2VideoUnit* vu, unsigned short toggle) {
  vu->initAddr=vu->memCapacity;
  vu->vcount=1;
  vu->hcount=1;
  vu->vpos=0;
  vu->hpos=0;
  vu->vmode=3;
  vu->hmode=3;
  vu->vslatch=0;
  vu->divider=0;
  vu->bitmap=0;
  for (int i=0; i<3; i++) {
    vu->tileX[i]=0;
    vu->tileY[i]=0;
  }
  for (int i=0; i<8; i++) {
    vu->sprite[i].count=0;
    vu->sprite[i].width=0;
    vu->sprite[i].y=0;
  }
  vu->bptr=0;
  vu->bvptr=0;
  vu->bshift=0;
  vu->bfmt=0;
  vu->toggle=toggle;
}

#define VU_USHORT(_x) (*((unsigned short*)(&vu->mem[_x])))
#define VU_UCHAR(_x) (vu->mem[_x])
#define VU_COLOR(_x) (((unsigned short*)&vu->mem[v_MASTER])[_x])

unsigned short vuClockSync(s2VideoUnit* vu) {
  if (vu->syncTrigger) {
    vu->syncTrigger=false;
  }
  if (vu->hmode) {
    return 0;
  }

  return VU_SYNC;
}

unsigned short vuClockActive(s2VideoUnit* vu) {
  if (vu->syncTrigger) {
    vu->syncTrigger=false;
    vu->divider=0;
    if (!(VU_UCHAR(v_BMPFMT)&2)) {
      if (vu->vpos&1) {
        vu->bptr=vu->bvptr;
      } else {
        vu->bvptr=vu->bptr;
      }
    }
  }

  if (vu->hmode==0) {
    return VU_SYNC;
  }
  if (vu->hmode&2) return 0;

  if (++vu->divider>(VU_UCHAR(v_VCTRL)&3)) {
    vu->divider=0;
  } else {
    return VU_HOLD;
  }

  vu->hpos++;

  // output next
  if (vu->bitmap) {
    unsigned char data=vu->mem[(vu->bptr>>vu->bshift)&vu->memMask];
    data>>=shiftTable[vu->bshift][vu->bptr&((1<<vu->bshift)-1)];
    data&=shiftAndTable[vu->bshift];

    const unsigned char palette=VU_UCHAR(v_BMPFMT)&0x70;
    switch (vu->bfmt) {
      case 1: // 16-color
        data=VU_UCHAR(v_COLS0|palette|data);
        break;
      case 2: // 4-color
        data=VU_UCHAR(v_COL4|(palette>>2)|data);
        break;
      case 3: // 2-color
        data=VU_UCHAR(v_COL2|(palette>>3)|data);
        break;
    }

    vu->bptr++;
    return VU_COLOR(data);
  } else {
    // run tiles
    for (int i=0; i<3; i++) {

    }

    // run sprites

    // output
    return VU_USHORT(v_COLBK);
  }

  return 0;
}

unsigned short vuClockBlank(s2VideoUnit* vu) {
  if (vu->syncTrigger) {
    vu->syncTrigger=false;
  }
  if (!vu->hmode) {
    if (vu->vmode==2) {
      vu->bitmap=VU_USHORT(v_BMPPTR)?1:0;
      vu->bshift=(VU_UCHAR(v_BMPFMT)&0x0c)>>2;
      vu->bptr=VU_UCHAR(v_BMPPTR)<<(8+vu->bshift);
      vu->bvptr=vu->bptr;
    }
    return VU_SYNC;
  }
  return 0;
}

unsigned short vuClock(s2VideoUnit* vu) {
  if (vu->initAddr) {
    vu->mem[--vu->initAddr]=0;
    if (!vu->initAddr) {
      // initialize video timings
      memcpy(vu->mem+0x10,videoParams[vu->toggle&1][(vu->toggle&2)>>1],16);
    }
    return VU_SYNC;
  }

  // clock
  if (!--vu->hcount) {
    if (vu->vmode==0) {
      vu->hmode^=2;
      if (!vu->hmode) {
        if (!--vu->vcount) {
          if (++vu->vslatch==3) {
            vu->vslatch=0;
            vu->vmode=2;
          }
        }
      }
    } else {
      if (vu->hmode&2) {
        vu->hmode^=3;
      } else {
        vu->hmode^=2;
      }

      if (!vu->hmode) {
        if (!--vu->vcount) {
          if (vu->vmode&2) {
            vu->vmode^=3;
          } else {
            vu->vmode^=2;
          }
        }
      }
    }
  }

  if (!vu->vcount) {
    switch (vu->vmode) {
      case 0:
        vu->vcount=5+(!vu->vslatch?1:0); // always
        vu->vpos=0xffff;
        break;
      case 1:
        vu->vcount=VU_USHORT(v_VCAACT);
        break;
      case 2:
        vu->vcount=VU_UCHAR(v_VCABLU);
        break;
      case 3:
        vu->vcount=VU_UCHAR(v_VCABLL);
        break;
    }
  }
  if (!vu->hcount) {
    if (vu->vmode==0) {
      if (vu->vslatch&1) {
        if (vu->hmode) {
          vu->hcount=VU_UCHAR(v_VCHSY1);
        } else {
          vu->hcount=VU_USHORT(v_VCHSZ1);
        }
      } else {
        if (vu->hmode) {
          vu->hcount=VU_USHORT(v_VCHSZ0);
        } else {
          vu->hcount=VU_UCHAR(v_VCHSY0);
        }
      }
    } else {
      switch (vu->hmode) {
        case 0:
          vu->hcount=VU_UCHAR(v_VCHSY0);
          vu->hpos=0xffff;
          vu->syncTrigger=true;
          if (vu->vmode==1) vu->vpos++;
          break;
        case 1:
          vu->hcount=VU_USHORT(v_VCACNT);
          break;
        case 2:
          vu->hcount=VU_UCHAR(v_VCAHBL);
          break;
        case 3:
          vu->hcount=VU_UCHAR(v_VCAHBP);
          break;
      }
    }
  }
  
  // output
  switch (vu->vmode) {
    case 0: // sync
      return vuClockSync(vu);
      break;
    case 1: // active
      return vuClockActive(vu);
      break;
    case 2: case 3: // blank
      return vuClockBlank(vu);
      break;
  }

  return VU_SYNC;
}
