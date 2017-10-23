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

#include "esc.h"
#include "esc_eep.h"

#include "xmc_gpio.h"
#include "xmc_ecat.h"


#ifdef XMC4800_F144x2048
/* ESC to PHY interconnect setup */
/* PHY management interface signal definitions*/
#define ECAT_MDO                    P0_12
#define ECAT_MCLK                   P3_3

#define ECAT_PORT_CTRL_LATCHIN0     XMC_ECAT_PORT_CTRL_LATCHIN0_P14_5
#define ECAT_PORT_CTRL_LATCHIN1     XMC_ECAT_PORT_CTRL_LATCHIN1_P14_4
#define ECAT_PHYADDR_OFFSET         0

/* EtherCAT slave physical layer pin configurations */
#define ECAT_CLK25                  P6_0
#define ECAT_PHY_RESET              P0_0
#define ECAT_LED_RUN                P0_8
#define ECAT_LED_ERR                P0_7

/* EtherCAT slave physical layer Port 0 pin configurations */
#define ECAT_P0_LINK_STATUS         P1_15
#define ECAT_P0_LED_LINK_ACT        P6_3
#define ECAT_P0_RXD3                P5_7
#define ECAT_P0_RXD2                P5_2
#define ECAT_P0_RXD1                P5_1
#define ECAT_P0_RXD0                P5_0
#define ECAT_P0_RX_DV               P5_6
#define ECAT_P0_RX_CLK              P5_4
#define ECAT_P0_RX_ERR              P2_6
#define ECAT_P0_TXD3                P6_6
#define ECAT_P0_TXD2                P6_5
#define ECAT_P0_TXD1                P6_4
#define ECAT_P0_TXD0                P6_2
#define ECAT_P0_TX_EN               P6_1
#define ECAT_P0_TX_CLK              P5_5

/* EtherCAT slave physical layer Port 1 pin configurations */
#define ECAT_P1_LINK_STATUS         P3_4
#define ECAT_P1_LED_LINK_ACT        P3_12
#define ECAT_P1_RXD3                P0_4
#define ECAT_P1_RXD2                P0_5
#define ECAT_P1_RXD1                P0_6
#define ECAT_P1_RXD0                P0_11
#define ECAT_P1_RX_DV               P0_9
#define ECAT_P1_RX_CLK              P0_1
#define ECAT_P1_RX_ERR              P15_2
#define ECAT_P1_TXD3                P0_3
#define ECAT_P1_TXD2                P0_2
#define ECAT_P1_TXD1                P3_2
#define ECAT_P1_TXD0                P3_1
#define ECAT_P1_TX_EN               P3_0
#define ECAT_P1_TX_CLK              P0_10

  /* configure outputs */
#define AF_ECAT0_P0_TXD3            P6_6_AF_ECAT0_P0_TXD3
#define AF_ECAT0_P0_TXD2            P6_5_AF_ECAT0_P0_TXD2
#define AF_ECAT0_P0_TXD1            P6_4_AF_ECAT0_P0_TXD1
#define AF_ECAT0_P0_TXD0            P6_2_AF_ECAT0_P0_TXD0
#define AF_ECAT0_P0_TX_EN           P6_1_AF_ECAT0_P0_TX_ENA
#define AF_ECAT0_P1_TXD3            P0_3_AF_ECAT0_P1_TXD3
#define AF_ECAT0_P1_TXD2            P0_2_AF_ECAT0_P1_TXD2
#define AF_ECAT0_P1_TXD1            P3_2_AF_ECAT0_P1_TXD1
#define AF_ECAT0_P1_TXD0            P3_1_AF_ECAT0_P1_TXD0
#define AF_ECAT0_P1_TX_EN           P3_0_AF_ECAT0_P1_TX_ENA
#define AF_ECAT0_CLK25              P6_0_AF_ECAT0_PHY_CLK25
#define AF_ECAT0_MCLK               P3_3_AF_ECAT0_MCLK

