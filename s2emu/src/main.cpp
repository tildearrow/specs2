#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include "core/specs2.h"

#define SCDuration std::chrono::steady_clock::duration
#define SCTimePoint std::chrono::steady_clock::time_point
#define SCNow std::chrono::steady_clock::now
#define durationCast std::chrono::duration_cast
#define SCMicro std::chrono::microseconds

s2System sys;

SCTimePoint lastFrame;
SCTimePoint curFrame;

#define S2_MEMORY 1048576

int main(int argc, char** argv) {
  printf("s2emu " S2EMU_VERSION "\n");
  printf("starting up specs2 machine with %d bytes RAM.\n",S2_MEMORY);

  systemInit(&sys,S2_MEMORY);

  while (1) {
    lastFrame=SCNow();
    systemAdvance(&sys,595000);
    curFrame=SCNow();
    long total=durationCast<SCMicro>(curFrame-lastFrame).count();
    //printf("update took %ldμs\n",total);
    if (total<20000) usleep(20000-total);
  }

  /*while (!feof(stdin)) {
    int ch=getchar();
    if (ch==EOF) break;
    systemAdvance(&sys,5);
  }*/

  printf("\n");
  systemQuit(&sys);
  return 0;
}
