/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for esc_eoe.c
 */

#ifndef __esc_eoe__
#define __esc_eoe__

#include <cc.h>

typedef struct eoe_pbuf
{
   /** Pointer to frame buffer type used by a TCP/IP stack. (Not mandatory) */
   void * pbuf;
   /** Pointer to frame buffer to send or read from */
   uint8_t * payload;
   /** Length of data in frame buffer */
   size_t len;
} eoe_pbuf_t;

typedef struct eoe_cfg
{
   /** Callback function to get a frame buffer for storage of received frame */
   void (*get_buffer) (eoe_pbuf_t * ebuf);
   /** Callback function to free a frame buffer */
   void (*free_buffer) (eoe_pbuf_t * ebuf);
   /** Callback function to read local settings and update EtherCAT variables
    *  to be delivered to the EtherCAT Master
    */
   int  (*load_eth_settings) (void);
   /** Callback function to read settings provided by the EtherCAT master
    * and store to local settings.
    */
   int  (*store_ethernet_settings) (void);
   /** Callback to frame receive function in TCP(IP stack,
    *  caller should free the buffer
    * */
   void (*handle_recv_buffer) (uint8_t port, eoe_pbuf_t * ebuf);
   /** Callback to fetch a buffer to send */
   int (*fetch_send_buffer) (uint8_t port, eoe_pbuf_t * ebuf);
} eoe_cfg_t;

int EOE_ecat_get_mac (uint8_t port, uint8_t mac[]);
int EOE_ecat_get_ip (uint8_t port, uint32_t * ip);
int EOE_ecat_get_subnet (uint8_t port, uint32_t * subnet);
int EOE_ecat_get_gateway (uint8_t port, uint32_t * default_gateway);
int EOE_ecat_get_dns_ip (uint8_t port, uint32_t * dns_ip);
int EOE_ecat_get_dns_name (uint8_t port, char * dns_name);
int EOE_ecat_set_mac (uint8_t port, uint8_t mac[]);
int EOE_ecat_set_ip (uint8_t port, uint32_t ip);
int EOE_ecat_set_subnet (uint8_t port, uint32_t subnet);
int EOE_ecat_set_gateway (uint8_t port, uint32_t default_gateway);
int EOE_ecat_set_dns_ip (uint8_t port, uint32_t dns_ip);
int EOE_ecat_set_dns_name (uint8_t port, char * dns_name);

void EOE_config (eoe_cfg_t * cfg);
void EOE_init (void);
void ESC_eoeprocess (void);
void ESC_eoeprocess_tx (void);

#endif
