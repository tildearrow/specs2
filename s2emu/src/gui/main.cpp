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

ImFont* mainFont;

bool quit=false;

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

      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Video Monitor")) {
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