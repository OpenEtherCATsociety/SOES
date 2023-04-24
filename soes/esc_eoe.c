/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * Ethernet over EtherCAT (EoE) module.
 */

#include <cc.h>
#include <string.h>
#include "esc.h"
#include "esc_eoe.h"


#if defined(EC_BIG_ENDIAN)
#define EOE_HTONS(x) (x)
#define EOE_NTOHS(x) (x)
#define EOE_HTONL(x) (x)
#define EOE_NTOHL(x) (x)
#else
#define EOE_HTONS(x) ((((x) & 0x00ffU) << 8) | (((x) & 0xff00U) >> 8))
#define EOE_NTOHS(x) EOE_HTONS(x)
#define EOE_HTONL(x) ((((x) & 0x000000ffU) << 24) | \
                     (((x) & 0x0000ff00U) <<  8) | \
                     (((x) & 0x00ff0000U) >>  8) | \
                     (((x) & 0xff000000U) >> 24))
#define EOE_NTOHL(x) EOE_HTONL(x)
#endif /* #if defined(EC_BIG_ENDIAN) */

#define EOE_MAKEU32(a,b,c,d) (((uint32_t)((a) & 0xff) << 24) | \
                            ((uint32_t)((b) & 0xff) << 16) | \
                            ((uint32_t)((c) & 0xff) << 8)  | \
                            (uint32_t)((d) & 0xff))

/** Get one byte from the 4-byte address */
#define eoe_ip4_addr1(ipaddr) (((const uint8_t*)(&(ipaddr)->addr))[0])
#define eoe_ip4_addr2(ipaddr) (((const uint8_t*)(&(ipaddr)->addr))[1])
#define eoe_ip4_addr3(ipaddr) (((const uint8_t*)(&(ipaddr)->addr))[2])
#define eoe_ip4_addr4(ipaddr) (((const uint8_t*)(&(ipaddr)->addr))[3])

/** Set an IP address given by the four byte-parts */
#define EOE_IP4_ADDR_TO_U32(ipaddr,a,b,c,d)  \
   (ipaddr)->addr = EOE_HTONL(EOE_MAKEU32(a,b,c,d))

/** Header frame info 1 */
#define EOE_HDR_FRAME_TYPE_OFFSET      0
#define EOE_HDR_FRAME_TYPE             (0xF << 0)
#define EOE_HDR_FRAME_TYPE_SET(x)      ((uint16_t)(((x) & 0xF) << 0))
#define EOE_HDR_FRAME_TYPE_GET(x)      (((x) >> 0) & 0xF)
#define EOE_HDR_FRAME_PORT_OFFSET      4
#define EOE_HDR_FRAME_PORT             (0xF << 4)
#define EOE_HDR_FRAME_PORT_SET(x)      ((uint16_t)(((x) & 0xF) << 4))
#define EOE_HDR_FRAME_PORT_GET(x)      (((x) >> 4) & 0xF)
#define EOE_HDR_LAST_FRAGMENT_OFFSET   8
#define EOE_HDR_LAST_FRAGMENT          (0x1 << 8)
#define EOE_HDR_LAST_FRAGMENT_SET(x)   ((uint16_t)(((x) & 0x1) << 8))
#define EOE_HDR_LAST_FRAGMENT_GET(x)   (((x) >> 8) & 0x1)
#define EOE_HDR_TIME_APPEND_OFFSET     9
#define EOE_HDR_TIME_APPEND            (0x1 << 9)
#define EOE_HDR_TIME_APPEND_SET(x)     ((uint16_t)(((x) & 0x1) << 9))
#define EOE_HDR_TIME_APPEND_GET(x)     (((x) >> 9) & 0x1)
#define EOE_HDR_TIME_REQUEST_OFFSET    10
#define EOE_HDR_TIME_REQUEST           (0x1 << 10)
#define EOE_HDR_TIME_REQUEST_SET(x)    ((uint16_t)(((x) & 0x1) << 10))
#define EOE_HDR_TIME_REQUEST_GET(x)    (((x) >> 10) & 0x1)

/** Header frame info 2 */
#define EOE_HDR_FRAG_NO_OFFSET         0
#define EOE_HDR_FRAG_NO                (0x3F << 0)
#define EOE_HDR_FRAG_NO_SET(x)         ((uint16_t)(((x) & 0x3F) << 0))
#define EOE_HDR_FRAG_NO_GET(x)         (((x) >> 0) & 0x3F)
#define EOE_HDR_FRAME_OFFSET_OFFSET    6
#define EOE_HDR_FRAME_OFFSET           (0x3F << 6)
#define EOE_HDR_FRAME_OFFSET_SET(x)    ((uint16_t)(((x) & 0x3F) << 6))
#define EOE_HDR_FRAME_OFFSET_GET(x)    (((x) >> 6) & 0x3F)
#define EOE_HDR_FRAME_NO_OFFSET        12
#define EOE_HDR_FRAME_NO               (0xF << 12)
#define EOE_HDR_FRAME_NO_SET(x)        ((uint16_t)(((x) & 0xF) << 12))
#define EOE_HDR_FRAME_NO_GET(x)        (((x) >> 12) & 0xF)

