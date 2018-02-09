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
#include <string.h>
#include "cc.h"
#include "esc_hw.h"
#include "esc_eep.h"
#include "k2gice.h"
#include "tiescbsp.h"
#include "tieschw.h"
#include "config.h"

extern PRUICSS_Handle pruIcss1Handle;
extern uint32_t pd_read_addr_err, pd_write_addr_err;
extern uint32_t pdi_read_fail_cnt, pdi_write_fail_cnt;

static PRUICSS_Handle escHwPruIcssHandle;

int bRunApplication;

static void Esc_readmbx(uint8_t *pData, uint16_t Address, uint16_t Len)
{
    bsp_pdi_mbx_read_start(escHwPruIcssHandle);
    bsp_read(escHwPruIcssHandle, pData, Address, Len);

    if(Len >= MBX0_sml - 2)
    {
        bsp_pdi_mbx_read_complete(escHwPruIcssHandle);
    }
}

static void ESC_readRXPDO(uint8_t *pData, uint16_t Address, uint16_t Len)
{
    int16_t sm_index;
    uint16_t ActAddress = bsp_get_process_data_address(escHwPruIcssHandle, Address, Len,
                        &sm_index);

    if(ActAddress < ESC_ADDR_MEMORY)
    {
        pd_read_addr_err++;
        return;
    }

    bsp_read(escHwPruIcssHandle, pData, ActAddress, Len);
    bsp_process_data_access_complete(escHwPruIcssHandle, Address, Len, sm_index);
}

static void ESC_writembox(uint8_t *pData, uint16_t Address, uint16_t Len)
{

    //Do not write to mailbox if already full
    if((bsp_read_byte(escHwPruIcssHandle,
                      ESC_ADDR_SM1_STATUS) & SM_STATUS_MBX_FULL))
    {
        return;
    }

    bsp_pdi_mbx_write_start(escHwPruIcssHandle);
    bsp_write(escHwPruIcssHandle, pData, Address, Len);

    if(Len >= MBX1_sml - 2)
    {
        bsp_pdi_mbx_write_complete(escHwPruIcssHandle);
    }
}

static void ESC_writeTXPDO(uint8_t *pData, uint16_t Address, uint16_t Len)
{
    int16_t sm_index;
    uint16_t ActualAddr = bsp_get_process_data_address(escHwPruIcssHandle, Address, Len,
                        &sm_index);

    if(ActualAddr < ESC_ADDR_MEMORY)
    {
        pd_write_addr_err++;
        return;
    }

    bsp_write(escHwPruIcssHandle, pData, ActualAddr, Len);
    bsp_process_data_access_complete(escHwPruIcssHandle, Address, Len, sm_index);
}


/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
    uint16_t alevent;

    switch(address)
    {
        case MBX0_sma:
        /* TODO *//*case MBX0_sma_b:*/
        {
            Esc_readmbx(buf, address, len);
            break;
        }
        case SM2_sma:
        {
            ESC_readRXPDO(buf, address, len);
            break;
        }
        case ESCREG_SM0:
        case ESCREG_SM1:
        case ESCREG_SM2:
        case ESCREG_SM3:
        {
            bsp_read(escHwPruIcssHandle, buf, address, len);
            /* Handle special case when SOES batch read SM settings,
             * indicate that SM ACTIVATE is read
             */
            if(len > 5)
            {
                uint8_t n = (address - ESC_ADDR_SYNCMAN) >> 3;
                bsp_pdi_post_read_indication(escHwPruIcssHandle, ESCREG_SM0ACTIVATE + (n << 3));
            }
            break;
        }
        default:
        {
            switch(len)
            {
                case 1:
                {
                    uint8_t *p = buf;
                    *p = bsp_read_byte(escHwPruIcssHandle, address);
                    break;
                }
                case 2:
                {
                    uint16_t *p = buf;
                    *p = bsp_read_word(escHwPruIcssHandle, address);
                    break;
                }
                case 4:
                {
                    uint32_t *p = buf;
                    *p = bsp_read_dword(escHwPruIcssHandle, address);
                    break;
                }
                default:
                {
                    bsp_read(escHwPruIcssHandle, buf, address, len);
                    bsp_pdi_post_read_indication(escHwPruIcssHandle, address);
                    break;
                }
            }
            break;
        }
    }

    alevent = bsp_read_word_isr(escHwPruIcssHandle, ESCREG_ALEVENT);
    CC_ATOMIC_SET(ESCvar.ALevent, (etohs(alevent)));
}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write (uint16_t address, void *buf, uint16_t len)
{
    uint16_t alevent;

    switch(address)
    {
        case MBX1_sma:
        /* TODO *//*case MBX1_sma_b:*/
        {
            if(len != 1)
            {
                ESC_writembox(buf, address, len);
            }
            else
            {
                bsp_pdi_mbx_write_start(escHwPruIcssHandle);
            }
            break;
        }
        case (MBX1_sma + MBX1_sml - 1):
        {
            /* Handle SM end byte is written */
            uint8_t * p = buf;
            bsp_write_byte(escHwPruIcssHandle, *p, address);
            bsp_pdi_mbx_write_complete(escHwPruIcssHandle);
            break;
        }
        case SM3_sma:
        {
            ESC_writeTXPDO(buf, address, len);
            break;
        }
        default:
        {
            switch(len)
            {
                case 1:
                {
                    uint8_t * p = buf;
                    bsp_write_byte(escHwPruIcssHandle, *p, address);
                    bsp_pdi_write_indication(escHwPruIcssHandle, address, *p);
                    break;
                }
                case 2:
                {
                    uint16_t * p = buf;
                    bsp_write_word(escHwPruIcssHandle, *p, address);
                    bsp_pdi_write_indication(escHwPruIcssHandle, address, *p);
                    break;
                }
                case 4:
                {
                    uint32_t * p = buf;
                    bsp_write_dword(escHwPruIcssHandle, *p, address);
                    bsp_pdi_write_indication(escHwPruIcssHandle, address, *p);
                    break;
                }
                default:
                {
                    bsp_write(escHwPruIcssHandle, buf, address, len);
                    bsp_pdi_write_indication(escHwPruIcssHandle, address, 0);
                    break;
                }
            }
            break;
        }

    }
    alevent = bsp_read_word_isr(escHwPruIcssHandle, ESCREG_ALEVENT);
    CC_ATOMIC_SET(ESCvar.ALevent, (etohs(alevent)));
}

