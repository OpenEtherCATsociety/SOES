cmake_minimum_required(VERSION 3.6)

###############################################################################
## Force compiler #############################################################
###############################################################################
include (CMakeForceCompiler)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_CROSSCOMPILING TRUE)

SET(USE_TI_COMPILER TRUE)

if(WIN32)
   set(EXE_SUFFIX .exe)
   if (USE_TI_COMPILER)
      set(COMPILER_PATH C:/ti/ti_cgt_tiarmclang_3.2.0.LTS/bin)
   else()
      set(COMPILER_PATH "C:/Program Files/arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi/bin")
   endif()
else()
   set(EXE_SUFFIX )
   if (USE_TI_COMPILER)
      set(COMPILER_PATH $ENV{HOME}/ti/ti-cgt-armllvm_5.0.0.STS/bin)
   else()
      # Untested, good luck!
   endif()
endif()

set(CLANG_TARGET_TRIPLE arm-none-eabi)
if (USE_TI_COMPILER)
   set(CMAKE_C_COMPILER ${COMPILER_PATH}/tiarmclang${EXE_SUFFIX})
   set(CMAKE_AR ${COMPILER_PATH}/tiarmar${EXE_SUFFIX})
else()
   set(CMAKE_C_COMPILER ${COMPILER_PATH}/arm-none-eabi-gcc${EXE_SUFFIX})
   set(CMAKE_AR ${COMPILER_PATH}/arm-none-eabi-ar${EXE_SUFFIX})
endif()
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_LINKER ${CMAKE_C_COMPILER})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(soes_am263x_lp_example C CXX ASM)

if(WIN32)
   set(IND_COMMS_SDK_PATH C:/ti/ind_comms_sdk_am263x_09_01_00_03)
   set(SYSCFG_CLI_PATH C:/ti/sysconfig_1.19.0)
   set(SYSCFG_NODE ${SYSCFG_CLI_PATH}/nodejs/node.exe)
else()
   set(IND_COMMS_SDK_PATH $ENV{HOME}/ti/ind_comms_sdk_am263x_09_01_00_03)
   set(SYSCFG_CLI_PATH $ENV{HOME}/ti/sysconfig_1.19.0)
   set(SYSCFG_NODE ${SYSCFG_CLI_PATH}/nodejs/node)
endif()

set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")
###############################################################################
## Create names ###############################################################
###############################################################################
set(SOES_PATH ../../soes)
set(AM263X_MCU_PLUS_SKD ${IND_COMMS_SDK_PATH}/mcu_plus_sdk/source)
set(FREERTOS_PATH ${AM263X_MCU_PLUS_SKD}/kernel/freertos)
set(NORTOS_PATH ${AM263X_MCU_PLUS_SKD}/kernel/nortos)
set(BOARD_DRIVER_PATH ${AM263X_MCU_PLUS_SKD}/board)

set(EXECUTABLE ${PROJECT_NAME})
set(MAP_FILE ${PROJECT_NAME}.map)
set(PROFILE debug)
set(TI_DRIVERS_LIB drivers.am263x.r5f.ti-arm-clang.${PROFILE}.lib)
set(TI_ETHERCAT_STACK ethercat_slave_icss_fwhal.am263x.r5f.ti-arm-clang.${PROFILE}.lib)
set(FREERTOS_LIB freertos.am263x.r5f.ti-arm-clang.${PROFILE}.lib)
set(BOARD_LIB board.am263x.r5f.ti-arm-clang.${PROFILE}.lib)
set(SYSCONF_FILE ${IND_COMMS_SDK_PATH}/examples/industrial_comms/ethercat_slave_beckhoff_ssc_demo/am263x-lp/r5fss0-0_freertos/example.syscfg)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(SYSCFG_SDKPRODUCT ${AM263X_MCU_PLUS_SKD}/../.metadata/product.json)

set (CMAKE_CXX_STANDARD 17)