/** EOE param */
#define EOE_PARAM_OFFSET                  4
#define EOE_PARAM_MAC_INCLUDE             (0x1 << 0)
#define EOE_PARAM_IP_INCLUDE              (0x1 << 1)
#define EOE_PARAM_SUBNET_IP_INCLUDE       (0x1 << 2)
#define EOE_PARAM_DEFAULT_GATEWAY_INCLUDE (0x1 << 3)
#define EOE_PARAM_DNS_IP_INCLUDE          (0x1 << 4)
#define EOE_PARAM_DNS_NAME_INCLUDE        (0x1 << 5)

/** EoE frame types */
#define EOE_FRAG_DATA                  0
#define EOE_INIT_RESP_TIMESTAMP        1
#define EOE_INIT_REQ                   2 /* Spec SET IP REQ */
#define EOE_INIT_RESP                  3 /* Spec SET IP RESP */
#define EOE_SET_ADDR_FILTER_REQ        4
#define EOE_SET_ADDR_FILTER_RESP       5
#define EOE_GET_IP_PARAM_REQ           6
#define EOE_GET_IP_PARAM_RESP          7
#define EOE_GET_ADDR_FILTER_REQ        8
#define EOE_GET_ADDR_FILTER_RESP       9

/** Define number of ports available.(Only one is supported currently */
#define EOE_NUMBER_OF_PORTS   1
#define EOE_PORT_INDEX(x)     ((x > 0) ? (x - 1) : 0)
/** DNS length according to ETG 1000.6 */
#define EOE_DNS_NAME_LENGTH  32
/** Ethernet address length not including VLAN */
#define EOE_ETHADDR_LENGTH    6
/** IPv4 address length */
#define EOE_IP4_LENGTH        4U /* sizeof(uint32_t) */

/** EOE ip4 address in network order */
struct eoe_ip4_addr {
  uint32_t addr;
};
typedef struct eoe_ip4_addr eoe_ip4_addr_t;

/** EOE ethernet address */
CC_PACKED_BEGIN
typedef struct CC_PACKED eoe_ethaddr
{
  uint8_t addr[EOE_ETHADDR_LENGTH];
} eoe_ethaddr_t;
CC_PACKED_END

typedef struct
{
   /** Pointer to current RX buffer to fill */
   eoe_pbuf_t rxebuf;
   /** Pointer to current TX buff to Send */
   eoe_pbuf_t txebuf;

   /** Current RX fragment number */
   uint8_t rxfragmentno;
   /** Complete RX frame size of current frame */
   uint32_t rxframesize;
   /** Current RX data offset in frame */
   uint32_t rxframeoffset;
   /** Current RX frame number */
   uint16_t rxframeno;

   /** Current TX fragment number */
   uint8_t txfragmentno;
   /** Complete TX frame size of current frame */
   uint32_t txframesize;
   /** Current TX data offset in frame */
   uint32_t txframeoffset;
} _EOEvar;

/** EoE IP request structure */
typedef struct eoe_param
{
   uint8_t mac_set:1;
   uint8_t ip_set:1;
   uint8_t subnet_set:1;
   uint8_t default_gateway_set:1;
   uint8_t dns_ip_set:1;
   uint8_t dns_name_set:1;
   eoe_ethaddr_t mac;
   eoe_ip4_addr_t ip;
   eoe_ip4_addr_t subnet;
   eoe_ip4_addr_t default_gateway;
   eoe_ip4_addr_t dns_ip;
   char dns_name[EOE_DNS_NAME_LENGTH];
} eoe_param_t;

/** Main EoE status data array. Structure gets filled with current information
 * variables during EoE receive and send operations.
 */
static _EOEvar EOEvar;

/** Main FoE configuration pointer data array. Structure is allocated and filled
 * by the application defining what preferences it requires.
 */
static eoe_cfg_t * eoe_cfg;

/** Local EoE variable holding cached IP information values.
 * To be set or read from the user application, eg. TCP/IP stack.
 */
static eoe_param_t nic_ports[EOE_NUMBER_OF_PORTS];

/** Local init/reset functions on frame receive init */
static void EOE_init_rx ();
/** Local init/reset functions on frame send completion */
static void EOE_init_tx ();

