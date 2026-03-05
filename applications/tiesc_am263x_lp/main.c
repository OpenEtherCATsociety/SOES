/*
 *  Copyright (C) 2018-2022 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <ecat_def.h>
#include "utypes.h"
#include "ecat_slv.h"
#include "esc_hw.h"
#include <kernel/dpl/TaskP.h>

#if COE_SUPPORTED
#include <objdef.h>
#endif

#include <drivers/gpio.h>

#define _TIESC_HW_
#include <ecatslv.h>
// #include <tiescappl.h>
#undef _TIESC_HW_

#include <tiescsoc.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "tieschw.h"
#include <drivers/pinmux.h>

#define MAIN_TASK_PRI           (1)
#define PDI_TASK_PRIORITY       (6)
#define LED_TASK_PRIORITY       (4)
#define SYNC0_TASK_PRIORITY     (8)
#define SYNC1_TASK_PRIORITY     (8)

#define MAIN_TASK_SIZE          (16384U)
#define PDI_TASK_STACK_SIZE     (1152*2)
#define LED_TASK_STACK_SIZE     (1512*2)
#define SYNC0_TASK_STACK_SIZE   (1152*2)
#define SYNC1_TASK_STACK_SIZE   (1152*2)

#define BUTTON_IN_PIN 123

static Pinmux_PerCfg_t sPinMuxButtonIn[] = {
   /* GPIO123 -> SDFM0_D0 (D14) */
    {
        PIN_SDFM0_D0,
        ( PIN_MODE(7) | PIN_PULL_DISABLE | PIN_SLEW_RATE_LOW | PIN_QUAL_SYNC | PIN_GPIO_R5SS0_0 )
    },
    {PINMUX_END, PINMUX_END}
   };

/** \brief Global Structure pointer holding PRUSS0 memory Map. */
PRUICSS_Handle pruIcss1Handle;
/** \brief Global Structure pointer holding PRUSS1 memory Map. */
PRUICSS_Handle pruIcss0Handle;

StackType_t gMainTaskStack[MAIN_TASK_SIZE/sizeof(configSTACK_DEPTH_TYPE)] __attribute__((aligned(32)));
StackType_t gPdiTaskStack[PDI_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE)] __attribute__((aligned(32)));
StackType_t gLedTaskStack[LED_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE)] __attribute__((aligned(32)));
StackType_t gSync0TaskStack[SYNC0_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE)] __attribute__((aligned(32)));
StackType_t gSync1TaskStack[SYNC1_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE)] __attribute__((aligned(32)));

StaticTask_t gMainTaskObj;
StaticTask_t gPdiTaskObj;
StaticTask_t gLedTaskObj;
StaticTask_t gSync0TaskObj;
StaticTask_t gSync1TaskObj;

TaskHandle_t gMainTask;
TaskHandle_t gPdiTask;
TaskHandle_t gLedTask;
TaskHandle_t gSync0Task;
TaskHandle_t gSync1Task;

extern uint32_t appState;
_Objects Obj = {0};

static uint32_t sGpioBaseAddr = 0;

void cb_get_inputs (void)
{
   Obj.BUTTON = GPIO_pinRead(sGpioBaseAddr, BUTTON_IN_PIN);
}

void cb_set_outputs (void)
{
    tiesc_setErrorLed(false);
   if (Obj.LED)
   {
    //   XMC_GPIO_SetOutputHigh(P_LED);
    tiesc_setRunLed(true);
    // GPIO_pinWriteHigh(sGpioBaseAddr, BUTTON_IN_PIN);
   }
   else
   {
    //   XMC_GPIO_SetOutputLow(P_LED);
    tiesc_setRunLed(false);
    // GPIO_pinWriteLow(sGpioBaseAddr, BUTTON_IN_PIN);
   }
}

uint32_t post_object_download_hook (uint16_t index, uint8_t subindex,
                                uint16_t flags)
{
   uint32_t returnVal = 1;
   switch(index)
   {
      case 0x7100:
      {
         switch (subindex)
         {
            case 0x01:
            {
               returnVal = 0;
               break;
            }
         }
         break;
      }
      case 0x8001:
      {
         switch (subindex)
         {
            case 0x01:
            {
               returnVal = 0;
               break;
            }
         }
         break;
      }
   }
   return returnVal;
}

