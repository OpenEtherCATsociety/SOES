/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware layer functions.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 */
#include <kern.h>
#include <bsp.h>
#include <xmc4.h>
#include <string.h>
#include "esc_hw.h"
#include "esc.h"
#include "esc_eep.h"
#include <eru.h>

static int use_all_interrupts = 0;

#define ESCADDR(x)   (((uint8_t *) ECAT0_BASE) + x)
static volatile esc_registers_t * ecat0 = (esc_registers_t *)ECAT0_BASE;

sem_t * ecat_isr_sem;
void ecat_isr(void * arg);
void sync0_isr(void * arg);
static volatile uint8_t read_ack;
static const eru_cfg_t cfg = {
    .base = ERU1_BASE,
    .channel = 2,
    .in_a = 0,
    .in_b = 3,
    .exicon = ERU_EXICON_PE_ENABLED |
              ERU_EXICON_LD_ENABLED |
              ERU_EXICON_RE_ENABLED |
              ERU_EXICON_OCS(2) |
              ERU_EXICON_SS(1),
    .exocon = ERU_EXOCON_ISS(0) |
              ERU_EXOCON_GP(1) |
              ERU_EXOCON_IPEEN(0),
    .irq = IRQ_ERU1_SR7,
    .isr = sync0_isr,
    .arg = NULL,
};

static const scu_ecat_cfg_t port_control =
{
   .con = {0},
   .conp0 = {
      .rxd[0] = ECAT_PORT0_CTRL_RXDO0,
      .rxd[1] = ECAT_PORT0_CTRL_RXDO1,
      .rxd[2] = ECAT_PORT0_CTRL_RXDO2,
      .rxd[3] = ECAT_PORT0_CTRL_RXDO3,
      .rx_clk = ECAT_PORT0_CTRL_RX_CLK,
      .rx_dv = ECAT_PORT0_CTRL_RX_DV,
      .rx_err = ECAT_PORT0_CTRL_RX_ERR,
      .link = ECAT_PORT0_CTRL_LINK,
      .tx_clk = ECAT_PORT0_CTRL_TX_CLK,
      .tx_shift = ECAT_PORT0_CTRL_TX_SHIFT
   },
   .conp1 = {
      .rxd[0] = ECAT_PORT1_CTRL_RXDO0,
      .rxd[1] = ECAT_PORT1_CTRL_RXDO1,
      .rxd[2] = ECAT_PORT1_CTRL_RXDO2,
      .rxd[3] = ECAT_PORT1_CTRL_RXDO3,
      .rx_clk = ECAT_PORT1_CTRL_RX_CLK,
      .rx_dv = ECAT_PORT1_CTRL_RX_DV,
      .rx_err = ECAT_PORT1_CTRL_RX_ERR,
      .link = ECAT_PORT1_CTRL_LINK,
      .tx_clk = ECAT_PORT1_CTRL_TX_CLK,
      .tx_shift = ECAT_PORT1_CTRL_TX_SHIFT
   }
};

/* EtherCAT module clock ungating and deassert reset API (Enables ECAT) */
void ESC_enable(void)
{
   scu_put_peripheral_in_reset (SCU_PERIPHERAL_ECAT0);
   scu_ungate_clock_to_peripheral (SCU_PERIPHERAL_ECAT0);
   scu_release_peripheral_from_reset (SCU_PERIPHERAL_ECAT0);
}

/* EtherCAT module clock gating and assert reset API (Disables ECAT)*/
void ESC_disable(void)
{
   scu_put_peripheral_in_reset (SCU_PERIPHERAL_ECAT0);
   scu_gate_clock_to_peripheral (SCU_PERIPHERAL_ECAT0);
}

/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
   if(use_all_interrupts == 0)
   {
      ESCvar.ALevent = etohs ((uint16_t)ecat0->AL_EVENT_REQ);
   }
   memcpy (buf, ESCADDR(address), len);
}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write (uint16_t address, void *buf, uint16_t len)
{
   if(use_all_interrupts == 0)
   {
      ESCvar.ALevent = etohs ((uint16_t)ecat0->AL_EVENT_REQ);
   }
   memcpy (ESCADDR(address), buf, len);
}

void ESC_reset (void)
{
  /* disable ESC to force reset */
  ESC_disable ();
  /* initialize EEPROM emulation */
  EEP_init ();
}

/** ESC interrupt enable function by the Slave stack in IRQ mode.
 *
 * @param[in]   mask     = of interrupts to enable
 */
void ESC_interrupt_enable (uint32_t mask)
{
   if(ESCREG_ALEVENT_DC_SYNC0 & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_SYNC0;
      int_enable(cfg.irq);
   }
   if(ESCREG_ALEVENT_DC_SYNC1 & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_SYNC1;
      UASSERT(0,EARG);
   }
   if(ESCREG_ALEVENT_DC_LATCH & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_LATCH;
      UASSERT(0,EARG);
   }
   ecat0->AL_EVENT_MASK |= mask;
}

/** ESC interrupt disable function by the Slave stack in IRQ mode.
 *
 * @param[in]   mask     = interrupts to disable
 */
