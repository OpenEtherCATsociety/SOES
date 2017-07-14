/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2007-2017 Arthur Ketels
 * Copyright (C) 2012-2017 rt-labs.
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
 * ESC hardware layer functions for LAN9252.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 */
#include "utypes.h"
#include "esc.h"
#include <string.h>
#ifdef __linux__
   #include <fcntl.h>
   #include <stdlib.h>
   #define BIT(x)	1 << (x)
#else 
   #include <spi/spi.h>
   #include <gpio.h>
#endif   

#define ESC_CMD_SERIAL_WRITE     0x02
#define ESC_CMD_SERIAL_READ      0x03
#define ESC_CMD_FAST_READ        0x0B
#define ESC_CMD_RESET_SQI        0xFF

#define ESC_CMD_FAST_READ_DUMMY  1
#define ESC_CMD_ADDR_INC         BIT(6)

#define ESC_PRAM_RD_FIFO_REG     0x000
#define ESC_PRAM_WR_FIFO_REG     0x020
#define ESC_PRAM_RD_ADDR_LEN_REG 0x308
#define ESC_PRAM_RD_CMD_REG      0x30C
#define ESC_PRAM_WR_ADDR_LEN_REG 0x310
#define ESC_PRAM_WR_CMD_REG      0x314

#define ESC_PRAM_CMD_BUSY        BIT(31)
#define ESC_PRAM_CMD_ABORT       BIT(30)

#define ESC_PRAM_CMD_CNT(x)      ((x >> 8) & 0x1F)
#define ESC_PRAM_CMD_AVAIL       BIT(0)

#define ESC_PRAM_SIZE(x)         ((x) << 16)
#define ESC_PRAM_ADDR(x)         ((x) << 0)

#define ESC_CSR_DATA_REG         0x300
#define ESC_CSR_CMD_REG          0x304

#define ESC_CSR_CMD_BUSY         BIT(31)
#define ESC_CSR_CMD_READ         (BIT(31) | BIT(30))
#define ESC_CSR_CMD_WRITE        BIT(31)
#define ESC_CSR_CMD_SIZE(x)      (x << 16)

#define ESC_RESET_CTRL_REG       0x1F8
#define ESC_RESET_CTRL_RST       BIT(6)

static int lan9252 = -1;

/* lan9252 singel write */
static void lan9252_write_32 (uint16_t address, uint32_t val)
{
    uint8_t data[7];

    data[0] = ESC_CMD_SERIAL_WRITE;
    data[1] = ((address >> 8) & 0xFF);
    data[2] = (address & 0xFF);
    data[3] = (val & 0xFF);
    data[4] = ((val >> 8) & 0xFF);
    data[5] = ((val >> 16) & 0xFF);
    data[6] = ((val >> 24) & 0xFF);

#ifndef __linux__
    /* Select device. */		
    spi_select (lan9252);
#endif
    /* Write data */
    write (lan9252, data, sizeof(data));
#ifndef __linux__    
    /* Un-select device. */		
    spi_unselect (lan9252);    
#endif    
}

/* lan9252 single read */
static uint32_t lan9252_read_32 (uint32_t address)
{
#ifdef __linux__
   uint8_t data[2];
   uint16_t lseek_addr;
#else
   uint8_t data[4];
#endif      
   uint8_t result[4];

#ifdef __linux__
   data[0] = ((address >>8) & 0xFF);
   data[1] = (address & 0xFF);
#else
   data[0] = ESC_CMD_FAST_READ;
   data[1] = ((address >> 8) & 0xFF);
   data[2] = (address & 0xFF);
   data[3] = ESC_CMD_FAST_READ_DUMMY;
#endif

#ifdef __linux__
	lseek_addr=((uint16_t)data[0] << 8) | data[1];
	lseek (lan9252, lseek_addr, SEEK_SET);
#else
   /* Select device. */
   spi_select (lan9252);
   /* Read data */
   write (lan9252, data, sizeof(data));
#endif

   read (lan9252, result, sizeof(result));
   
#ifndef __linux__   
   /* Un-select device. */
   spi_unselect (lan9252);
#endif
   
   return ((result[3] << 24) |
           (result[2] << 16) |
           (result[1] << 8) |
            result[0]);
}

/* ESC read CSR function */
static void ESC_read_csr (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;

   value = (ESC_CSR_CMD_READ | ESC_CSR_CMD_SIZE(len) | address);
   lan9252_write_32(ESC_CSR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_CSR_CMD_BUSY);

   value = lan9252_read_32(ESC_CSR_DATA_REG);
   memcpy(buf, (uint8_t *)&value, len);
}

/* ESC write CSR function */
static void ESC_write_csr (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;

   memcpy((uint8_t*)&value, buf,len);
   lan9252_write_32(ESC_CSR_DATA_REG, value);
   value = (ESC_CSR_CMD_WRITE | ESC_CSR_CMD_SIZE(len) | address);
   lan9252_write_32(ESC_CSR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_CSR_CMD_BUSY);
}

