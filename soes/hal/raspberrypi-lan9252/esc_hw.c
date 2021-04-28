/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware layer functions for LAN9252 through BCM2835 SPI on Raspberry PI.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 */
#include "esc.h"
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <bcm2835.h>

#define BIT(x)	1 << (x)

#define ESC_CMD_SERIAL_WRITE     0x02
#define ESC_CMD_SERIAL_READ      0x03

#define ESC_CMD_RESET_CTL        0x01F8      // reset register
#define ESC_CMD_HW_CFG           0x0074      // hardware configuration register
#define ESC_CMD_BYTE_TEST        0x0064      // byte order test register
#define ESC_CMD_ID_REV           0x0050      // chip ID and revision
#define ESC_CMD_IRQ_CFG          0x0054      // interrupt configuration
#define ESC_CMD_INT_EN           0x005C      // interrupt enable

#define ESC_RESET_DIGITAL        0x00000001
#define ESC_RESET_ETHERCAT       0x00000040
#define ESC_RESET_CTRL_RST       (ESC_RESET_DIGITAL & ESC_RESET_ETHERCAT)
#define ESC_HW_CFG_READY         0x08000000
#define ESC_BYTE_TEST_OK         0x87654321

#define ESC_PRAM_RD_FIFO_REG     0x0000
#define ESC_PRAM_WR_FIFO_REG     0x0020
#define ESC_PRAM_RD_ADDR_LEN_REG 0x0308
#define ESC_PRAM_RD_CMD_REG      0x030C
#define ESC_PRAM_WR_ADDR_LEN_REG 0x0310
#define ESC_PRAM_WR_CMD_REG      0x0314

#define ESC_PRAM_CMD_BUSY        0x80000000
#define ESC_PRAM_CMD_ABORT       0x40000000
#define ESC_PRAM_CMD_AVAIL       0x00000001
#define ESC_PRAM_CMD_CNT(x)      ((x >> 8) & 0x1F)
#define ESC_PRAM_SIZE(x)         ((x) << 16)
#define ESC_PRAM_ADDR(x)         ((x) << 0)

#define ESC_CSR_DATA_REG         0x0300
#define ESC_CSR_CMD_REG          0x0304

#define ESC_CSR_CMD_BUSY         0x80000000
#define ESC_CSR_CMD_READ         (0x80000000 | 0x40000000)
#define ESC_CSR_CMD_WRITE        0x80000000
#define ESC_CSR_CMD_SIZE(x)      (x << 16)


/* bcm2835 spi singel write */
static void bcm2835_spi_write_32 (uint16_t address, uint32_t val)
{
    char data[7];

    data[0] = ESC_CMD_SERIAL_WRITE;
    data[1] = ((address >> 8) & 0xFF);
    data[2] = (address & 0xFF);
    data[3] = (val & 0xFF);
    data[4] = ((val >> 8) & 0xFF);
    data[5] = ((val >> 16) & 0xFF);
    data[6] = ((val >> 24) & 0xFF);

    /* Write data */
    bcm2835_spi_transfern(data, 7);
}

/* bcm2835 spi single read */
static uint32_t bcm2835_spi_read_32 (uint16_t address)
{
   char data[7];

   data[0] = ESC_CMD_SERIAL_READ;
   data[1] = ((address >> 8) & 0xFF);
   data[2] = (address & 0xFF);
   
   /* Read data */
   bcm2835_spi_transfern(data, 7);

   return ((data[6] << 24) |
           (data[5] << 16) |
           (data[4] << 8) |
            data[3]);
}

/* ESC read CSR function */
static void ESC_read_csr (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;

   value = (ESC_CSR_CMD_READ | ESC_CSR_CMD_SIZE(len) | address);
   bcm2835_spi_write_32(ESC_CSR_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_CSR_CMD_BUSY);

   value = bcm2835_spi_read_32(ESC_CSR_DATA_REG);
   memcpy(buf, (uint8_t *)&value, len);
}

/* ESC write CSR function */
static void ESC_write_csr (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;

   memcpy((uint8_t*)&value, buf,len);
   bcm2835_spi_write_32(ESC_CSR_DATA_REG, value);
   value = (ESC_CSR_CMD_WRITE | ESC_CSR_CMD_SIZE(len) | address);
   bcm2835_spi_write_32(ESC_CSR_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_CSR_CMD_BUSY);
}

