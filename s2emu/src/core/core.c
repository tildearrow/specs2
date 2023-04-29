#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

const unsigned char s2ISL[]={
  0xe5,
  0xe9,
  0xa1, 0x06, 0x00, 0x80, 0x00, 0x00,
  0x22, 0x00, 0x9e, 0x80, 0xff, 0x00,
  0xc2, 0x0e, 0x00,
  0xc0, 0xf4, 0xff,
  0x22, 0x01, 0x9f, 0x80, 0xff, 0x00,
  0x2f, 0x61,
  0xcf, 0xe9, 0xff,
  0xd0, 0x00, 0x80, 0x00, 0x00
};

void coreInit(s2Core* core) {
  core->mem=NULL;
  core->memCapacity=0;

  core->bios=NULL;
  core->biosLen=0;

  memset(core->isl,0,256);
  memset(core->pageTable,0,16384*sizeof(unsigned short));

  // ISL
  memcpy(core->isl,s2ISL,sizeof(s2ISL));
  
  coreReset(core);
}

void coreReset(s2Core* core) {
  core->irq=0;
  core->ex=0;
  core->cpuClock=0;
  core->suClock=0;

  core->clockCPU=false;
  core->clockSU=false;

  // state
  core->biosPos=0;
}

void coreSetMemory(s2Core* core, unsigned char* memory, unsigned int capacity) {
  core->mem=memory;
  core->memCapacity=capacity;
}

void coreSetBIOS(s2Core* core, unsigned char* src, unsigned short len) {
  core->bios=src;
  core->biosLen=len;
}

void coreClock(s2Core* core) {
  if (++core->cpuClock>=5) {
    core->cpuClock=0;
    core->clockCPU=true;
  } else {
    core->clockCPU=false;
  }
  if (++core->suClock>=100) {
    core->suClock=0;
    core->clockSU=true;
  } else {
    core->clockSU=false;
  }
}

bool coreReadCtrl(s2Core* core, unsigned int addr, unsigned char* out) {
  addr&=0xffff;
  if (addr>=0xff00) { // nothing
    return false;
  }
  if (addr>=0xfe00) { // ISL
    *out=core->isl[addr&0xff];
    return true;
  }
  if (addr>=0x8100) { // nothing
    return false;
  }
  if (addr>=0x8000) { // control registers
    switch (addr&0xff) {
      case 0x9e: // BIOSSTAT
        if (core->biosPos>=core->biosLen) {
          *out=0xff; // complete
        } else {
          *out=0x01; // has data
        }
        break;
      case 0x9f: // BIOSREAD
        if (core->bios==NULL) {
          *out=0;
        } else {
          *out=core->bios[core->biosPos];
          if (core->biosPos<core->biosLen) core->biosPos++;
        }
        break;
      default: // not mapped
        *out=0;
        break;
    }
    return true;
  }

  // page table
  if (addr&1) {
    *out=core->pageTable[addr>>1]>>8;
  } else {
    *out=core->pageTable[addr>>1];
  }
  return true;
}

bool coreWriteCtrl(s2Core* core, unsigned int addr, unsigned char val) {
  addr&=0xffff;
  if (addr>=0x8100) { // non-writable
    return false;
  }
  if (addr>=0x8000) { // control registers
    switch (addr&0xff) {
      case 0x7d: // MOUT
        fputc(val,stderr);
        break;
      default: // not mapped
        break;
    }
    return true;
  }

  // page table
  if (addr&1) {
    core->pageTable[addr>>1]&=0xff;
    core->pageTable[addr>>1]|=val<<8;
  } else {
    core->pageTable[addr>>1]&=0xff00;
    core->pageTable[addr>>1]|=val;
  }
  return true;
}

// MMU currently not implemented.

bool coreRead8(s2Core* core, unsigned int addr, unsigned char* out) {
  addr&=0xffffff;

  if (addr>=0xff0000) { // core
    return coreReadCtrl(core,addr,out);
  }

  if (addr>=core->memCapacity) {
    return false;
  }

  *out=core->mem[addr];
  return true;
}

