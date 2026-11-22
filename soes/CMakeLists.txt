
# NOTE: add headers to make them show up in an IDE
add_library (soes
  esc.c
  esc.h
  esc_coe.c
  esc_coe.h
  esc_foe.c
  esc_foe.h
  esc_voe.c
  esc_voe.h
  esc_eoe.c
  esc_eoe.h
  esc_eep.c
  esc_eep.h
  ecat_slv.c
  ecat_slv.h
  options.h
  ${HAL_SOURCES}
  )

include_directories(${HAL_INCLUDES})

if (SOES_FOE_READ)
  target_compile_definitions (soes PRIVATE FOE_READ_SUPPORTED=1)
endif ()

if (SOES_VOE)
  target_compile_definitions (soes PRIVATE USE_VOE=1)
endif ()

install (TARGETS soes DESTINATION bin)
install (FILES
  esc.h
  esc_coe.h
  esc_foe.h
  esc_voe.h
  esc_eoe.h
  esc_eep.h
  DESTINATION include)