/* Called from stack when stopping outputs */
void user_safeoutput (void)
{
    tiesc_setRunLed(false);
    tiesc_setErrorLed(true);
}

#ifdef ENABLE_PDI_TASK
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief Interrupt service routine for the interrupts from the EtherCAT Slave Controller
*////////////////////////////////////////////////////////////////////////////////////////
#if AL_EVENT_ENABLED
void HW_EcatIsr(void)
{
#ifdef PROFILE_ECAT_STACK
    uintptr_t key1;
    uint32_t pdi_start, pdi_stop;
    /* get the AL event register */

    key1 = HwiP_disable();
    bsp_get_local_sys_time(&pdi_start,  0);
#endif //PROFILE_ECAT_STACK
    PDI_Isr();
#ifdef PROFILE_ECAT_STACK
    bsp_get_local_sys_time(&pdi_stop,   0);
    HwiP_restore(key1);

    if(pdi_stop >= pdi_start)
    {
        pdi_delta = pdi_stop - pdi_start;
    }
    else
    {
        pdi_delta = 0xFFFFFFFFF - pdi_start + pdi_stop;
    }

    if(pdi_delta > pdi_max)
    {
        pdi_max = pdi_delta;
    }

#endif //PROFILE_ECAT_STACK
}
#endif //AL_EVENT_ENABLED

void PDItask(void *arg)
{
//    TaskP_sleep(10 * OS_TICKS_IN_MILLI_SEC);
    /*TODO: Check if following is correct*/
    ClockP_usleep(10 * 1000);
#if AL_EVENT_ENABLED
    uint32_t evtOutNum = HOST_AL_EVENT - 20;
    while(1)
    {
        PRUICSS_waitEvent((PRUICSS_Handle)arg, evtOutNum);
        /* ISR processing */
        HW_EcatIsr();
    }
#endif //AL_EVENT_ENABLED
}
#endif //ENABLE_PDI_TASK
void LEDtask(void *args)
{
    VARVOLATILE uint8_t state = STATE_INIT;
#ifdef ENABLE_STARTUP_DIGOUT_ANIMATION
        VARVOLATILE uint8_t state = STATE_INIT;
        static uint16_t digout_led_mask = 1;

        while(STATE_INIT == state)
        {
            I2CSetLed(&gI2CObj, digout_led_mask);
            digout_led_mask <<= 1;

            if(digout_led_mask >= 256)
            {
                digout_led_mask = 1;
            }

            HW_EscReadByte(state, ESC_ADDR_ALSTATUS);
            TaskP_sleep(200 * OS_TICKS_IN_MILLI_SEC);
        }

#else
        ClockP_usleep(200 * 1000);
        tiesc_setOutputLed(0x6a);
#endif
    while(1)
    {
        ClockP_usleep(50 * 1000);

        uint32_t reset_reg_val;

#if ESC_EEPROM_EMULATION
        HW_EscReadByte(state, ESC_ADDR_ALSTATUS);
        state &= 0xF;
        {
            if(bsp_get_eeprom_update_status())
            {
                uint32_t t_cur_time;
#ifdef USE_ECAT_TIMER
                bsp_get_local_sys_time(&t_cur_time, NULL);
#else
                t_cur_time = Timestamp_get32();
#endif
                uint32_t t_last_time = bsp_get_eeprom_updated_time();
                t_cur_time = ((t_cur_time >= t_last_time) ? (t_cur_time - t_last_time) :
                              t_cur_time + (0xFFFFFFFF - t_last_time));

                if(t_cur_time >= (ECAT_TIMER_INC_P_MS * 100))
                {
                    bsp_set_eeprom_update_status(0);
                    bsp_eeprom_emulation_flush();
                }
            }
        }

#endif
        reset_reg_val = bsp_read_dword(pruIcss1Handle, ESC_ADDR_TI_ESC_RESET);

        if((reset_reg_val == TI_ESC_RST_CMD_U) ||
                (reset_reg_val == TI_ESC_RST_CMD_L))
        {
            /*Write 0x0 to 0xE14, else application will request reset continuously.*/
            bsp_write_dword(pruIcss1Handle, (uint32_t)0x0, ESC_ADDR_TI_ESC_RESET);
            ASSERT_DMB();
            ASSERT_DSB();
            //EtherCAT master has requested S/W RESET
            HW_RestartTarget();
        }
    }
}