bool coreRead16(s2Core* core, unsigned int addr, unsigned short* out) {
  addr&=0xffffff;

  if (addr&1) return false;

  if (addr>=0xff0000) { // core
    if (!coreReadCtrl(core,addr,(unsigned char*)out)) return false;
    if (!coreReadCtrl(core,addr+1,(unsigned char*)out+1)) return false;
    return true;
  }

  if (addr>=core->memCapacity) {
    return false;
  }

  *out=((unsigned short*)core->mem)[addr>>1];
  return true;
}

bool coreRead32(s2Core* core, unsigned int addr, unsigned int* out) {
  addr&=0xffffff;

  if (addr&3) return false;

  if (addr>=0xff0000) { // core
    if (!coreReadCtrl(core,addr,(unsigned char*)out)) return false;
    if (!coreReadCtrl(core,addr+1,(unsigned char*)out+1)) return false;
    if (!coreReadCtrl(core,addr+2,(unsigned char*)out+2)) return false;
    if (!coreReadCtrl(core,addr+3,(unsigned char*)out+3)) return false;
    return true;
  }

  if (addr>=core->memCapacity) {
    return false;
  }

  *out=((unsigned int*)core->mem)[addr>>2];
  return true;
}

bool coreReadIns(s2Core* core, unsigned int addr, unsigned char* out) {
  addr&=0xffffff;

  if (addr>=0xffff00) {
    return false;
  }

  if (addr>=0xfffe00) {
    addr&=0xff;
    out[0]=core->isl[addr];
    out[1]=core->isl[(unsigned char)(++addr)];
    out[2]=core->isl[(unsigned char)(++addr)];
    out[3]=core->isl[(unsigned char)(++addr)];
    out[4]=core->isl[(unsigned char)(++addr)];
    out[5]=core->isl[(unsigned char)(++addr)];
    out[6]=core->isl[(unsigned char)(++addr)];
    out[7]=core->isl[(unsigned char)(++addr)];
    return true;
  }

  if ((addr+7)>=core->memCapacity) {
    return false;
  }

  out[0]=core->mem[addr];
  out[1]=core->mem[++addr];
  out[2]=core->mem[++addr];
  out[3]=core->mem[++addr];
  out[4]=core->mem[++addr];
  out[5]=core->mem[++addr];
  out[6]=core->mem[++addr];
  out[7]=core->mem[++addr];
  return true;
}

bool coreWrite8(s2Core* core, unsigned int addr, unsigned char val) {
  addr&=0xffffff;

  if (addr>=0xff0000) { // core
    return coreWriteCtrl(core,addr,val);
  }

  if (addr>=core->memCapacity) {
    return false;
  }

  core->mem[addr]=val;
  return true;
}

bool coreWrite16(s2Core* core, unsigned int addr, unsigned short val) {
  addr&=0xffffff;

  if (addr&1) return false;

  if (addr>=0xff0000) { // core
    if (!coreWriteCtrl(core,addr,val)) return false;
    if (!coreWriteCtrl(core,addr+1,val>>8)) return false;
    return true;
  }

  if ((addr+1)>=core->memCapacity) {
    return false;
  }

  ((unsigned short*)core->mem)[addr>>1]=val;
  return true;
}

bool coreWrite32(s2Core* core, unsigned int addr, unsigned int val) {
  addr&=0xffffff;

  if (addr&3) return false;

  if (addr>=0xff0000) { // core
    if (!coreWriteCtrl(core,addr,val)) return false;
    if (!coreWriteCtrl(core,addr+1,val>>8)) return false;
    if (!coreWriteCtrl(core,addr+2,val>>16)) return false;
    if (!coreWriteCtrl(core,addr+3,val>>24)) return false;
    return true;
  }

  if ((addr+3)>=core->memCapacity) {
    return false;
  }

  ((unsigned int*)core->mem)[addr>>2]=val;
  return true;
}

unsigned char coreGetIRQ(s2Core* core) {
  return core->irq;
}

unsigned char coreGetException(s2Core* core) {
  return core->ex;
}

unsigned char coreRaiseIRQ(s2Core* core, unsigned char irq) {
  core->irq=irq;
  // TODO: raise IRQ on CPU here?
  return core->irq;
}
