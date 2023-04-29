/* su.cpp/su.h - Sound Unit emulator
 * Copyright (C) 2015-2023 tildearrow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// C translation by tildearrow

#define _USE_MATH_DEFINES
#include "sound.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define minval(a,b) (((a)<(b))?(a):(b))
#define maxval(a,b) (((a)>(b))?(a):(b))

#define FILVOL su->chan[4].special1C
#define ILCTRL su->chan[4].special1D
#define ILSIZE su->chan[5].special1C
#define FIL1 su->chan[5].special1D
#define IL1 su->chan[6].special1C
#define IL2 su->chan[6].special1D
#define IL0 su->chan[7].special1C
#define MVOL su->chan[7].special1D

void suClock(s2SoundUnit* su, short* l, short* r) {
  // run channels
  for (int i=0; i<8; i++) {
    if (su->chan[i].vol==0 && !(su->chan[i].flags1&32)) {
      su->fns[i]=0;
      continue;
    }
    if (su->chan[i].flags0&8) {
      su->ns[i]=su->pcm[su->chan[i].pcmpos];
    } else switch (su->chan[i].flags0&7) {
      case 0:
        su->ns[i]=(((su->cycle[i]>>15)&127)>su->chan[i].duty)*127;
        break;
      case 1:
        su->ns[i]=su->cycle[i]>>14;
        break;
      case 2:
        su->ns[i]=su->SCsine[(su->cycle[i]>>14)&255];
        break;
      case 3:
        su->ns[i]=su->SCtriangle[(su->cycle[i]>>14)&255];
        break;
      case 4: case 5:
        su->ns[i]=(su->lfsr[i]&1)*127;
        break;
      case 6:
        su->ns[i]=((((su->cycle[i]>>15)&127)>su->chan[i].duty)*127)^(short)su->SCsine[(su->cycle[i]>>14)&255];
        break;
      case 7:
        su->ns[i]=((((su->cycle[i]>>15)&127)>su->chan[i].duty)*127)^(short)su->SCtriangle[(su->cycle[i]>>14)&255];
        break;
    }

    // ring mod
    if (su->chan[i].flags0&16) {
      su->ns[i]=(su->ns[i]*su->ns[(i+1)&7])>>7;
    }
    
    // PCM
    if (su->chan[i].flags0&8) {
      if (su->chan[i].freq>0x8000) {
        su->pcmdec[i]+=0x8000;
      } else {
        su->pcmdec[i]+=su->chan[i].freq;
      }
      if (su->pcmdec[i]>=32768) {
        su->pcmdec[i]-=32768;
        if (su->chan[i].pcmpos<su->chan[i].pcmbnd) {
          su->chan[i].pcmpos++;
          if (su->chan[i].pcmpos==su->chan[i].pcmbnd) {
            if (su->chan[i].flags1&4) {
              su->chan[i].pcmpos=su->chan[i].pcmrst;
            }
          }
          su->chan[i].pcmpos&=(su->pcmSize-1);
        } else if (su->chan[i].flags1&4) {
          su->chan[i].pcmpos=su->chan[i].pcmrst;
        }
      }
    } else {
      su->ocycle[i]=su->cycle[i];
      if ((su->chan[i].flags0&7)==5) {
        switch ((su->chan[i].duty>>4)&3) {
          case 0:
            su->cycle[i]+=su->chan[i].freq*1-(su->chan[i].freq>>3);
            break;
          case 1:
            su->cycle[i]+=su->chan[i].freq*2-(su->chan[i].freq>>3);
            break;
          case 2:
            su->cycle[i]+=su->chan[i].freq*4-(su->chan[i].freq>>3);
            break;
          case 3:
            su->cycle[i]+=su->chan[i].freq*8-(su->chan[i].freq>>3);
            break;
        }
      } else {
        su->cycle[i]+=su->chan[i].freq;
      }
      if ((su->cycle[i]&0xf80000)!=(su->ocycle[i]&0xf80000)) {
        if ((su->chan[i].flags0&7)==4) {
          su->lfsr[i]=(su->lfsr[i]>>1|(((su->lfsr[i])^(su->lfsr[i] >> 2)^(su->lfsr[i]>>3)^(su->lfsr[i]>>5))&1)<<31);
        } else {
          switch ((su->chan[i].duty>>4)&3) {
            case 0:
              su->lfsr[i]=(su->lfsr[i]>>1|(((su->lfsr[i]>>3)^(su->lfsr[i]>>4))&1)<<5);
              break;
            case 1:
              su->lfsr[i]=(su->lfsr[i]>>1|(((su->lfsr[i]>>2)^(su->lfsr[i]>>3))&1)<<5);
              break;
            case 2:
              su->lfsr[i]=(su->lfsr[i]>>1|(((su->lfsr[i])^(su->lfsr[i]>>2)^(su->lfsr[i]>>3))&1)<<5);
              break;
            case 3:
              su->lfsr[i]=(su->lfsr[i]>>1|(((su->lfsr[i])^(su->lfsr[i]>>2)^(su->lfsr[i]>>3)^(su->lfsr[i]>>5))&1)<<5);
              break;
          }
          if ((su->lfsr[i]&63)==0) {
            su->lfsr[i]=0xaaaa;
          }
        }
      }
      if (su->chan[i].flags1&8) {
        if (--su->rcycle[i]<=0) {
          su->cycle[i]=0;
          su->rcycle[i]=su->chan[i].restimer;
          su->lfsr[i]=0xaaaa;
        }
      }
    }
    su->fns[i]=su->ns[i]*su->chan[i].vol*((su->chan[i].flags0&8)?4:2);
    if ((su->chan[i].flags0&0xe0)!=0) {
      int ff=su->chan[i].cutoff;
      su->nslow[i]=su->nslow[i]+(((ff)*su->nsband[i])>>16);
      su->nshigh[i]=su->fns[i]-su->nslow[i]-(((256-su->chan[i].reson)*su->nsband[i])>>8);
      su->nsband[i]=(((ff)*su->nshigh[i])>>16)+su->nsband[i];
      su->fns[i]=(((su->chan[i].flags0&32)?(su->nslow[i]):(0))+((su->chan[i].flags0&64)?(su->nshigh[i]):(0))+((su->chan[i].flags0&128)?(su->nsband[i]):(0)));
    }
    su->nsL[i]=(su->fns[i]*su->SCpantabL[(unsigned char)su->chan[i].pan])>>8;
    su->nsR[i]=(su->fns[i]*su->SCpantabR[(unsigned char)su->chan[i].pan])>>8;
    su->oldfreq[i]=su->chan[i].freq;
    if (su->chan[i].flags1&32) {
      if (--su->swvolt[i]<=0) {
        su->swvolt[i]=su->chan[i].swvol.speed;
        if (su->chan[i].swvol.amt&32) {
          su->chan[i].vol+=su->chan[i].swvol.amt&31;
          if (su->chan[i].vol>su->chan[i].swvol.bound && !(su->chan[i].swvol.amt&64)) {
            su->chan[i].vol=su->chan[i].swvol.bound;
          }
          if (su->chan[i].vol&0x80) {
            if (su->chan[i].swvol.amt&64) {
              if (su->chan[i].swvol.amt&128) {
                su->chan[i].swvol.amt^=32;
                su->chan[i].vol=0xff-su->chan[i].vol;
              } else {
                su->chan[i].vol&=~0x80;
              }
            } else {
              su->chan[i].vol=0x7f;
            }
          }
        } else {
          su->chan[i].vol-=su->chan[i].swvol.amt&31;
          if (su->chan[i].vol&0x80) {
            if (su->chan[i].swvol.amt&64) {
              if (su->chan[i].swvol.amt&128) {
                su->chan[i].swvol.amt^=32;
                su->chan[i].vol=-su->chan[i].vol;
              } else {
                su->chan[i].vol&=~0x80;
              }
            } else {
              su->chan[i].vol=0x0;
            }
          }
          if (su->chan[i].vol<su->chan[i].swvol.bound && !(su->chan[i].swvol.amt&64)) {
            su->chan[i].vol=su->chan[i].swvol.bound;
          }
        }
      }
    }
    if (su->chan[i].flags1&16) {
      if (--su->swfreqt[i]<=0) {
        su->swfreqt[i]=su->chan[i].swfreq.speed;
        if (su->chan[i].swfreq.amt&128) {
          if (su->chan[i].freq>(0xffff-(su->chan[i].swfreq.amt&127))) {
            su->chan[i].freq=0xffff;
          } else {
            su->chan[i].freq=(su->chan[i].freq*(0x80+(su->chan[i].swfreq.amt&127)))>>7;
            if ((su->chan[i].freq>>8)>su->chan[i].swfreq.bound) {
              su->chan[i].freq=su->chan[i].swfreq.bound<<8;
            }
          }
        } else {
          if (su->chan[i].freq<(su->chan[i].swfreq.amt&127)) {
            su->chan[i].freq=0;
          } else {
            su->chan[i].freq=(su->chan[i].freq*(0xff-(su->chan[i].swfreq.amt&127)))>>8;
            if ((su->chan[i].freq>>8)<su->chan[i].swfreq.bound) {
              su->chan[i].freq=su->chan[i].swfreq.bound<<8;
            }
          }
        }
      }
    }
    if (su->chan[i].flags1&64) {
      if (--su->swcutt[i]<=0) {
        su->swcutt[i]=su->chan[i].swcut.speed;
        if (su->chan[i].swcut.amt&128) {
          if (su->chan[i].cutoff>(0xffff-(su->chan[i].swcut.amt&127))) {
            su->chan[i].cutoff=0xffff;
          } else {
            su->chan[i].cutoff+=su->chan[i].swcut.amt&127;
            if ((su->chan[i].cutoff>>8)>su->chan[i].swcut.bound) {
              su->chan[i].cutoff=su->chan[i].swcut.bound<<8;
            }
          }
        } else {
          if (su->chan[i].cutoff<(su->chan[i].swcut.amt&127)) {
            su->chan[i].cutoff=0;
          } else {
            su->chan[i].cutoff=((2048-(unsigned int)(su->chan[i].swcut.amt&127))*(unsigned int)su->chan[i].cutoff)>>11;
            if ((su->chan[i].cutoff>>8)<su->chan[i].swcut.bound) {
              su->chan[i].cutoff=su->chan[i].swcut.bound<<8;
            }
          }
        }
      }
    }
    if (su->chan[i].flags1&1) {
      su->cycle[i]=0;
      su->rcycle[i]=su->chan[i].restimer;
      su->ocycle[i]=0;
      su->chan[i].flags1&=~1;
    }
    if (su->muted[i]) {
      su->nsL[i]=0;
      su->nsR[i]=0;
    }
  }

  // mix
  if (su->dsOut) {
    su->tnsL=su->nsL[su->dsChannel]<<1;
    su->tnsR=su->nsR[su->dsChannel]<<1;
    su->dsChannel=(su->dsChannel+1)&7;
  } else {
    su->tnsL=(su->nsL[0]+su->nsL[1]+su->nsL[2]+su->nsL[3]+su->nsL[4]+su->nsL[5]+su->nsL[6]+su->nsL[7])>>2;
    su->tnsR=(su->nsR[0]+su->nsR[1]+su->nsR[2]+su->nsR[3]+su->nsR[4]+su->nsR[5]+su->nsR[6]+su->nsR[7])>>2;

    IL1=minval(32767,maxval(-32767,su->tnsL))>>8;
    IL2=minval(32767,maxval(-32767,su->tnsR))>>8;
  }

  // write input lines to sample memory
  if (ILSIZE&64) {
    if (++su->ilBufPeriod>=((1+(FIL1>>4))<<2)) {
      su->ilBufPeriod=0;
      unsigned short ilLowerBound=su->pcmSize-((1+(ILSIZE&63))<<7);
      short next;
      if (su->ilBufPos<ilLowerBound) su->ilBufPos=ilLowerBound;
      switch (ILCTRL&3) {
        case 0:
          su->ilFeedback0=su->ilFeedback1=su->pcm[su->ilBufPos];
          next=((signed char)IL0)+((su->pcm[su->ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          su->pcm[su->ilBufPos]=next;
          if (++su->ilBufPos>=su->pcmSize) su->ilBufPos=ilLowerBound;
          break;
        case 1:
          su->ilFeedback0=su->ilFeedback1=su->pcm[su->ilBufPos];
          next=((signed char)IL1)+((su->pcm[su->ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          su->pcm[su->ilBufPos]=next;
          if (++su->ilBufPos>=su->pcmSize) su->ilBufPos=ilLowerBound;
          break;
        case 2:
          su->ilFeedback0=su->ilFeedback1=su->pcm[su->ilBufPos];
          next=((signed char)IL2)+((su->pcm[su->ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          su->pcm[su->ilBufPos]=next;
          if (++su->ilBufPos>=su->pcmSize) su->ilBufPos=ilLowerBound;
          break;
        case 3:
          su->ilFeedback0=su->pcm[su->ilBufPos];
          next=((signed char)IL1)+((su->pcm[su->ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          su->pcm[su->ilBufPos]=next;
          if (++su->ilBufPos>=su->pcmSize) su->ilBufPos=ilLowerBound;
          su->ilFeedback1=su->pcm[su->ilBufPos];
          next=((signed char)IL2)+((su->pcm[su->ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          su->pcm[su->ilBufPos]=next;
          if (++su->ilBufPos>=su->pcmSize) su->ilBufPos=ilLowerBound;
          break;
      }
    }
    if (ILCTRL&4) {
      if (ILSIZE&128) {
        su->tnsL+=su->ilFeedback1*(signed char)FILVOL;
        su->tnsR+=su->ilFeedback0*(signed char)FILVOL;
      } else {
        su->tnsL+=su->ilFeedback0*(signed char)FILVOL;
        su->tnsR+=su->ilFeedback1*(signed char)FILVOL;
      }
    }
  }

  if (su->dsOut) {
    *l=minval(32767,maxval(-32767,su->tnsL))&0xff00;
    *r=minval(32767,maxval(-32767,su->tnsR))&0xff00;
  } else {
    *l=minval(32767,maxval(-32767,su->tnsL));
    *r=minval(32767,maxval(-32767,su->tnsR));
  }
}

void suInit(s2SoundUnit* su, int sampleMemSize, bool dsOutMode) {
  su->pcmSize=sampleMemSize;
  su->pcm=malloc(65536);
  memset(su->pcm,0,su->pcmSize);

  su->dsOut=dsOutMode;
  suReset(su);
  for (int i=0; i<256; i++) {
    su->SCsine[i]=sin((i/128.0f)*M_PI)*127;
    su->SCtriangle[i]=(i>127)?(255-i):(i);
    su->SCpantabL[i]=127;
    su->SCpantabR[i]=127;
  }
  for (int i=0; i<128; i++) {
    su->SCpantabL[i]=127-i;
    su->SCpantabR[128+i]=i-1;
  }
  su->SCpantabR[128]=0;
}

void suReset(s2SoundUnit* su) {
  for (int i=0; i<8; i++) {
    su->ocycle[i]=0;
    su->cycle[i]=0;
    su->rcycle[i]=0;
    su->resetfreq[i]=0;
    su->voldcycles[i]=0;
    su->volicycles[i]=0;
    su->fscycles[i]=0;
    su->sweep[i]=0;
    su->ns[i]=0;
    su->fns[i]=0;
    su->nsL[i]=0;
    su->nsR[i]=0;
    su->nslow[i]=0;
    su->nshigh[i]=0;
    su->nsband[i]=0;
    su->swvolt[i]=1;
    su->swfreqt[i]=1;
    su->swcutt[i]=1;
    su->lfsr[i]=0xaaaa;
    su->oldfreq[i]=0;
    su->pcmdec[i]=0;
  }
  su->dsChannel=0;
  su->tnsL=0;
  su->tnsR=0;
  su->ilBufPos=0;
  su->ilBufPeriod=0;
  su->ilFeedback0=0;
  su->ilFeedback1=0;
  memset(su->chan,0,256);
}

#ifdef TA_BIG_ENDIAN
const unsigned char suBERemap[32]={
  0x01, 0x00, 0x02, 0x03, 0x04, 0x05, 0x07, 0x06, 0x08, 0x09, 0x0b, 0x0a, 0x0d, 0x0c, 0x0f, 0x0e,
  0x11, 0x10, 0x12, 0x13, 0x15, 0x14, 0x16, 0x17, 0x19, 0x18, 0x1a, 0x1b, 0x1c, 0x1d, 0x1f, 0x1e
};
#endif

bool suRead8(s2SoundUnit* su, unsigned char addr, unsigned char* out) {
  *out=((unsigned char*)su->chan)[addr];
  return true;
}

bool suWrite8(s2SoundUnit* su, unsigned char addr, unsigned char val) {
#ifdef TA_BIG_ENDIAN
  // remap
  addr=(addr&0xe0)|(suBERemap[addr&0x1f]);
#endif
  ((unsigned char*)su->chan)[addr]=val;
  return true;
}
