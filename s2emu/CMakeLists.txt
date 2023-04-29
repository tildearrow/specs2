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

set(CORE_SOURCES
src/core/core.c
src/core/cpu.c
src/core/video.c
src/core/specs2.c
)

set(USED_SOURCES ${CORE_SOURCES} src/main.cpp)

add_executable(s2emu ${USED_SOURCES})
