# Install script for directory: /Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/sdl2ttf/external/harfbuzz

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/ignacioestrada/Library/Android/sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/harfbuzz" TYPE FILE FILES
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-aat-layout.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-aat.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-blob.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-buffer.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-common.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-cplusplus.hh"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-deprecated.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-draw.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-face.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-font.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-map.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-color.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-deprecated.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-font.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-layout.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-math.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-meta.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-metrics.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-name.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-shape.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot-var.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ot.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-paint.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-set.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-shape-plan.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-shape.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-style.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-unicode.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-version.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-ft.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/harfbuzz/src/hb-subset.h"
    )
endif()

