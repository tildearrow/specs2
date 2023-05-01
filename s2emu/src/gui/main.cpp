#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>
#include "fonts.h"
#include "../core/specs2.h"

s2System sys;

#define S2_MEMORY 1048576

double dpiScale;
int scrX, scrY, scrW, scrH;

SDL_Window* sdlWin;
SDL_Renderer* sdlRend;
SDL_Texture* tex=NULL;

ImFont* mainFont;

int* colorTable=NULL;

bool quit=false;

void buildTable(double wR, double wB, double uMax, double vMax, double uFactor, double vFactor) {
  const double wG=1.0-wR-wB;
  const double uW=(1.0-wB)/uMax;
  const double vW=(1.0-wR)/vMax;
  const double gU=(wB*(1.0-wB))/(uMax*wG);
  const double gV=(wR*(1.0-wR))/(vMax*wG);

  if (colorTable==NULL) colorTable=new int[65536];

  for (int i=0; i<65536; i++) {
    unsigned char yi=(i>>10)&63;
    signed char ui=(i>>5)&31;
    signed char vi=i&31;

    if (ui&16) ui|=0xf0;
    if (vi&16) vi|=0xf0;

    if (ui<-15) ui=-15;
    if (vi<-15) vi=-15;

    const double y=(double)yi/63.0;
    const double u=((double)ui/15.0)*0.436;
    const double v=((double)vi/15.0)*0.615;

    double r=y+v*vW;
    double g=y-u*gU-v*gV;
    double b=y+u*uW;

    r=round(r*255);
    g=round(g*255);
    b=round(b*255);

    if (r<0) r=0;
    if (r>255) r=255;
    if (g<0) g=0;
    if (g>255) g=255;
    if (b<0) b=0;
    if (b>255) b=255;

    unsigned char* out=(unsigned char*)(&colorTable[i]);
    out[0]=255;
    out[1]=b;
    out[2]=g;
    out[3]=r;
  }
}

bool initGUI() {
  printf("initializing GUI.\n");

  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"0");
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
  // don't disable compositing on KWin
#if SDL_VERSION_ATLEAST(2,0,22)
  logV("setting window type to NORMAL.");
  SDL_SetHint(SDL_HINT_X11_WINDOW_TYPE,"_NET_WM_WINDOW_TYPE_NORMAL");
#endif

  // initialize SDL
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_HAPTIC);

  dpiScale=2.0f;

  scrW=1280*dpiScale;
  scrH=800*dpiScale;
  scrX=SDL_WINDOWPOS_CENTERED;
  scrY=SDL_WINDOWPOS_CENTERED;

  printf("window size: %dx%d\n",scrW,scrH);

  sdlWin=SDL_CreateWindow("s2emu",scrX,scrY,scrW,scrH,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    return false;
  }

  sdlRend=SDL_CreateRenderer(sdlWin,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);

  if (sdlRend==NULL) {
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin,sdlRend);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  ImGui::GetStyle().ScaleAllSizes(dpiScale);

  if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_plexMono_compressed_data,font_plexMono_compressed_size,18*dpiScale))==NULL) {
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
  }

  if (!ImGui::GetIO().Fonts->Build()) {
    printf("error while building font atlas!\n");
    ImGui::GetIO().Fonts->Clear();
  }

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;

  return true;
}



int main(int argc, char** argv) {
  systemInit(&sys,S2_MEMORY);

  if (!initGUI()) return 1;

  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL2_ProcessEvent(&ev);
      switch (ev.type) {
        case SDL_QUIT:
          quit=true;
          break;
      }
    }

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("window")) {
        ImGui::Text("TODO: implement...");
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("settings")) {
        ImGui::Text("output format:");
        ImGui::RadioButton("YUV -> RGB (0.34, 0.30)",false);
        ImGui::RadioButton("YUV -> RGB (0.436, 0.615)",false);
        ImGui::RadioButton("YUV -> RGB (0.5, 0.5)",false);
        ImGui::RadioButton("YUV -> YUV (native)",false);
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Video Monitor")) {
      if (colorTable==NULL) buildTable();
      systemAdvance(&sys,1000);

      if (tex==NULL) {
        tex=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,1024,576);
      }
      if (tex!=NULL) {
        unsigned int* dataT=NULL;
        int pitch=0;
        if (SDL_LockTexture(tex,NULL,(void**)&dataT,&pitch)!=0) {
          printf("could not lock texture\n");
        } else {
          for (int i=0; i<1024*576; i++) {
            dataT[i]=colorTable[sys->frame[i]];
          }
          SDL_UnlockTexture(tex);
        }

        ImGui::ImageButton(tex,ImVec2(1024*dpiScale,576*dpiScale),ImVec2(0,0),ImVec2(1,1),0);
      }

      ImGui::Text("Hello");
    }
    ImGui::End();

    SDL_SetRenderDrawColor(sdlRend,0,0,0,255);
    SDL_RenderClear(sdlRend);
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRend);
  }

  return 0;
}
