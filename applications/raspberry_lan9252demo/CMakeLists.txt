
add_executable (soes-demo
  main.c
  slave_objectlist.c
  )
target_link_libraries(soes-demo LINK_PUBLIC soes bcm2835)
install (TARGETS soes-demo DESTINATION sbin)
install (PROGRAMS S60soes DESTINATION /etc/init.d)