#define AF_ECAT0_P0_LED_LINK_ACT    P6_3_AF_ECAT0_P0_LED_LINK_ACT
#define AF_ECAT0_P1_LED_LINK_ACT    P3_12_AF_ECAT0_P1_LED_LINK_ACT
#define AF_ECAT0_LED_RUN            P0_8_AF_ECAT0_LED_RUN
#define AF_ECAT0_LED_ERR            P0_7_AF_ECAT0_LED_ERR

#define AF_ECAT0_MDO                P0_12_HWCTRL_ECAT0_MDO

#define AF_ECAT0_PHY_RESET          P0_0_AF_ECAT0_PHY_RESET

#define ECAT_PORT_CTRL_MDIO         XMC_ECAT_PORT_CTRL_MDIO_P0_12
#define ECAT_PORT0_CTRL_RXD0        XMC_ECAT_PORT0_CTRL_RXD0_P5_0
#define ECAT_PORT0_CTRL_RXD1        XMC_ECAT_PORT0_CTRL_RXD1_P5_1
#define ECAT_PORT0_CTRL_RXD2        XMC_ECAT_PORT0_CTRL_RXD2_P5_2
#define ECAT_PORT0_CTRL_RXD3        XMC_ECAT_PORT0_CTRL_RXD3_P5_7
#define ECAT_PORT0_CTRL_RX_CLK      XMC_ECAT_PORT0_CTRL_RX_CLK_P5_4
#define ECAT_PORT0_CTRL_RX_DV       XMC_ECAT_PORT0_CTRL_RX_DV_P5_6
#define ECAT_PORT0_CTRL_RX_ERR      XMC_ECAT_PORT0_CTRL_RX_ERR_P2_6
#define ECAT_PORT0_CTRL_LINK        XMC_ECAT_PORT0_CTRL_LINK_P1_15
#define ECAT_PORT0_CTRL_TX_CLK      XMC_ECAT_PORT0_CTRL_TX_CLK_P5_5
#define ECAT_PORT0_CTRL_TX_SHIFT    XMC_ECAT_PORT0_CTRL_TX_SHIFT_0NS
#define ECAT_PORT1_CTRL_RXD0        XMC_ECAT_PORT1_CTRL_RXD0_P0_11
#define ECAT_PORT1_CTRL_RXD1        XMC_ECAT_PORT1_CTRL_RXD1_P0_6
#define ECAT_PORT1_CTRL_RXD2        XMC_ECAT_PORT1_CTRL_RXD2_P0_5
#define ECAT_PORT1_CTRL_RXD3        XMC_ECAT_PORT1_CTRL_RXD3_P0_4
#define ECAT_PORT1_CTRL_RX_CLK      XMC_ECAT_PORT1_CTRL_RX_CLK_P0_1
#define ECAT_PORT1_CTRL_RX_DV       XMC_ECAT_PORT1_CTRL_RX_DV_P0_9
#define ECAT_PORT1_CTRL_RX_ERR      XMC_ECAT_PORT1_CTRL_RX_ERR_P15_2
#define ECAT_PORT1_CTRL_LINK        XMC_ECAT_PORT1_CTRL_LINK_P3_4
#define ECAT_PORT1_CTRL_TX_CLK      XMC_ECAT_PORT1_CTRL_TX_CLK_P0_10
#define ECAT_PORT1_CTRL_TX_SHIFT    XMC_ECAT_PORT1_CTRL_TX_SHIFT_0NS

#endif

#ifdef XMC4300_F100x256
#define ECAT_MDO                    P0_12
#define ECAT_MCLK                   P3_3

#define ECAT_PORT_CTRL_LATCHIN0     XMC_ECAT_PORT_CTRL_LATCHIN0_P14_5
#define ECAT_PORT_CTRL_LATCHIN1     XMC_ECAT_PORT_CTRL_LATCHIN1_P14_4
#define ECAT_PHYADDR_OFFSET         0

#define ECAT_CLK25                  P1_13
#define ECAT_PHY_RESET              P2_10
#define ECAT_LED_RUN                P1_11
#define ECAT_LED_ERR                P1_10