###############################################################################
## Compiler flags #############################################################
###############################################################################
set(C_CPP_FLAGS
   -mcpu=cortex-r5
   -mfloat-abi=hard
   -mfpu=vfpv3-d16
   -mlittle-endian
   -mthumb
# -O2 causes un-aligned memory accesses in ICSS memory. This is fine for most of
# the memories on this MCU, so disabling it globally would reduce performance.
# To fix this, add `__attribute__ ((noinline))` to `tiesc_memcpy` and `tiesc_memset`
# in ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/icss_fwhal/tiescbsp.c.
# Then -O2 can safely be used.
   # -O2
   -O0
   -g
   -Wall
   -Wno-gnu-variable-sized-type-not-at-end
   -Wno-unused-function
   -Wno-self-assign
   -Wno-parentheses-equality
   -Wno-tautological-constant-out-of-range-compare
   -Wno-address-of-packed-member
   -Wno-tautological-pointer-compare
)
set(C_FLAGS )
set(CPP_FLAGS )

###############################################################################
## Sources ####################################################################
###############################################################################
set(APP_SOURCES
   main.c
   am263x_lp_objectlist.c
   tiescsoc.c
   ${SOES_PATH}/esc_coe.c
   ${SOES_PATH}/ecat_slv.c
   ${SOES_PATH}/esc_eep.c
   ${SOES_PATH}/esc_eoe.c
   ${SOES_PATH}/esc_foe.c
   ${SOES_PATH}/esc.c
   ${SOES_PATH}/hal/tiesc/esc_hw_eep.c
   ${SOES_PATH}/hal/tiesc/esc_hw.c
   ${GENERATED_DIR}/ti_dpl_config.c
   ${GENERATED_DIR}/ti_board_config.c
   ${GENERATED_DIR}/ti_board_open_close.c
   ${GENERATED_DIR}/ti_drivers_config.c
   ${GENERATED_DIR}/ti_drivers_open_close.c
   ${GENERATED_DIR}/ti_enet_config.c
   ${GENERATED_DIR}/ti_enet_lwipif.c
   ${GENERATED_DIR}/ti_enet_open_close.c
   ${GENERATED_DIR}/ti_enet_soc.c
   ${GENERATED_DIR}/ti_pinmux_config.c
   ${GENERATED_DIR}/ti_power_clock_config.c
)

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE
add_executable(${EXECUTABLE} ${APP_SOURCES})

# this lets me include files relative to the root source directory with a <> pair
target_include_directories(${EXECUTABLE} PUBLIC
   .
   ${GENERATED_DIR}
   ${SOES_PATH}
   ${SOES_PATH}/hal/tiesc
   ${AM263X_MCU_PLUS_SKD}/source/industrial_comms/ethercat_slave/beckhoff_stack/stack_hal
)
if (USE_TI_COMPILER)
   target_include_directories(${EXECUTABLE} PUBLIC ${SOES_PATH}/include/sys/clang/)
else()
   target_include_directories(${EXECUTABLE} PUBLIC ${SOES_PATH}/include/sys/gcc/)
endif()

target_compile_definitions(${EXECUTABLE} PUBLIC
   SOC_AM263X
   TIESC_APPLICATION=1
   _DEBUG_=1
)

target_compile_options(${EXECUTABLE} PRIVATE ${C_CPP_FLAGS}
            "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
            "$<$<COMPILE_LANGUAGE:CXX>:${CPP_FLAGS}>")

target_link_libraries(${EXECUTABLE} PUBLIC
  ${TI_DRIVERS_LIB}
  ${TI_ETHERCAT_STACK}
  ${FREERTOS_LIB}
  ${BOARD_LIB}
)

