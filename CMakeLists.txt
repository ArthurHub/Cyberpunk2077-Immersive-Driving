cmake_minimum_required(VERSION 3.25)

project(
  ImmersiveDriving
  VERSION 1.1.0
  DESCRIPTION "Configurable immersive driving for Cyberpunk 2077: throttle and steering control, slow turn, cruise control"
  LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed, use a build directory (for example: cmake --preset default).")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Static CRT, so the plugin does not depend on the version of the VC++ redistributable players have installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(IMMERSIVE_DRIVING_BUILD_PLUGIN "Build the RED4ext plugin (downloads RED4ext.SDK)." ON)
option(IMMERSIVE_DRIVING_BUILD_TESTS "Build the driving logic tests." ON)

if(MSVC)
  add_compile_options(/W4 /permissive- /utf-8 /Zc:__cplusplus /MP)
endif()

# -----------------------------------------------------------------------------
# Driving logic, independent of the game so it can be tested anywhere
# -----------------------------------------------------------------------------
add_library(
  ImmersiveDrivingCore STATIC
  src/core/Config.cpp
  src/core/Config.h
  src/core/DriveController.cpp
  src/core/DriveController.h
  src/core/MathUtil.h
)
target_include_directories(ImmersiveDrivingCore PUBLIC src)

if(IMMERSIVE_DRIVING_BUILD_TESTS)
  enable_testing()
  add_executable(ImmersiveDrivingTests tests/CoreTests.cpp)
  target_link_libraries(ImmersiveDrivingTests PRIVATE ImmersiveDrivingCore)
  add_test(NAME ImmersiveDrivingTests COMMAND ImmersiveDrivingTests)
endif()

# -----------------------------------------------------------------------------
# RED4ext plugin
# -----------------------------------------------------------------------------
if(IMMERSIVE_DRIVING_BUILD_PLUGIN)
  if(NOT WIN32)
    message(FATAL_ERROR "The plugin targets Windows only.")
  endif()

  include(FetchContent)
  # RED4ext.SDK master from 2026-07-30 (supports patch 2.31). Set FETCHCONTENT_SOURCE_DIR_RED4EXT_SDK to use a local checkout.
  FetchContent_Declare(
    RED4ext_SDK
    URL https://github.com/wopss/RED4ext.SDK/archive/ad7277714ad30d6885d7050c5ba24fa0102f6920.zip
    URL_HASH SHA256=0dcaf53ff4da4a69ef31069fb0a82e93ffd546b4299b6e727999c57c3d4f7621
    DOWNLOAD_EXTRACT_TIMESTAMP ON
  )
  FetchContent_MakeAvailable(RED4ext_SDK)

  configure_file(src/plugin/Version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h" @ONLY)
  configure_file(src/plugin/Version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.rc" @ONLY)

  add_library(
    ImmersiveDriving SHARED
    src/plugin/DrivingRuntime.cpp
    src/plugin/DrivingRuntime.h
    src/plugin/Log.cpp
    src/plugin/Log.h
    src/plugin/Main.cpp
    src/plugin/Natives.cpp
    src/plugin/Natives.h
    src/plugin/VehicleFunctions.cpp
    src/plugin/VehicleFunctions.h
    src/plugin/VehicleHook.cpp
    src/plugin/VehicleHook.h
    "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.rc"
  )
  target_include_directories(ImmersiveDriving PRIVATE src "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_compile_definitions(ImmersiveDriving PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
  target_link_libraries(ImmersiveDriving PRIVATE ImmersiveDrivingCore RED4ext::SDK)

  # Game folder layout. Install with --prefix pointing at the game folder, or at a staging folder for packaging.
  install(TARGETS ImmersiveDriving RUNTIME DESTINATION red4ext/plugins/ImmersiveDriving)
  install(DIRECTORY scripts/ DESTINATION red4ext/plugins/ImmersiveDriving/scripts FILES_MATCHING PATTERN "*.reds")
  install(FILES input/ImmersiveDriving.xml DESTINATION r6/input)
endif()