#define ECAT_P0_LINK_STATUS         P1_15
#define ECAT_P0_LED_LINK_ACT        P1_12
#define ECAT_P0_RXD3                P5_7
#define ECAT_P0_RXD2                P5_2
#define ECAT_P0_RXD1                P5_1
#define ECAT_P0_RXD0                P1_4
#define ECAT_P0_RX_DV               P1_9
#define ECAT_P0_RX_CLK              P1_1
#define ECAT_P0_RX_ERR              P2_6
#define ECAT_P0_TXD3                P1_2
#define ECAT_P0_TXD2                P1_8
#define ECAT_P0_TXD1                P1_7
#define ECAT_P0_TXD0                P1_6
#define ECAT_P0_TX_EN               P1_3
#define ECAT_P0_TX_CLK              P1_0

#define ECAT_P1_LINK_STATUS         P15_3
#define ECAT_P1_LED_LINK_ACT        P0_11
#define ECAT_P1_RXD3                P14_14
#define ECAT_P1_RXD2                P14_13
#define ECAT_P1_RXD1                P14_12
#define ECAT_P1_RXD0                P14_7
#define ECAT_P1_RX_DV               P14_15
#define ECAT_P1_RX_CLK              P14_6
#define ECAT_P1_RX_ERR              P15_2
#define ECAT_P1_TXD3                P0_3
#define ECAT_P1_TXD2                P0_2
#define ECAT_P1_TXD1                P3_2
#define ECAT_P1_TXD0                P3_1
#define ECAT_P1_TX_EN               P3_0
#define ECAT_P1_TX_CLK              P0_10

  /* configure outputs */
#define AF_ECAT0_P0_TXD3            P1_2_AF_ECAT0_P0_TXD3
#define AF_ECAT0_P0_TXD2            P1_8_AF_ECAT0_P0_TXD2
#define AF_ECAT0_P0_TXD1            P1_7_AF_ECAT0_P0_TXD1
#define AF_ECAT0_P0_TXD0            P1_6_AF_ECAT0_P0_TXD0
#define AF_ECAT0_P0_TX_EN           P1_3_AF_ECAT0_P0_TX_ENA
#define AF_ECAT0_P1_TXD3            P0_3_AF_ECAT0_P1_TXD3
#define AF_ECAT0_P1_TXD2            P0_2_AF_ECAT0_P1_TXD2
#define AF_ECAT0_P1_TXD1            P3_2_AF_ECAT0_P1_TXD1
#define AF_ECAT0_P1_TXD0            P3_1_AF_ECAT0_P1_TXD0
#define AF_ECAT0_P1_TX_EN           P3_0_AF_ECAT0_P1_TX_ENA
#define AF_ECAT0_CLK25              P1_13_AF_ECAT0_PHY_CLK25
#define AF_ECAT0_MCLK               P3_3_AF_ECAT0_MCLK

#define AF_ECAT0_P0_LED_LINK_ACT    P1_12_AF_ECAT0_P0_LED_LINK_ACT
#define AF_ECAT0_P1_LED_LINK_ACT    P0_11_AF_ECAT0_P1_LED_LINK_ACT
#define AF_ECAT0_LED_RUN            P1_11_AF_ECAT0_LED_RUN
#define AF_ECAT0_LED_ERR            P1_10_AF_ECAT0_LED_ERR

#define AF_ECAT0_MDO                P0_12_HWCTRL_ECAT0_MDO

#define AF_ECAT0_PHY_RESET          P2_10_AF_ECAT0_PHY_RESET

