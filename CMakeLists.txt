cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(${CMAKE_SOURCE_DIR}/CMakeModules/backport/FetchContent.cmake)
else()
  include(FetchContent)
endif()

# automatically use vcpkg if VCPKG_ROOT is defined
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using VCPKG_ROOT $ENV{VCPKG_ROOT}")
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(openblack)

option(OPENBLACK_FETCH_DEPENDENCIES
  "Fetch and build dependencies alongside project (such as with Android)" OFF)
option(OPENBLACK_BUILD_BGFX
  "Build bgfx alongside project (only use for developing bgfx)" OFF)

# Configure C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# don't allow in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Output binaries to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# check compiler for various features
include(CheckIncludeFileCXX)
check_include_file_cxx(filesystem HAS_FILESYSTEM)

option(OPENBLACK_WARNINGS_AS_ERRORS "Treat warnings as errors (disable easier prototyping, enable for CI)" OFF)

if(OPENBLACK_FETCH_DEPENDENCIES)
  FetchContent_Declare(
    sdl2
    URL https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
    URL_HASH SHA1=e8ba91bf5eb438dbcf1fedf0530a1bcbe30d3bb6
  )
  FetchContent_MakeAvailable(sdl2)
  add_library(SDL2::SDL2 ALIAS SDL2)
# Special case for SDL2 dependency, goal is to find a config that exports SDL2::SDL2 target
# libsdl2-dev has a `sdl2-config.cmake` that doesn't export this, but vcpkg does..
else()
  find_package(SDL2 CONFIG QUIET)
  if(NOT TARGET SDL2::SDL2)
    find_package(SDL2 MODULE REQUIRED)
  endif()
endif()

if(OPENBLACK_FETCH_DEPENDENCIES)
  FetchContent_Declare(
    cxxopts
    URL https://github.com/jarro2783/cxxopts/archive/v2.2.1.tar.gz
    URL_HASH SHA1=a94880856d9112e6a32374d1ab454ca968f82b1a
  )
  FetchContent_MakeAvailable(cxxopts)
  add_library(cxxopts::cxxopts ALIAS cxxopts)

  FetchContent_Declare(
    entt
    URL https://github.com/skypjack/entt/archive/v3.5.2.tar.gz
    URL_HASH SHA1=edd675864814622b50ac074f70c55f18c23a2807
  )
  FetchContent_MakeAvailable(entt)
  add_library(EnTT::EnTT ALIAS EnTT)

  FetchContent_Declare(spdlog
    URL https://github.com/gabime/spdlog/archive/v1.4.1.tar.gz
    URL_HASH SHA1=efe84923e6dd4628550a297d77bc75e145ca3ac3
  )
  FetchContent_MakeAvailable(spdlog)
  add_library(spdlog::spdlog ALIAS spdlog)

  FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
    URL_HASH SHA1=45408897f419944fb28d8fd835791f237be2ec19
  )
  FetchContent_MakeAvailable(glm)

# Dependencies: we search for config first, then module this helps package managers.
elseif(CMAKE_VERSION VERSION_GREATER 3.15 OR CMAKE_VERSION VERSION_EQUAL 3.15)
  set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

  find_package(spdlog 1.3.0 REQUIRED)
  find_package(glm REQUIRED)
  find_package(EnTT 3.5.0 CONFIG REQUIRED) # only available as a config
  find_package(cxxopts REQUIRED)
else()
  find_package(spdlog 1.3.0 CONFIG QUIET)
  find_package(glm CONFIG QUIET)
  find_package(EnTT 3.5.0 CONFIG REQUIRED) # only available as a config
  find_package(cxxopts CONFIG REQUIRED)

  if(NOT DEFINED spdlog_DIR)
    find_package(spdlog 1.3.0 MODULE REQUIRED)
  endif()

  if(NOT DEFINED glm_DIR)
    find_package(glm MODULE REQUIRED)
  endif()

  if(NOT DEFINED cxxopts_DIR)
    find_package(cxxopts MODULE REQUIRED)
  endif()
endif()

if(NOT OPENBLACK_BUILD_BGFX)
  find_package(bgfx CONFIG)
  if(NOT bgfx_DIR)
  if (WIN32)
    message(STATUS "Precompiled bgfx binaries can be found here: https://github.com/openblack/bgfx.cmake/releases/tag/latest")
  endif()

    message(FATAL_ERROR " \
Failed to find bgfx, either install from: https://github.com/openblack/bgfx.cmake \
Or set the variable OPENBLACK_BUILD_BGFX to build it with the project. \
")
  endif()
else()
  message(AUTHOR_WARNING "Using OPENBLACK_BUILD_BGFX, if you are not developing use an install target instead.")

  FetchContent_Declare(
    bgfx
    GIT_REPOSITORY https://github.com/openblack/bgfx.cmake.git
    GIT_TAG master
  )

  # don't build anymore targets then what we need
  set(BGFX_BUILD_EXAMPLES
      OFF
      CACHE BOOL "Build bgfx examples.")
  set(BGFX_INSTALL
      OFF
      CACHE BOOL "Create installation target.")
  set(BGFX_CUSTOM_TARGETS
      OFF
      CACHE BOOL "Include convenience custom targets.")

  # declare this here so bgfx compiles in a reasonable time
  if(MSVC)
    add_compile_options(/MP)
  endif()

  FetchContent_MakeAvailable(bgfx)
  set_property(TARGET bx PROPERTY POSITION_INDEPENDENT_CODE ON)
  set_property(TARGET bimg PROPERTY POSITION_INDEPENDENT_CODE ON)
  set_property(TARGET astc-codec PROPERTY POSITION_INDEPENDENT_CODE ON)
  add_library(bgfx::bgfx ALIAS bgfx)
  # If cross compiling, we need a host-compatible version of shaderc to compile shaders
  if(CMAKE_CROSSCOMPILING)
    find_program(shaderc_EXECUTABLE REQUIRED
      NAMES shaderc
      PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug/bin
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-release/bin
    )
    add_executable(bgfx::shaderc IMPORTED)
    set_target_properties(bgfx::shaderc PROPERTIES
      IMPORTED_LOCATION "${shaderc_EXECUTABLE}"
      )
  else()
    add_executable(bgfx::shaderc ALIAS shaderc)
  endif()
endif()

FetchContent_Declare(
  imgui-flame-graph
  GIT_REPOSITORY https://github.com/bwrsandman/imgui-flame-graph.git
  GIT_TAG master)
set(IMGUI_FLAME_GRAPH_BUILD_EXAMPLE
    OFF
    CACHE BOOL "imgui-flame bgfx examples.")
FetchContent_MakeAvailable(imgui-flame-graph)

include(ClangFormat)

# Include git hash in source
include(CMakeModules/GetGitRevisionDescription.cmake)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
message(STATUS "Building ${CMAKE_PROJECT_NAME} GIT SHA1: ${GIT_SHA1}")

add_subdirectory(external/imgui)

include(CMakeModules/Shaders.cmake)

add_subdirectory(components/l3d)
add_subdirectory(components/pack)
add_subdirectory(components/lnd)
add_subdirectory(components/anm)
add_subdirectory(components/morph)
add_subdirectory(components/ScriptLibrary)
add_subdirectory(src)
add_subdirectory(apps/l3dtool)
add_subdirectory(apps/packtool)
add_subdirectory(apps/lndtool)
add_subdirectory(apps/anmtool)
add_subdirectory(apps/morphtool)
if(NOT CMAKE_CROSSCOMPILING)  # Requires python and running compiled apps
  add_subdirectory(ci/mock)
endif()

# Set openblack project as default startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                            openblack)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
