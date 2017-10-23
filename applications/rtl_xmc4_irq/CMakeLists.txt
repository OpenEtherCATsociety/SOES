
add_executable (rtl_xmc4_irq
  slave.c
  slave_objectlist.c
  main.c
  )
target_link_libraries(rtl_xmc_irq LINK_PUBLIC soes)
