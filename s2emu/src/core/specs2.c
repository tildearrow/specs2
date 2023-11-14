#include "specs2.h"
#include <stdio.h>
#include <stdlib.h>

void systemInit(s2System* sys, unsigned int memCapacity) {
  coreInit(&sys->core);
  cpuInit(&sys->cpu,&sys->core);
  cuInit(&sys->cu);
  vuInit(&sys->vu,65536);
  suInit(&sys->su,8192,false);

  sys->memCapacity=memCapacity;
  sys->memory=malloc(memCapacity);

  sys->frame=malloc(1024*768*sizeof(unsigned short));
  sys->framePos=0;
  sys->frameSyncCycles=0;
  sys->prevFrameSyncCycles=0;
  sys->frameCycles=0;
  sys->frameCyclesCur=0;
  sys->vuOutput=0;
  sys->cuOutput=0;
  sys->videoOut=0;

  coreSetMemory(&sys->core,sys->memory,sys->memCapacity);
  coreBind(&sys->core,&sys->cu,&sys->vu,&sys->su);

  printf("reading BIOS...\n");
  FILE* biosf=fopen("bios.bin","rb");
  if (biosf==NULL) {
    perror("couldn't open bios.bin");
  } else {
    fseek(biosf,0,SEEK_END);
    unsigned int biosLen=ftell(biosf);
    if (biosLen>32768) biosLen=32768;
    unsigned char* bios=malloc(biosLen);
    fseek(biosf,0,SEEK_SET);
    fread(bios,1,biosLen,biosf);
    fclose(biosf);
    coreSetBIOS(&sys->core,bios,biosLen);
  }

  printf("reading char palette...\n");
  FILE* palf=fopen("palette.bin","rb");
  if (palf==NULL) {
    perror("couldn't open palette.bin");
  } else {
    fseek(palf,0,SEEK_END);
    unsigned int palLen=ftell(palf);
    if (palLen>512) palLen=512;
    unsigned short* pal=malloc(512);
    fseek(palf,0,SEEK_SET);
    fread(pal,1,palLen,palf);
    fclose(palf);
    cuSetPalette(&sys->cu,pal);
    free(pal);
  }

  printf("reading char font...\n");
  FILE* fontf=fopen("font0.bin","rb");
  if (fontf==NULL) {
    perror("couldn't open font0.bin");
  } else {
    fseek(fontf,0,SEEK_END);
    unsigned int fontLen=ftell(fontf);
    if (fontLen>4096) fontLen=4096;
    unsigned char* font=malloc(4096);
    fseek(fontf,0,SEEK_SET);
    fread(font,1,fontLen,fontf);
    fclose(fontf);
    cuSetFont(&sys->cu,font);
  }
}

void systemAdvance(s2System* sys, unsigned int cycles) {
  //static unsigned short vuOut;
  static unsigned short cuOut;
  do {
    coreClock(&sys->core);
    //vuOut=vuClock(&sys->vu);
    cuOut=cuClock(&sys->cu);
    /*if (vuOut!=VU_HOLD) {
      sys->vuOutput=vuOut;
    }*/
    if (cuOut!=CU_HOLD) {
      sys->videoOut=cuOut;
    }
    if (sys->core.clockCPU) cpuClock(&sys->cpu);
    if (sys->core.clockSU) suClock(&sys->su,&sys->suOutL,&sys->suOutR);

    sys->frameCyclesCur++;

    // terrible sync detection code
    if (sys->videoOut==VU_SYNC) {
      sys->prevFrameSyncCycles=sys->frameSyncCycles;
      if (++sys->frameSyncCycles>32 && sys->prevFrameSyncCycles<=32) {
        sys->framePos=(sys->framePos+1023)&(~1023);
      }
      if (sys->frameSyncCycles>1024) {
        sys->frameSyncCycles=1024;
        sys->framePos=0;
        if (sys->frameCyclesCur>10000) {
          sys->frameCycles=sys->frameCyclesCur;
          sys->frameCyclesCur=0;
        }
      }
    } else {
      sys->frameSyncCycles-=4;
      if (sys->frameSyncCycles<0) sys->frameSyncCycles=0;
    }
    if (sys->framePos>=0 && sys->framePos<1024*768) {
      sys->frame[sys->framePos]=sys->videoOut;
    }
    if (++sys->framePos>1024*768) {
      sys->framePos=0;
    }
  } while (--cycles && sys->frameCyclesCur!=0);
}

void systemReset(s2System* sys) {
  coreReset(&sys->core);
  cpuReset(&sys->cpu);
  vuReset(&sys->vu,0);
  cuReset(&sys->cu,0);
  suReset(&sys->su);
}

void systemQuit(s2System* sys) {
  free(sys->memory);
  sys->memory=NULL;
  sys->memCapacity=0;
}
