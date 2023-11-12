#include "char.h"
#include <stdlib.h>
#include <string.h>

enum s2CharReg {
  c_VCACNT  =0x5fe0,
  c_VCAHBL  =0x5fe2,
  c_VCAHBP  =0x5fe3,
  c_VCABLU  =0x5fe4,
  c_VCABLL  =0x5fe5,
  c_VCAACT  =0x5fe6,
  c_VCHSY0  =0x5fe8,
  c_VCHSY1  =0x5fe9,
  c_VCHSZ0  =0x5fea,
  c_VCHSZ1  =0x5fec,
  c_VCTRL   =0x5fee,
  c_EXTSYN  =0x5fef,
  c_CURX    =0x5ff0,
  c_CURY    =0x5ff1,
  c_TSIZEX  =0x5ff2,
  c_TSIZEY  =0x5ff3,
  c_TSCRX   =0x5ff4,
  c_TSCRY   =0x5ff6,
  c_COLBK   =0x5ff8,
  c_TPOS    =0x5ff9,
  c_STATUS  =0x5ffa,
  c_FLAGS   =0x5ffb,
  c_CMD     =0x5ffc,
  c_PUTCBK  =0x5ffd,
  c_PUTCOL  =0x5ffe,
  c_PUTCHR  =0x5fff,
};