/** EoE utility function to convert uint32 to eoe ip bytes.
 * @param[in] ip       = ip in uint32
 * @param[out] byte_ip = eoe ip 4th octet, 3ed octet, 2nd octet, 1st octet
 */
static void EOE_ip_uint32_to_byte (eoe_ip4_addr_t * ip, uint8_t * byte_ip)
{
   byte_ip[3] = eoe_ip4_addr1(ip); /* 1st octet */
   byte_ip[2] = eoe_ip4_addr2(ip); /* 2nd octet */
   byte_ip[1] = eoe_ip4_addr3(ip); /* 3ed octet */
   byte_ip[0] = eoe_ip4_addr4(ip); /* 4th octet */
}

/** EoE utility function to convert eoe ip bytes to uint32.
 * @param[in] byte_ip = eoe ip 4th octet, 3ed octet, 2nd octet, 1st octet
 * @param[out] ip     = ip in uint32
 */
static void EOE_ip_byte_to_uint32 (uint8_t * byte_ip, eoe_ip4_addr_t * ip)
{
   EOE_IP4_ADDR_TO_U32(ip,
         byte_ip[3],  /* 1st octet */
         byte_ip[2],  /* 2nd octet */
         byte_ip[1],  /* 3ed octet */
         byte_ip[0]) ;/* 4th octet */
}

/** Get EoE cached MAC address
 *
 * @param[in] port   = get MAC for port
 * @param[out] mac   = variable to store mac in, should fit EOE_ETHADDR_LENGTH
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_mac(uint8_t port, uint8_t mac[])
{
   int ret = -1;
   int port_ix;

   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].mac_set)
      {
         memcpy(mac, nic_ports[port_ix].mac.addr,
               sizeof(nic_ports[port_ix].mac));
         nic_ports[port_ix].mac_set = 1;
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached MAC address
 *
 * @param[in] port   = get MAC for port
 * @param[in] mac    = mac address to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_mac(uint8_t port, uint8_t mac[])
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      memcpy(nic_ports[port_ix].mac.addr, mac,
            sizeof(nic_ports[port_ix].mac));
      ret = 0;
   }
   return ret;
}

/** Get EoE cached ip address
 *
 * @param[in] port  = get ip address for port
 * @param[out] ip   = variable to store ip in
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_ip(uint8_t port, uint32_t * ip)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].ip_set)
      {
         *ip = EOE_NTOHL(nic_ports[port_ix].ip.addr);
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached ip address
 *
 * @param[in] port   = get ip for port
 * @param[in] ip     = ip address to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_ip(uint8_t port, uint32_t  ip)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      nic_ports[port_ix].ip.addr = EOE_HTONL(ip);
      nic_ports[port_ix].ip_set = 1;
      ret = 0;
   }
   return ret;
}

/** Get EoE cached subnet ip address
 *
 * @param[in] port    = get ip address for port
 * @param[out] subnet = variable to store ip in
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_subnet(uint8_t port, uint32_t * subnet)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].subnet_set)
      {
         *subnet = EOE_NTOHL(nic_ports[port_ix].subnet.addr);
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached subnet ip address
 *
 * @param[in] port   = get ip for port
 * @param[in] subnet = ip address to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_subnet(uint8_t port, uint32_t subnet)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      nic_ports[port_ix].subnet.addr = EOE_HTONL(subnet);
      nic_ports[port_ix].subnet_set = 1;
      ret = 0;
   }
   return ret;
}

/** Get EoE cached default gateway ip address
 *
 * @param[in] port             = get ip address for port
 * @param[out] default_gateway = variable to store ip in
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_gateway(uint8_t port, uint32_t * default_gateway)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].default_gateway_set)
      {
         *default_gateway =
               EOE_NTOHL(nic_ports[port_ix].default_gateway.addr);
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached default gateway ip address
 *
 * @param[in] port            = get ip for port
 * @param[in] default_gateway = ip address to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_gateway(uint8_t port, uint32_t default_gateway)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      nic_ports[port_ix].default_gateway.addr =
            EOE_HTONL(default_gateway);
      nic_ports[port_ix].default_gateway_set = 1;
      ret = 0;
   }
   return ret;
}

/** Get EoE cached dns ip address
 *
 * @param[in] port    = get ip address for port
 * @param[out] dns_ip = variable to store ip in
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_dns_ip(uint8_t port, uint32_t * dns_ip)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].dns_ip_set)
      {
         *dns_ip = EOE_NTOHL(nic_ports[port_ix].dns_ip.addr);
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached dns ip address
 *
 * @param[in] port   = get ip for port
 * @param[in] dns_ip = ip address to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_dns_ip(uint8_t port, uint32_t dns_ip)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      nic_ports[port_ix].dns_ip.addr = EOE_HTONL(dns_ip);
      nic_ports[port_ix].dns_ip_set = 1;
      ret = 0;
   }
   return ret;
}

/** Get EoE cached dns name
 *
 * @param[in] port      = get dns name for port
 * @param[out] dns_name = variable to store dns name in
 * @return 0= if we succeed, -1 if not set
 */