#ifdef ENABLE_SYNC_TASK
void Sync0task(void *arg)
{
#ifdef PROFILE_ECAT_STACK
    uint32_t sync_start, sync_stop;
#endif
#ifndef EXEC_FROM_IRAM
    DebugP_log("SYNC0 task started\n\r");
#endif
    uint32_t evtOutNum = HOST_SYNC0_EVENT - 20;
    while(1)
    {
        PRUICSS_waitEvent((PRUICSS_Handle)arg, evtOutNum);

        //Do sync0 event handling
        DISABLE_ESC_INT();
#ifdef PROFILE_ECAT_STACK
        uintptr_t key1;
        key1 = HwiP_disable();
        bsp_get_local_sys_time(&sync_start, 0);
#endif
        Sync0_Isr();

        if(!bsp_read_word((PRUICSS_Handle)arg, ESC_ADDR_SYNC_PULSE_LENGTH))
        {
            bsp_read_byte((PRUICSS_Handle)arg, ESC_ADDR_SYNC_STATUS);
        }

#ifdef PROFILE_ECAT_STACK
        bsp_get_local_sys_time(&sync_stop,  0);
        HwiP_restore(key1);

        if(sync_stop >= sync_start)
        {
            sync_delta = sync_stop - sync_start;
        }
        else
        {
            sync_delta = 0xFFFFFFFFF - sync_start + sync_stop;
        }

        if(sync_delta > sync_max)
        {
            sync_max = sync_delta;
        }

#endif
        ENABLE_ESC_INT();
    }
}

void Sync1task(void *arg)
{

#ifndef EXEC_FROM_IRAM
    DebugP_log("SYNC1 task started\n\r");
#endif
    uint32_t evtOutNum = HOST_SYNC1_EVENT - 20;
    while(1)
    {
        PRUICSS_waitEvent((PRUICSS_Handle)arg, evtOutNum);

        //Do sync1 event handling
        DISABLE_ESC_INT();
        Sync1_Isr();

        if(!bsp_read_word((PRUICSS_Handle)arg, ESC_ADDR_SYNC_PULSE_LENGTH))
        {
            bsp_read_byte((PRUICSS_Handle)arg, ESC_ADDR_SYNC_STATUS + 1);
        }

        ENABLE_ESC_INT();
    }
}
#endif

static esc_cfg_t config =
{
   .user_arg = NULL,
   .use_interrupt = 0,
   .watchdog_cnt = 9998,
   .set_defaults_hook = NULL,
   .pre_state_change_hook = NULL,
   .post_state_change_hook = NULL,
   .application_hook = NULL,
   .safeoutput_override = user_safeoutput,
   .pre_object_download_hook = NULL,
   .post_object_download_hook = NULL,
   .rxpdo_override = NULL,
   .txpdo_override = NULL,
   .esc_hw_interrupt_enable = NULL,
   .esc_hw_interrupt_disable = NULL,
   .esc_hw_eep_handler = ESC_eep_handler,
   .esc_check_dc_handler = NULL
};

void freertos_main(void *args)
{
   uint32_t status;

   Drivers_open();
   status = Board_driversOpen();
   DebugP_assert(status==SystemP_SUCCESS);

   tiesc_socEvmInit();
   #ifdef MDIO_MANUAL_MODE_ENABLED
   tiesc_mdioManualModeSetup();
   #endif

   /* Enable IEEE Power Down mode so that PHY does not establish any link */
   tiesc_ethphyEnablePowerDown();

   if(tiesc_isEthercatDevice())
   {
      DebugP_log("EtherCAT Device\n\r");
   }
   else
   {
      DebugP_log("Non-EtherCAT Device\n\r");
   }

    DebugP_log("EtherCAT Sample application\n\r");

    /* initialize the Hardware and the EtherCAT Slave Controller */
    HW_Init();

    /* Disable IEEE Power Down mode so that PHY can establish link */
    tiesc_ethphyDisablePowerDown();

   Pinmux_config(sPinMuxButtonIn, PINMUX_DOMAIN_ID_MAIN);
   sGpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CSL_GPIO0_U_BASE);
   GPIO_setDirMode(sGpioBaseAddr, BUTTON_IN_PIN, GPIO_DIRECTION_INPUT);

   ecat_slv_init(&config); // EtherCAT stack init

