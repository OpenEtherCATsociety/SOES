#include "k2gice.h"
#include "esc_hw.h"
#include "config.h"
#include <string.h>
#include "tiescutils.h"
#include <examples/board/include/board_i2cLed.h>
#include <examples/board/include/board_rotary_switch.h>

/**
 * This function reads physical input values and assigns the corresponding members
 * of Rb.Buttons
 */
void cb_get_BUTTON()
{
    volatile uint8_t io_input;
    Board_readRotarySwitch(&io_input);
    Rb.BUTTON = io_input;
}

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */
void cb_set_LED()
{
    volatile uint8_t io_output;
    io_output = Wb.LED;
    Board_setDigOutput(io_output);
}


/* Called from stack when stopping outputs */
void user_safeoutput (void)
{
   memset(&Wb, 0, (sizeof(Wb)));
   Board_setDigOutput(0);
}

/* Configuration parameters for SOES
 * SM and Mailbox parameters comes from the
 * generated config.h
 */
static esc_cfg_t config =
{
   .user_arg = NULL,
   .use_interrupt = 1,
   .watchdog_cnt = 9999,
   .mbxsize = MBXSIZE,
   .mbxsizeboot = MBXSIZEBOOT,
   .mbxbuffers = MBXBUFFERS,
   .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
   .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
   .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
   .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
   .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
   .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},
   .pre_state_change_hook = NULL,
   .post_state_change_hook = NULL,
   .application_hook = NULL,
   .safeoutput_override = user_safeoutput,
   .pre_object_download_hook = NULL,
   .post_object_download_hook = NULL,
   .rxpdo_override = NULL,
   .txpdo_override = NULL,
   .esc_hw_interrupt_enable = ESC_interrupt_enable,
   .esc_hw_interrupt_disable = ESC_interrupt_disable,
   .esc_hw_eep_handler = ESC_eep_handler
};

int MainInit(void)
{
   ecat_slv_init(&config);
   return 0;
}

void MainLoop(void)
{
   ecat_slv_poll();
   DIG_process(DIG_PROCESS_WD_FLAG);
}

int main()
{
   common_main();
   return 0;
}