int EOE_ecat_get_dns_name(uint8_t port, char * dns_name)
{
   int ret = -1;
   int port_ix;
   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].dns_name_set)
      {
         memcpy(dns_name,
               nic_ports[port_ix].dns_name,
               sizeof(nic_ports[port_ix].dns_name));
         ret = 0;
      }
   }
   return ret;
}

/** Set EoE cached dns name
 *
 * @param[in] port     = get dns name for port
 * @param[in] dns_name = dns name to store
 * @return 0= if we succeed, else -1.
 */
int EOE_ecat_set_dns_name(uint8_t port, char * dns_name)
{
   int ret = -1;
   int port_ix;

   if(port < EOE_NUMBER_OF_PORTS)
   {
      port_ix = EOE_PORT_INDEX(port);
      memcpy(nic_ports[port_ix].dns_name,
            dns_name,
            sizeof(nic_ports[port_ix].dns_name));
      nic_ports[port_ix].dns_name_set = 1;
      ret = 0;
   }
   return ret;
}

/** Function for sending an simple EOE response frame.
 *
 * @param[in] frametype1 = frame type of response
 * @param[in] result    = result code
 */
static void EOE_no_data_response (uint16_t frameinfo1, uint16_t result)
{
   _EOE *eoembx;
   uint8_t mbxhandle;

   /* Send back a response packet. */
   mbxhandle = ESC_claimbuffer ();
   if (mbxhandle)
   {
      eoembx = (_EOE *) &MBX[mbxhandle * ESC_MBXSIZE];
      eoembx->mbxheader.length = htoes (ESC_EOEHSIZE);
      eoembx->mbxheader.mbxtype = MBXEOE;
      eoembx->eoeheader.frameinfo1 = htoes(frameinfo1);
      eoembx->eoeheader.result = htoes(result);
      MBXcontrol[mbxhandle].state = MBXstate_outreq;
   }
}

/** EoE get IP param request handler. Will send a get IP param response.
 */
