#include <stdio.h>
#include <string>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "misc/cpp/imgui_stdlib.h"
#include <SDL.h>
#include "fonts.h"
#include "../core/specs2.h"

typedef std::string String;

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
bool shallBuild=false;

double wR=0.299;
double wB=0.114;
double uMax=0.436;
double vMax=0.615;
double uFactor=0.436;
double vFactor=0.615;

int outFormat=0;

int clocksPerFrame=595000;
int hexTarget=0;
int hexPage=0;
int hexPokeAddr=0;
int hexPokeData=0;
String hexPokeFile;

const char* hexTargets[]={
  "System",
  "Sound",
  "Video",
  "Character",
  "Core"
};

const int hexTargetSize[]={
  (S2_MEMORY-1)>>8,
  0,
  255,
  127,
  63
};

void buildTable(double wR, double wB, double uMax, double vMax, double uFactor, double vFactor) {
  if (outFormat==3) {
    for (int i=0; i<65536; i++) {
      unsigned char yi=((i>>10)*255)/63;
      unsigned char ui=(((i>>5)&31)+16)<<3;
      unsigned char vi=((i+16)&31)<<3;

      unsigned char* out=(unsigned char*)(&colorTable[i]);
      out[0]=ui;
      out[1]=yi;
      out[2]=vi;
      out[3]=yi;
    }
  } else {
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
      const double u=((double)ui/15.0)*uFactor;
      const double v=((double)vi/15.0)*vFactor;

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
      out[0]=b;
      out[1]=g;
      out[2]=r;
      out[3]=255;
    }
  }
}

bool initGUI() {
  printf("initializing GUI.\n");

  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"0");
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
  // don't disable compositing on KWin