set(GENERATED_SOURCES
   ${GENERATED_DIR}/ti_dpl_config.c
   ${GENERATED_DIR}/ti_dpl_config.h
   ${GENERATED_DIR}/ti_drivers_config.c
   ${GENERATED_DIR}/ti_drivers_config.h
   ${GENERATED_DIR}/ti_drivers_open_close.c
   ${GENERATED_DIR}/ti_drivers_open_close.h
   ${GENERATED_DIR}/ti_pinmux_config.c
   ${GENERATED_DIR}/ti_power_clock_config.c
   ${GENERATED_DIR}/ti_board_config.c
   ${GENERATED_DIR}/ti_board_config.h
   ${GENERATED_DIR}/ti_board_open_close.c
   ${GENERATED_DIR}/ti_board_open_close.h
   ${GENERATED_DIR}/ti_enet_config.c
   ${GENERATED_DIR}/ti_enet_config.h
   ${GENERATED_DIR}/ti_enet_open_close.c
   ${GENERATED_DIR}/ti_enet_open_close.h
   ${GENERATED_DIR}/ti_enet_soc.c
   ${GENERATED_DIR}/ti_enet_lwipif.c
   ${GENERATED_DIR}/ti_enet_lwipif.h
)

get_filename_component(LNK_FILE ${IND_COMMS_SDK_PATH}/examples/industrial_comms/ethercat_slave_beckhoff_ssc_demo/am263x-lp/r5fss0-0_freertos/ti-arm-clang/linker.cmd ABSOLUTE )

target_link_options(${EXECUTABLE} PRIVATE
   LINKER:--reread_libs
   LINKER:--diag_suppress=10063
   LINKER:--diag_wrap=off
   LINKER:--display_error_number
   LINKER:--warn_sections
   LINKER:--rom_model
   LINKER:--use_memcpy=fast
   LINKER:--use_memset=fast
   LINKER:-m=${MAP_FILE}
   ${LNK_FILE}
)

###############################################################################
## Syscfg generator ###########################################################
###############################################################################
add_custom_target(generated_files
   ${SYSCFG_NODE} ${SYSCFG_CLI_PATH}/dist/cli.js --product ${SYSCFG_SDKPRODUCT} --context r5fss0-0 --part AM263x --package ZCZ --output ${GENERATED_DIR} ${SYSCONF_FILE}
   COMMAND echo "I keep track of the last build time" >> dummy.txt
   BYPRODUCTS ${GENERATED_SOURCES}
   # MAIN_DEPENDENCY ${SYSCONF_FILE}
   DEPENDS ${SYSCONF_FILE}
)
add_dependencies(${EXECUTABLE} generated_files)

