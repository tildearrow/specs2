#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const unsigned char cpuInsAdvance[256]={
  /// CHAR
  // ↓ <- immediate
  2, 3, 6, 6, 6, 6, 6, 2,
  2, 3, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2,
  3, 3, 3, 3, // immediate
  6, 6, 6, 6,
  2, 2, 2, 2,
  2, 3 /* imm */, 6, 6, 6, 6, 6, 2,
  5 /* imm+addr24 */, 1, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2, 2, 2, 2, 2, // < immediate
  2, 3 /* imm */, 2, 3 /* imm */, 2, 3 /* imm */, 1, 1,

  /// SHORT
  // ↓ <- immediate
  2, 4, 6, 6, 6, 6, 6, 2,
  2, 4, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2,
  4, 4, 4, 4, // immediate
  6, 6, 6, 6,
  2, 2, 2, 2,
  2, 4 /* imm */, 6, 6, 6, 6, 6, 2,
  6 /* imm+addr24 */, 1, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2, 2, 2, 2, 3, // < immediate
  2, 4 /* imm */, 2, 4 /* imm */, 2, 4 /* imm */, 1, 1,

  /// INT
  // ↓ <- immediate
  2, 6, 6, 6, 6, 6, 6, 2,
  2, 6, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2,
  6, 6, 6, 6, // immediate
  6, 6, 6, 6,
  2, 2, 2, 2,
  2, 6 /* imm */, 6, 6, 6, 6, 6, 2,
  1 /* imm+addr24 */, 1, 6, 6, 6, 6, 6, 2,
  2, 2, 2, 2, 2, 2, 2, 5, // < immediate
  2, 6 /* imm */, 2, 6 /* imm */, 2, 6 /* imm */, 1, 1,

  /// SPECIAL
  3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 2, 2, 3, 3,
  5, 5, 5, 5, 2, 2, 2, 2,
  2, 2, 2, 2, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 2, 2, 2, 2,
  2, 2, 1, 1, 1, 2, 2, 2
};

// operation timing:
// - reg/imm: 1 cycle
// - abs/regptr: 1 cycle (+1 if int)
// - index: 2 cycles (index > op)
// - indirect:
//   - r1: 3 cycles (dereference (2) > op)
//   - r2: 2 cycles (dereference (1) > op)
// - indirect+index:
//   - r1: 4 cycles (dereference (2) + index > op)
//   - r2: 3 cycles (dereference (1) + index > op)
// - push/pop:
//   - r1: 2 cycles
//   - r2: 1 cycle
// - mul:
//   - 2 cycles (char)
//   - 2 cycles (short)
//   - 4 cycles (int)
// - div:
//   - 1 cycle (put/fetch)
//   - variable time
const unsigned char cpuInsTiming[256]={
  // CHAR
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 
  1, 2, 3, 4, 4, 1,
  1, 1, 1, 1, 1,
  2, 2, 2,
  2, 2, 2, 2,
  1, 1, 1, 1,

  // SHORT
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 2, 3, 4, 4, 1,
  1, 1, 
  1, 2, 3, 4, 4, 1,
  1, 1, 1, 1, 1,
  2, 2, 2,
  2, 2, 2, 2,
  1, 1, 1, 1,

  // INT
  1, 1, 2, 3, 4, 5, 5, 2,
  1, 1, 2, 3, 4, 5, 5, 2,
  1, 1, 1, 1,
  1, 1, 1, 1,
  2, 2, 2, 2,
  2, 2, 2, 2,
  1, 1, 2, 3, 4, 5, 5, 2,
  1, 1, 
  2, 3, 4, 5, 5, 2,
  1, 1, 1, 1, 1,
  2, 2, 2,
  4, 4, 4, 4,
  1, 1, 1, 1,

  // SPECIAL
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 2, 4, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  8,24, 8,24, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 3, 3
};

const unsigned char cpuRegDest[256]={
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7
};

