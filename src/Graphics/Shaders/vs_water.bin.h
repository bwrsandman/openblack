#pragma once

// Generated compiled shaders using shaderc with the bin2c option
#include <generated/shaders/vs_water.sc.glsl.bin.h>
#include <generated/shaders/vs_water.sc.spv.bin.h>
#if defined(_WIN32)
#include <generated/shaders/vs_water.sc.dx11.bin.h>
#include <generated/shaders/vs_water.sc.dx9.bin.h>
#endif //  defined(_WIN32)
#if __APPLE__
#include <generated/shaders/vs_water.sc.mtl.bin.h>
#endif // __APPLE__
