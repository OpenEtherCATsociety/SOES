
message (STATUS "Building for ${CMAKE_SYSTEM_NAME}")

# NOTE: add headers to make them show up in an IDE
add_library (soes
  esc.c
  esc.h
  esc_coe.c
  esc_coe.h
  esc_foe.c
  esc_foe.h
  ${OSAL_SOURCES}
  )

include_directories(${OSAL_INCLUDES})

install (TARGETS soes DESTINATION bin)
install (FILES
  esc.h
  esc_coe.h
  esc_foe.h
  DESTINATION include)
