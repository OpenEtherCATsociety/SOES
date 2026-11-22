/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <tiescsoc.h>
#include <drivers/hw_include/hw_types.h>
#include <drivers/hw_include/cslr_spinlock.h>
#include <drivers/mdio.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/AddrTranslateP.h>
#include "ti_board_open_close.h"
#include "ti_drivers_config.h"
#include <industrial_comms/ethercat_slave/beckhoff_stack/stack_hal/tieschw.h>
#include "tiesc_eeprom.h" /* header equivalent of ESI bin file */
#include <drivers/hw_include/cslr_soc.h>
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  Value to be set in ICSSM_PRU0_GPIO_OUT_CTRL and ICSSM_PRU1_GPIO_OUT_CTRL registers
 *  when using pins in MII mode.
 *  This configures PRG0_PRUx_GPIO0, PRG0_PRUx_GPIO1, PRG0_PRUx_GPIO2, PRG0_PRUx_GPIO3,
 *  PRG0_PRUx_GPIO4, PRG0_PRUx_GPIO5, PRG0_PRUx_GPIO6, PRG0_PRUx_GPIO8, PRG0_PRUx_GPIO9,
 *  PRG0_PRUx_GPIO10 and PRG0_PRUx_GPIO16 as input pins.
 **/
#define MSS_CTRL_ICSSM_PRU_GPIO_OUT_CTRL_VALUE   (0x0001077F)
#define MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL_VALUE  (0x0001037F)

/* EEPROM data offset in I2C EEPROM Flash */
#define I2C_EEPROM_DATA_OFFSET                  (0x8000)

#define TIESC_LINK0_POL                         TIESC_LINK_POL_ACTIVE_HIGH
#define TIESC_LINK1_POL                         TIESC_LINK_POL_ACTIVE_HIGH

/*SPI Flash offset at which application binary downloaded over FOE will be stored*/
#define FOE_APPL_BIN_OFFSET                     (0x80000)

#define DP83869_RX_ERR_CNT_REG_ADDRESS          (0x15)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern PRUICSS_Handle pruIcss1Handle;

extern const unsigned char *tiesc_eeprom;  /* EEPROM array corresponding to XML */

uint8_t* pEEPROM[2048];                    /* EEPROM array used by the stack */
/* ========================================================================== */
/*                       Function Declarations                                */
/* ========================================================================== */

int32_t tiesc_getArmInterruptOffset();

int32_t tiesc_eepromRead(uint8_t *buf, uint32_t len);

int32_t tiesc_eepromWrite(uint8_t *buf,uint32_t len);

void tiesc_icssmMuxSelection();

/**
*  @brief    This API configures port0 and port1 phys for enhanced link detection, enable RXERR detection during IDLE,
*   disable detection of transmit error in odd-nibble boundary (for odd nibble insertion) and fast RXDV detection.
*   This is called only during initialization.
*
*   @param pruIcssHandle PRUSS Handle
*   @param phy0addr Phy address of port0
*   @param phy1addr Phy address of port1
*   @param enhancedlink_enable Fast link detect (enhanced link detection) enable/disable flag
*/
void tiesc_ethphyInit(PRUICSS_Handle pruIcssHandle, uint8_t phy0addr,
                      uint8_t phy1addr, uint8_t enhancedlink_enable);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint8_t tiesc_isEthercatDevice(void)
{
    uint8_t data;

    PRUICSS_readEfuse(pruIcss1Handle, &data);

    data = (data && 0x01);
    if(0x00 == data)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void tiesc_bspSoftReset()
{
    /* FIXME */
    /* Sciclient_pmDeviceReset(0xFFFFFFFFU); */
    return;
}

void tiesc_socEvmInit()
{
    /* Set bits for input pins in ICSSM_PRU0_GPIO_OUT_CTRL and ICSSM_PRU1_GPIO_OUT_CTRL registers */
    HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM_PRU0_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU_GPIO_OUT_CTRL_VALUE);
    HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL_VALUE);

    pruIcss1Handle = PRUICSS_open(CONFIG_PRU_ICSS0);

    /* Selecting MII-RT mode in GPCFG mux */
    PRUICSS_setGpMuxSelect(pruIcss1Handle, PRUICSS_PRU0, PRUICSS_GP_MUX_SEL_MODE_MII);
    PRUICSS_setGpMuxSelect(pruIcss1Handle, PRUICSS_PRU1, PRUICSS_GP_MUX_SEL_MODE_MII);

    PRUICSS_setGpiMode(pruIcss1Handle, PRUICSS_PRU0, PRUICSS_GPI_MODE_MII_RT);
    PRUICSS_setGpiMode(pruIcss1Handle, PRUICSS_PRU1, PRUICSS_GPI_MODE_MII_RT);

    PRUICSS_setIcssCfgMiiMode(pruIcss1Handle, 0, PRUICSS_ICSS_CFG_MII_MODE_MII);
    PRUICSS_setIcssCfgMiiMode(pruIcss1Handle, 1, PRUICSS_ICSS_CFG_MII_MODE_MII);

    PRUICSS_setIcssCfgTxFifo(pruIcss1Handle, PRUICSS_TX_L1_FIFO, 1);
    PRUICSS_setIcssCfgTxFifo(pruIcss1Handle, PRUICSS_TX_L2_FIFO, 0);

    /*Making the clock for ICSS core and IEP same */
    PRUICSS_setIepClkSrc(pruIcss1Handle, 1U);

    /*Disable PRUs. This is to ensure PRUs are not running when application is not initialized */
    PRUICSS_disableCore(pruIcss1Handle, PRUICSS_PRU0);
    PRUICSS_disableCore(pruIcss1Handle, PRUICSS_PRU1);
}