###############################################################################
## TI drivers #################################################################
###############################################################################
add_library(${TI_DRIVERS_LIB} STATIC
   ${AM263X_MCU_PLUS_SKD}/drivers/adc/v1/adc.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_buf_io.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_can.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_flash.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_mem.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/soc/am263x/bootloader_soc.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/soc/am263x/bootloader_hsmRt_load.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_xmodem.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_uniflash.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/bootloader_profile.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/xmodem.c
   ${AM263X_MCU_PLUS_SKD}/drivers/bootloader/crc16.c
   ${AM263X_MCU_PLUS_SKD}/drivers/cmpss/v0/cmpss.c
   ${AM263X_MCU_PLUS_SKD}/drivers/dac/v0/dac.c
   ${AM263X_MCU_PLUS_SKD}/drivers/ecap/v1/ecap.c
   ${AM263X_MCU_PLUS_SKD}/drivers/edma/v0/edma.c
   ${AM263X_MCU_PLUS_SKD}/drivers/eqep/v1/eqep.c
   ${AM263X_MCU_PLUS_SKD}/drivers/epwm/v1/etpwm.c
   ${AM263X_MCU_PLUS_SKD}/drivers/fsi/v1/fsi_rx.c
   ${AM263X_MCU_PLUS_SKD}/drivers/fsi/v1/fsi_tx.c
   ${AM263X_MCU_PLUS_SKD}/drivers/gpio/v0/gpio.c
   ${AM263X_MCU_PLUS_SKD}/drivers/gpmc/v0/gpmc_v0.c
   ${AM263X_MCU_PLUS_SKD}/drivers/gpmc/v0/dma/gpmc_dma.c
   ${AM263X_MCU_PLUS_SKD}/drivers/hsmclient/hsmclient.c
   ${AM263X_MCU_PLUS_SKD}/drivers/hsmclient/soc/am263x/hsmclient_loadhsmrt.c
   ${AM263X_MCU_PLUS_SKD}/drivers/hsmclient/utils/hsmclient_utils.c
   ${AM263X_MCU_PLUS_SKD}/drivers/i2c/v1/i2c_v1.c
   ${AM263X_MCU_PLUS_SKD}/drivers/ipc_notify/v1/ipc_notify_v1.c
   ${AM263X_MCU_PLUS_SKD}/drivers/ipc_notify/v1/soc/am263x/ipc_notify_v1_cfg.c
   ${AM263X_MCU_PLUS_SKD}/drivers/ipc_rpmsg/ipc_rpmsg.c
   ${AM263X_MCU_PLUS_SKD}/drivers/ipc_rpmsg/ipc_rpmsg_vring.c
   ${AM263X_MCU_PLUS_SKD}/drivers/lin/v0/lin.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mcan/v0/mcan.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mcspi/v0/mcspi_v0.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mcspi/v0/lld/mcspi_v0_lld.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mcspi/v0/lld/dma/edma/mcspi_dma_edma.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mdio/v0/mdio_v0.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mmcsd/v1/mmcsd_v1.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mpu_firewall/v0/mpu_firewall.c
   ${AM263X_MCU_PLUS_SKD}/drivers/mpu_firewall/v0/soc/am263x/mpu_firewall_v0_cfg.c
   ${AM263X_MCU_PLUS_SKD}/drivers/pinmux/am263x/pinmux.c
   ${AM263X_MCU_PLUS_SKD}/drivers/pmu/pmu.c
   ${AM263X_MCU_PLUS_SKD}/drivers/pruicss/m_v0/pruicss_m_v0.c
   ${AM263X_MCU_PLUS_SKD}/drivers/pruicss/soc/am263x/pruicss_m_v0_cfg.c
   ${AM263X_MCU_PLUS_SKD}/drivers/qspi/v0/qspi.c
   ${AM263X_MCU_PLUS_SKD}/drivers/qspi/v0/edma/qspi_edma.c
   ${AM263X_MCU_PLUS_SKD}/drivers/rti/v0/rti.c
   ${AM263X_MCU_PLUS_SKD}/drivers/sdfm/v0/sdfm.c
   ${AM263X_MCU_PLUS_SKD}/drivers/secure_ipc_notify/sipc_notify_src.c
   ${AM263X_MCU_PLUS_SKD}/drivers/secure_ipc_notify/soc/am263x/sipc_notify_cfg.c
   ${AM263X_MCU_PLUS_SKD}/drivers/spinlock/v0/spinlock.c
   ${AM263X_MCU_PLUS_SKD}/drivers/soc/am263x/soc.c
   ${AM263X_MCU_PLUS_SKD}/drivers/soc/am263x/soc_rcm.c
   ${AM263X_MCU_PLUS_SKD}/drivers/uart/v0/uart_v0.c
   ${AM263X_MCU_PLUS_SKD}/drivers/uart/v0/lld/uart_v0_lld.c
   ${AM263X_MCU_PLUS_SKD}/drivers/uart/v0/lld/dma/edma/uart_dma_edma.c
   ${AM263X_MCU_PLUS_SKD}/drivers/watchdog/v0/watchdog_rti.c
   ${AM263X_MCU_PLUS_SKD}/drivers/watchdog/v0/soc/am263x/watchdog_soc.c
   ${AM263X_MCU_PLUS_SKD}/drivers/pmu/r5f/csl_arm_r5_pmu.S
)
target_include_directories(${TI_DRIVERS_LIB} PUBLIC
   ${AM263X_MCU_PLUS_SKD}
)
target_compile_definitions(${TI_DRIVERS_LIB} PUBLIC SOC_AM263X)
target_compile_options(${TI_DRIVERS_LIB} PRIVATE ${C_CPP_FLAGS}
            "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
            "$<$<COMPILE_LANGUAGE:CXX>:${CPP_FLAGS}>")
