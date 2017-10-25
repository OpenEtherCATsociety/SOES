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

#include "utypes.h"
#include "../../esc.h"
#include <spi/spi.h>
#include <string.h>
#include <gpio.h>

#define ESC_CMD_READ    0x02
#define ESC_CMD_READWS  0x03
#define ESC_CMD_WRITE   0x04
#define ESC_CMD_NOP     0x00
#define ESC_TERM        0xff
#define ESC_NEXT        0x00

static int et1100 = -1;
static uint8_t read_termination[MAX(sizeof(Wb), 128)] = { 0 };

#define GPIO_ECAT_RESET    1 /* specific function to hold ESC reset on startup
                              * when emulating EEPROM
                              */

static void esc_address (uint16_t address, uint8_t command)
{
   /* Device is selected already.
    * We use 2 bytes addressing.
    */
   uint8_t data[2];

   /* address 12:5 */
   data[0] = (address >> 5);
   /* address 4:0 and cmd 2:0 */
   data[1] = ((address & 0x1F) << 3) | command;

   /* Write (and read AL interrupt register) */
   spi_bidirectionally_transfer (et1100, (uint8_t *) &ESCvar.ALevent,
                                 data, sizeof (data));
   ESCvar.ALevent = etohs (ESCvar.ALevent);
}

/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
   ASSERT(len <= sizeof(read_termination));

   /* Select device. */
   spi_select (et1100);

   /* Write address and command to device. */
   esc_address (address, ESC_CMD_READ);

   /* Here we want to read data and keep MOSI low (0x00) during
    * all bytes except the last one where we want to pull it high (0xFF).
    * Read (and write termination bytes).
    */
   spi_bidirectionally_transfer (et1100, buf, read_termination +
                                 (sizeof(read_termination) - len), len);

   /* Un-select device. */
   spi_unselect (et1100);
}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write (uint16_t address, void *buf, uint16_t len)
{
   /* Select device. */
   spi_select (et1100);
   /* Write address and command to device. */
   esc_address (address, ESC_CMD_WRITE);
   /* Write data. */
   write (et1100, buf, len);
   /* Un-select device. */
   spi_unselect (et1100);
}

void ESC_reset (void)
{
   volatile int timeout;

   DPRINT("esc_reset_started\n");

   gpio_set (GPIO_ECAT_RESET,0); /* pin =0 */
   gpio_configure_pin (GPIO_ECAT_RESET,MUX_GPIO,IRQC_DISABLED,MODE_OUTPUT);

   task_delay (1000);

   gpio_configure_pin (GPIO_ECAT_RESET,MUX_GPIO,IRQC_DISABLED,MODE_INPUT);
   while(timeout<10000000)
   {
      /* ECAT releases resetpin */
      if(gpio_get (GPIO_ECAT_RESET)!=0)
      {
         break; // OK
      }
      timeout++;
      task_delay (30);
   }
   DPRINT("esc_reset_ended\n");
}

void ESC_init (const void * arg)
{
   const char * spi_name = (char *)arg;
   et1100 = open (spi_name, O_RDWR, 0);
   read_termination[sizeof(read_termination) - 1] = 0xFF;
}
