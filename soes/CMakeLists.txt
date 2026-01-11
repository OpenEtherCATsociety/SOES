
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
  esc_mdp.c
  esc_mdp.h
  device_obj_areas.h
  device_obj_areas.c
  ecat_slv.c
  ecat_slv.h
  options.h
  ${HAL_SOURCES}
  )

include_directories(${HAL_INCLUDES})
target_include_directories(soes PUBLIC include/sys/gcc)#//TODO #fix compilation bug.

install (TARGETS soes DESTINATION bin)
install (FILES
  esc.h
  esc_coe.h
  esc_foe.h
  esc_eoe.h
  esc_eep.h
  esc_mdp.h
  device_obj_areas.h
  DESTINATION include)