void tiesc_socParamsInit(bsp_params *bspInitParams)
{
    bsp_params_init(bspInitParams);
    bspInitParams->pruicss_handle = pruIcss1Handle;
    bspInitParams->interrupt_offset = tiesc_getArmInterruptOffset();
    bspInitParams->eeprom_read = tiesc_eepromRead;
    bspInitParams->eeprom_write = tiesc_eepromWrite;
    bspInitParams->spinlock_base_address = CSL_SPINLOCK0_BASE;
    bspInitParams->ethphy_init = tiesc_ethphyInit;
    /* For EtherCAT, Fast link detection using MLINK mode is required to support complete functionality */
    /* Check example documentation for more details */
    bspInitParams->enhancedlink_enable = TIESC_MDIO_RX_LINK_ENABLE;
    bspInitParams->link0_polarity = TIESC_LINK0_POL;
    bspInitParams->link1_polarity = TIESC_LINK1_POL;
    bspInitParams->phy0_address = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY0))->phyAddress;
    bspInitParams->phy1_address = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY1))->phyAddress;
    bspInitParams->default_tiesc_eeprom = tiesc_eeprom;
    bspInitParams->eeprom_pointer_for_stack = pEEPROM;
#ifndef ENABLE_PDI_TASK
    bspInitParams->pdi_isr = PDI_Isr;
#endif
#ifndef ENABLE_SYNC_TASK
    bspInitParams->sync0_isr = Sync0_Isr;
    bspInitParams->sync1_isr = Sync1_Isr;
#endif
}

void tiesc_displayEscVersion(uint16_t revision, uint16_t build)
{
    DebugP_log("Revision/Type : x%04X Build : x%04X\n\r", revision, build);
    DebugP_log("Firmware Version : %d.%d.%d\n\r", (revision >> 8), (build >> 8), (build & 0xFF));
}

void tiesc_setOutputLed(uint8_t mask)
{
	LED_setMask(gLedHandle[CONFIG_LED_DIGITAL_OUTPUT], (uint32_t)(0xFFFF & mask));
}

void tiesc_setRunLed(uint8_t value)
{
     if(value == 0U)
         LED_off(gLedHandle[CONFIG_LED_RUN], 0);
     if(value == 1U)
         LED_on(gLedHandle[CONFIG_LED_RUN], 0);
}

void tiesc_setErrorLed(uint8_t value)
{
    /* FIXME : To be added */
    /* if(value == 0U) */
    /*     LED_off(gLedHandle[CONFIG_LED_ERROR], 0); */
    /* if(value == 1U) */
    /*     LED_on(gLedHandle[CONFIG_LED_ERROR], 0); */
}

void tiesc_getFoeFlashOffset(uint32_t *offset)
{
    *offset = (uint32_t)FOE_APPL_BIN_OFFSET;
}

int32_t tiesc_getArmInterruptOffset()
{
    int32_t interruptOffset = CSLR_R5FSS0_CORE0_INTR_PRU_ICSSM0_PR1_HOST_INTR_PEND_0;
    return interruptOffset;
}

int32_t tiesc_eepromRead(uint8_t *buf, uint32_t len)
{
    int32_t retVal;
    retVal = EEPROM_read(gEepromHandle[CONFIG_EEPROM0], I2C_EEPROM_DATA_OFFSET, buf, len);
    return retVal;
}

int32_t tiesc_eepromWrite(uint8_t *buf,uint32_t len)
{
    int32_t retVal;
    retVal = EEPROM_write(gEepromHandle[CONFIG_EEPROM0], I2C_EEPROM_DATA_OFFSET, buf, len);
    return retVal;
}