#define ECAT_PORT_CTRL_MDIO         XMC_ECAT_PORT_CTRL_MDIO_P0_12
#define ECAT_PORT0_CTRL_RXD0        XMC_ECAT_PORT0_CTRL_RXD0_P1_4
#define ECAT_PORT0_CTRL_RXD1        XMC_ECAT_PORT0_CTRL_RXD1_P5_1
#define ECAT_PORT0_CTRL_RXD2        XMC_ECAT_PORT0_CTRL_RXD2_P5_2
#define ECAT_PORT0_CTRL_RXD3        XMC_ECAT_PORT0_CTRL_RXD3_P5_7
#define ECAT_PORT0_CTRL_RX_CLK      XMC_ECAT_PORT0_CTRL_RX_CLK_P1_1
#define ECAT_PORT0_CTRL_RX_DV       XMC_ECAT_PORT0_CTRL_RX_DV_P1_9
#define ECAT_PORT0_CTRL_RX_ERR      XMC_ECAT_PORT0_CTRL_RX_ERR_P2_6
#define ECAT_PORT0_CTRL_LINK        XMC_ECAT_PORT0_CTRL_LINK_P1_15
#define ECAT_PORT0_CTRL_TX_CLK      XMC_ECAT_PORT0_CTRL_TX_CLK_P1_0
#define ECAT_PORT1_CTRL_RXD0        XMC_ECAT_PORT1_CTRL_RXD0_P14_7
#define ECAT_PORT1_CTRL_RXD1        XMC_ECAT_PORT1_CTRL_RXD1_P14_12
#define ECAT_PORT1_CTRL_RXD2        XMC_ECAT_PORT1_CTRL_RXD2_P14_13
#define ECAT_PORT1_CTRL_RXD3        XMC_ECAT_PORT1_CTRL_RXD3_P14_14
#define ECAT_PORT1_CTRL_RX_CLK      XMC_ECAT_PORT1_CTRL_RX_CLK_P14_6
#define ECAT_PORT1_CTRL_RX_DV       XMC_ECAT_PORT1_CTRL_RX_DV_P14_15
#define ECAT_PORT1_CTRL_RX_ERR      XMC_ECAT_PORT1_CTRL_RX_ERR_P15_2
#define ECAT_PORT1_CTRL_LINK        XMC_ECAT_PORT1_CTRL_LINK_P15_3
#define ECAT_PORT1_CTRL_TX_CLK      XMC_ECAT_PORT1_CTRL_TX_CLK_P0_10

#endif

#define ESCADDR(x) (((uint8_t *) ECAT0_BASE) + x)

static const XMC_ECAT_PORT_CTRL_t port_control = {
   .common = {
      .mdio = ECAT_PORT_CTRL_MDIO
   },
   .port0 = {
      .rxd0 = ECAT_PORT0_CTRL_RXD0,
      .rxd1 = ECAT_PORT0_CTRL_RXD1,
      .rxd2 = ECAT_PORT0_CTRL_RXD2,
      .rxd3 = ECAT_PORT0_CTRL_RXD3,
      .rx_clk = ECAT_PORT0_CTRL_RX_CLK,
      .rx_dv = ECAT_PORT0_CTRL_RX_DV,
      .rx_err = ECAT_PORT0_CTRL_RX_ERR,
      .link = ECAT_PORT0_CTRL_LINK,
      .tx_clk = ECAT_PORT0_CTRL_TX_CLK
   },
   .port1 = {
      .rxd0 = ECAT_PORT1_CTRL_RXD0,
      .rxd1 = ECAT_PORT1_CTRL_RXD1,
      .rxd2 = ECAT_PORT1_CTRL_RXD2,
      .rxd3 = ECAT_PORT1_CTRL_RXD3,
      .rx_clk = ECAT_PORT1_CTRL_RX_CLK,
      .rx_dv = ECAT_PORT1_CTRL_RX_DV,
      .rx_err = ECAT_PORT1_CTRL_RX_ERR,
      .link = ECAT_PORT1_CTRL_LINK,
      .tx_clk = ECAT_PORT1_CTRL_TX_CLK
   }
};

static const XMC_GPIO_CONFIG_t gpio_config_input = {
  .mode = XMC_GPIO_MODE_INPUT_TRISTATE,
  .output_level = 0,
  .output_strength = 0
};


static void init_input(XMC_GPIO_PORT_t *const port, const uint8_t pin)
{
  XMC_GPIO_Init(port, pin, &gpio_config_input);
}

static void init_output(XMC_GPIO_PORT_t *const port, const uint8_t pin,
                                      uint32_t function, XMC_GPIO_OUTPUT_STRENGTH_t strength)
{
  XMC_GPIO_CONFIG_t config;

  config.mode = (XMC_GPIO_MODE_t)((uint32_t)XMC_GPIO_MODE_OUTPUT_PUSH_PULL | function);
  config.output_level = XMC_GPIO_OUTPUT_LEVEL_LOW;
  config.output_strength = strength;

  XMC_GPIO_Init(port, pin, &config);
}

