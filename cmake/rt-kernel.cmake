
if(${ARCH} STREQUAL "xmc4")
    set(SOES_DEMO applications/rtl_xmc4_dynpdo)

    include_directories(
        ${SOES_SOURCE_DIR}/soes/include/sys/gcc
        ${SOES_SOURCE_DIR}/${SOES_DEMO}
        ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4
    )

    set(HAL_SOURCES
        ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4/esc_hw.c
        ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-xmc4/esc_hw_eep.c
    )

endif()

if(${ARCH} STREQUAL "kinetis")
    set(SOES_DEMO applications/rtl_slavedemo)

    include_directories(
        ${SOES_SOURCE_DIR}/${SOES_DEMO}
    )

    set(HAL_SOURCES
        ${SOES_SOURCE_DIR}/soes/hal/rt-kernel-twrk60/esc_hw.c
    )
endif()


find_package(rtkernel REQUIRED)

# Common compile flags
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror -fomit-frame-pointer -fno-strict-aliasing)

link_libraries(${BSP} ${ARCH} kern dev sio block fs usb lwip ptpd eth i2c rtc can nand spi nor pwm adc trace counter c m shell)