// format: width pre-h|post-h up|down height sy0|sy1 sz0 sz1 flags
static unsigned short videoParams[2][2][8]={
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

void cuInit(s2CharUnit* cu) {
  cu->mem=malloc(32768);

  cuReset(cu,0);
}

void cuSetPalette(s2CharUnit* cu, unsigned short* pal) {
  memcpy(cu->color,pal,256*sizeof(short));
}

void cuSetFont(s2CharUnit* cu, unsigned char* font) {
  cu->font=font;
}

// toggles:
// 1: TV mode (15KHz interlaced)
// 0: NTSC/PAL (NTSC if on)
void cuReset(s2CharUnit* cu, unsigned short toggle) {
  cu->initAddr=0x8000;
  cu->vcount=1;
  cu->hcount=1;
  cu->vpos=0;
  cu->hpos=0;
  cu->vmode=3;
  cu->hmode=3;
  cu->vslatch=0;
  cu->divider=0;
  cu->toggle=toggle;
  cu->pauseX=0;
  cu->pauseY=0;
  cu->textX=0;
  cu->textY=0;
}

#define CU_USHORT(_x) (*((unsigned short*)(&cu->mem[_x])))
#define CU_UCHAR(_x) (cu->mem[_x])
#define CU_COLOR(_x) (cu->color[_x])

unsigned short cuClockSync(s2CharUnit* cu) {
  if (cu->syncTrigger) {
    cu->textY=0;
    cu->charY=0;
    cu->syncTrigger=false;
  }
  if (cu->hmode) {
    return 0;
  }

  return CU_SYNC;
}

unsigned short cuClockActive(s2CharUnit* cu) {
  if (cu->syncTrigger) {
    cu->syncTrigger=false;
    cu->divider=0;
    cu->textX=0;
    cu->charX=0;
    if (cu->pauseY>0) {
      cu->pauseY--;
    } else {
      if (++cu->charY>CU_UCHAR(c_FLAGS&15)) {
        cu->charY=0;
        if (cu->textY<CU_UCHAR(c_TSIZEY)) cu->textY++;
      }
    }
  }

  if (cu->hmode==0) {
    return CU_SYNC;
  }
  if (cu->hmode&2) {
    cu->pauseX=CU_USHORT(c_TSCRX);
    return 0;
  }

  if (++cu->divider>(CU_UCHAR(c_VCTRL)&3)) {
    cu->divider=0;
  } else {
    return CU_HOLD;
  }

  cu->hpos++;

  // output next
  if (cu->pauseY>0) {
    return CU_COLOR(CU_UCHAR(c_COLBK));
  }

  if (cu->pauseX>0) {
    cu->pauseX--;
    return CU_COLOR(CU_UCHAR(c_COLBK));
  }

  if (cu->textX<CU_USHORT(c_TSIZEX) && cu->textY<CU_USHORT(c_TSIZEY)) {
    unsigned char out=CU_UCHAR(c_COLBK);
    const unsigned short addr=(cu->textX|(cu->textY<<7))<<2;
    const unsigned short chaddr=CU_USHORT(addr)&0x3fff;
    const unsigned char chcol=CU_UCHAR(addr|2);
    const unsigned char chbk=CU_UCHAR(addr|3);
    if (CU_UCHAR(c_FLAGS)&16) {
      if (chaddr<0x200) {
        // RAM
        const unsigned char ch=CU_UCHAR(((cu->charX&8)?0x7000:0x6000)|((chaddr&0xff)<<4)|(cu->charY&15));
        if (ch&(1<<(cu->charX&7))) {
          out=chcol;
        } else if (chbk) {
          out=chbk;
        }
      } else {
        // character ROM (TODO)
      }
    } else {
      if (chaddr<0x200) {
        // RAM
        const unsigned char ch=CU_UCHAR(0x6000|(chaddr<<4)|(cu->charY&15));
        if (ch&(1<<cu->charX)) {
          out=chcol;
        } else if (chbk) {
          out=chbk;
        }
      } else {
        // character ROM (TODO)
      }
    }

    if (CU_UCHAR(c_FLAGS)&16) {
      if (++cu->charX>=16) {
        cu->charX=0;
        cu->textX++;
      }
    } else {
      if (++cu->charX>=8) {
        cu->charX=0;
        cu->textX++;
      }
    }
    return CU_COLOR(out);
  }

  return CU_COLOR(CU_UCHAR(c_COLBK));
}

unsigned short cuClockBlank(s2CharUnit* cu) {
  if (cu->syncTrigger) {
    cu->syncTrigger=false;
    cu->textY=0;
    cu->charY=0;
  }
  cu->pauseY=CU_USHORT(c_TSCRY);
  if (cu->pauseY==0) cu->charY=255;
  if (!cu->hmode) {
    if (cu->vmode==2) {
    }
    return CU_SYNC;
  }
  return 0;
}

unsigned short cuClock(s2CharUnit* cu) {
  if (cu->initAddr) {
    cu->mem[--cu->initAddr]=0;
    if (cu->initAddr>=0x6000 && cu->font) {
      cu->mem[cu->initAddr]=cu->font[cu->initAddr&0xfff];
    }
    if (!cu->initAddr) {
      // initialize video timings
      memcpy(cu->mem+0x5fe0,videoParams[cu->toggle&1][(cu->toggle&2)>>1],16);
    }
    return CU_SYNC;
  }

  // clock
  if (!--cu->hcount) {
    if (cu->vmode==0) {
      cu->hmode^=2;
      if (!cu->hmode) {
        if (!--cu->vcount) {
          if (++cu->vslatch==3) {
            cu->vslatch=0;
            cu->vmode=2;
          }
        }
      }
    } else {
      if (cu->hmode&2) {
        cu->hmode^=3;
      } else {
        cu->hmode^=2;
      }

      if (!cu->hmode) {
        if (!--cu->vcount) {
          if (cu->vmode&2) {
            cu->vmode^=3;
          } else {
            cu->vmode^=2;
          }
        }
      }
    }
  }

  if (!cu->vcount) {
    switch (cu->vmode) {
      case 0:
        cu->vcount=5+(!cu->vslatch?1:0); // always
        cu->vpos=0xffff;
        break;
      case 1:
        cu->vcount=CU_USHORT(c_VCAACT);
        break;
      case 2:
        cu->vcount=CU_UCHAR(c_VCABLU);
        break;
      case 3:
        cu->vcount=CU_UCHAR(c_VCABLL);
        break;
    }
  }
  if (!cu->hcount) {
    if (cu->vmode==0) {
      if (cu->vslatch&1) {
        if (cu->hmode) {
          cu->hcount=CU_UCHAR(c_VCHSY1);
        } else {
          cu->hcount=CU_USHORT(c_VCHSZ1);
        }
      } else {
        if (cu->hmode) {
          cu->hcount=CU_USHORT(c_VCHSZ0);
        } else {
          cu->hcount=CU_UCHAR(c_VCHSY0);
        }
      }
    } else {
      switch (cu->hmode) {
        case 0:
          cu->hcount=CU_UCHAR(c_VCHSY0);
          cu->hpos=0xffff;
          cu->syncTrigger=true;
          if (cu->vmode==1) cu->vpos++;
          break;
        case 1:
          cu->hcount=CU_USHORT(c_VCACNT);
          break;
        case 2:
          cu->hcount=CU_UCHAR(c_VCAHBL);
          break;
        case 3:
          cu->hcount=CU_UCHAR(c_VCAHBP);
          break;
      }
    }
  }
  
  // output
  switch (cu->vmode) {
    case 0: // sync
      return cuClockSync(cu);
      break;
    case 1: // active
      return cuClockActive(cu);
      break;
    case 2: case 3: // blank
      return cuClockBlank(cu);
      break;
  }

  return CU_SYNC;
}
