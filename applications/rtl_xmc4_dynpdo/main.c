/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <kern.h>
#include "esc.h"
#include "esc_hw.h"
#include "ecat_slv.h"
#include "config.h"
#include "utypes.h"

/* Global variables used by the stack */
uint8_t     MBX[MBXBUFFERS * MAX(MBXSIZE,MBXSIZEBOOT)];
_MBXcontrol MBXcontrol[MBXBUFFERS];
_ESCvar     ESCvar;
_SMmap      SMmap2[MAX_MAPPINGS_SM2];
_SMmap      SMmap3[MAX_MAPPINGS_SM3];

/* Application variables */
_Objects    Obj;

/* Private variables */
static uint8_t inputs[MAX_INPUT_SIZE] __attribute__((aligned (8)));
static uint8_t outputs[MAX_OUTPUT_SIZE] __attribute__((aligned (8)));

void cb_get_inputs (void)
{
   Obj.IN1 = 1;
   Obj.IN2 = 2;
   Obj.IN3 = 3;
   Obj.IN4 = 4;
}

void cb_set_outputs (void)
{
}


/* Setup of DC */
uint16_t dc_checker (void)
{
   /* Indicate we run DC */
   ESCvar.dcsync = 1;
   /* Fetch the sync counter limit  SDO10F1*/
   ESCvar.synccounterlimit = Obj.ErrorSettings.SyncErrorCounterLimit;
   return 0;
}

int main (void)
{
   static esc_cfg_t config =
   {
      .user_arg = NULL,
      .use_interrupt = 1,
      .watchdog_cnt = INT32_MAX, /* Use HW SM watchdog instead */
      .mbxsize = MBXSIZE,
      .mbxsizeboot = MBXSIZEBOOT,
      .mbxbuffers = MBXBUFFERS,
      .rxpdos_address = outputs,
      .rxpdos_mappings = MAX_SM_MAPPINGS,
      .txpdos_address = inputs,
      .txpdos_mappings = MAX_SM_MAPPINGS,
      .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
      .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
      .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
      .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
      .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
      .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},
      .pre_state_change_hook = NULL,
      .post_state_change_hook = NULL,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = ESC_interrupt_enable,
      .esc_hw_interrupt_disable = ESC_interrupt_disable,
      .esc_hw_eep_handler = ESC_eep_handler,
      .esc_check_dc_handler = dc_checker
   };

   rprintf ("Hello world\n");
   ecat_slv_init (&config);
   return 0;
}
