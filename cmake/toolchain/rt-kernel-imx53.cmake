# the name of the target operating system
set(CMAKE_SYSTEM_NAME rt-kernel)

# which compiler to use
set(CMAKE_C_COMPILER $ENV{COMPILERS}/arm-eabi/bin/arm-eabi-gcc)
set(CMAKE_CXX_COMPILER $ENV{COMPILERS}/arm-eabi/bin/arm-eabi-gcc)
if(CMAKE_HOST_WIN32)
  set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER}.exe)
  set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}.exe)
endif(CMAKE_HOST_WIN32)

set(RTK_ROOT rt-kernel-imx53)
set(RTK_LIBS rt-kernel-imx53/lib/imx53/cortex-a8)
set(RTK_TARGET_LIBS "-limx53qsb -limx53")

add_definitions(-mfpu=neon -mcpu=cortex-a8)
