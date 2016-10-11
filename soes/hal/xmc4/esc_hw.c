/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : esc_hw.c
 * Version : 0.9.2
 * Date    : 22-02-2010
 * Copyright (C) 2007-2013 Arthur Ketels
 * Copyright (C) 2012-2013 rt-labs
 *
 * SOES is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * SOES is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * The EtherCAT Technology, the trade name and logo "EtherCAT" are the intellectual
 * property of, and protected by Beckhoff Automation GmbH.
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

/* ESC to PHY interconnect setup */

#define ECAT_MDO		P0_12
#define ECAT_MCLK		P3_3
#define ECAT_CLK25		P1_13
#define ECAT_PHY_RESET		P2_10
#define ECAT_LED_RUN		P1_11
#define ECAT_LED_ERR		P1_10

#define ECAT_P0_LINK_STATUS	P1_15
#define ECAT_P0_LED_LINK_ACT	P1_12
#define ECAT_P0_RXD3		P5_7
#define ECAT_P0_RXD2		P5_2
#define ECAT_P0_RXD1		P5_1
#define ECAT_P0_RXD0		P1_4
#define ECAT_P0_RX_DV		P1_9
#define ECAT_P0_RX_CLK		P1_1
#define ECAT_P0_RX_ERR		P2_6
#define ECAT_P0_TXD3		P1_2
#define ECAT_P0_TXD2		P1_8
#define ECAT_P0_TXD1		P1_7
#define ECAT_P0_TXD0		P1_6
#define ECAT_P0_TX_EN		P1_3
#define ECAT_P0_TX_CLK		P1_0

#define ECAT_P1_LINK_STATUS	P15_3
#define ECAT_P1_LED_LINK_ACT	P0_11
#define ECAT_P1_RXD3		P14_14
#define ECAT_P1_RXD2		P14_13
#define ECAT_P1_RXD1		P14_12
#define ECAT_P1_RXD0		P14_7
#define ECAT_P1_RX_DV		P14_15
#define ECAT_P1_RX_CLK		P14_6
#define ECAT_P1_RX_ERR		P15_2
#define ECAT_P1_TXD3		P0_3
#define ECAT_P1_TXD2		P0_2
#define ECAT_P1_TXD1		P3_2
#define ECAT_P1_TXD0		P3_1
#define ECAT_P1_TX_EN		P3_0
#define ECAT_P1_TX_CLK		P0_10

#define ESCADDR(x) (((uint8_t *) ECAT0_BASE) + x)

static const XMC_ECAT_PORT_CTRL_t port_control = {
   .common = {
      .mdio = XMC_ECAT_PORT_CTRL_MDIO_P0_12
   },
   .port0 = {
      .rxd0 = XMC_ECAT_PORT0_CTRL_RXD0_P1_4,
      .rxd1 = XMC_ECAT_PORT0_CTRL_RXD1_P5_1,
      .rxd2 = XMC_ECAT_PORT0_CTRL_RXD2_P5_2,
      .rxd3 = XMC_ECAT_PORT0_CTRL_RXD3_P5_7,
      .rx_clk = XMC_ECAT_PORT0_CTRL_RX_CLK_P1_1,
      .rx_dv = XMC_ECAT_PORT0_CTRL_RX_DV_P1_9,
      .rx_err = XMC_ECAT_PORT0_CTRL_RX_ERR_P2_6,
      .link = XMC_ECAT_PORT0_CTRL_LINK_P1_15,
      .tx_clk = XMC_ECAT_PORT0_CTRL_TX_CLK_P1_0
   },
   .port1 = {
      .rxd0 = XMC_ECAT_PORT1_CTRL_RXD0_P14_7,
      .rxd1 = XMC_ECAT_PORT1_CTRL_RXD1_P14_12,
      .rxd2 = XMC_ECAT_PORT1_CTRL_RXD2_P14_13,
      .rxd3 = XMC_ECAT_PORT1_CTRL_RXD3_P14_14,
      .rx_clk = XMC_ECAT_PORT1_CTRL_RX_CLK_P14_6,
      .rx_dv = XMC_ECAT_PORT1_CTRL_RX_DV_P14_15,
      .rx_err = XMC_ECAT_PORT1_CTRL_RX_ERR_P15_2,
      .link = XMC_ECAT_PORT1_CTRL_LINK_P15_3,
      .tx_clk = XMC_ECAT_PORT1_CTRL_TX_CLK_P0_10
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
  init_output_sharp(ECAT_P0_TXD3, P1_2_AF_ECAT0_P0_TXD3);
  init_output_sharp(ECAT_P0_TXD2, P1_8_AF_ECAT0_P0_TXD2);
  init_output_sharp(ECAT_P0_TXD1, P1_7_AF_ECAT0_P0_TXD1);
  init_output_sharp(ECAT_P0_TXD0, P1_6_AF_ECAT0_P0_TXD0);
  init_output_sharp(ECAT_P0_TX_EN, P1_3_AF_ECAT0_P0_TX_ENA);
  init_output_sharp(ECAT_P1_TXD3, P0_3_AF_ECAT0_P1_TXD3);
  init_output_sharp(ECAT_P1_TXD2, P0_2_AF_ECAT0_P1_TXD2);
  init_output_sharp(ECAT_P1_TXD1, P3_2_AF_ECAT0_P1_TXD1);
  init_output_sharp(ECAT_P1_TXD0, P3_1_AF_ECAT0_P1_TXD0);
  init_output_sharp(ECAT_P1_TX_EN, P3_0_AF_ECAT0_P1_TX_ENA);
  init_output_sharp(ECAT_CLK25, P1_13_AF_ECAT0_PHY_CLK25);
  init_output_sharp(ECAT_MCLK, P3_3_AF_ECAT0_MCLK);

  init_output_soft(ECAT_P0_LED_LINK_ACT, P1_12_AF_ECAT0_P0_LED_LINK_ACT);
  init_output_soft(ECAT_P1_LED_LINK_ACT, P0_11_AF_ECAT0_P1_LED_LINK_ACT);
  init_output_soft(ECAT_LED_RUN, P1_11_AF_ECAT0_LED_RUN);
  init_output_soft(ECAT_LED_ERR, P1_10_AF_ECAT0_LED_ERR);

  XMC_GPIO_SetHardwareControl(ECAT_MDO, P0_12_HWCTRL_ECAT0_MDO);

  init_output_soft(ECAT_PHY_RESET, P2_10_AF_ECAT0_PHY_RESET);
}

