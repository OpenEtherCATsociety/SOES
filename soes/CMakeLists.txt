
# NOTE: add headers to make them show up in an IDE
add_library (soes
  esc.c
  esc.h
  esc_coe.c
  esc_coe.h
  esc_foe.c
  esc_foe.h
  esc_eoe.c
  esc_eoe.h
  esc_eep.c
  esc_eep.h
  ${HAL_SOURCES}
  )

include_directories(${HAL_INCLUDES})

install (TARGETS soes DESTINATION bin)
install (FILES
  esc.h
  esc_coe.h
  esc_foe.h
  esc_eoe.h
  esc_eep.h
  DESTINATION include)
