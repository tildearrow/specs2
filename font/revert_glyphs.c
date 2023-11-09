#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  fread(glyphs,1,256*16,f);
  fclose(f);

  for (int i=0; i<256; i++) {
    fprintf(of,"%.2X\n",i);
    for (int j=0; j<16; j++) {
      fprintf(of,"%c %c %c %c %c %c %c %c\n",
        (glyphs[i][j]&128)?'X':'.',
        (glyphs[i][j]&64)?'X':'.',
        (glyphs[i][j]&32)?'X':'.',
        (glyphs[i][j]&16)?'X':'.',
        (glyphs[i][j]&8)?'X':'.',
        (glyphs[i][j]&4)?'X':'.',
        (glyphs[i][j]&2)?'X':'.',
        (glyphs[i][j]&1)?'X':'.'
      );
    }
    fprintf(of,"\n");
  }
  
  fclose(of);
  return 0;
}
