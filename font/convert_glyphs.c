#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// define to create 128x256 bitmap loadable in GIMP
#define EDITING

char nextLine[4096];

unsigned char glyphs[256][16];

unsigned char curGlyph, curPos;

int main(int argc, char** argv) {
  FILE* f=fopen(argv[1],"rb");
  FILE* of=fopen(argv[2],"wb");

  if (f==NULL || of==NULL) {
    printf("sorry\n");
    return 1;
  }

  memset(glyphs,0,256*16);
  curGlyph=0;
  curPos=0;

  while (!feof(f)) {
    if (fgets(nextLine,4095,f)==NULL) break;

    if (strlen(nextLine)>=15) {
      // push crap
      unsigned char nextRow=0;
      for (int i=0; i<8; i++) {
        nextRow<<=1;
        if (nextLine[i<<1]=='X' || nextLine[i<<1]=='x') {
          nextRow|=1;
        }
      }
      glyphs[curGlyph][curPos]=nextRow;
      if (++curPos>15) curPos=15;
    } else if (strlen(nextLine)>=2) {
      int gl=0;
      sscanf(nextLine,"%X",&gl);
      curGlyph=gl;
      curPos=0;
    }
  }

#ifdef EDITING
  for (int i=0; i<16; i++) {
    for (int j=0; j<16; j++) {
      for (int k=0; k<16; k++) {
        fputc(glyphs[(i<<4)|k][j],of);
      }
    }
  }
#else
  fwrite(glyphs,1,256*16,of);
#endif

  fclose(of);
  fclose(f);
  
  return 0;
}