#define init_output_sharp(pin, function) init_output(pin, function, XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE);
#define init_output_soft(pin, function) init_output(pin, function, XMC_GPIO_OUTPUT_STRENGTH_STRONG_SOFT_EDGE);

/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
   ESCvar.ALevent = etohs ((uint16_t)ECAT0->AL_EVENT_REQ);
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
   ESCvar.ALevent = etohs ((uint16_t)ECAT0->AL_EVENT_REQ);
   memcpy(ESCADDR(address), buf, len);
}

void ESC_reset (void)
{
  /* disable ESC to force reset */
  XMC_ECAT_Disable();

  /* initialize EEPROM emulation */
  EEP_init();
}

void ESC_init (const void * arg)
{
  XMC_ECAT_CONFIG_t ecat_config;

  /* configure inputs */
  init_input(ECAT_P0_LINK_STATUS);
  init_input(ECAT_P0_RXD3);
  init_input(ECAT_P0_RXD2);
  init_input(ECAT_P0_RXD1);
  init_input(ECAT_P0_RXD0);
  init_input(ECAT_P0_RX_DV);
  init_input(ECAT_P0_RX_CLK);
  init_input(ECAT_P0_RX_ERR);
  init_input(ECAT_P0_TX_CLK);

  init_input(ECAT_P1_LINK_STATUS);
  init_input(ECAT_P1_RXD3);
  init_input(ECAT_P1_RXD2);
  init_input(ECAT_P1_RXD1);
  init_input(ECAT_P1_RXD0);
  init_input(ECAT_P1_RX_DV);
  init_input(ECAT_P1_RX_CLK);
  init_input(ECAT_P1_RX_ERR);
  init_input(ECAT_P1_TX_CLK);

  init_input(ECAT_MDO);

  XMC_ECAT_SetPortControl(port_control);

  /* read config from emulated EEPROM */
  memset(&ecat_config, 0, sizeof(XMC_ECAT_CONFIG_t));
  EEP_read (0, (uint8_t *) &ecat_config, sizeof(XMC_ECAT_CONFIG_t));
  XMC_ECAT_Init(&ecat_config);

  /* configure outputs */
  init_output_sharp(ECAT_P0_TXD3, AF_ECAT0_P0_TXD3);
  init_output_sharp(ECAT_P0_TXD2, AF_ECAT0_P0_TXD2);
  init_output_sharp(ECAT_P0_TXD1, AF_ECAT0_P0_TXD1);
  init_output_sharp(ECAT_P0_TXD0, AF_ECAT0_P0_TXD0);
  init_output_sharp(ECAT_P0_TX_EN, AF_ECAT0_P0_TX_EN);
  init_output_sharp(ECAT_P1_TXD3, AF_ECAT0_P1_TXD3);
  init_output_sharp(ECAT_P1_TXD2, AF_ECAT0_P1_TXD2);
  init_output_sharp(ECAT_P1_TXD1, AF_ECAT0_P1_TXD1);
  init_output_sharp(ECAT_P1_TXD0, AF_ECAT0_P1_TXD0);
  init_output_sharp(ECAT_P1_TX_EN, AF_ECAT0_P1_TX_EN);
  init_output_sharp(ECAT_CLK25, AF_ECAT0_CLK25);
  init_output_sharp(ECAT_MCLK, AF_ECAT0_MCLK);

  init_output_soft(ECAT_P0_LED_LINK_ACT, AF_ECAT0_P0_LED_LINK_ACT);
  init_output_soft(ECAT_P1_LED_LINK_ACT, AF_ECAT0_P1_LED_LINK_ACT);
  init_output_soft(ECAT_LED_RUN, AF_ECAT0_LED_RUN);
  init_output_soft(ECAT_LED_ERR, AF_ECAT0_LED_ERR);

  init_input(ECAT_MDO);
  XMC_GPIO_SetHardwareControl(ECAT_MDO, AF_ECAT0_MDO);

  init_output_soft(ECAT_PHY_RESET, AF_ECAT0_PHY_RESET);
}

