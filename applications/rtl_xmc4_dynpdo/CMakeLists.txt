
add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sii_eeprom.o
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sii_eeprom.bin
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND arm-eabi-objcopy -I binary -O elf32-littlearm -B arm sii_eeprom.bin ${CMAKE_CURRENT_BINARY_DIR}/sii_eeprom.o
  )

add_executable (demo
  slave_objectlist.c
  main.c
  ${CMAKE_CURRENT_BINARY_DIR}/sii_eeprom.o
  )
target_link_libraries(demo LINK_PUBLIC soes)
