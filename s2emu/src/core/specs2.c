#include "specs2.h"
#include <stdio.h>
#include <stdlib.h>

void systemInit(s2System* sys, unsigned int memCapacity) {
  coreInit(&sys->core);
  cpuInit(&sys->cpu,&sys->core);
  vuInit(&sys->vu,65536);
  suInit(&sys->su,8192,false);

  sys->memCapacity=memCapacity;
  sys->memory=malloc(memCapacity);

  coreSetMemory(&sys->core,sys->memory,sys->memCapacity);

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
}

void systemAdvance(s2System* sys, unsigned int cycles) {
  do {
    coreClock(&sys->core);
    sys->vuOutput=vuClock(&sys->vu);
    if (sys->core.clockCPU) cpuClock(&sys->cpu);
    if (sys->core.clockSU) suClock(&sys->su,&sys->suOutL,&sys->suOutR);
  } while (--cycles);
}

void systemReset(s2System* sys) {
  coreReset(&sys->core);
  cpuReset(&sys->cpu);
  vuReset(&sys->vu,0);
  suReset(&sys->su);
}

void systemQuit(s2System* sys) {
  free(sys->memory);
  sys->memory=NULL;
  sys->memCapacity=0;
}
