# the name of the target operating system
set(CMAKE_SYSTEM_NAME rt-kernel)

# which compiler to use
set(CMAKE_C_COMPILER $ENV{COMPILERS}/arm-eabi/bin/arm-eabi-gcc)
set(CMAKE_CXX_COMPILER $ENV{COMPILERS}/arm-eabi/bin/arm-eabi-gcc)
if(CMAKE_HOST_WIN32)
  set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER}.exe)
  set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}.exe)
endif(CMAKE_HOST_WIN32)

set(ARCH arm9e)
set(CPU arm7tdmi)

add_definitions(-mlittle-endian -mcpu=arm7tdmi)
