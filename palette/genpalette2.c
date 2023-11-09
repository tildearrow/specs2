#include <stdio.h>

int main(int argc, char** argv) {
  for (int i=0; i<24; i++) {
    int y=1+((i*5)/2);
    printf("%d: %d,0,0\n",i+232,y);
  }
}
