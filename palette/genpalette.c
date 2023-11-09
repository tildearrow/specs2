#include <stdio.h>
#include <math.h>

const double col[6]={
  0.0,
  95.0/255.0,
  135.0/255.0,
  175.0/255.0,
  215.0/255.0,
  255.0/255.0
};

int main(int argc, char** argv) {
  const double wR=0.299;
  const double wB=0.114;
  const double wG=1.0-wR-wB;
  const double uMax=0.436;
  const double vMax=0.615;
  const double uW=uMax*(1.0-wB);
  const double vW=vMax*(1.0-wR);

  int index=16;

  for (int i=0; i<6; i++) {
    for (int j=0; j<6; j++) {
      for (int k=0; k<6; k++) {
        const double r=col[i];
        const double g=col[j];
        const double b=col[k];

        double y=wR*r+wG*g+wB*b;
        double u=uW*(b-y);
        double v=vW*(r-y);

        y=round(y*63);
        u=round(15*(u/0.34));
        v=round(15*(v/0.30));

        if (y<0) y=0;
        if (y>63) y=63;
        if (u<-15) u=-15;
        if (u>15) u=15;
        if (v<-15) v=-15;
        if (v>15) v=15;

        printf("%d: %d,%d,%d\n",index++,(int)y,(int)u,(int)v);
      }
    }
  }
}
