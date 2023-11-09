cmake_minimum_required(VERSION 3.0)

if (APPLE)
  set(MACOSX_DEPLOYMENT_TARGET 10.9)
endif()

project(s2emu)

if (APPLE)
  enable_language(OBJC)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)

find_package(PkgConfig)

set(BUILD_GUI_DEFAULT ON)

option(BUILD_GUI "Build GUI" ${BUILD_GUI_DEFAULT})

set(DEPENDENCIES_INCLUDE_DIRS "")
set(DEPENDENCIES_DEFINES "")
set(DEPENDENCIES_COMPILE_OPTIONS "")
set(DEPENDENCIES_LIBRARIES "")
set(DEPENDENCIES_LIBRARY_DIRS "")
set(DEPENDENCIES_LINK_OPTIONS "")
set(DEPENDENCIES_LEGACY_LDFLAGS "")

set(CORE_SOURCES
src/core/char.c
src/core/core.c
src/core/cpu.c
src/core/sound.c
src/core/video.c
src/core/specs2.c
)

set(GUI_SOURCES
${CORE_SOURCES}

extern/imgui_patched/imgui.cpp
extern/imgui_patched/imgui_draw.cpp
extern/imgui_patched/imgui_tables.cpp
extern/imgui_patched/imgui_widgets.cpp
extern/imgui_patched/backends/imgui_impl_sdlrenderer.cpp
extern/imgui_patched/backends/imgui_impl_sdl.cpp
extern/imgui_patched/misc/cpp/imgui_stdlib.cpp

src/gui/font_plexMono.cpp
src/gui/main.cpp
)

set(CLI_SOURCES
${CORE_SOURCES}
src/main.cpp
)

add_executable(s2emu ${CLI_SOURCES})

if (BUILD_GUI)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(SDL2 sdl2>=2.0.18)
    if (SDL2_FOUND)
      list(APPEND DEPENDENCIES_DEFINES HAVE_SDL2)
      list(APPEND DEPENDENCIES_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
      list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${SDL2_CFLAGS_OTHER})
      list(APPEND DEPENDENCIES_LIBRARIES ${SDL2_LIBRARIES})
      list(APPEND DEPENDENCIES_LIBRARY_DIRS ${SDL2_LIBRARY_DIRS})
      list(APPEND DEPENDENCIES_LINK_OPTIONS ${SDL2_LDFLAGS_OTHER})
      list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${SDL2_LDFLAGS})
    endif()
  endif()
  if (NOT SDL2_FOUND)
    find_package(SDL2 ${SYSTEM_SDL_MIN_VER} REQUIRED)
    list(APPEND DEPENDENCIES_DEFINES HAVE_SDL2)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    list(APPEND DEPENDENCIES_LIBRARIES ${SDL2_LIBRARY})
  endif()
  message(STATUS "Using system-installed SDL2")

  list(APPEND DEPENDENCIES_INCLUDE_DIRS
    extern/imgui_patched
    extern/imgui_patched/backends
  )

  add_executable(s2emu-gui ${GUI_SOURCES})

  target_include_directories(s2emu-gui SYSTEM PRIVATE ${DEPENDENCIES_INCLUDE_DIRS})
  target_compile_definitions(s2emu-gui PRIVATE ${DEPENDENCIES_DEFINES})
  target_compile_options(s2emu-gui PRIVATE ${DEPENDENCIES_COMPILE_OPTIONS})
  target_link_libraries(s2emu-gui PRIVATE ${DEPENDENCIES_LIBRARIES})
endif()