static void EOE_get_ip (void)
{
   _EOE *req_eoembx;
   _EOE *eoembx;
   uint8_t mbxhandle;
   uint16_t frameinfo1;
   uint8_t port;
   uint8_t  flags;
   uint32_t  data_offset;
   int port_ix;

   req_eoembx = (_EOE *) &MBX[0];
   frameinfo1 = etohs(req_eoembx->eoeheader.frameinfo1);
   port = EOE_HDR_FRAME_PORT_GET(frameinfo1);
   data_offset = EOE_PARAM_OFFSET;
   flags = 0;

   if(port  > EOE_NUMBER_OF_PORTS)
   {
      DPRINT("Invalid port\n");
      frameinfo1 = EOE_HDR_FRAME_PORT_SET(port);
      frameinfo1 |= EOE_INIT_RESP;
      frameinfo1 |= EOE_HDR_LAST_FRAGMENT;
      /* Return error response on given port */
      EOE_no_data_response(frameinfo1,
            EOE_RESULT_UNSPECIFIED_ERROR);
      return;
   }

   /* Refresh settings if needed */
   if(eoe_cfg->load_eth_settings != NULL)
   {
      (void)eoe_cfg->load_eth_settings();
   }

   /* Send back an response packet. */
   mbxhandle = ESC_claimbuffer ();
   if (mbxhandle)
   {
      eoembx = (_EOE *) &MBX[mbxhandle * ESC_MBXSIZE];
      eoembx->mbxheader.mbxtype = MBXEOE;
      MBXcontrol[mbxhandle].state = MBXstate_outreq;
      frameinfo1 = EOE_HDR_FRAME_PORT_SET(port);
      frameinfo1 |= EOE_HDR_FRAME_TYPE_SET(EOE_GET_IP_PARAM_RESP);
      frameinfo1 |= EOE_HDR_LAST_FRAGMENT;
      eoembx->eoeheader.frameinfo1 = htoes(frameinfo1);
      eoembx->eoeheader.frameinfo2 = 0;

      /* include mac in get ip request */
      port_ix = EOE_PORT_INDEX(port);
      if(nic_ports[port_ix].mac_set)
      {
         flags |= EOE_PARAM_MAC_INCLUDE;
         memcpy(&eoembx->data[data_offset] ,
               nic_ports[port_ix].mac.addr,
               EOE_ETHADDR_LENGTH);
         /* Add size of mac address */
         data_offset += EOE_ETHADDR_LENGTH;

      }
      /* include ip in get ip request */
      if(nic_ports[port_ix].ip_set)
      {
         flags |= EOE_PARAM_IP_INCLUDE;
         EOE_ip_uint32_to_byte(&nic_ports[port_ix].ip,
               &eoembx->data[data_offset]);
         /* Add size of uint32 IP address */
         data_offset += EOE_IP4_LENGTH;
      }

      /* include subnet in get ip request */
      if(nic_ports[port_ix].subnet_set)
      {
         flags |= EOE_PARAM_SUBNET_IP_INCLUDE;
         EOE_ip_uint32_to_byte(&nic_ports[port_ix].subnet,
               &eoembx->data[data_offset]);
         /* Add size of uint32 IP address */
         data_offset += EOE_IP4_LENGTH;
      }

      /* include default gateway in get ip request */
      if(nic_ports[port_ix].default_gateway_set)
      {
         flags |= EOE_PARAM_DEFAULT_GATEWAY_INCLUDE;
         EOE_ip_uint32_to_byte(&nic_ports[port_ix].default_gateway,
               &eoembx->data[data_offset]);
         /* Add size of uint32 IP address */
         data_offset += EOE_IP4_LENGTH;
      }
      /* include dns ip in get ip request */
      if(nic_ports[port_ix].dns_ip_set)
      {
         flags |= EOE_PARAM_DNS_IP_INCLUDE;
         EOE_ip_uint32_to_byte(&nic_ports[port_ix].dns_ip,
               &eoembx->data[data_offset]);
         /* Add size of uint32 IP address */
         data_offset += EOE_IP4_LENGTH;
      }

      /* include dns name in get ip request */
      if(nic_ports[port_ix].dns_name_set)
      {
         /* TwinCAT include EOE_DNS_NAME_LENGTH chars even if name is shorter */
         flags |= EOE_PARAM_DNS_NAME_INCLUDE;
         memcpy(&eoembx->data[data_offset],
               nic_ports[port_ix].dns_name,
               EOE_DNS_NAME_LENGTH);
         /* Add size of dns name length */
         data_offset += EOE_DNS_NAME_LENGTH;
      }

      eoembx->data[0] = flags;
      eoembx->mbxheader.length = htoes (ESC_EOEHSIZE + data_offset);
   }
}

/** EoE set IP param request handler. Will send a set IP param response.
 */