###############################################################################
## Ethercat stack #############################################################
###############################################################################
add_library(${TI_ETHERCAT_STACK} STATIC
   ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/icss_fwhal/tiescbsp.c
   ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/beckhoff_stack/stack_hal/tieschw.c
)
target_include_directories(${TI_ETHERCAT_STACK} PUBLIC
   .
   ${AM263X_MCU_PLUS_SKD}
   ${IND_COMMS_SDK_PATH}/source
   ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/beckhoff_stack/stack_hal
   ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/icss_fwhal
   ${IND_COMMS_SDK_PATH}/source/industrial_comms/ethercat_slave/icss_fwhal/firmware/m_v2.3
)
target_compile_definitions(${TI_ETHERCAT_STACK} PUBLIC SOC_AM263X)
target_compile_options(${TI_ETHERCAT_STACK} PRIVATE ${C_CPP_FLAGS}
            "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
            "$<$<COMPILE_LANGUAGE:CXX>:${CPP_FLAGS}>")
###############################################################################
## FreeRTOS ###################################################################
###############################################################################
add_library(${FREERTOS_LIB} STATIC
   ${NORTOS_PATH}/dpl/common/AddrTranslateP_null.c
   ${NORTOS_PATH}/dpl/r5/boot_armv7r.c
   ${NORTOS_PATH}/dpl/common/DebugP_shmLogWriter.c
   ${NORTOS_PATH}/dpl/common/DebugP_shmLogReader.c
   ${NORTOS_PATH}/dpl/common/DebugP_uartLogWriter.c
   ${NORTOS_PATH}/dpl/common/DebugP_uartScanf.c
   ${NORTOS_PATH}/dpl/common/DebugP_memTraceLogWriter.c
   ${NORTOS_PATH}/dpl/common/DebugP_log.c
   ${NORTOS_PATH}/dpl/common/TimerP_rti.c
   ${NORTOS_PATH}/dpl/r5/HwiP_armv7r_vim.c
   ${NORTOS_PATH}/dpl/r5/MpuP_armv7r.c
   ${NORTOS_PATH}/dpl/r5/CacheP_armv7r.c
   ${NORTOS_PATH}/dpl/r5/CpuId_armv7r.c
   ${NORTOS_PATH}/dpl/common/HeapP_internal.c
   ${NORTOS_PATH}/dpl/common/printf.c
   ${NORTOS_PATH}/dpl/common/AddrTranslateP.c
   ${NORTOS_PATH}/dpl/r5/PmuP_armv7r.c
   ${NORTOS_PATH}/dpl/common/QueueP_nortos.c
   ${FREERTOS_PATH}/dpl/common/ClockP_freertos.c
   ${FREERTOS_PATH}/dpl/r5/ClockP_freertos_r5.c
   ${FREERTOS_PATH}/dpl/common/SemaphoreP_freertos.c
   ${FREERTOS_PATH}/dpl/r5/HwiP_armv7r_handlers_freertos.c
   ${FREERTOS_PATH}/dpl/common/HeapP_freertos.c
   ${FREERTOS_PATH}/dpl/common/DebugP_freertos.c
   ${FREERTOS_PATH}/dpl/common/DebugP_shmLogReader_freertos.c
   ${FREERTOS_PATH}/dpl/common/TaskP_freertos.c
   ${FREERTOS_PATH}/dpl/common/EventP_freertos.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/event_groups.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/tasks.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/timers.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/queue.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/list.c
   ${FREERTOS_PATH}/FreeRTOS-Kernel/portable/MemMang/heap_3.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_clock.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_mqueue.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_barrier.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_cond.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_mutex.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_sched.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_semaphore.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_timer.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_unistd.c
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_utils.c
   ${FREERTOS_PATH}/portable/TI_ARM_CLANG/ARM_CR5F/port.c
   ${NORTOS_PATH}/dpl/r5/boot_armv7r_asm.S
   ${NORTOS_PATH}/dpl/r5/HwiP_armv7r_asm.S
   ${NORTOS_PATH}/dpl/r5/MpuP_armv7r_asm.S
   ${NORTOS_PATH}/dpl/r5/CacheP_armv7r_asm.S
   ${NORTOS_PATH}/dpl/r5/CpuId_armv7r_asm.S
   ${NORTOS_PATH}/dpl/r5/PmuP_armv7r_asm.S
   ${FREERTOS_PATH}/dpl/r5/HwiP_armv7r_vectors_freertos_asm.S
   ${FREERTOS_PATH}/dpl/r5/HwiP_armv7r_handlers_freertos_asm.S
   ${FREERTOS_PATH}/portable/TI_ARM_CLANG/ARM_CR5F/portASM.S
   ${FREERTOS_PATH}/dpl/r5/Mutex_armv7r_asm.S
)
target_include_directories(${FREERTOS_LIB} PUBLIC
   ${AM263X_MCU_PLUS_SKD}
   ${FREERTOS_PATH}/FreeRTOS-Kernel/include
   ${FREERTOS_PATH}/portable/TI_ARM_CLANG/ARM_CR5F
   ${FREERTOS_PATH}/config/am263x/r5f
   ${FREERTOS_PATH}/FreeRTOS-POSIX/include
   ${FREERTOS_PATH}/FreeRTOS-POSIX/include/private
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/include
   ${FREERTOS_PATH}/FreeRTOS-POSIX/FreeRTOS-Plus-POSIX/include/portable
)
target_compile_definitions(${FREERTOS_LIB} PUBLIC SOC_AM263X)
target_compile_options(${FREERTOS_LIB} PRIVATE ${C_CPP_FLAGS}
            "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
            "$<$<COMPILE_LANGUAGE:CXX>:${CPP_FLAGS}>")