void ESC_interrupt_disable (uint32_t mask)
{
   if(ESCREG_ALEVENT_DC_SYNC0 & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_SYNC0;
      int_disable(cfg.irq);
   }
   if(ESCREG_ALEVENT_DC_SYNC1 & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_SYNC1;
      UASSERT(0,EARG);
   }
   if(ESCREG_ALEVENT_DC_LATCH & mask)
   {
      mask &= ~ESCREG_ALEVENT_DC_LATCH;
      UASSERT(0,EARG);
   }

   ecat0->AL_EVENT_MASK &= ~mask;
}

/** ESC emulated EEPROm handler
 */
void ESC_eep_handler(void)
{
   EEP_process ();
   EEP_hw_process();
}

/* Test variables to measure and print DC/SM info for demonstration purpose */
uint32_t sync0_stat_cnt;
uint32_t sm_stat_cnt;
uint32_t local_time_sm;
uint32_t local_time_sm_old;
uint32_t local_time_dc;
uint32_t local_time_dc_old;
uint32_t local_time_sm_diff;
uint32_t local_time_dc_diff;
uint32_t local_time_sm_dc_diff;
void sync0_isr (void * arg)
{
   sync0_stat_cnt++;
   local_time_dc =  ecat0->READMode_DC_SYS_TIME[0];
   local_time_dc_diff = local_time_dc - local_time_dc_old;
   local_time_dc_old = local_time_dc;

   if(sync_counter == 1 || sync_counter == 0)
   {
      local_time_sm_dc_diff = local_time_dc - local_time_sm;
   }
   else
   {
      local_time_sm_dc_diff = 666;
   }
   DIG_process(DIG_PROCESS_APP_HOOK_FLAG | DIG_PROCESS_INPUTS_FLAG);
   read_ack = ecat0->DC_SYNC0_STAT;
}

void ecat_isr (void * arg)
{

   ESC_read (ESCREG_LOCALTIME, (void *) &ESCvar.Time, sizeof (ESCvar.Time));
   ESCvar.Time = etohl (ESCvar.Time);
   CC_ATOMIC_SET(ESCvar.ALevent, etohl(ecat0->AL_EVENT_REQ));

   if(ESCvar.ALevent & ESCREG_ALEVENT_SM2)
   {
      sm_stat_cnt++;
      local_time_sm =  ecat0->READMode_DC_SYS_TIME[0];
      local_time_sm_diff = local_time_sm - local_time_sm_old;
      local_time_sm_old = local_time_sm;
      if(dc_sync == 0)
      {
         DIG_process(DIG_PROCESS_OUTPUTS_FLAG | DIG_PROCESS_APP_HOOK_FLAG |
               DIG_PROCESS_INPUTS_FLAG);
      }
      else
      {
         DIG_process(DIG_PROCESS_OUTPUTS_FLAG);
      }
   }

   if(ESCvar.ALevent & (ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
         | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1 | ESCREG_ALEVENT_EEP))
   {
      /* Mask interrupts while servicing them */
      ecat0->AL_EVENT_MASK &= ~(ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
            | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1 | ESCREG_ALEVENT_EEP);
      sem_signal(ecat_isr_sem);
   }

}

void ESC_init (const esc_cfg_t * config)
{
   eep_config_t ecat_config;

   ESC_reset();

   scu_configure_ethercat_signals(&port_control);

   /* read config from emulated EEPROM */
   memset(&ecat_config, 0, sizeof(eep_config_t));
   EEP_read (0, (uint8_t *) &ecat_config, sizeof(eep_config_t));

   ESC_enable();

   /* words 0x0-0x3 */
   ecat0->EEP_DATA[0U] = ecat_config.dword[0U];
   ecat0->EEP_DATA[1U] = ecat_config.dword[1U];
   ecat0->EEP_CONT_STAT |= (uint16_t)(BIT(10)); /* ESI EEPROM Reload */

   /* words 0x4-0x7 */
   ecat0->EEP_DATA[0U] = ecat_config.dword[2U];
   ecat0->EEP_DATA[1U] = ecat_config.dword[3U];
   ecat0->EEP_CONT_STAT |= (uint16_t)(BIT(10)); /* ESI EEPROM Reload */

   while (ecat0->EEP_CONT_STAT & BIT(12)) /* ESI EEPROM loading status */
   {
     /* Wait until the EEPROM_Loaded signal is active */
   }

   if(config->use_interrupt != 0)
   {
      use_all_interrupts = 1;
      ecat0->AL_EVENT_MASK = 0;
      ecat0->AL_EVENT_MASK = (ESCREG_ALEVENT_SMCHANGE |
                              ESCREG_ALEVENT_EEP |
                              ESCREG_ALEVENT_CONTROL |
                              ESCREG_ALEVENT_SM0 |
                              ESCREG_ALEVENT_SM1);

      int_connect (IRQ_ECAT0_SR0, ecat_isr, NULL);
      int_enable (IRQ_ECAT0_SR0);

      /* Activate for running external sync IRQ */
      scu_put_peripheral_in_reset (SCU_PERIPHERAL_ERU1);
      scu_ungate_clock_to_peripheral (SCU_PERIPHERAL_ERU1);
      scu_release_peripheral_from_reset (SCU_PERIPHERAL_ERU1);

      eru_configure(&cfg);
      /* Let the stack decide when to enable */
      int_disable(cfg.irq);
   }
}