static void EOE_set_ip (void)
{
   _EOE *eoembx;
   uint32_t eoedatasize, data_offset;
   uint16_t frameinfo1;
   uint8_t port;
   uint8_t  flags;
   uint16_t result;
   int port_ix;

   eoembx = (_EOE *) &MBX[0];
   eoedatasize = etohs(eoembx->mbxheader.length) - ESC_EOEHSIZE;
   frameinfo1 = etohs(eoembx->eoeheader.frameinfo1);
   port = EOE_HDR_FRAME_PORT_GET(frameinfo1);
   flags = eoembx->data[0];
   data_offset = EOE_PARAM_OFFSET;

   if(port  > EOE_NUMBER_OF_PORTS)
   {
      DPRINT("Invalid port\n");
      /* Return error response on given port */
      frameinfo1 = EOE_HDR_FRAME_PORT_SET(port);
      frameinfo1 |= EOE_INIT_RESP;
      frameinfo1 |= EOE_HDR_LAST_FRAGMENT;
      EOE_no_data_response(frameinfo1, EOE_RESULT_UNSPECIFIED_ERROR);
      return;
   }

   /* mac included in set ip request? */
   port_ix = EOE_PORT_INDEX(port);
   if(flags & EOE_PARAM_MAC_INCLUDE)
   {
      memcpy(&nic_ports[port_ix].mac.addr,
            &eoembx->data[data_offset],
            EOE_ETHADDR_LENGTH);
      nic_ports[port_ix].mac_set = 1;
      /* Add size of mac address */
      data_offset += EOE_ETHADDR_LENGTH;
   }
   /* ip included in set ip request? */
   if(flags & EOE_PARAM_IP_INCLUDE)
   {
      EOE_ip_byte_to_uint32(&eoembx->data[data_offset],
            &nic_ports[port_ix].ip);
      nic_ports[port_ix].ip_set = 1;
      /* Add size of uint32 IP address */
      data_offset += EOE_IP4_LENGTH;
   }
   /* subnet included in set ip request? */
   if(flags & EOE_PARAM_SUBNET_IP_INCLUDE)
   {
      EOE_ip_byte_to_uint32(&eoembx->data[data_offset],
            &nic_ports[port_ix].subnet);
      nic_ports[port_ix].subnet_set = 1;
      /* Add size of uint32 IP address */
      data_offset += EOE_IP4_LENGTH;
   }
   /* default gateway included in set ip request? */
   if(flags & EOE_PARAM_DEFAULT_GATEWAY_INCLUDE)
   {
      EOE_ip_byte_to_uint32(&eoembx->data[data_offset],
            &nic_ports[port_ix].default_gateway);
      nic_ports[port_ix].default_gateway_set = 1;
      /* Add size of uint32 IP address */
      data_offset += EOE_IP4_LENGTH;
   }
   /* dns ip included in set ip request? */
   if(flags & EOE_PARAM_DNS_IP_INCLUDE)
   {
      EOE_ip_byte_to_uint32(&eoembx->data[data_offset],
            &nic_ports[port_ix].dns_ip);
      nic_ports[port_ix].dns_ip_set = 1;
      /* Add size of uint32 IP address */
      data_offset += EOE_IP4_LENGTH;
   }
   /* dns name included in set ip request? */
   if(flags & EOE_PARAM_DNS_NAME_INCLUDE)
   {
      uint32_t dns_len = MIN((eoedatasize - data_offset), EOE_DNS_NAME_LENGTH);
      memcpy(nic_ports[port_ix].dns_name,
            &eoembx->data[data_offset],
            dns_len);
      nic_ports[port_ix].dns_name_set = 1;
      data_offset += dns_len; /* expected 1- EOE_DNS_NAME_LENGTH; */
   }

   if(data_offset > eoedatasize)
   {
      result = MBXERR_SIZETOOSHORT;
   }
   else
   {
      /* Application specific store settings function. From there
       * you typically set the IP for the TCP/IP stack */
      if(eoe_cfg->store_ethernet_settings != NULL)
      {
         result = (uint16_t)eoe_cfg->store_ethernet_settings();
      }
      else
      {
         result = EOE_RESULT_NO_IP_SUPPORT;
      }
   }
   frameinfo1 = EOE_HDR_FRAME_PORT_SET(port);
   frameinfo1 |= EOE_INIT_RESP;
   frameinfo1 |= EOE_HDR_LAST_FRAGMENT;
   EOE_no_data_response(frameinfo1, result);
}

/** EoE receive fragment handler.
 */
static void EOE_receive_fragment (void)
{
   _EOE *eoembx;
   eoembx = (_EOE *) &MBX[0];
   uint32_t eoedatasize = etohs(eoembx->mbxheader.length) - ESC_EOEHSIZE;
   uint16_t frameinfo1 = etohs(eoembx->eoeheader.frameinfo1);
   uint16_t frameinfo2 = etohs(eoembx->eoeheader.frameinfo2);

   /* Capture error case */
   if(EOEvar.rxfragmentno != EOE_HDR_FRAG_NO_GET(frameinfo2))
   {
      DPRINT("Unexpected fragment number %"PRIu32", expected: %"PRIu32"\n",
            EOE_HDR_FRAG_NO_GET(frameinfo2), EOEvar.rxfragmentno);
      /* Clean up existing saved data */
      if(EOEvar.rxfragmentno != 0)
      {
         EOE_init_rx();
      }
      /* Skip fragment if not start of new frame */
      if(EOE_HDR_FRAG_NO_GET(frameinfo2) > 0)
      {
         return;
      }
   }

   /* Start of new frame at fragment 0 */
   if(EOEvar.rxfragmentno == 0)
   {
      EOEvar.rxframesize = (EOE_HDR_FRAME_OFFSET_GET(frameinfo2) << 5);

      if(EOEvar.rxebuf.payload != NULL)
      {
         EOEvar.rxebuf.len = EOEvar.rxframesize;
         EOEvar.rxframeoffset = 0;
         EOEvar.rxframeno = EOE_HDR_FRAME_NO_GET(frameinfo2);
      }
      else
      {
         DPRINT("Receive buffer is invalid\n");
         EOE_init_rx ();
         return;
      }
   }
   /* In frame fragment received */
   else
   {
      uint32_t offset = (EOE_HDR_FRAME_OFFSET_GET(frameinfo2) << 5);
      /* Validate received fragment */
      if(EOEvar.rxframeno != EOE_HDR_FRAME_NO_GET(frameinfo2))
      {
         DPRINT("Unexpected frame number %"PRIu32", expected: %"PRIu32"\n",
               EOE_HDR_FRAME_NO_GET(frameinfo2), EOEvar.rxframeno);
         EOE_init_rx ();
         return;
      }
      else if(EOEvar.rxframeoffset != offset)
      {
         DPRINT("Unexpected frame offset %"PRIu32", expected: %"PRIu32"\n",
               offset, EOEvar.rxframeoffset);
         EOE_init_rx ();
         return;
      }
   }

   /* Check so allocated buffer is sufficient */
   if ((EOEvar.rxframeoffset + eoedatasize) <= EOEvar.rxframesize)
   {
      memcpy((uint8_t *)(EOEvar.rxebuf.payload + EOEvar.rxframeoffset),
            eoembx->data,
            eoedatasize);
      EOEvar.rxframeoffset += eoedatasize;
      EOEvar.rxfragmentno++;
   }
   else
   {
      DPRINT("Size of data exceed available buffer size\n");
      EOE_init_rx ();
      return;
   }

   if(EOE_HDR_LAST_FRAGMENT_GET(frameinfo1))
   {
      /* Remove time stamp, TODO support for time stamp? */
      if(EOE_HDR_TIME_APPEND_GET(frameinfo1))
      {
         EOEvar.rxframeoffset -= 4U;
      }
      EOEvar.rxebuf.len =  EOEvar.rxframeoffset;
      eoe_cfg->handle_recv_buffer(EOE_HDR_FRAME_PORT_GET(frameinfo1),
            &EOEvar.rxebuf);
      /* Pass ownership of buf to receive function */
      EOEvar.rxebuf.payload = NULL;
      EOE_init_rx ();
   }
}

