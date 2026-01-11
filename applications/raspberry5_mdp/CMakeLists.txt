
add_executable (soes-mdp-demo
  main.c
  slave_objectlist.c
  )
target_link_libraries(soes-mdp-demo LINK_PUBLIC soes bcm2835)
install (TARGETS soes-mdp-demo DESTINATION sbin)
