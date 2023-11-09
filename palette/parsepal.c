#include <stdio.h>

int main(int argc, char** argv) {
  if (argc<2) return 1;
  FILE* f=fopen(argv[1],"r");

  if (f==NULL) return 1;

  char line[4096];

  while (!feof(f)) {
    int index, y, u, v;
    if (fgets(line,4095,f)==NULL) break;

    if (sscanf(line,"%d: %d,%d,%d\n",&index,&y,&u,&v)!=4) continue;

    unsigned short h=(
      ((y&63)<<10)|
      ((u&31)<<5)|
      (v&31)
    );

    printf("%d: %d,%d,%d ($%.4x)\n",index,y,u,v,h);
  }
}