#if SDL_VERSION_ATLEAST(2,0,22)
  //logV("setting window type to NORMAL.");
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

    SDL_GetRendererOutputSize(sdlRend,&scrW,&scrH);

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
        if (ImGui::RadioButton("YUV -> RGB (0.34, 0.30)",outFormat==0)) {
          wR=0.299;
          wB=0.114;
          uMax=0.436;
          vMax=0.615;
          uFactor=0.436;
          vFactor=0.615;
          shallBuild=true;
          outFormat=0;
        }
        if (ImGui::RadioButton("YUV -> RGB (0.436, 0.615)",outFormat==1)) {
          wR=0.299;
          wB=0.114;
          uMax=0.436;
          vMax=0.615;
          uFactor=0.34;
          vFactor=0.30;
          shallBuild=true;
          outFormat=1;
        }
        if (ImGui::RadioButton("YUV -> RGB (0.5, 0.5)",outFormat==2)) {
          wR=0.299;
          wB=0.114;
          uMax=0.5;
          vMax=0.5;
          uFactor=0.34;
          vFactor=0.30;
          shallBuild=true;
          outFormat=2;
        }
        if (ImGui::RadioButton("YUV -> YUV (native)",outFormat==3)) {
          shallBuild=true;
          outFormat=3;
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Video Monitor")) {
      if (colorTable==NULL || shallBuild) {
        buildTable(wR,wB,uMax,vMax,uFactor,vFactor);
        if (tex!=NULL) {
          SDL_DestroyTexture(tex);
          tex=NULL;
        }
        shallBuild=false;
      }
      ImGui::InputInt("Clocks/Frame",&clocksPerFrame,1,100);

      if (clocksPerFrame>0) {
        systemAdvance(&sys,clocksPerFrame);
      }

      if (sys.vuOutput==VU_SYNC) {
        ImGui::Text("H: %4d V: %4d O: SYNC S: %d C: %d",sys.framePos&1023,sys.framePos>>10,sys.frameSyncCycles,sys.frameCycles);
      } else {
        ImGui::Text("H: %4d V: %4d O: %.4x S: %d C: %d",sys.framePos&1023,sys.framePos>>10,sys.vuOutput,sys.frameSyncCycles,sys.frameCycles);
      }
      ImGui::Text("HMODE: %d VMODE: %d HCOUNT: %4d VCOUNT: %4d VSLATCH: %d INIT: %d",sys.vu.hmode,sys.vu.vmode,sys.vu.hcount,sys.vu.vcount,sys.vu.vslatch,sys.vu.initAddr);

      if (tex==NULL) {
        tex=SDL_CreateTexture(sdlRend,outFormat==3?SDL_PIXELFORMAT_UYVY:SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,outFormat==3?2048:1024,768);
        if (tex==NULL) {
          printf("ERROR while creating texture\n");
        }
      }
      if (tex!=NULL) {
        unsigned int* dataT=NULL;
        int pitch=0;
        if (SDL_LockTexture(tex,NULL,(void**)&dataT,&pitch)!=0) {
          printf("could not lock texture\n");
        } else {
          for (int i=0; i<1024*768; i++) {
            dataT[i]=colorTable[sys.frame[i]];
          }
          SDL_UnlockTexture(tex);
        }

        //ImGui::Image(tex,ImVec2(1024*dpiScale,768*dpiScale),ImVec2(0,0),ImVec2(1,1));
      }
    }
    ImGui::End();

    if (ImGui::Begin("Hex Editor")) {
      ImGui::Combo("Target",&hexTarget,hexTargets,5);
      ImGui::SliderInt("Page",&hexPage,0,hexTargetSize[hexTarget],"%.4x");

      unsigned char* memRegion=NULL;

      switch (hexTarget) {
        case 0:
          memRegion=sys.memory;
          break;
        case 1:
          memRegion=(unsigned char*)sys.su.chan;
          break;
        case 2:
          memRegion=sys.vu.mem;
          break;
        case 3:
          memRegion=sys.cu.mem;
          break;
      }

      if (ImGui::BeginTable("HexView",17)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        for (int i=0; i<16; i++) {
          ImGui::TableNextColumn();
          ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(ImGuiCol_HeaderActive));
          ImGui::Text("%X",i);
          ImGui::PopStyleColor();
        }
        for (int i=0; i<16; i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(ImGuiCol_HeaderActive));
          ImGui::Text("%X",i);
          ImGui::PopStyleColor();

          for (int j=0; j<16; j++) {
            ImGui::TableNextColumn();
            if (memRegion==NULL) {
              ImGui::Text("??");
            } else {
              unsigned int addr=(hexPage<<8)|(i<<4)|j;
              if (addr>(unsigned int)((hexTargetSize[hexTarget]<<8)|0xff)) {
                ImGui::Text("??");
              } else {
                ImGui::Text("%.2x",memRegion[addr]);
              }
            }
          }
        }
        ImGui::EndTable();
      }

      ImGui::SetNextItemWidth(240.0*dpiScale);
      ImGui::InputInt("##Address",&hexPokeAddr,1,16,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(240.0*dpiScale);
      ImGui::InputInt("##Data",&hexPokeData,1,16,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      if (ImGui::Button("Write")) {
        if (hexPokeAddr<=((hexTargetSize[hexTarget]<<8)|0xff)) {
          memRegion[hexPokeAddr]=hexPokeData;
        }
      }
      ImGui::InputText("##File",&hexPokeFile);
      ImGui::SameLine();
      if (ImGui::Button("Poke File")) {
        FILE* f=fopen(hexPokeFile.c_str(),"rb");
        if (f!=NULL) {
          fread(memRegion+hexPokeAddr,1,((hexTargetSize[hexTarget]<<8)|0xff)-hexPokeAddr,f);
          fclose(f);
        }
      }
    }
    ImGui::End();

    SDL_SetRenderDrawColor(sdlRend,0,0,0,255);
    SDL_RenderClear(sdlRend);
    ImGui::Render();
    SDL_Rect renderRect, renderPos;
    renderRect.x=96*(outFormat==3?2:1);
    renderRect.y=23;
    renderRect.w=800;
    renderRect.h=576;

    renderPos.w=renderRect.w;
    renderPos.h=renderRect.h;

    while (renderPos.w<scrW && renderPos.h<scrH) {
      renderPos.w+=renderRect.w;
      renderPos.h+=renderRect.h;
    }
    renderPos.w-=renderRect.w;
    renderPos.h-=renderRect.h;

    renderPos.x=(scrW-renderPos.w)/2;
    renderPos.y=(scrH-renderPos.h)/2;

    if (outFormat==3) renderRect.w*=2;
    SDL_RenderCopy(sdlRend,tex,&renderRect,&renderPos);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRend);
  }

  return 0;
}
