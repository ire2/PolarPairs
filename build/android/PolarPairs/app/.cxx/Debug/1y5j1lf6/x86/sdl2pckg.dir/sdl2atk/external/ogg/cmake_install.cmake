# Install script for directory: /Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/sdl2atk/external/ogg

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/libogg.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ogg" TYPE FILE FILES
    "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/include/ogg/config_types.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/include/ogg/ogg.h"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/include/ogg/os_types.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg/OggTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg/OggTargets.cmake"
         "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/CMakeFiles/Export/lib/cmake/Ogg/OggTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg/OggTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg/OggTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg" TYPE FILE FILES "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/CMakeFiles/Export/lib/cmake/Ogg/OggTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg" TYPE FILE FILES "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/CMakeFiles/Export/lib/cmake/Ogg/OggTargets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Ogg" TYPE FILE FILES
    "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/OggConfig.cmake"
    "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/OggConfigVersion.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/ignacioestrada/game_design/skipclass_studios/build/android/PolarPairs/app/.cxx/Debug/1y5j1lf6/x86/sdl2pckg.dir/sdl2atk/external/ogg/ogg.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/SDL2_atk/html" TYPE FILE FILES
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/framing.html"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/index.html"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/oggstream.html"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/ogg-multiplex.html"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/fish_xiph_org.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/multiplex1.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/packets.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/pages.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/stream.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/vorbisword2.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/white-ogg.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/white-xifish.png"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/rfc3533.txt"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/rfc5334.txt"
    "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/skeleton.html"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/SDL2_atk/html" TYPE DIRECTORY FILES "/Users/ignacioestrada/game_design/cugl/sdlapp/buildfiles/cmake/../../external/ogg/doc/libogg")
endif()