void tiesc_ethphyInit(PRUICSS_Handle pruIcssHandle, uint8_t phy0addr,
                     uint8_t phy1addr, uint8_t enhancedlink_enable)
{
    uint32_t mdioBaseAddress = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY0))->mdioBaseAddress;

    ETHPHY_DP83869_LedSourceConfig ledConfig;
    ETHPHY_DP83869_LedBlinkRateConfig ledBlinkConfig;
    ETHPHY_DP83869_FastLinkDownDetectionConfig fastLinkDownDetConfig;

    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_AUTO_MDIX, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_AUTO_MDIX, NULL, 0);

    if(TIESC_MDIO_RX_LINK_ENABLE == enhancedlink_enable)
    {
        /*TODO: Review these 2 calls*/
        ledConfig.ledNum = ETHPHY_DP83869_LED0;
        ledConfig.mode = ETHPHY_DP83869_LED_MODE_LINK_OK;

        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    }

    /* Enable Extended Full-Duplex */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_EXTENDED_FD_ABILITY, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_EXTENDED_FD_ABILITY, NULL, 0);

    /* Enable Odd Nibble Detection */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_ODD_NIBBLE_DETECTION, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_ODD_NIBBLE_DETECTION, NULL, 0);

    /* Enable detection of RXERR during IDLE */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_ENHANCED_IPG_DETECTION, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_ENHANCED_IPG_DETECTION, NULL, 0);

    /* PHY pin LED_0 as link for fast link detection */
    ledConfig.ledNum = ETHPHY_DP83869_LED0;
    ledConfig.mode = ETHPHY_DP83869_LED_MODE_100BTX_LINK_UP;
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

    /* PHY pin LED_1 as RX_ER. Needed for detecting RX_ER during frame. */
    ledConfig.ledNum = ETHPHY_DP83869_LED1;
    ledConfig.mode = ETHPHY_DP83869_LED_MODE_RX_ERROR;
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

    /* PHY pin LED_2 as Rx/Tx Activity */
    ledConfig.ledNum = ETHPHY_DP83869_LED2;
    ledConfig.mode = ETHPHY_DP83869_LED_MODE_LINK_OK_AND_BLINK_ON_RX_TX;
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

    ledBlinkConfig.rate = ETHPHY_DP83869_LED_BLINK_RATE_200_MS;
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));

    /* Enable fast link drop detection for EtherCAT
     * Bit3: Drop the link based on RX Error count of the MII interface, when a predefined number
     * of 32 RX Error occurrences in a 10us interval is reached, the link will be dropped
     * Bit0(not enabled by following lines): Drop the link based on Signal/Energy loss indication,
     * when the Energy detector indicates Energy Loss, the link will be dropped. Typical reaction
     * time is 10us. If it needs to be enabled, set fastLinkDownDetConfig.mode as
     * (ETHPHY_DP83869_FAST_LINKDOWN_MODE_ENERGY_LOST | ETHPHY_DP83869_FAST_LINKDOWN_MODE_RX_ERR)
     */
    fastLinkDownDetConfig.mode = ETHPHY_DP83869_FAST_LINKDOWN_MODE_RX_ERR;

    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));

    if(enhancedlink_enable == 0)
    {
        MDIO_enableLinkInterrupt(mdioBaseAddress, 0, phy0addr, MDIO_LINKSEL_MDIO_MODE);
        MDIO_enableLinkInterrupt(mdioBaseAddress, 1, phy1addr, MDIO_LINKSEL_MDIO_MODE);
    }
    else
    {
        MDIO_enableLinkInterrupt(mdioBaseAddress, 0, phy0addr, MDIO_LINKSEL_MLINK_MODE);
        MDIO_enableLinkInterrupt(mdioBaseAddress, 1, phy1addr, MDIO_LINKSEL_MLINK_MODE);
    }

    /* Enable MII mode for DP83869 PHY */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_MII, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_MII, NULL, 0);

    /* Disable 1G advertisement and sof-reset to restart auto-negotiation in case 1G link was establised */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_DISABLE_1000M_ADVERTISEMENT, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_DISABLE_1000M_ADVERTISEMENT, NULL, 0);

    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_SOFT_RESTART, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_SOFT_RESTART, NULL, 0);
}

void tiesc_ethphyEnablePowerDown()
{
    /*TODO: Need to enable the mux selection from the generated files*/
    tiesc_icssmMuxSelection();
    /* Ensure that PHY register access is working by checking the Identifier register */
    while(SystemP_SUCCESS != ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_VERIFY_IDENTIFIER_REGISTER, NULL, 0));
    while(SystemP_SUCCESS != ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_VERIFY_IDENTIFIER_REGISTER, NULL, 0));

    /* Enable IEEE Power Down mode so that PHY does not establish any link */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_IEEE_POWER_DOWN, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_IEEE_POWER_DOWN, NULL, 0);
}

void tiesc_ethphyDisablePowerDown()
{
    /* Disable IEEE Power Down mode so that PHY does not establish any link */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_DISABLE_IEEE_POWER_DOWN, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_DISABLE_IEEE_POWER_DOWN, NULL, 0);
}

void tiesc_icssmMuxSelection(void)
{
    uint32_t pinNum[CONFIG_GPIO_NUM_INSTANCES-1] = {CONFIG_GPIO0_PIN, CONFIG_GPIO1_PIN, CONFIG_GPIO2_PIN};
    uint32_t pinDir[CONFIG_GPIO_NUM_INSTANCES-1] = {CONFIG_GPIO0_DIR, CONFIG_GPIO1_DIR, CONFIG_GPIO2_DIR};

    for(uint32_t index = 0; index < CONFIG_GPIO_NUM_INSTANCES-1; index++)
    {
        /* Address translate */
        uint32_t gGpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_GPIO0_BASE_ADDR);

        /* Setup GPIO for ICSSM MDIO Mux selection */
        GPIO_setDirMode(gGpioBaseAddr, pinNum[index], pinDir[index]);
        GPIO_pinWriteHigh(gGpioBaseAddr, pinNum[index]);
    }
}
