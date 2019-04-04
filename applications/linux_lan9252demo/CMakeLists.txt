
add_executable (demo
  main.c
  slave_objectlist.c
  )
target_link_libraries(demo LINK_PUBLIC soes)
