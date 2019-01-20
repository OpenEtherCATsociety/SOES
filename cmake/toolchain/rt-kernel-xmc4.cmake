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

set(ARCH xmc4)
set(CPU cortex-m4f)
set(BSP xmc48relax)
set(MACHINE_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(SOES_DEMO applications/rtl_xmc4_dynpdo)

include_directories(
  ${SOES_SOURCE_DIR}/${SOES_DEMO}
  ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4
  )

set(HAL_SOURCES
  ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4/esc_hw.c
  ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4/esc_hw_eep.c
  )
