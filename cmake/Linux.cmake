# Allow user to override SOES_DEMO/HAL_SOURCES from command line.
if(NOT DEFINED SOES_DEMO)
  if(RPI_VARIANT)
    set(SOES_DEMO applications/raspberry_lan9252demo)
  else()
    set(SOES_DEMO applications/linux_lan9252demo)
  endif()
endif()

if(NOT DEFINED HAL_SOURCES)
  if(SOES_DEMO STREQUAL "applications/raspberry_lan9252demo")
    set(HAL_SOURCES
      ${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.c
      ${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.h
    )
  elseif(SOES_DEMO STREQUAL "applications/linux_lan9252demo")
    set(HAL_SOURCES
      ${SOES_SOURCE_DIR}/soes/hal/linux-lan9252/esc_hw.c
    )
  endif()
endif()

include_directories(
  ${SOES_SOURCE_DIR}/soes/include/sys/gcc
  ${SOES_SOURCE_DIR}/${SOES_DEMO}
  )

# Common compile flags
add_compile_options(-Wall -Wextra -Wconversion -Wno-unused-parameter -Werror)