#if EOE_SUPPORTED || FOE_SUPPORTED
    tiesc_foe_eoe_init();
#endif

   /* Create tasks */
#ifdef ENABLE_PDI_TASK
   gPdiTask = xTaskCreateStatic( PDItask, // Task func
         "PDItask", // Name
         PDI_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE),  // Stack size
         (void *)pruIcss1Handle,            // Arg
         PDI_TASK_PRIORITY,   // task priority
         gPdiTaskStack,  // pointer to stack
         &gPdiTaskObj ); // statically allocated task object

   DebugP_assert(gPdiTask != NULL);
#endif
#ifdef ENABLE_SYNC_TASK
   gSync0Task = xTaskCreateStatic( Sync0task, // Task func
         "Sync0task", // Name
         SYNC0_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE),  // Stack size
         (void *)pruIcss1Handle,            // Arg
         SYNC0_TASK_PRIORITY,   // task priority
         gSync0TaskStack,  // pointer to stack
         &gSync0TaskObj ); // statically allocated task object
   DebugP_assert(gSync0Task != NULL);

   gSync1Task = xTaskCreateStatic( Sync1task, // Task func
         "Sync1task", // Name
         SYNC1_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE),  // Stack size
         (void *)pruIcss1Handle,            // Arg
         SYNC1_TASK_PRIORITY,   // task priority
         gSync1TaskStack,  // pointer to stack
         &gSync1TaskObj ); // statically allocated task object
   DebugP_assert(gSync1Task != NULL);
#endif
   gLedTask = xTaskCreateStatic( LEDtask, // Task func
         "LEDtask", // Name
         LED_TASK_STACK_SIZE/sizeof(configSTACK_DEPTH_TYPE),  // Stack size
         (void *)pruIcss1Handle,            // Arg
         LED_TASK_PRIORITY,   // task priority
         gLedTaskStack,  // pointer to stack
         &gLedTaskObj ); // statically allocated task object
   DebugP_assert(gLedTask != NULL);

   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   bRunApplication = TRUE;
   do
   {
#ifdef PROFILE_ECAT_STACK
      {
         uint32_t mainloop_start, mainloop_stop;
         bsp_get_local_sys_time(&mainloop_start, 0);
#endif

         MainLoop();
#ifdef PROFILE_ECAT_STACK
         bsp_get_local_sys_time(&mainloop_stop,  0);

         if(mainloop_stop >= mainloop_start)
         {
               mainloop_delta = mainloop_stop - mainloop_start;
         }

         else
         {
               mainloop_delta = 0xFFFFFFFFF - mainloop_start + mainloop_stop;
         }

         if(mainloop_delta > mainloop_max)
         {
               mainloop_max = mainloop_delta;
         }
      }
#endif
      TaskP_yield();
   }
   while(bRunApplication == TRUE);

   HW_Release();
   vTaskDelete(NULL);
}

void MainLoop (void)
{
    ecat_slv();
    DIG_process(DIG_PROCESS_WD_FLAG);
}

int main(void)
{
   /* init SOC specific modules */
   System_init();
   Board_init();

    /* This task is created at highest priority, it should create more tasks and then delete itself */
    gMainTask = xTaskCreateStatic( freertos_main,   /* Pointer to the function that implements the task. */
                                  "freertos_main", /* Text name for the task.  This is to facilitate debugging only. */
                                  MAIN_TASK_SIZE/sizeof(configSTACK_DEPTH_TYPE),  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                  NULL,            /* We are not using the task parameter. */
                                  MAIN_TASK_PRI,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                  gMainTaskStack,  /* pointer to stack base */
                                  &gMainTaskObj ); /* pointer to statically allocated task object memory */
    configASSERT(gMainTask != NULL);

    /* Start the scheduler to start the tasks executing. */
    vTaskStartScheduler();

    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return 0;
}