/* ESC read process data ram function */
static void ESC_read_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint16_t byte_offset = 0;
   uint8_t fifo_cnt, first_byte_position, temp_len;
   uint8_t *buffer;
   int i, array_size, size;
   float quotient,remainder;
   uint32_t temp;

   value = ESC_PRAM_CMD_ABORT;
   bcm2835_spi_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_PRAM_RD_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   bcm2835_spi_write_32(ESC_PRAM_RD_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   bcm2835_spi_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_PRAM_RD_CMD_REG);
   }while((value & ESC_PRAM_CMD_AVAIL) == 0);

   /* Fifo count */
   fifo_cnt = ESC_PRAM_CMD_CNT(value);

   /* Read first value from FIFO */
   value = bcm2835_spi_read_32(ESC_PRAM_RD_FIFO_REG);
   fifo_cnt--;

   /* Find out first byte position and adjust the copy from that
    * according to LAN9252 datasheet and MicroChip SDK code
    */
   first_byte_position = (address & 0x03);
   temp_len = ((4 - first_byte_position) > len) ? len : (4 - first_byte_position);

   memcpy(temp_buf ,((uint8_t *)&value + first_byte_position), temp_len);
   len -= temp_len;
   byte_offset += temp_len;

   /* Continue reading until we have read len */
    if (len > 0){

        quotient = len/4;
        remainder = len%4;
        
        if (remainder == 0)
            array_size = quotient;
        else
            array_size = quotient+1;

        size = 3+4*array_size;

        buffer = (uint8_t *)malloc(size);
        buffer[0] = size;
        memset(buffer,0,size);

        buffer[0] = ESC_CMD_SERIAL_READ;
        buffer[1] = ((ESC_PRAM_RD_FIFO_REG >>8) & 0xFF);
        buffer[2] = ( ESC_PRAM_RD_FIFO_REG & 0xFF);
   
         /* Read data */
        bcm2835_spi_transfern((char *)buffer, size);
                   
        while(len > 0)
        {
        
            for (i=3; i<size; i=i+4) {
                temp_len = (len > 4) ? 4: len;

                temp = buffer[i] | (buffer[i+1] << 8) | (buffer[i+2] << 16) | (buffer[i+3] << 24);
                memcpy(temp_buf + byte_offset ,&temp, temp_len);
                fifo_cnt--;
                len -= temp_len;
                byte_offset += temp_len;
            }
        }
        free(buffer);
    }
}

/* ESC write process data ram function */
static void ESC_write_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint16_t byte_offset = 0;
   uint8_t fifo_cnt, first_byte_position, temp_len;
   uint8_t *buffer;
   int i, array_size, size;
   float quotient,remainder;   

   value = ESC_PRAM_CMD_ABORT;
   bcm2835_spi_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_PRAM_WR_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   bcm2835_spi_write_32(ESC_PRAM_WR_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   bcm2835_spi_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = bcm2835_spi_read_32(ESC_PRAM_WR_CMD_REG);
   }while((value & ESC_PRAM_CMD_AVAIL) == 0);

   /* Fifo count */
   fifo_cnt = ESC_PRAM_CMD_CNT(value);

   /* Find out first byte position and adjust the copy from that
    * according to LAN9252 datasheet
    */
   first_byte_position = (address & 0x03);
   temp_len = ((4 - first_byte_position) > len) ? len : (4 - first_byte_position);

   memcpy(((uint8_t *)&value + first_byte_position), temp_buf, temp_len);

   /* Write first value from FIFO */
   bcm2835_spi_write_32(ESC_PRAM_WR_FIFO_REG, value);

   len -= temp_len;
   byte_offset += temp_len;
   fifo_cnt--;

    if (len > 0){

        quotient = len/4;
        remainder = len%4;
        
        if (remainder == 0)
            array_size = quotient;
        else
            array_size = quotient+1;

        size = 3+4*array_size;

        buffer = (uint8_t *)malloc(size);
        buffer[0] = size;
        memset(buffer,0,size);
        
        buffer[0] = ESC_CMD_SERIAL_WRITE;
        buffer[1] = ((ESC_PRAM_WR_FIFO_REG >> 8) & 0xFF);
        buffer[2] = (ESC_PRAM_WR_FIFO_REG & 0xFF);

        while(len > 0)
        {                        
            for (i=3; i<size; i=i+4) {           
                temp_len = (len > 4) ? 4 : len;

                memcpy((uint8_t *)&value, (temp_buf + byte_offset), temp_len);
                buffer[i] = (value & 0xFF);
                buffer[i+1] = ((value >> 8) & 0xFF);
                buffer[i+2] = ((value >> 16) & 0xFF);
                buffer[i+3] = ((value >> 24) & 0xFF);

                fifo_cnt--;
                len -= temp_len;
                byte_offset += temp_len;
            }
        }

        bcm2835_spi_transfern((char *)buffer, size);
        free(buffer);
    }
}