/** ESC interrupt enable function by the Slave stack in IRQ mode.
 *
 * @param[in]   mask     = of interrupts to enable
 */
void ESC_interrupt_enable (uint32_t mask)
{
    uint32_t readmask;
    readmask = bsp_read_dword_isr(escHwPruIcssHandle,ESCREG_ALEVENTMASK);
    bsp_write_dword(escHwPruIcssHandle, (mask | readmask), ESCREG_ALEVENTMASK);
}

/** ESC interrupt disable function by the Slave stack in IRQ mode.
 *
 * @param[in]   mask     = interrupts to disable
 */
void ESC_interrupt_disable (uint32_t mask)
{
    uint32_t readmask;
    readmask = bsp_read_dword_isr(escHwPruIcssHandle,ESCREG_ALEVENTMASK);
    bsp_write_dword(escHwPruIcssHandle, (~mask & readmask), ESCREG_ALEVENTMASK);
}

/** ESC emulated EEPROM handler
 */
void ESC_eep_handler(void)
{
    EEP_process ();
    EEP_hw_process();
}

/** SYNC0 ISR handler
 *
 * @param[in]   arg     = NOT USED
 */
void Sync0_Isr (void)
{
}

/** SYNC1 ISR handler
 *
 * @param[in]   arg     = NOT USED
 */
void Sync1_Isr (void)
{
}

/** PDI ISR handler
 *
 * @param[in]   arg     = NOT USED
 */
void PDI_Isr(void)
{
    uint16_t alevent;

    alevent = bsp_read_word_isr(escHwPruIcssHandle,ESCREG_ALEVENT);
    CC_ATOMIC_SET(ESCvar.ALevent, etohs(alevent));

    if(ESCvar.ALevent & ESCREG_ALEVENT_SM2)
    {
        DIG_process(DIG_PROCESS_OUTPUTS_FLAG | DIG_PROCESS_APP_HOOK_FLAG | DIG_PROCESS_INPUTS_FLAG);
    }
}


/** ESC and CPU related HW init
 *
 * @param[in]   arg     = esc_cfg provided by the application
 */
void ESC_init (const esc_cfg_t * config)
{
    escHwPruIcssHandle = pruIcss1Handle;
    bsp_set_sm_properties(escHwPruIcssHandle, 0, MBX0_sma, MBX0_sml);
    bsp_set_sm_properties(escHwPruIcssHandle, 1, MBX1_sma, MBX1_sml);
    bsp_set_sm_properties(escHwPruIcssHandle, 2, SM2_sma, ESCvar.RXPDOsize);
    bsp_set_sm_properties(escHwPruIcssHandle, 3, SM3_sma, ESCvar.TXPDOsize);

    bsp_write_dword(escHwPruIcssHandle, 0 , ESCREG_ALEVENTMASK);
}



