#include "esc_hw.h"
#include "ecat_slv.h"
#include "options.h"
#include "utypes.h"
#include <string.h>
#include "tiescutils.h"
#include <examples/board/include/board_i2cLed.h>
#include <examples/board/include/board_rotary_switch.h>

#if 1
/* Application variables */
_Objects    Obj;

/**
 * This function reads physical input values and assigns the corresponding members
 * of Rb.Buttons
 */
void cb_get_inputs()
{
    volatile uint8_t io_input;
    Board_readRotarySwitch((uint8_t *)&io_input);
    Obj.BUTTON = io_input;
}

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */
void cb_set_outputs()
{
    volatile uint8_t io_output;
    io_output = Obj.LED;
    Board_setDigOutput(io_output);
}
#endif


/* Called from stack when stopping outputs */
void user_safeoutput (void)
{
   memset(&Obj.LED, 0, (sizeof(Obj.LED)));
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
   .esc_hw_interrupt_enable = ESC_interrupt_enable,
   .esc_hw_interrupt_disable = ESC_interrupt_disable,
   .esc_hw_eep_handler = ESC_eep_handler,
   .esc_check_dc_handler = NULL
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