/** EoE send fragment handler.
 */
static void EOE_send_fragment ()
{
   _EOE *eoembx;
   uint8_t mbxhandle;
   int len;
   uint32_t len_to_send;
   uint16_t frameinfo1;
   uint16_t frameinfo2;
   static uint8_t frameno = 0;

   /* Do we have a current transfer on-going */
   if(EOEvar.txebuf.payload == NULL)
   {
      /* Fetch a buffer if available */
      len = eoe_cfg->fetch_send_buffer(0, &EOEvar.txebuf);
      if(len > 0)
      {
         EOEvar.txframesize = (uint32_t)len;
      }
      else
      {
         return;
      }
   }

   /* Process the frame if we can get a free mailbox */
   mbxhandle = ESC_claimbuffer ();
   if (mbxhandle)
   {
      len_to_send = (EOEvar.txframesize - EOEvar.txframeoffset);
      if((len_to_send + ESC_EOEHSIZE + ESC_MBXHSIZE) > ESC_MBXSIZE)
      {
         /* Adjust to len in whole 32 octet blocks to fit specification*/
         len_to_send =
               (((ESC_MBXSIZE - ESC_EOEHSIZE - ESC_MBXHSIZE) >> 5) << 5);
      }

      /* TODO: port handling? */
      if(len_to_send == (EOEvar.txframesize - EOEvar.txframeoffset))
      {
         frameinfo1 = EOE_HDR_LAST_FRAGMENT_SET(1);
      }
      else
      {
         frameinfo1 = 0;
      }

      uint16_t tempframe2;
      /* Set fragment number */
      frameinfo2 = EOE_HDR_FRAG_NO_SET(EOEvar.txfragmentno);

      /* Set complete size for fragment 0 or offset for in frame fragments */
      if(EOEvar.txfragmentno > 0)
      {
         tempframe2 = EOE_HDR_FRAME_OFFSET_SET((EOEvar.txframeoffset >> 5));
         frameinfo2 |= tempframe2;
      }
      else
      {
         tempframe2 = EOE_HDR_FRAME_OFFSET_SET(((EOEvar.txframesize + 31) >> 5));
         frameinfo2 |= tempframe2;
         frameno++;
      }

      /* Set frame number */
      tempframe2 = EOE_HDR_FRAME_NO_SET(frameno);
      frameinfo2 |= tempframe2;

      eoembx = (_EOE *) &MBX[mbxhandle * ESC_MBXSIZE];
      eoembx->mbxheader.length = htoes (len_to_send + ESC_EOEHSIZE);
      eoembx->mbxheader.mbxtype = MBXEOE;
      eoembx->eoeheader.frameinfo1 = htoes(frameinfo1);
      eoembx->eoeheader.frameinfo2 = htoes(frameinfo2);

      /* Copy data to mailbox */
      memcpy(eoembx->data,
            &EOEvar.txebuf.payload[EOEvar.txframeoffset],
            len_to_send);
      MBXcontrol[mbxhandle].state = MBXstate_outreq;

      /* Did we complete the frame? */
      if(len_to_send == (EOEvar.txframesize - EOEvar.txframeoffset))
      {
         EOE_init_tx ();
      }
      else
      {
         EOEvar.txframeoffset += len_to_send;
         EOEvar.txfragmentno++;
      }
      if(eoe_cfg->fragment_sent_event != NULL)
      {
         eoe_cfg->fragment_sent_event();
      }
   }
}