const unsigned char cpuRegSrc[256]={
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

const unsigned char cpuRegIndex[256]={
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

const char* cpuRegNames[8]={
  "a",
  "b",
  "c",
  "d",
  "w",
  "x",
  "y",
  "z"
};

const unsigned char cpuCtx[8]={
  0, 1, 2, 3, 4, 5, 5, 5
};

const char* cpuCtxNames[8]={
  "K0",
  "K1",
  "K2",
  "K3",
  "K4",
  "K?",
  "K?",
  "K?"
};

#define FLAG_N 0x80
#define FLAG_D 0x40
#define FLAG_K 0x20
#define FLAG_E 0x10
#define FLAG_F 8
#define FLAG_I 4
#define FLAG_Z 2
#define FLAG_C 1

void cpuInit(s2CPU* cpu, s2Core* core) {
  cpu->core=core;
  cpu->debug=0;
  cpuReset(cpu);
}

void cpuReset(s2CPU* cpu) {
  cpu->curIns[0]=0xfc; // nop
  cpu->nextIns[0]=0xfc; // nop

  for (int r=0; r<8; r++) {
    cpu->reg[r].i=0;
  }

  cpu->pc=0xfffe00; // ISL location
  cpu->sp=0;
  cpu->st=0;
  cpu->sb=0;
  cpu->cc=0;
  cpu->cycleTotal=0;
  cpu->ev=0;
  cpu->iv=0;
  cpu->flags=FLAG_K;
  cpu->ex=0;
  cpu->ir=0;
  cpu->initFetch=1;
  cpu->halted=0;

  cpu->divN=0;
  cpu->divD=0;
  cpu->divQ=0;
  cpu->divR=0;
  cpu->divBusy=0;

  cpu->cycles=0;
}

#define DA_DEST cpuRegNames[cpuRegDest[ptr[1]]]
#define DA_SRC cpuRegNames[cpuRegSrc[ptr[1]]]
#define DA_INDEX cpuRegNames[cpuRegIndex[ptr[1]]]
#define DA_CTX cpuCtxNames[cpuCtx[ptr[1]&7]]

#define DA_IMM_C ptr[2]
#define DA_IMM_S (ptr[2]|(ptr[3]<<8))
#define DA_IMM_I (ptr[2]|(ptr[3]<<8)|(ptr[4]<<16)|(ptr[5]<<24))
#define DA_ADDR (ptr[2]|(ptr[3]<<8)|(ptr[4]<<16))
#define DA_ADEST (ptr[1]|(ptr[2]<<8)|(ptr[3]<<16))
#define DA_AIMM_C ptr[4]
#define DA_AIMM_S (ptr[4]|(ptr[5]<<8))
#define DA_PIMM_C ptr[1]
#define DA_PIMM_S (ptr[1]|(ptr[2]<<8))
#define DA_PIMM_I (ptr[1]|(ptr[2]<<8)|(ptr[3]<<16)|(ptr[4]<<24))
#define DA_BROFF ((signed short)DA_PIMM_S+3)
#define DA_BBROFF ((signed short)DA_IMM_S+4)

int cpuDisAsm(const unsigned char* ptr, char* str, unsigned int len, unsigned int pc) {
  // don't write anything if buffer or length are invalid
  if (str==NULL) return 0;
  if (!len) return 0;

  int ret=-1;
  switch (ptr[0]) {
    case 0x00:
      ret=snprintf(str,len,"addc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x01:
      ret=snprintf(str,len,"addc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x02:
      ret=snprintf(str,len,"addc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x03:
      ret=snprintf(str,len,"addc %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x04:
      ret=snprintf(str,len,"addc %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x05:
      ret=snprintf(str,len,"addc %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x06:
      ret=snprintf(str,len,"addc %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x07:
      ret=snprintf(str,len,"addc %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x08:
      ret=snprintf(str,len,"subc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x09:
      ret=snprintf(str,len,"subc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x0a:
      ret=snprintf(str,len,"subc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x0b:
      ret=snprintf(str,len,"subc %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x0c:
      ret=snprintf(str,len,"subc %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x0d:
      ret=snprintf(str,len,"subc %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x0e:
      ret=snprintf(str,len,"subc %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x0f:
      ret=snprintf(str,len,"subc %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x10:
      ret=snprintf(str,len,"andc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x11:
      ret=snprintf(str,len,"orc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x12:
      ret=snprintf(str,len,"xorc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x13:
      ret=snprintf(str,len,"cmpc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x14:
      ret=snprintf(str,len,"andc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x15:
      ret=snprintf(str,len,"orc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x16:
      ret=snprintf(str,len,"xorc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x17:
      ret=snprintf(str,len,"cmpc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x18:
      ret=snprintf(str,len,"andc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x19:
      ret=snprintf(str,len,"orc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x1a:
      ret=snprintf(str,len,"xorc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x1b:
      ret=snprintf(str,len,"cmpc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x1c:
      ret=snprintf(str,len,"andc %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x1d:
      ret=snprintf(str,len,"orc %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x1e:
      ret=snprintf(str,len,"xorc %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x1f:
      ret=snprintf(str,len,"cmpc %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x20:
      ret=snprintf(str,len,"movc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x21:
      ret=snprintf(str,len,"movc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x22:
      ret=snprintf(str,len,"movc %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x23:
      ret=snprintf(str,len,"movc %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x24:
      ret=snprintf(str,len,"movc %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x25:
      ret=snprintf(str,len,"movc %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x26:
      ret=snprintf(str,len,"movc %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x27:
      ret=snprintf(str,len,"movc %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x28:
      ret=snprintf(str,len,"movc $%x, #$%.2x",DA_ADEST,DA_AIMM_C);
      break;
    case 0x2a:
      ret=snprintf(str,len,"movc $%x, %s",DA_ADDR,DA_DEST);
      break;
    case 0x2b:
      ret=snprintf(str,len,"movc $%x+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x2c:
      ret=snprintf(str,len,"movc ($%x), %s",DA_ADDR,DA_DEST);
      break;
    case 0x2d:
      ret=snprintf(str,len,"movc ($%x)+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x2e:
      ret=snprintf(str,len,"movc ($%x+%s), %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x2f:
      ret=snprintf(str,len,"movc (%s), %s",DA_SRC,DA_DEST);
      break;
    case 0x30:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sa%cc %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sa%cc %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0x31:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sh%cc %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sh%cc %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0x32:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"rolc %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"rolc %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0x33:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"rorc %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"rorc %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0x34:
      ret=snprintf(str,len,"swpc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x35:
      ret=snprintf(str,len,"pushc %s",DA_DEST);
      break;
    case 0x36:
      ret=snprintf(str,len,"popc %s",DA_DEST);
      break;
    case 0x37:
      ret=snprintf(str,len,"pushc #$%.2x",DA_PIMM_C);
      break;
    case 0x38:
      ret=snprintf(str,len,"mulc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x39:
      ret=snprintf(str,len,"mulc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x3a:
      ret=snprintf(str,len,"muluc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x3b:
      ret=snprintf(str,len,"muluc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x3c:
      ret=snprintf(str,len,"divc %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x3d:
      ret=snprintf(str,len,"divc %s, #$%.2x",DA_DEST,DA_IMM_C);
      break;
    case 0x3e:
      ret=snprintf(str,len,"divqc %s",DA_DEST);
      break;
    case 0x3f:
      ret=snprintf(str,len,"divrc %s",DA_DEST);
      break;

    case 0x40:
      ret=snprintf(str,len,"adds %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x41:
      ret=snprintf(str,len,"adds %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x42:
      ret=snprintf(str,len,"adds %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x43:
      ret=snprintf(str,len,"adds %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x44:
      ret=snprintf(str,len,"adds %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x45:
      ret=snprintf(str,len,"adds %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x46:
      ret=snprintf(str,len,"adds %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x47:
      ret=snprintf(str,len,"adds %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x48:
      ret=snprintf(str,len,"subs %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x49:
      ret=snprintf(str,len,"subs %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x4a:
      ret=snprintf(str,len,"subs %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x4b:
      ret=snprintf(str,len,"subs %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x4c:
      ret=snprintf(str,len,"subs %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x4d:
      ret=snprintf(str,len,"subs %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x4e:
      ret=snprintf(str,len,"subs %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x4f:
      ret=snprintf(str,len,"subs %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x50:
      ret=snprintf(str,len,"ands %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x51:
      ret=snprintf(str,len,"ors %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x52:
      ret=snprintf(str,len,"xors %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x53:
      ret=snprintf(str,len,"cmps %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x54:
      ret=snprintf(str,len,"ands %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x55:
      ret=snprintf(str,len,"ors %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x56:
      ret=snprintf(str,len,"xors %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x57:
      ret=snprintf(str,len,"cmps %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x58:
      ret=snprintf(str,len,"ands %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x59:
      ret=snprintf(str,len,"ors %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x5a:
      ret=snprintf(str,len,"xors %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x5b:
      ret=snprintf(str,len,"cmps %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x5c:
      ret=snprintf(str,len,"ands %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x5d:
      ret=snprintf(str,len,"ors %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x5e:
      ret=snprintf(str,len,"xors %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x5f:
      ret=snprintf(str,len,"cmps %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x60:
      ret=snprintf(str,len,"movs %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x61:
      ret=snprintf(str,len,"movs %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x62:
      ret=snprintf(str,len,"movs %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x63:
      ret=snprintf(str,len,"movs %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x64:
      ret=snprintf(str,len,"movs %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x65:
      ret=snprintf(str,len,"movs %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x66:
      ret=snprintf(str,len,"movs %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x67:
      ret=snprintf(str,len,"movs %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x68:
      ret=snprintf(str,len,"movs $%x, #$%.4x",DA_ADEST,DA_AIMM_S);
      break;
    case 0x6a:
      ret=snprintf(str,len,"movs $%x, %s",DA_ADDR,DA_DEST);
      break;
    case 0x6b:
      ret=snprintf(str,len,"movs $%x+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x6c:
      ret=snprintf(str,len,"movs ($%x), %s",DA_ADDR,DA_DEST);
      break;
    case 0x6d:
      ret=snprintf(str,len,"movs ($%x)+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x6e:
      ret=snprintf(str,len,"movs ($%x+%s), %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0x6f:
      ret=snprintf(str,len,"movs (%s), %s",DA_SRC,DA_DEST);
      break;
    case 0x70:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sa%cs %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sa%cs %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0x71:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sh%cs %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sh%cs %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0x72:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"rols %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"rols %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0x73:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"rors %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"rors %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0x74:
      ret=snprintf(str,len,"swps %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x75:
      ret=snprintf(str,len,"pushs %s",DA_DEST);
      break;
    case 0x76:
      ret=snprintf(str,len,"pops %s",DA_DEST);
      break;
    case 0x77:
      ret=snprintf(str,len,"pushs #$%.4x",DA_PIMM_S);
      break;
    case 0x78:
      ret=snprintf(str,len,"muls %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x79:
      ret=snprintf(str,len,"muls %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x7a:
      ret=snprintf(str,len,"mulus %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x7b:
      ret=snprintf(str,len,"mulus %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x7c:
      ret=snprintf(str,len,"divs %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x7d:
      ret=snprintf(str,len,"divs %s, #$%.4x",DA_DEST,DA_IMM_S);
      break;
    case 0x7e:
      ret=snprintf(str,len,"divqs %s",DA_DEST);
      break;
    case 0x7f:
      ret=snprintf(str,len,"divrs %s",DA_DEST);
      break;

    case 0x80:
      ret=snprintf(str,len,"add %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x81:
      ret=snprintf(str,len,"add %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x82:
      ret=snprintf(str,len,"add %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x83:
      ret=snprintf(str,len,"add %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x84:
      ret=snprintf(str,len,"add %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x85:
      ret=snprintf(str,len,"add %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x86:
      ret=snprintf(str,len,"add %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x87:
      ret=snprintf(str,len,"add %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x88:
      ret=snprintf(str,len,"sub %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x89:
      ret=snprintf(str,len,"sub %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x8a:
      ret=snprintf(str,len,"sub %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x8b:
      ret=snprintf(str,len,"sub %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x8c:
      ret=snprintf(str,len,"sub %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0x8d:
      ret=snprintf(str,len,"sub %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x8e:
      ret=snprintf(str,len,"sub %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0x8f:
      ret=snprintf(str,len,"sub %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0x90:
      ret=snprintf(str,len,"and %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x91:
      ret=snprintf(str,len,"or %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x92:
      ret=snprintf(str,len,"xor %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x93:
      ret=snprintf(str,len,"cmp %s, %s",DA_DEST,DA_SRC);
      break;
    case 0x94:
      ret=snprintf(str,len,"and %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x95:
      ret=snprintf(str,len,"or %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x96:
      ret=snprintf(str,len,"xor %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x97:
      ret=snprintf(str,len,"cmp %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0x98:
      ret=snprintf(str,len,"and %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x99:
      ret=snprintf(str,len,"or %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x9a:
      ret=snprintf(str,len,"xor %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x9b:
      ret=snprintf(str,len,"cmp %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0x9c:
      ret=snprintf(str,len,"and %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x9d:
      ret=snprintf(str,len,"or %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x9e:
      ret=snprintf(str,len,"xor %s, (%s)",DA_DEST,DA_SRC);
      break;
    case 0x9f:
      ret=snprintf(str,len,"cmp %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0xa0:
      ret=snprintf(str,len,"mov %s, %s",DA_DEST,DA_SRC);
      break;
    case 0xa1:
      ret=snprintf(str,len,"mov %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0xa2:
      ret=snprintf(str,len,"mov %s, $%x",DA_DEST,DA_ADDR);
      break;
    case 0xa3:
      ret=snprintf(str,len,"mov %s, $%x+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0xa4:
      ret=snprintf(str,len,"mov %s, ($%x)",DA_DEST,DA_ADDR);
      break;
    case 0xa5:
      ret=snprintf(str,len,"mov %s, ($%x)+%s",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0xa6:
      ret=snprintf(str,len,"mov %s, ($%x+%s)",DA_DEST,DA_ADDR,DA_INDEX);
      break;
    case 0xa7:
      ret=snprintf(str,len,"mov %s, (%s)",DA_DEST,DA_SRC);
      break;

    case 0xaa:
      ret=snprintf(str,len,"mov $%x, %s",DA_ADDR,DA_DEST);
      break;
    case 0xab:
      ret=snprintf(str,len,"mov $%x+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0xac:
      ret=snprintf(str,len,"mov ($%x), %s",DA_ADDR,DA_DEST);
      break;
    case 0xad:
      ret=snprintf(str,len,"mov ($%x)+%s, %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0xae:
      ret=snprintf(str,len,"mov ($%x+%s), %s",DA_ADDR,DA_INDEX,DA_DEST);
      break;
    case 0xaf:
      ret=snprintf(str,len,"mov (%s), %s",DA_SRC,DA_DEST);
      break;
    case 0xb0:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sa%c %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sa%c %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0xb1:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"sh%c %s, #%d",(ptr[1]&0x80)?'l':'r',DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"sh%c %s, %s",(ptr[1]&0x80)?'l':'r',DA_DEST,DA_SRC);
      }
      break;
    case 0xb2:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"rol %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"rol %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0xb3:
      if (ptr[1]&8) {
        ret=snprintf(str,len,"ror %s, #%d",DA_DEST,ptr[1]&7);
      } else {
        ret=snprintf(str,len,"ror %s, %s",DA_DEST,DA_SRC);
      }
      break;
    case 0xb4:
      ret=snprintf(str,len,"swp %s, %s",DA_DEST,DA_SRC);
      break;
    case 0xb5:
      ret=snprintf(str,len,"push %s",DA_DEST);
      break;
    case 0xb6:
      ret=snprintf(str,len,"pop %s",DA_DEST);
      break;
    case 0xb7:
      ret=snprintf(str,len,"push #$%x",DA_PIMM_I);
      break;
    case 0xb8:
      ret=snprintf(str,len,"mul %s, %s",DA_DEST,DA_SRC);
      break;
    case 0xb9:
      ret=snprintf(str,len,"mul %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0xba:
      ret=snprintf(str,len,"mulu %s, %s",DA_DEST,DA_SRC);
      break;
    case 0xbb:
      ret=snprintf(str,len,"mulu %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0xbc:
      ret=snprintf(str,len,"div %s, %s",DA_DEST,DA_SRC);
      break;
    case 0xbd:
      ret=snprintf(str,len,"div %s, #$%x",DA_DEST,DA_IMM_I);
      break;
    case 0xbe:
      ret=snprintf(str,len,"divq %s",DA_DEST);
      break;
    case 0xbf:
      ret=snprintf(str,len,"divr %s",DA_DEST);
      break;

    case 0xc0:
      ret=snprintf(str,len,"beq $%x",pc+DA_BROFF);
      break;
    case 0xc1:
      ret=snprintf(str,len,"bne $%x",pc+DA_BROFF);
      break;
    case 0xc2:
      ret=snprintf(str,len,"bmi $%x",pc+DA_BROFF);
      break;
    case 0xc3:
      ret=snprintf(str,len,"bpl $%x",pc+DA_BROFF);
      break;
    case 0xc4:
      ret=snprintf(str,len,"bcs $%x",pc+DA_BROFF);
      break;
    case 0xc5:
      ret=snprintf(str,len,"bcc $%x",pc+DA_BROFF);
      break;

    case 0xc6:
      ret=snprintf(str,len,"loopc $%x",pc+DA_BROFF);
      break;
    case 0xc7:
      ret=snprintf(str,len,"loops $%x",pc+DA_BROFF);
      break;

    case 0xc8:
      ret=snprintf(str,len,"bbs %s.#%d, $%x",DA_DEST,ptr[1]>>3,pc+DA_BBROFF);
      break;
    case 0xc9:
      ret=snprintf(str,len,"bbc %s.#%d, $%x",DA_DEST,ptr[1]>>3,pc+DA_BBROFF);
      break;
    case 0xca:
      ret=snprintf(str,len,"bbs %s.%s, $%x",DA_DEST,DA_SRC,pc+DA_BBROFF);
      break;
    case 0xcb:
      ret=snprintf(str,len,"bbc %s.%s, $%x",DA_DEST,DA_SRC,pc+DA_BBROFF);
      break;
    case 0xcc:
      ret=snprintf(str,len,"mov %s, EX",DA_DEST);
      break;
    case 0xcd:
      ret=snprintf(str,len,"mov %s, IR",DA_DEST);
      break;
    case 0xce:
      ret=snprintf(str,len,"loop $%x",pc+DA_BROFF);
      break;
    case 0xcf:
      ret=snprintf(str,len,"bra $%x",pc+DA_BROFF);
      break;
    case 0xd0:
      ret=snprintf(str,len,"jmp $%x",DA_PIMM_I);
      break;
    case 0xd1:
      ret=snprintf(str,len,"jmp ($%x)",DA_PIMM_I);
      break;
    case 0xd2:
      ret=snprintf(str,len,"jsr $%x",DA_PIMM_I);
      break;
    case 0xd3:
      ret=snprintf(str,len,"jmp ($%x)",DA_PIMM_I);
      break;
    case 0xd4:
      ret=snprintf(str,len,"extc %s",DA_DEST);
      break;
    case 0xd5:
      ret=snprintf(str,len,"exts %s",DA_DEST);
      break;
    case 0xd6:
      ret=snprintf(str,len,"mov EV, %s",DA_DEST);
      break;
    case 0xd7:
      ret=snprintf(str,len,"mov IV, %s",DA_DEST);
      break;
    case 0xd8:
      ret=snprintf(str,len,"mov %s, ST",DA_DEST);
      break;
    case 0xd9:
      ret=snprintf(str,len,"mov ST, %s",DA_DEST);
      break;
    case 0xda:
      ret=snprintf(str,len,"mov %s, SB",DA_DEST);
      break;
    case 0xdb:
      ret=snprintf(str,len,"mov SB, %s",DA_DEST);
      break;

    case 0xdc:
      ret=snprintf(str,len,"flush");
      break;
    case 0xdd:
      ret=snprintf(str,len,"brk");
      break;
    case 0xde:
      ret=snprintf(str,len,"hlt");
      break;
    case 0xdf:
      ret=snprintf(str,len,"ret");
      break;

    case 0xe0:
      ret=snprintf(str,len,"set n");
      break;
    case 0xe1:
      ret=snprintf(str,len,"set d");
      break;
    case 0xe2:
      ret=snprintf(str,len,"set k");
      break;
    case 0xe3:
      ret=snprintf(str,len,"set e");
      break;
    case 0xe4:
      ret=snprintf(str,len,"set f");
      break;
    case 0xe5:
      ret=snprintf(str,len,"set i");
      break;
    case 0xe6:
      ret=snprintf(str,len,"set z");
      break;
    case 0xe7:
      ret=snprintf(str,len,"set c");
      break;
    case 0xe8:
      ret=snprintf(str,len,"clr n");
      break;
    case 0xe9:
      ret=snprintf(str,len,"clr d");
      break;
    case 0xea:
      ret=snprintf(str,len,"clr k");
      break;
    case 0xeb:
      ret=snprintf(str,len,"clr e");
      break;
    case 0xec:
      ret=snprintf(str,len,"clr f");
      break;
    case 0xed:
      ret=snprintf(str,len,"clr i");
      break;
    case 0xee:
      ret=snprintf(str,len,"clr z");
      break;
    case 0xef:
      ret=snprintf(str,len,"clr c");
      break;

    case 0xf0:
      ret=snprintf(str,len,"pushds");
      break;
    case 0xf1:
      ret=snprintf(str,len,"pushall");
      break;
    case 0xf2:
      ret=snprintf(str,len,"popds");
      break;
    case 0xf3:
      ret=snprintf(str,len,"popall");
      break;

    case 0xf4:
      ret=snprintf(str,len,"mov %s, SP",DA_DEST);
      break;
    case 0xf5:
      ret=snprintf(str,len,"mov SP, %s",DA_DEST);
      break;
    case 0xf6:
      ret=snprintf(str,len,"movc %s, F",DA_DEST);
      break;
    case 0xf7:
      ret=snprintf(str,len,"mov %s, PC",DA_DEST);
      break;
    case 0xf8:
      ret=snprintf(str,len,"lcc %s",DA_DEST);
      break;
    case 0xf9:
      ret=snprintf(str,len,"scc %s",DA_DEST);
      break;
    case 0xfa:
      ret=snprintf(str,len,"rcc");
      break;

    case 0xfb:
      ret=snprintf(str,len,"mov %s, %s",DA_SRC,DA_CTX);
      break;

    case 0xfc:
      ret=snprintf(str,len,"nop");
      break;

    case 0xfd:
      ret=snprintf(str,len,"mov %s, %s",DA_CTX,DA_SRC);
      break;
    case 0xfe:
      ret=snprintf(str,len,"store %s",DA_CTX);
      break;
    case 0xff:
      ret=snprintf(str,len,"rec %s",DA_CTX);
      break;

    default:
      ret=snprintf(str,len,"ILL ($%.2x)",ptr[0]);
      break;
  }

  return ret;
}

#define _DEST cpuRegDest[cpu->curIns[1]]
#define _SRC cpuRegSrc[cpu->curIns[1]]
#define _INDEX cpuRegIndex[cpu->curIns[1]]
#define _CTX cpuCtx[cpu->curIns[1]&7]
#define _ADDR (cpu->curIns[2]|(cpu->curIns[3]<<8)|(cpu->curIns[4]<<16)|(cpu->curIns[5]<<24))
#define _ADDR_DIRECT (cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16))
#define _ADDR_DIRECT2 (cpu->curIns[4]|(cpu->curIns[5]<<8)|(cpu->curIns[6]<<16))

#define _EXCEPTION(e) \
  cpu->flags|=FLAG_E; \
  cpu->nextIns[0]=0xfc; \
  cpu->initFetch=1; \
  cpu->ex=e; \
  printf("CPU: exception %.2x @ %.8x!\n",cpu->ex,cpu->pc); \
  cpu->ir=0xff; \
  cpu->pc=cpu->ev;

// Y/Z special props
#define _TOUCH(r,rl) \
  if (r==6 && !(cpu->flags&FLAG_D)) { \
    cpu->reg[6].i+=rl; \
  } \
  if (r==7) { \
    if (cpu->flags&FLAG_F) { \
      cpu->reg[7].i-=rl; \
    } else { \
      cpu->reg[7].i+=rl; \
    } \
  }

#define _ADD(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z|FLAG_C); \
  if ((d+=s)<s) cpu->flags|=FLAG_C; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _SUB(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z|FLAG_C); \
  if ((d-=s)<=s) cpu->flags|=FLAG_C; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _AND(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  d&=s; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _OR(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  d|=s; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _XOR(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  d^=s; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _CMP(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z|FLAG_C); \
  if ((d-s)<=s) cpu->flags|=FLAG_C; \
  if ((d-s)==0) cpu->flags|=FLAG_Z; \
  if ((d-s)&nb) cpu->flags|=FLAG_N;

#define _MOV(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  if ((d=s)==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _PUSH(s) \
  if (cpu->sp<=cpu->sb) { \
    _EXCEPTION(7); \
    break; \
  } \
  if (!coreWrite32(cpu->core,cpu->sp,s)) { \
    _EXCEPTION(2); \
    break; \
  } \
  cpu->sp-=4;

#define _POP(d,nb) \
  if (cpu->sp>=cpu->st) { \
    _EXCEPTION(7); \
    break; \
  } \
  if (!coreRead32(cpu->core,cpu->sp,&operand.i)) { \
    _EXCEPTION(1); \
    break; \
  } \
  d=operand.i; \
  cpu->sp+=4; \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _OP_REG_C \
   operand.c=cpu->reg[_SRC].c; \
   _TOUCH(_SRC,1);

#define _OP_REG_S \
   operand.s=cpu->reg[_SRC].s; \
   _TOUCH(_SRC,2);

#define _OP_REG_I \
   operand.i=cpu->reg[_SRC].i; \
   _TOUCH(_SRC,4);

#define _OP_IMM_C \
  operand.c=cpu->curIns[2];

#define _OP_IMM_S \
  operand.s=(cpu->curIns[2]|(cpu->curIns[3]<<8));

#define _OP_IMM_I \
  operand.i=(cpu->curIns[2]|(cpu->curIns[3]<<8)|(cpu->curIns[4]<<16)|(cpu->curIns[5]<<24));

#define _MUL(d,s,nb,t) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  d=(t)(d)*(t)(s); \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _MULU(d,s,nb) \
  cpu->flags&=~(FLAG_N|FLAG_Z); \
  d*=s; \
  if (d==0) cpu->flags|=FLAG_Z; \
  if (d&nb) cpu->flags|=FLAG_N;

#define _OP_ABS_C \
  if (!coreRead8(cpu->core,_ADDR,&operand.c)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_ABS_S \
  if (!coreRead16(cpu->core,_ADDR,&operand.s)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_ABS_I \
  if (!coreRead32(cpu->core,_ADDR,&operand.i)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_IND_C \
   if (!coreRead8(cpu->core,_ADDR+cpu->reg[_INDEX].i,&operand.c)) { \
     _TOUCH(_INDEX,1); \
     _EXCEPTION(1); \
     break; \
   } \
   _TOUCH(_INDEX,1);

#define _OP_IND_S \
   if (!coreRead16(cpu->core,_ADDR+cpu->reg[_INDEX].i,&operand.s)) { \
     _TOUCH(_INDEX,1); \
     _EXCEPTION(1); \
     break; \
   } \
   _TOUCH(_INDEX,2);

#define _OP_IND_I \
   if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&operand.i)) { \
     _TOUCH(_INDEX,1); \
     _EXCEPTION(1); \
     break; \
   } \
   _TOUCH(_INDEX,4);

#define _OP_PTR_ABS_C \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead8(cpu->core,indirectResol,&operand.c)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_ABS_S \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead16(cpu->core,indirectResol,&operand.s)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_ABS_I \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead32(cpu->core,indirectResol,&operand.i)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_POST_C \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead8(cpu->core,indirectResol+cpu->reg[_INDEX].i,&operand.c)) { \
     _TOUCH(_INDEX,1); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,1);

#define _OP_PTR_POST_S \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead16(cpu->core,indirectResol+cpu->reg[_INDEX].i,&operand.s)) { \
     _TOUCH(_INDEX,2); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,2);

#define _OP_PTR_POST_I \
  if (!coreRead32(cpu->core,_ADDR,&indirectResol)) { \
    _EXCEPTION(1); \
    break; \
  } \
  if (!coreRead32(cpu->core,indirectResol+cpu->reg[_INDEX].i,&operand.i)) { \
     _TOUCH(_INDEX,2); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,2);

#define _OP_PTR_PRE_C \
  if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) { \
    _TOUCH(_INDEX,4); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,4); \
  if (!coreRead8(cpu->core,indirectResol,&operand.c)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_PRE_S \
  if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) { \
    _TOUCH(_INDEX,4); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,4); \
  if (!coreRead16(cpu->core,indirectResol,&operand.s)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_PRE_I \
  if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) { \
    _TOUCH(_INDEX,4); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_INDEX,4); \
  if (!coreRead32(cpu->core,indirectResol,&operand.i)) { \
    _EXCEPTION(1); \
    break; \
  }

#define _OP_PTR_REG_C \
  if (!coreRead8(cpu->core,cpu->reg[_SRC].i,&operand.c)) { \
    _TOUCH(_SRC,1); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_SRC,1);

#define _OP_PTR_REG_S \
  if (!coreRead16(cpu->core,cpu->reg[_SRC].i,&operand.s)) { \
    _TOUCH(_SRC,2); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_SRC,2);

#define _OP_PTR_REG_I \
  if (!coreRead32(cpu->core,cpu->reg[_SRC].i,&operand.i)) { \
    _TOUCH(_SRC,4); \
    _EXCEPTION(1); \
    break; \
  } \
  _TOUCH(_SRC,4);

#define _BRANCH \
  cpu->initFetch=1; \
  cpu->pc+=(signed short)(cpu->curIns[1]|(cpu->curIns[2]<<8)); \
  cpu->nextIns[0]=0xfc;

#define _BRANCH_BIT \
  cpu->initFetch=1; \
  cpu->pc+=(signed short)(cpu->curIns[2]|(cpu->curIns[3]<<8)); \
  cpu->nextIns[0]=0xfc;

void cpuDumpState(s2CPU* cpu) {
  char daString[4096];
  printf(
    " A: %.8x   W: %.8x   F: %c%c%c%c%c%c%c%c\n"
    " B: %.8x   X: %.8x  PC: %.8x\n"
    " C: %.8x   Y: %.8x  CC: %.8x\n"
    " D: %.8x   Z: %.8x\n"
    "SP: %.8x  SB: %.8x  ST: %.8x\n"
    "EV: %.8x  IV: %.8x  EX: %.2x  IR: %.2x\n",
    cpu->reg[0].i,cpu->reg[4].i,
    (cpu->flags&0x80)?'N':'-',
    (cpu->flags&0x40)?'D':'-',
    (cpu->flags&0x20)?'K':'-',
    (cpu->flags&0x10)?'E':'-',
    (cpu->flags&8)?'F':'-',
    (cpu->flags&4)?'I':'-',
    (cpu->flags&2)?'Z':'-',
    (cpu->flags&1)?'C':'-',
    cpu->reg[1].i,cpu->reg[5].i,
    cpu->pc,
    cpu->reg[2].i,cpu->reg[6].i,
    cpu->cc,
    cpu->reg[3].i,cpu->reg[7].i,
    cpu->sp,cpu->sb,cpu->st,
    cpu->ev,cpu->iv,cpu->ex,cpu->ir
  );
  cpuDisAsm(cpu->curIns,daString,4095,cpu->pc);
  printf("-> %s\n",daString);
}

void cpuClock(s2CPU* cpu) {
  cpu->cycleTotal++;
  cpu->cc++;

  if (cpu->cycles) {
    if (cpu->debug) {
      printf("--- cycle %d (busy)\n",cpu->cycleTotal);
      cpuDumpState(cpu);
    }
    cpu->cycles--;
    return;
  }

  if (cpu->halted) {
    if (cpu->debug) {
      printf("--- cycle %d (halted)\n",cpu->cycleTotal);
      cpuDumpState(cpu);
    }
    return;
  }

  s2CPURegister operand;
  unsigned int indirectResol;

  *((uint64_t*)cpu->curIns)=*((uint64_t*)cpu->nextIns);
  if (cpu->debug) {
    if (cpu->initFetch) {
      printf("--- cycle %d (fetching)\n",cpu->cycleTotal);
    } else {
      printf("--- cycle %d\n",cpu->cycleTotal);
    }
    cpuDumpState(cpu);
  }

  cpu->nextIns[0]=0xfc;
  if (cpu->initFetch) {
    cpu->cycles=0;
    cpu->initFetch=0;
  } else {
    cpu->pc+=cpuInsAdvance[cpu->curIns[0]];
    cpu->cycles=cpuInsTiming[cpu->curIns[0]]-1;
  }
  if (!coreReadIns(cpu->core,cpu->pc,cpu->nextIns)) {
    if (cpu->debug) printf("CPU: couldn't read %.8x for execution!\n",cpu->pc);
    cpu->nextIns[0]=0xfc;
    //_EXCEPTION(3);
  }

  switch (cpu->curIns[0]) {
    /// CHAR
    case 0x00: // addc R, R
      _OP_REG_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x01: // addc R, #$val
      _OP_IMM_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x02: // addc R, $val
      _OP_ABS_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x03: // addc R, $val+R
      _OP_IND_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x04: // addc R, ($val)
      _OP_PTR_ABS_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x05: // addc R, ($val)+R
      _OP_PTR_POST_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x06: // addc R, ($val+R)
      _OP_PTR_PRE_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x07: // addc R, (R)
      _OP_PTR_REG_C;
      _ADD(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x08: // subc R, R
      _OP_REG_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x09: // subc R, #$val
      _OP_IMM_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0a: // subc R, $val
      _OP_ABS_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0b: // subc R, $val+R
      _OP_IND_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0c: // subc R, ($val)
      _OP_PTR_ABS_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0d: // subc R, ($val)+R
      _OP_PTR_POST_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0e: // subc R, ($val+R)
      _OP_PTR_PRE_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x0f: // subc R, (R)
      _OP_PTR_REG_C;
      _SUB(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x10: // andc R, R
      _OP_REG_C;
      _AND(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x11: // orc R, R
      _OP_REG_C;
      _OR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x12: // xorc R, R
      _OP_REG_C;
      _XOR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x13: // cmpc R, R
      _OP_REG_C;
      _CMP(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x14: // andc R, #$val
      _OP_IMM_C;
      _AND(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x15: // orc R, #$val
      _OP_IMM_C;
      _OR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x16: // xorc R, #$val
      _OP_IMM_C;
      _XOR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x17: // cmpc R, #$val
      _OP_IMM_C;
      _CMP(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x18: // andc R, $val
      _OP_ABS_C;
      _AND(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x19: // orc R, $val
      _OP_ABS_C;
      _OR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x1a: // xorc R, $val
      _OP_ABS_C;
      _XOR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x1b: // cmpc R, $val
      _OP_ABS_C;
      _CMP(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x1c: // andc R, (R)
      _OP_PTR_REG_C;
      _AND(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x1d: // orc R, (R)
      _OP_PTR_REG_C;
      _OR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x1e: // xorc R, (R)
      _OP_PTR_REG_C;
      _XOR(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x1f: // cmpc R, (R)
      _OP_PTR_REG_C;
      _CMP(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x20: // movc R, R
      _OP_REG_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x21: // movc R, #$val
      _OP_IMM_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x22: // movc R, $val
      _OP_ABS_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x23: // movc R, $val+R
      _OP_IND_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x24: // movc R, ($val)
      _OP_PTR_ABS_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x25: // movc R, ($val)+R
      _OP_PTR_POST_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x26: // movc R, ($val+R)
      _OP_PTR_PRE_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x27: // movc R, (R)
      _OP_PTR_REG_C;
      _MOV(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x28: // movc $dest, #$val
      operand.c=cpu->curIns[4];
      if (!coreWrite8(cpu->core,_ADDR_DIRECT,operand.c)) {
        _EXCEPTION(2);
        break;
      }
      break;

    case 0x2a: // movc $addr, R
      if (!coreWrite8(cpu->core,_ADDR,cpu->reg[_DEST].c)) {
        _TOUCH(_DEST,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      break;
    case 0x2b: // movc $addr+R, R
      if (!coreWrite8(cpu->core,_ADDR+cpu->reg[_INDEX].i,cpu->reg[_DEST].c)) {
        _TOUCH(_DEST,1);
        _TOUCH(_INDEX,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      _TOUCH(_INDEX,1);
      break;
    case 0x2c: // movc ($addr), R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite8(cpu->core,indirectResol,cpu->reg[_DEST].c)) {
        _TOUCH(_DEST,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      break;
    case 0x2d: // movc ($addr)+R, R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite8(cpu->core,indirectResol+cpu->reg[_INDEX].i,cpu->reg[_DEST].c)) {
        _TOUCH(_DEST,1);
        _TOUCH(_INDEX,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      _TOUCH(_INDEX,1);
      break;
    case 0x2e: // movc ($addr+R), R
      if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) {
        _TOUCH(_INDEX,4);
        _EXCEPTION(1);
        break;
      }
      _TOUCH(_INDEX,4);
      if (!coreWrite8(cpu->core,indirectResol,cpu->reg[_DEST].c)) {
        _TOUCH(_DEST,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      break;
    case 0x2f: // movc (R), R
      if (!coreWrite8(cpu->core,cpu->reg[_SRC].i,cpu->reg[_DEST].c)) {
        _TOUCH(_SRC,1);
        _TOUCH(_DEST,1);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_SRC,1);
      _TOUCH(_DEST,1);
      break;

    case 0x30: // shac R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].c=((signed char)cpu->reg[_DEST].c)<<(_SRC);
        } else { // register
          cpu->reg[_DEST].c=((signed char)cpu->reg[_DEST].c)<<(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].c=((signed char)cpu->reg[_DEST].c)>>(_SRC);
        } else { // register
          cpu->reg[_DEST].c=((signed char)cpu->reg[_DEST].c)>>(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].c==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].c&0x80) cpu->flags|=FLAG_N;
      break;
    case 0x31: // shlc R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].c<<=_SRC;
        } else { // register
          cpu->reg[_DEST].c<<=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].c>>=_SRC;
        } else { // register
          cpu->reg[_DEST].c>>=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].c==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].c&0x80) cpu->flags|=FLAG_N;
      break;

    case 0x32: // rolc R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.c&=7;
      cpu->reg[_DEST].c=(cpu->reg[_DEST].c<<operand.c)|(cpu->reg[_DEST].c>>(-operand.c&7));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].c==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].c&0x80) cpu->flags|=FLAG_N;
      break;
    case 0x33: // rorc R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.c&=7;
      cpu->reg[_DEST].c=(cpu->reg[_DEST].c>>operand.c)|(cpu->reg[_DEST].c<<(-operand.c&7));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].c==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].c&0x80) cpu->flags|=FLAG_N;
      break;

    case 0x34: // swpc R, R
      cpu->flags&=~(FLAG_N|FLAG_Z);
      cpu->reg[_DEST].c^=cpu->reg[_SRC].c;
      cpu->reg[_SRC].c^=cpu->reg[_DEST].c;
      cpu->reg[_DEST].c^=cpu->reg[_SRC].c;
      if (cpu->reg[_DEST].c==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].c&0x80) cpu->flags|=FLAG_N;
      break;

    case 0x35: // pushc R
      _PUSH(cpu->reg[_DEST].c);
      break;
    case 0x36: // popc R
      _POP(cpu->reg[_DEST].c,0x80);
      break;
    case 0x37: // pushc #$val
      _PUSH(cpu->curIns[1]);
      break;

    case 0x38: // mulc R, R
      _OP_REG_C;
      _MUL(cpu->reg[_DEST].c,operand.c,0x80,signed char);
      break;
    case 0x39: // mulc R, #$val
      _OP_IMM_C;
      _MUL(cpu->reg[_DEST].c,operand.c,0x80,signed char);
      break;
    case 0x3a: // muluc R, R
      _OP_REG_C;
      _MULU(cpu->reg[_DEST].c,operand.c,0x80);
      break;
    case 0x3b: // muluc R, #$val
      _OP_IMM_C;
      _MULU(cpu->reg[_DEST].c,operand.c,0x80);
      break;

    case 0x3c: // divc R, R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_REG_C;
      cpu->divN=cpu->reg[_DEST].c;
      cpu->divD=operand.c;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;
    case 0x3d: // divc R, #$val
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_IMM_C;
      cpu->divN=cpu->reg[_DEST].c;
      cpu->divD=operand.c;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;

    case 0x3e: // divqc R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].c,cpu->divQ,0x80);
      break;
    case 0x3f: // divrc R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].c,cpu->divR,0x80);
      break;

    /// SHORT
    case 0x40: // adds R, R
      _OP_REG_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x41: // adds R, #$val
      _OP_IMM_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x42: // adds R, $val
      _OP_ABS_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x43: // adds R, $val+R
      _OP_IND_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x44: // adds R, ($val)
      _OP_PTR_ABS_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x45: // adds R, ($val)+R
      _OP_PTR_POST_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x46: // adds R, ($val+R)
      _OP_PTR_PRE_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x47: // adds R, (R)
      _OP_PTR_REG_S;
      _ADD(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x48: // subs R, R
      _OP_REG_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x49: // subs R, #$val
      _OP_IMM_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4a: // subs R, $val
      _OP_ABS_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4b: // subs R, $val+R
      _OP_IND_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4c: // subs R, ($val)
      _OP_PTR_ABS_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4d: // subs R, ($val)+R
      _OP_PTR_POST_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4e: // subs R, ($val+R)
      _OP_PTR_PRE_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x4f: // subs R, (R)
      _OP_PTR_REG_S;
      _SUB(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x50: // ands R, R
      _OP_REG_S;
      _AND(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x51: // ors R, R
      _OP_REG_S;
      _OR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x52: // xors R, R
      _OP_REG_S;
      _XOR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x53: // cmps R, R
      _OP_REG_S;
      _CMP(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x54: // ands R, #$val
      _OP_IMM_S;
      _AND(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x55: // ors R, #$val
      _OP_IMM_S;
      _OR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x56: // xors R, #$val
      _OP_IMM_S;
      _XOR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x57: // cmps R, #$val
      _OP_IMM_S;
      _CMP(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x58: // ands R, $val
      _OP_ABS_S;
      _AND(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x59: // ors R, $val
      _OP_ABS_S;
      _OR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x5a: // xors R, $val
      _OP_ABS_S;
      _XOR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x5b: // cmps R, $val
      _OP_ABS_S;
      _CMP(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x5c: // ands R, (R)
      _OP_PTR_REG_S;
      _AND(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x5d: // ors R, (R)
      _OP_PTR_REG_S;
      _OR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x5e: // xors R, (R)
      _OP_PTR_REG_S;
      _XOR(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x5f: // cmps R, (R)
      _OP_PTR_REG_S;
      _CMP(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x60: // movs R, R
      _OP_REG_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x61: // movs R, #$val
      _OP_IMM_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x62: // movs R, $val
      _OP_ABS_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x63: // movs R, $val+R
      _OP_IND_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x64: // movs R, ($val)
      _OP_PTR_ABS_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x65: // movs R, ($val)+R
      _OP_PTR_POST_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x66: // movs R, ($val+R)
      _OP_PTR_PRE_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x67: // movs R, (R)
      _OP_PTR_REG_S;
      _MOV(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x68: // movs $dest, #$val
      operand.s=cpu->curIns[4]|(cpu->curIns[5]<<8);
      if (!coreWrite16(cpu->core,_ADDR_DIRECT,operand.s)) {
        _EXCEPTION(2);
        break;
      }
      break;

    case 0x6a: // movs $addr, R
      if (!coreWrite16(cpu->core,_ADDR,cpu->reg[_DEST].s)) {
        _TOUCH(_DEST,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,2);
      break;
    case 0x6b: // movs $addr+R, R
      if (!coreWrite16(cpu->core,_ADDR+cpu->reg[_INDEX].i,cpu->reg[_DEST].s)) {
        _TOUCH(_DEST,2);
        _TOUCH(_INDEX,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,2);
      _TOUCH(_INDEX,2);
      break;
    case 0x6c: // movs ($addr), R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite16(cpu->core,indirectResol,cpu->reg[_DEST].s)) {
        _TOUCH(_DEST,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,1);
      break;
    case 0x6d: // movs ($addr)+R, R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite16(cpu->core,indirectResol+cpu->reg[_INDEX].i,cpu->reg[_DEST].s)) {
        _TOUCH(_DEST,2);
        _TOUCH(_INDEX,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,2);
      _TOUCH(_INDEX,2);
      break;
    case 0x6e: // movs ($addr+R), R
      if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) {
        _TOUCH(_INDEX,4);
        _EXCEPTION(1);
        break;
      }
      _TOUCH(_INDEX,4);
      if (!coreWrite16(cpu->core,indirectResol,cpu->reg[_DEST].s)) {
        _TOUCH(_DEST,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,2);
      break;
    case 0x6f: // movs (R), R
      if (!coreWrite16(cpu->core,cpu->reg[_SRC].i,cpu->reg[_DEST].s)) {
        _TOUCH(_SRC,2);
        _TOUCH(_DEST,2);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_SRC,2);
      _TOUCH(_DEST,2);
      break;

    case 0x70: // shas R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].s=((signed short)cpu->reg[_DEST].s)<<(_SRC);
        } else { // register
          cpu->reg[_DEST].s=((signed short)cpu->reg[_DEST].s)<<(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].s=((signed short)cpu->reg[_DEST].s)>>(_SRC);
        } else { // register
          cpu->reg[_DEST].s=((signed short)cpu->reg[_DEST].s)>>(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].s==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].s&0x8000) cpu->flags|=FLAG_N;
      break;
    case 0x71: // shls R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].s<<=_SRC;
        } else { // register
          cpu->reg[_DEST].s<<=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].s>>=_SRC;
        } else { // register
          cpu->reg[_DEST].s>>=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].s==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].s&0x8000) cpu->flags|=FLAG_N;
      break;

    case 0x72: // rols R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.c&=15;
      cpu->reg[_DEST].s=(cpu->reg[_DEST].s<<operand.c)|(cpu->reg[_DEST].s>>(-operand.c&15));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].s==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].s&0x8000) cpu->flags|=FLAG_N;
      break;
    case 0x73: // rors R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.s&=15;
      cpu->reg[_DEST].s=(cpu->reg[_DEST].s>>operand.c)|(cpu->reg[_DEST].s<<(-operand.c&7));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].s==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].s&0x8000) cpu->flags|=FLAG_N;
      break;

    case 0x74: // swps R, R
      cpu->flags&=~(FLAG_N|FLAG_Z);
      cpu->reg[_DEST].s^=cpu->reg[_SRC].s;
      cpu->reg[_SRC].s^=cpu->reg[_DEST].s;
      cpu->reg[_DEST].s^=cpu->reg[_SRC].s;
      if (cpu->reg[_DEST].s==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].s&0x8000) cpu->flags|=FLAG_N;
      break;

    case 0x75: // pushs R
      _PUSH(cpu->reg[_DEST].s);
      break;
    case 0x76: // pops R
      _POP(cpu->reg[_DEST].s,0x8000);
      break;
    case 0x77: // pushs #$val
      _PUSH((cpu->curIns[1]|(cpu->curIns[2]<<8)));
      break;

    case 0x78: // muls R, R
      _OP_REG_S;
      _MUL(cpu->reg[_DEST].s,operand.s,0x8000,signed short);
      break;
    case 0x79: // muls R, #$val
      _OP_IMM_S;
      _MUL(cpu->reg[_DEST].s,operand.s,0x8000,signed short);
      break;
    case 0x7a: // mulus R, R
      _OP_REG_S;
      _MULU(cpu->reg[_DEST].s,operand.s,0x8000);
      break;
    case 0x7b: // mulus R, #$val
      _OP_IMM_S;
      _MULU(cpu->reg[_DEST].s,operand.s,0x8000);
      break;

    case 0x7c: // divs R, R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_REG_S;
      cpu->divN=cpu->reg[_DEST].s;
      cpu->divD=operand.s;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;
    case 0x7d: // divs R, #$val
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_IMM_S;
      cpu->divN=cpu->reg[_DEST].s;
      cpu->divD=operand.s;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;

    case 0x7e: // divqs R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].s,cpu->divQ,0x8000);
      break;
    case 0x7f: // divrs R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].s,cpu->divR,0x8000);
      break;

    /// INT
    case 0x80: // add R, R
      _OP_REG_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x81: // add R, #$val
      _OP_IMM_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x82: // add R, $val
      _OP_ABS_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x83: // add R, $val+R
      _OP_IND_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x84: // add R, ($val)
      _OP_PTR_ABS_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x85: // add R, ($val)+R
      _OP_PTR_POST_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x86: // add R, ($val+R)
      _OP_PTR_PRE_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x87: // add R, (R)
      _OP_PTR_REG_I;
      _ADD(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0x88: // sub R, R
      _OP_REG_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x89: // sub R, #$val
      _OP_IMM_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8a: // sub R, $val
      _OP_ABS_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8b: // sub R, $val+R
      _OP_IND_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8c: // sub R, ($val)
      _OP_PTR_ABS_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8d: // sub R, ($val)+R
      _OP_PTR_POST_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8e: // sub R, ($val+R)
      _OP_PTR_PRE_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x8f: // sub R, (R)
      _OP_PTR_REG_I;
      _SUB(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0x90: // and R, R
      _OP_REG_I;
      _AND(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x91: // or R, R
      _OP_REG_I;
      _OR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x92: // xor R, R
      _OP_REG_I;
      _XOR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x93: // cmp R, R
      _OP_REG_I;
      _CMP(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0x94: // and R, #$val
      _OP_IMM_I;
      _AND(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x95: // or R, #$val
      _OP_IMM_I;
      _OR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x96: // xor R, #$val
      _OP_IMM_I;
      _XOR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x97: // cmp R, #$val
      _OP_IMM_I;
      _CMP(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0x98: // and R, $val
      _OP_ABS_I;
      _AND(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x99: // or R, $val
      _OP_ABS_I;
      _OR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x9a: // xor R, $val
      _OP_ABS_I;
      _XOR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x9b: // cmp R, $val
      _OP_ABS_I;
      _CMP(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0x9c: // and R, (R)
      _OP_PTR_REG_I;
      _AND(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x9d: // or R, (R)
      _OP_PTR_REG_I;
      _OR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x9e: // xor R, (R)
      _OP_PTR_REG_I;
      _XOR(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0x9f: // cmp R, (R)
      _OP_PTR_REG_I;
      _CMP(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xa0: // mov R, R
      _OP_REG_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa1: // mov R, #$val
      _OP_IMM_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa2: // mov R, $val
      _OP_ABS_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa3: // mov R, $val+R
      _OP_IND_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa4: // mov R, ($val)
      _OP_PTR_ABS_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa5: // mov R, ($val)+R
      _OP_PTR_POST_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa6: // mov R, ($val+R)
      _OP_PTR_PRE_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xa7: // mov R, (R)
      _OP_PTR_REG_I;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xaa: // mov $addr, R
      if (!coreWrite32(cpu->core,_ADDR,cpu->reg[_DEST].i)) {
        _TOUCH(_DEST,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,4);
      break;
    case 0xab: // mov $addr+R, R
      if (!coreWrite32(cpu->core,_ADDR+cpu->reg[_INDEX].i,cpu->reg[_DEST].i)) {
        _TOUCH(_DEST,4);
        _TOUCH(_INDEX,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,4);
      _TOUCH(_INDEX,4);
      break;
    case 0xac: // mov ($addr), R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite32(cpu->core,indirectResol,cpu->reg[_DEST].i)) {
        _TOUCH(_DEST,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,4);
      break;
    case 0xad: // mov ($addr)+R, R
      if (!coreRead32(cpu->core,_ADDR,&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      if (!coreWrite32(cpu->core,indirectResol+cpu->reg[_INDEX].i,cpu->reg[_DEST].i)) {
        _TOUCH(_DEST,4);
        _TOUCH(_INDEX,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,4);
      _TOUCH(_INDEX,4);
      break;
    case 0xae: // mov ($addr+R), R
      if (!coreRead32(cpu->core,_ADDR+cpu->reg[_INDEX].i,&indirectResol)) {
        _TOUCH(_INDEX,4);
        _EXCEPTION(1);
        break;
      }
      _TOUCH(_INDEX,4);
      if (!coreWrite32(cpu->core,indirectResol,cpu->reg[_DEST].i)) {
        _TOUCH(_DEST,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_DEST,4);
      break;
    case 0xaf: // mov (R), R
      if (!coreWrite32(cpu->core,cpu->reg[_SRC].i,cpu->reg[_DEST].i)) {
        _TOUCH(_SRC,4);
        _TOUCH(_DEST,4);
        _EXCEPTION(2);
        break;
      }
      _TOUCH(_SRC,4);
      _TOUCH(_DEST,4);
      break;

    case 0xb0: // sha R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].i=((signed int)cpu->reg[_DEST].i)<<(_SRC);
        } else { // register
          cpu->reg[_DEST].i=((signed int)cpu->reg[_DEST].i)<<(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].i=((signed int)cpu->reg[_DEST].i)>>(_SRC);
        } else { // register
          cpu->reg[_DEST].i=((signed int)cpu->reg[_DEST].i)>>(cpu->reg[_SRC].c);
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].i==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].i&0x80000000) cpu->flags|=FLAG_N;
      break;
    case 0xb1: // shl R, d#R
      if (cpu->curIns[1]&0x80) { // left
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].i<<=_SRC;
        } else { // register
          cpu->reg[_DEST].i<<=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      } else { // right
        if (cpu->curIns[1]&8) { // fixed
          cpu->reg[_DEST].i>>=_SRC;
        } else { // register
          cpu->reg[_DEST].i>>=cpu->reg[_SRC].c;
          _TOUCH(_SRC,1);
        }
      }
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].i==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].i&0x80000000) cpu->flags|=FLAG_N;
      break;

    case 0xb2: // rol R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.c&=31;
      cpu->reg[_DEST].i=(cpu->reg[_DEST].i<<operand.c)|(cpu->reg[_DEST].i>>(-operand.c&31));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].i==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].i&0x80000000) cpu->flags|=FLAG_N;
      break;
    case 0xb3: // ror R, #R
      if (cpu->curIns[1]&8) { // fixed
        operand.c=_SRC;
      } else { // register
        operand.c=cpu->reg[_SRC].c;
        _TOUCH(_SRC,1);
      }
      operand.c&=31;
      cpu->reg[_DEST].i=(cpu->reg[_DEST].i>>operand.c)|(cpu->reg[_DEST].i<<(-operand.c&31));
      cpu->flags&=~(FLAG_N|FLAG_Z);
      if (cpu->reg[_DEST].i==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].i&0x80000000) cpu->flags|=FLAG_N;
      break;

    case 0xb4: // swp R, R
      cpu->flags&=~(FLAG_N|FLAG_Z);
      cpu->reg[_DEST].i^=cpu->reg[_SRC].i;
      cpu->reg[_SRC].i^=cpu->reg[_DEST].i;
      cpu->reg[_DEST].i^=cpu->reg[_SRC].i;
      if (cpu->reg[_DEST].i==0) cpu->flags|=FLAG_Z;
      if (cpu->reg[_DEST].i&0x80000000) cpu->flags|=FLAG_N;
      break;

    case 0xb5: // push R
      _PUSH(cpu->reg[_DEST].i);
      break;
    case 0xb6: // pop R
      _POP(cpu->reg[_DEST].i,0x80000000);
      break;
    case 0xb7: // push #$val
      _PUSH((cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16)|(cpu->curIns[4]<<24)));
      break;

    case 0xb8: // mul R, R
      _OP_REG_I;
      _MUL(cpu->reg[_DEST].i,operand.i,0x80000000,signed int);
      break;
    case 0xb9: // mul R, #$val
      _OP_IMM_I;
      _MUL(cpu->reg[_DEST].i,operand.i,0x80000000,signed int);
      break;
    case 0xba: // mulu R, R
      _OP_REG_I;
      _MULU(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xbb: // mulu R, #$val
      _OP_IMM_I;
      _MULU(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xbc: // div R, R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_REG_I;
      cpu->divN=cpu->reg[_DEST].i;
      cpu->divD=operand.i;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;
    case 0xbd: // div R, #$val
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _OP_IMM_I;
      cpu->divN=cpu->reg[_DEST].i;
      cpu->divD=operand.i;
      if (cpu->divD==0) {
        _EXCEPTION(4);
        break;
      }
      cpu->divBusy=1;
      break;

    case 0xbe: // divq R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].i,cpu->divQ,0x80000000);
      break;
    case 0xbf: // divr R
      cpu->flags&=~(FLAG_C);
      if (cpu->divBusy) {
        cpu->flags|=FLAG_C;
        break;
      }
      _MOV(cpu->reg[_DEST].i,cpu->divR,0x80000000);
      break;

    /// SPECIAL
    case 0xc0: // beq
      if (cpu->flags&FLAG_Z) {
        _BRANCH;
      }
      break;
    case 0xc1: // bne
      if (!(cpu->flags&FLAG_Z)) {
        _BRANCH;
      }
      break;
    case 0xc2: // bmi
      if (cpu->flags&FLAG_N) {
        _BRANCH;
      }
      break;
    case 0xc3: // bpl
      if (!(cpu->flags&FLAG_N)) {
        _BRANCH;
      }
      break;
    case 0xc4: // bcs
      if (cpu->flags&FLAG_C) {
        _BRANCH;
      }
      break;
    case 0xc5: // bcc
      if (!(cpu->flags&FLAG_C)) {
        _BRANCH;
      }
      break;

    case 0xc6: // loopc
      if ((cpu->reg[6].i-=1)!=0) {
        _BRANCH;
      }
      break;

    case 0xc7: // loops
      if ((cpu->reg[6].i-=2)!=0) {
        _BRANCH;
      }
      break;

    case 0xc8: // bbs R.#b, $off
      if (cpu->reg[_DEST].i&((cpu->curIns[1]>>3)&31)) {
        _BRANCH_BIT;
      }
      break;
    case 0xc9: // bbc R.#b, $off
      if (!(cpu->reg[_DEST].i&((cpu->curIns[1]>>3)&31))) {
        _BRANCH_BIT;
      }
      break;
    case 0xca: // bbs R.R, $off
      if (cpu->reg[_DEST].i&(cpu->reg[_SRC].i&31)) {
        _BRANCH_BIT;
      }
      break;
    case 0xcb: // bbc R.R, $off
      if (!(cpu->reg[_DEST].i&(cpu->reg[_SRC].i&31))) {
        _BRANCH_BIT;
      }
      break;
    case 0xcc: // mov R, EX
      operand.i=cpu->ex;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xcd: // mov R, IR
      operand.i=cpu->ir;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xce: // loop
      if ((cpu->reg[6].i-=4)!=0) {
        _BRANCH;
      }
      break;
    case 0xcf: // bra
      _BRANCH;
      break;

    case 0xd0: // jmp $addr
      cpu->initFetch=1;
      cpu->pc=cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16)|(cpu->curIns[4]<<24);
      cpu->nextIns[0]=0xfc;
      break;
    case 0xd1: // jmp ($addr)
      if (!coreRead32(cpu->core,cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16)|(cpu->curIns[4]<<24),&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      cpu->initFetch=1;
      cpu->pc=indirectResol;
      cpu->nextIns[0]=0xfc;
      break;
    case 0xd2: // jsr $addr
      _PUSH(cpu->pc);
      cpu->initFetch=1;
      cpu->pc=cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16)|(cpu->curIns[4]<<24);
      cpu->nextIns[0]=0xfc;
      break;
    case 0xd3: // jsr ($addr)
      if (!coreRead32(cpu->core,cpu->curIns[1]|(cpu->curIns[2]<<8)|(cpu->curIns[3]<<16)|(cpu->curIns[4]<<24),&indirectResol)) {
        _EXCEPTION(1);
        break;
      }
      _PUSH(cpu->pc);
      cpu->initFetch=1;
      cpu->pc=indirectResol;
      cpu->nextIns[0]=0xfc;
      break;

    case 0xd4: // extc R
      operand.i=cpu->reg[_DEST].i;
      if (operand.i&0x80) {
        operand.i|=0xffffff00;
      } else {
        operand.i&=0xff;
      }
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xd5: // exts R
      operand.i=cpu->reg[_DEST].i;
      if (operand.i&0x8000) {
        operand.i|=0xffff0000;
      } else {
        operand.i&=0xffff;
      }
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xd6: // mov EV, R
      if (cpu->flags&FLAG_K) {
        cpu->ev=cpu->reg[_DEST].i;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xd7: // mov IV, R
      if (cpu->flags&FLAG_K) {
        cpu->iv=cpu->reg[_DEST].i;
      } else {
        _EXCEPTION(5);
      }
      break;

    case 0xd8: // mov R, ST
      operand.i=cpu->st;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xd9: // mov ST, R
      cpu->st=cpu->reg[_DEST].i;
      break;

    case 0xda: // mov R, SB
      operand.i=cpu->sb;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xdb: // mov SB, R
      cpu->sb=cpu->reg[_DEST].i;
      break;

    case 0xdc: // flush
      // currently does nothing
      break;

    case 0xdd: // brk
      coreRaiseIRQ(cpu->core,1);
      break;

    case 0xde: // hlt
      if (cpu->flags&FLAG_I) {
        _EXCEPTION(9);
      } else {
        cpu->halted=1;
      }
      break;

    case 0xdf: // ret
      if (cpu->sp>=cpu->st) {
        _EXCEPTION(7);
        break;
      }
      if (!coreRead32(cpu->core,cpu->sp,&operand.i)) {
        _EXCEPTION(1);
        break;
      }
      cpu->initFetch=1;
      cpu->pc=operand.i;
      cpu->nextIns[0]=0xfc;
      cpu->sp+=4;
      break;

    case 0xe0: // set n
      cpu->flags|=FLAG_N;
      break;
    case 0xe1: // set d
      cpu->flags|=FLAG_D;
      break;
    case 0xe2: // set k
      if (cpu->flags&FLAG_K) {
        cpu->flags|=FLAG_K;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xe3: // set e
      if (cpu->flags&FLAG_K) {
        cpu->flags|=FLAG_E;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xe4: // set f
      cpu->flags|=FLAG_F;
      break;
    case 0xe5: // set i
      // TODO: think of a way to allow this in non-kernel mode
      if (cpu->flags&FLAG_K) {
        cpu->flags|=FLAG_I;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xe6: // set z
      cpu->flags|=FLAG_Z;
      break;
    case 0xe7: // set c
      cpu->flags|=FLAG_C;
      break;

    case 0xe8: // clr n
      cpu->flags&=~FLAG_N;
      break;
    case 0xe9: // clr d
      cpu->flags&=~FLAG_D;
      break;
    case 0xea: // clr k
      if (cpu->flags&FLAG_K) {
        cpu->flags&=~FLAG_K;
        // TODO: go into non-kernel mode
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xeb: // clr e
      if (cpu->flags&FLAG_K) {
        cpu->flags&=~FLAG_E;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xec: // clr f
      cpu->flags&=~FLAG_F;
      break;
    case 0xed: // clr i
      // TODO: think of a way to allow this in non-kernel mode
      if (cpu->flags&FLAG_K) {
        cpu->flags&=~FLAG_I;
      } else {
        _EXCEPTION(5);
      }
      break;
    case 0xee: // clr z
      cpu->flags&=~FLAG_Z;
      break;
    case 0xef: // clr c
      cpu->flags&=~FLAG_C;
      break;

    // TODO: implement these
    case 0xf0: // pushds
      break;
    case 0xf1: // pushall
      break;
    case 0xf2: // popds
      break;
    case 0xf3: // popall
      break;

    case 0xf4: // mov R, SP
      operand.i=cpu->sp;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xf5: // mov SP, R
      cpu->sp=cpu->reg[_DEST].i;
      break;
    case 0xf6: // movc R, F
      operand.c=cpu->flags;
      cpu->reg[_DEST].c=cpu->flags;
      break;
    case 0xf7: // mov R, PC
      operand.i=cpu->pc;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;

    case 0xf8: // lcc R
      operand.i=cpu->cc;
      _MOV(cpu->reg[_DEST].i,operand.i,0x80000000);
      break;
    case 0xf9: // scc R
      cpu->cc=cpu->reg[_DEST].i;
      break;
    case 0xfa: // rcc
      cpu->cc=0;
      break;

    case 0xfb: // mov R, K
      if (_CTX>4) {
        _EXCEPTION(11);
        break;
      }
      cpu->reg[_SRC].i=cpu->ctx[_CTX].reg[_SRC].i;
      break;

    case 0xfc: // nop
      break;

    case 0xfd: // mov K, R
      if (_CTX>4) {
        _EXCEPTION(11);
        break;
      }
      cpu->ctx[_CTX].reg[_SRC].i=cpu->reg[_SRC].i;
      break;
    case 0xfe: // store K
      if (_CTX>4) {
        _EXCEPTION(11);
        break;
      }
      cpu->ctx[_CTX].reg[0].i=cpu->reg[0].i;
      cpu->ctx[_CTX].reg[1].i=cpu->reg[1].i;
      cpu->ctx[_CTX].reg[2].i=cpu->reg[2].i;
      cpu->ctx[_CTX].reg[3].i=cpu->reg[3].i;
      cpu->ctx[_CTX].reg[4].i=cpu->reg[4].i;
      cpu->ctx[_CTX].reg[5].i=cpu->reg[5].i;
      cpu->ctx[_CTX].reg[6].i=cpu->reg[6].i;
      cpu->ctx[_CTX].reg[7].i=cpu->reg[7].i;
      cpu->ctx[_CTX].sp=cpu->sp;
      cpu->ctx[_CTX].st=cpu->st;
      cpu->ctx[_CTX].sb=cpu->sb;
      cpu->ctx[_CTX].flags=cpu->flags;
      break;
    case 0xff: // rec K
      if (_CTX>4) {
        _EXCEPTION(11);
        break;
      }
      cpu->reg[0].i=cpu->ctx[_CTX].reg[0].i;
      cpu->reg[1].i=cpu->ctx[_CTX].reg[1].i;
      cpu->reg[2].i=cpu->ctx[_CTX].reg[2].i;
      cpu->reg[3].i=cpu->ctx[_CTX].reg[3].i;
      cpu->reg[4].i=cpu->ctx[_CTX].reg[4].i;
      cpu->reg[5].i=cpu->ctx[_CTX].reg[5].i;
      cpu->reg[6].i=cpu->ctx[_CTX].reg[6].i;
      cpu->reg[7].i=cpu->ctx[_CTX].reg[7].i;
      cpu->sp=cpu->ctx[_CTX].sp;
      cpu->st=cpu->ctx[_CTX].st;
      cpu->sb=cpu->ctx[_CTX].sb;
      cpu->flags=cpu->ctx[_CTX].flags;
      break;

    default: // illegal instruction
      _EXCEPTION(6);
      break;
  }
}
