
add_executable (rtl_slavedemo
  soes.c
  objectlist.c
  led_handler.c
  bootstrap.c
  main.c
  )
target_link_libraries(rtl_slavedemo LINK_PUBLIC soes)