/** Initialize by clearing all current status variables and fetch new buffer.
 */
static void EOE_init_rx ()
{
   /* Reset RX transfer status variables */
   EOEvar.rxfragmentno = 0;
   EOEvar.rxframesize = 0;
   EOEvar.rxframeoffset = 0;
   EOEvar.rxframeno = 0;

   /* Fetch buffer */
   if(EOEvar.rxebuf.payload == NULL)
   {
      if(eoe_cfg->get_buffer != NULL)
      {
         /* TODO: verify size VS buffer size */
         eoe_cfg->get_buffer(&EOEvar.rxebuf);
      }
   }
}

/** Initialize by clearing all current status variables and release old buffer.
 */
static void EOE_init_tx ()
{
   /* Reset TX transfer status variables */
   EOEvar.txfragmentno = 0;
   EOEvar.txframesize = 0;
   EOEvar.txframeoffset = 0;

   /* Release what seems as an abandoned buffer */
   if((EOEvar.txebuf.payload != NULL))
   {
      if(eoe_cfg->free_buffer != NULL)
      {
         eoe_cfg->free_buffer(&EOEvar.txebuf);
         EOEvar.txebuf.pbuf = NULL;
         EOEvar.txebuf.payload = NULL;
         EOEvar.txebuf.len = 0;
      }
   }
}

/** Initialize by clearing all current status variables.
 */
void EOE_init ()
{
   DPRINT("EOE_init\n");
   EOE_init_tx ();
   EOE_init_rx ();
}

/** Function copying the application configuration variable
 * to the EoE module local pointer variable.
 *
 * @param[in] cfg       = Pointer to by the Application static declared
 * configuration variable holding application specific details.
 */
void EOE_config (eoe_cfg_t * cfg)
{
   eoe_cfg = cfg;
}

/** Main EoE receive function checking the status on current mailbox buffers
 * carrying data, distributing the mailboxes to appropriate EOE functions
 * depending on requested frametype.
 */
void ESC_eoeprocess (void)
{
   _MBXh *mbh;
   _EOE *eoembx;
   uint16_t frameinfo1;

   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   if (!ESCvar.xoe && (MBXcontrol[0].state == MBXstate_inclaim))
   {
      mbh = (_MBXh *) &MBX[0];
      if (mbh->mbxtype == MBXEOE)
      {
         ESCvar.xoe = MBXEOE;
      }
   }
   if (ESCvar.xoe == MBXEOE)
   {
      eoembx = (_EOE *) &MBX[0];
      /* Verify the size of the file data. */
      if (etohs (eoembx->mbxheader.length) < ESC_EOEHSIZE)
      {
         EOE_no_data_response (
               EOE_INIT_RESP | EOE_HDR_LAST_FRAGMENT,
               MBXERR_SIZETOOSHORT);
      }
      else
      {
         frameinfo1 = etohs(eoembx->eoeheader.frameinfo1);
         switch (EOE_HDR_FRAME_TYPE_GET(frameinfo1))
         {
            case EOE_FRAG_DATA:
            {
               EOE_receive_fragment ();
               break;
            }
            case EOE_INIT_REQ:
            {
               EOE_set_ip ();
               break;
            }
            case EOE_GET_IP_PARAM_REQ:
            {
               EOE_get_ip ();
               break;
            }
            case EOE_INIT_RESP_TIMESTAMP:
            case EOE_INIT_RESP:
            case EOE_SET_ADDR_FILTER_REQ:
            case EOE_SET_ADDR_FILTER_RESP:
            case EOE_GET_IP_PARAM_RESP:
            case EOE_GET_ADDR_FILTER_REQ:
            case EOE_GET_ADDR_FILTER_RESP:
            default:
            {
               DPRINT("EOE_RESULT_UNSUPPORTED_TYPE\n");
               EOE_no_data_response ((EOE_HDR_FRAME_PORT & frameinfo1) |
                     (EOE_HDR_FRAME_TYPE & frameinfo1) |
                     EOE_HDR_LAST_FRAGMENT,
                     EOE_RESULT_UNSUPPORTED_FRAME_TYPE);
               break;
            }
         }
      }
      MBXcontrol[0].state = MBXstate_idle;
      ESCvar.xoe = 0;
   }
}
/** EoE function to send a fragment.
 * NOTE: Not thread safe, should be called from the SOES task sequential
 * with other mailbox functions. Add support for threading by adding
 * a thread safe application fetch function, example a mailbox with buffers
 * to send, posted by TCP/IP stack and fetched by SOES task.
 */
void ESC_eoeprocess_tx (void)
{
   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   EOE_send_fragment ();
}
