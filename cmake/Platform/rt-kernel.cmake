# Guard against multiple inclusion
if(_RT_KERNEL_CMAKE_)
  return()
endif()
set(_RT_KERNEL_CMAKE_ TRUE)

cmake_minimum_required (VERSION 3.1.2)

# Avoid warning when re-running cmake
set(DUMMY ${CMAKE_TOOLCHAIN_FILE})

# No support for shared libs
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

#set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

# Get environment variables
set(RTK $ENV{RTK} CACHE STRING
  "Location of rt-kernel tree")
set(COMPILERS $ENV{COMPILERS} CACHE STRING
  "Location of compiler toolchain")
set(BSP $ENV{BSP} CACHE STRING
  "The name of the BSP to build for")

# Common flags
add_definitions(-Wall -Wextra -Wno-unused-parameter -Werror -fomit-frame-pointer -fno-strict-aliasing -fshort-wchar)

set(CMAKE_CXX_LINK_EXECUTABLE  "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -nostartfiles -L${RTK}/lib/${ARCH}/${CPU} -T${RTK}/bsp/${BSP}/${BSP}.ld -Wl,--start-group <LINK_LIBRARIES> -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -lptpd -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lcounter -lstdc++ -lc -lm -Wl,--end-group")
set(CMAKE_C_LINK_EXECUTABLE    "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> <OBJECTS> -o <TARGET> -nostartfiles -L${RTK}/lib/${ARCH}/${CPU} -T${RTK}/bsp/${BSP}/${BSP}.ld -Wl,--start-group <LINK_LIBRARIES> -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -lptpd -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lcounter -lc -lm -Wl,--end-group")