/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
   /* Select Read function depending on address, process data ram or not */
   if (address >= 0x1000)
   {
      ESC_read_pram(address, buf, len);
   }
   else
   {
      uint16_t size;
      uint8_t *temp_buf = (uint8_t *)buf;

      while(len > 0)
      {
         /* We write maximum 4 bytes at the time */
         size = (len > 4) ? 4 : len;
         /* Make size aligned to address according to LAN9252 datasheet
          * Table 12-14 EtherCAT CSR Address VS size and MicroChip SDK code
          */
         /* If we got an odd address size is 1 , 01b 11b is captured */
         if(address & BIT(0))
         {
            size = 1;
         }
         /* If address 1xb and size != 1 and 3 , allow size 2 else size 1 */
         else if (address & BIT(1))
         {
            size = (size & BIT(0)) ? 1 : 2;
         }
         /* size 3 not valid */
         else if (size == 3)
         {
            size = 1;
         }
         /* else size is kept AS IS */
         ESC_read_csr(address, temp_buf, size);

         /* next address */
         len -= size;
         temp_buf += size;
         address += size;
      }
   }
   /* To mimic the ET1100 always providing AlEvent on every read or write */
   ESC_read_csr(ESCREG_ALEVENT,(void *)&ESCvar.ALevent,sizeof(ESCvar.ALevent));
   ESCvar.ALevent = etohs (ESCvar.ALevent);

}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write (uint16_t address, void *buf, uint16_t len)
{
   /* Select Write function depending on address, process data ram or not */
   if (address >= 0x1000)
   {
      ESC_write_pram(address, buf, len);
   }
   else
   {
      uint16_t size;
      uint8_t *temp_buf = (uint8_t *)buf;

      while(len > 0)
      {
         /* We write maximum 4 bytes at the time */
         size = (len > 4) ? 4 : len;
         /* Make size aligned to address according to LAN9252 datasheet
          * Table 12-14 EtherCAT CSR Address VS size  and MicroChip SDK code
          */
         /* If we got an odd address size is 1 , 01b 11b is captured */
         if(address & BIT(0))
         {
            size = 1;
         }
         /* If address 1xb and size != 1 and 3 , allow size 2 else size 1 */
         else if (address & BIT(1))
         {
            size = (size & BIT(0)) ? 1 : 2;
         }
         /* size 3 not valid */
         else if (size == 3)
         {
            size = 1;
         }
         /* else size is kept AS IS */
         ESC_write_csr(address, temp_buf, size);

         /* next address */
         len -= size;
         temp_buf += size;
         address += size;
      }
   }

   /* To mimic the ET1x00 always providing AlEvent on every read or write */
   ESC_read_csr(ESCREG_ALEVENT,(void *)&ESCvar.ALevent,sizeof(ESCvar.ALevent));
   ESCvar.ALevent = etohs (ESCvar.ALevent);
}

/* Un-used due to evb-lan9252-digio not havning any possability to
 * reset except over SPI.
 */
void ESC_reset (void)
{

}

void ESC_init (const esc_cfg_t * config)
{
   uint32_t value;
   uint32_t counter = 0;
   uint32_t timeout = 1000; // wait 100msec
   
   if (bcm2835_init())
   {
      if (bcm2835_spi_begin())
      {
         bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);    // Set SPI bit order
         bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                 //  Set SPI data mode BCM2835_SPI_MODE0 = 0, CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
         #ifdef RPI4
         bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);  // Raspberry 4 due to a higher CPU speed this value is to change to: BCM2835_SPI_CLOCK_DIVIDER_32
         #else
         bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);  // Set SPI clock speed BCM2835_SPI_CLOCK_DIVIDER_16 = 16, 16 = 64ns = 15.625MHz
         #endif
         bcm2835_spi_chipSelect(BCM2835_SPI_CS0); //Enable management of CS pin
         bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);    // enable CS0 and set polarity  (for RPI_GPIO_P1_24)
         //bcm2835_spi_chipSelect(BCM2835_SPI_CS1); //Enable management of CS pin
         //bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);  // enable CS1 and set polarity (for RPI_GPIO_P1_26)
         //bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_OUTP); // EtherC only?
         
         // Reset the ecat core here due to evb-lan9252-digio not having any GPIO for that purpose.
         bcm2835_spi_write_32(ESC_CMD_RESET_CTL,ESC_RESET_CTRL_RST);
         
         // Wait until reset command has been executed
         do
         {
            usleep(100);
            counter++;
            value = bcm2835_spi_read_32(ESC_CMD_RESET_CTL);
         } while ((value & ESC_RESET_CTRL_RST) && (counter < timeout));

         // Perform byte test
         do
         {
            usleep(100);
            counter++;
            value = bcm2835_spi_read_32(ESC_CMD_BYTE_TEST);
         } while ((value != ESC_BYTE_TEST_OK) && (counter < timeout));
         
         // Check hardware is ready
         do
         {
            usleep(100);
            counter++;
            value = bcm2835_spi_read_32(ESC_CMD_HW_CFG);
         } while (!(value & ESC_HW_CFG_READY) && (counter < timeout));
         
         // Check if timeout occured
         if (counter < timeout)
         {
            // Read the chip identification and revision
            value = bcm2835_spi_read_32(ESC_CMD_ID_REV);  
            printf("Detected chip %x Rev %u \n", ((value >> 16) & 0xFFFF), (value & 0xFFFF));
         }
         else
         {
            printf("Timeout occurred during reset \n");
            bcm2835_spi_end();
            bcm2835_close();
         }
      }
      else
      {
         printf("bcm2835_spi_begin failed. Are you running as root ?\n");
         bcm2835_close();
      }
   }
   else
   { 
      printf("bcm2835_init failed. Are you running as root ?\n");
   }
}
