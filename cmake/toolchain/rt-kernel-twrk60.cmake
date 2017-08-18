# Guard against multiple inclusion
if(_TOOLCHAIN_CMAKE_)
  return()
endif()
set(_TOOLCHAIN_CMAKE_ TRUE)

INCLUDE(CMakeForceCompiler)
SET(CMAKE_SYSTEM_NAME rt-kernel)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(arm-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-eabi-g++ GNU)

set(ARCH kinetis)
set(CPU cortex-m4f)
set(BSP twrk60f)
set(MACHINE_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(HAL_SOURCES
  ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-twrk60/esc_hw.c
  )