/* ESC read process data ram function */
static void ESC_read_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint8_t byte_offset = 0;
#ifdef __linux__   
   uint8_t fifo_cnt, first_byte_position, temp_len, data[2];
   uint8_t *buffer;
   int i, array_size, size;
   float quotient,remainder;
   uint32_t temp;
#else
   uint8_t fifo_cnt, first_byte_position, temp_len, data[4];
#endif   
   

   value = ESC_PRAM_CMD_ABORT;
   lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   lan9252_write_32(ESC_PRAM_RD_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
   }while((value & ESC_PRAM_CMD_AVAIL) == 0);

   /* Fifo count */
   fifo_cnt = ESC_PRAM_CMD_CNT(value);

   /* Read first value from FIFO */
   value = lan9252_read_32(ESC_PRAM_RD_FIFO_REG);
   fifo_cnt--;

   /* Find out first byte position and adjust the copy from that
    * according to LAN9252 datasheet and MicroChip SDK code
    */
   first_byte_position = (address & 0x03);
   temp_len = ((4 - first_byte_position) > len) ? len : (4 - first_byte_position);

   memcpy(temp_buf ,((uint8_t *)&value + first_byte_position), temp_len);
   len -= temp_len;
   byte_offset += temp_len;


#ifdef __linux__   
   if (len > 0){
      quotient = len/4;
      remainder = len%4;

      if (remainder == 0)
         array_size = quotient;
      else
         array_size = quotient+1;            

      size = 4*array_size;        

      buffer = (uint8_t *)malloc(size);
      buffer[0] = size;
      memset(buffer,0,size);    

      lseek (lan9252, ESC_PRAM_RD_FIFO_REG, SEEK_SET);        
      read (lan9252, buffer, size);
      /* Continue reading until we have read len */                   
      while(len > 0)
      {
         for (i=0; i<size; i=i+4) {
            temp_len = (len > 4) ? 4: len;

            temp = buffer[i] | (buffer[i+1] << 8) | (buffer[i+2] << 16)
                  | (buffer[i+3] << 24);
            memcpy(temp_buf + byte_offset ,&temp, temp_len);
            fifo_cnt--;
            len -= temp_len;
            byte_offset += temp_len;
         }
      } 
      free(buffer);
   }
#else 
   /* Select device. */
   spi_select (lan9252);
   /* Send command and address for fifo read */
   data[0] = ESC_CMD_FAST_READ;
   data[1] = ((ESC_PRAM_RD_FIFO_REG >> 8) & 0xFF);
   data[2] = (ESC_PRAM_RD_FIFO_REG & 0xFF);
   data[3] = ESC_CMD_FAST_READ_DUMMY;
   write (lan9252, data, sizeof(data));

   /* Continue reading until we have read len */
   while(len > 0)
   {
      temp_len = (len > 4) ? 4: len;
      /* Always read 4 byte */
      read (lan9252, (temp_buf + byte_offset), sizeof(uint32_t));

      fifo_cnt--;
      len -= temp_len;
      byte_offset += temp_len;
   }
   /* Un-select device. */
   spi_unselect (lan9252);
#endif   
}

/* ESC write process data ram function */
static void ESC_write_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint8_t byte_offset = 0;
   uint8_t fifo_cnt, first_byte_position, temp_len, data[3];
#ifdef __linux__
   uint8_t *buffer;
   int i, array_size, size;
   float quotient, remainder;
#endif   
   

   value = ESC_PRAM_CMD_ABORT;
   lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   lan9252_write_32(ESC_PRAM_WR_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
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
   lan9252_write_32(ESC_PRAM_WR_FIFO_REG, value);

   len -= temp_len;
   byte_offset += temp_len;
   fifo_cnt--;
   
#ifdef __linux__
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
      write (lan9252, buffer, size);
      free(buffer);    
   }
#else
   /* Select device. */
   spi_select (lan9252);
   /* Send command and address for incrementing write */
   data[0] = ESC_CMD_SERIAL_WRITE;
   data[1] = ((ESC_PRAM_WR_FIFO_REG >> 8) & 0xFF);
   data[2] = (ESC_PRAM_WR_FIFO_REG & 0xFF);
   write (lan9252, data, sizeof(data));

   /* Continue reading until we have read len */
   while(len > 0)
   {
      temp_len = (len > 4) ? 4 : len;
      value = 0;
      memcpy((uint8_t *)&value, (temp_buf + byte_offset), temp_len);
      /* Always write 4 byte */
      write (lan9252, (void *)&value, sizeof(value));

      fifo_cnt--;
      len -= temp_len;
      byte_offset += temp_len;
   }
   /* Un-select device. */
   spi_unselect (lan9252);
#endif   
   
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

void ESC_init (const void * arg)
{
   uint32_t value;
   const char * spi_name = (char *)arg;
   lan9252 = open (spi_name, O_RDWR, 0);

   /* Reset the ecat core here due to evb-lan9252-digio not having any GPIO
    * for that purpose.
    */
   lan9252_write_32(ESC_RESET_CTRL_REG,ESC_RESET_CTRL_RST);
   do
   {
      value = lan9252_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_RESET_CTRL_RST);



}
