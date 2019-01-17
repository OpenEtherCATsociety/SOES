# Guard against multiple inclusion
if(_RT_KERNEL_CMAKE_)
  return()
endif()
set(_RT_KERNEL_CMAKE_ TRUE)

cmake_minimum_required (VERSION 3.1.2)

# Get environment variables
set(RTK $ENV{RTK} CACHE STRING
  "Location of rt-kernel tree")

# Avoid warning when re-running cmake
set(DUMMY ${CMAKE_TOOLCHAIN_FILE})

include_directories(
  ${RTK}/include/
  ${RTK}/include/arch/${ARCH}
  ${RTK}/include/kern
  ${RTK}/include/drivers
  ${RTK}/lwip/src/include
  ${RTK}/bsp/${BSP}/include
  ${SOES_SOURCE_DIR}/soes/include/sys/gcc
  )
link_directories(
  ${RTK}/lib/${ARCH}/${CPU}
  )

# No support for shared libs
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Common compile flags
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror -fomit-frame-pointer -fno-strict-aliasing)

# Add machine flags to C_FLAGS so they take effect for linking also
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MACHINE_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MACHINE_FLAGS}" CACHE STRING "" FORCE)

# Default libs
set(RTK_LIBS "-l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -lptpd -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lcounter -lc -lm -lshell")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -nostartfiles -T${RTK}/bsp/${BSP}/${BSP}.ld -Wl,--start-group <LINK_LIBRARIES> ${RTK_LIBS} -Wl,--end-group")
set(CMAKE_C_LINK_EXECUTABLE   "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> <OBJECTS> -o <TARGET> -nostartfiles -T${RTK}/bsp/${BSP}/${BSP}.ld -Wl,--start-group <LINK_LIBRARIES> ${RTK_LIBS} -Wl,--end-group")