###############################################################################
## Board drivers ##############################################################
###############################################################################
add_library(${BOARD_LIB} STATIC
   ${BOARD_DRIVER_PATH}/eeprom/eeprom.c
   ${BOARD_DRIVER_PATH}/eeprom/eeprom_cat24m.c
   ${BOARD_DRIVER_PATH}/ethphy/ethphy.c
   ${BOARD_DRIVER_PATH}/ethphy/ethphy_dp83869.c
   ${BOARD_DRIVER_PATH}/ethphy/ethphy_dp83826e.c
   ${BOARD_DRIVER_PATH}/flash/flash.c
   ${BOARD_DRIVER_PATH}/psram/psram.c
   ${BOARD_DRIVER_PATH}/psram/gpmc/psram_gpmc.c
   ${BOARD_DRIVER_PATH}/flash/qspi/flash_nor_qspi.c
   ${BOARD_DRIVER_PATH}/led/led.c
   ${BOARD_DRIVER_PATH}/led/led_gpio.c
   ${BOARD_DRIVER_PATH}/led/led_tpic2810.c
   ${BOARD_DRIVER_PATH}/flash/sfdp/nor_spi_sfdp.c
)
target_include_directories(${BOARD_LIB} PUBLIC
   ${AM263X_MCU_PLUS_SKD}
)
target_compile_definitions(${BOARD_LIB} PUBLIC SOC_AM263X)
target_compile_options(${BOARD_LIB} PRIVATE ${C_CPP_FLAGS}
            "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
            "$<$<COMPILE_LANGUAGE:CXX>:${CPP_FLAGS}>")
