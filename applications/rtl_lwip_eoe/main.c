
#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "esc.h"
#include "esc_eoe.h"
#include "esc_hw.h"
#include "ecat_slv.h"
#include "options.h"
#include "utypes.h"
#include <lwip/sys.h>
#include <lwip/netifapi.h>
#include <netif/etharp.h>
#include <string.h>

#define CFG_HOSTNAME "xmc48relax"

static struct netif * found_if;
static mbox_t * pbuf_mbox;
static uint8_t mac_address[6] = {0x1E, 0x30, 0x6C, 0xA2, 0x45, 0x5E};

static void appl_get_buffer (eoe_pbuf_t * ebuf);
static void appl_free_buffer (eoe_pbuf_t * ebuf);
static int appl_load_eth_settings (void);
static int appl_store_ethernet_settings (void);
static void appl_handle_recv_buffer (uint8_t port, eoe_pbuf_t * ebuf);
static int appl_fetch_send_buffer (uint8_t port, eoe_pbuf_t * ebuf);

/* Application variables */
_Objects    Obj;

extern sem_t * ecat_isr_sem;

struct netif * net_add_interface (err_t (*netif_fn) (struct netif * netif))
{
   struct netif * netif;
   ip_addr_t ipaddr;
   ip_addr_t netmask;
   ip_addr_t gateway;
   err_enum_t error;

   netif = malloc (sizeof(struct netif));
   UASSERT (netif != NULL, EMEM);

   /* Set default (zero) values */
   ip_addr_set_zero (&ipaddr);
   ip_addr_set_zero (&netmask);
   ip_addr_set_zero (&gateway);

   /* Let lwIP TCP/IP thread initialise and add the interface. The interface
    * will be down until net_configure() is called.
    */
   error = netifapi_netif_add (
         netif, &ipaddr, &netmask, &gateway, NULL, netif_fn, tcpip_input);
   UASSERT (error == ERR_OK, EARG);

   return netif;
}

void cb_get_inputs (void)
{
   static int count;
   Obj.Buttons.Button1 = gpio_get(GPIO_BUTTON1);
   if(Obj.Buttons.Button1 == 0)
   {
      count++;
      if(count > 1000)
      {
         ESC_ALstatusgotoerror((ESCsafeop | ESCerror), ALERR_WATCHDOG);
      }
   }
   else
   {
      count = 0;
   }
}

void cb_set_outputs (void)
{
   gpio_set(GPIO_LED1, Obj.LEDgroup0.LED0);
   gpio_set(GPIO_LED2, Obj.LEDgroup1.LED1);
}

void cb_state_change (uint8_t * as, uint8_t * an)
{
   if (*as == SAFEOP_TO_OP)
   {
      /* Enable watchdog interrupt */
      ESC_ALeventmaskwrite(ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
   }

   /* Clean up data if we have been in INIT state */
   if ((*as == INIT_TO_PREOP) && (*an == ESCinit))
   {
      struct pbuf *p;
      int i = 0;
      while(mbox_fetch_tmo(pbuf_mbox, (void **)&p, 0) == 0)
      {
         pbuf_free(p);
         i++;
      }
      if(i)
      {
         rprintf("Cleaned eoe pbuf: %d\n",i);
      }
      EOE_init();
   }
}

/* Callback to allocate a buffer */
static void appl_get_buffer (eoe_pbuf_t * ebuf)
{
   struct pbuf * p = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);

   if(p != NULL)
   {
      ebuf->payload = p->payload;
      ebuf->pbuf = p;
      ebuf->len = p->len;
   }
   else
   {
      ebuf->payload = NULL;
      ebuf->pbuf = NULL;
      ebuf->len = p->len;
   }
}

/* Callback to free a buffer */
 static void appl_free_buffer (eoe_pbuf_t * ebuf)
{
   if(ebuf->pbuf != NULL)
   {
      pbuf_free(ebuf->pbuf);
   }
}

/* Callback to act on a received get IP request.
 * From here we can retrieve locla IP and DNS information
 *
 */
static int appl_load_eth_settings (void)
{
   /*
    * ip_addr_t ip;
    * IP4_ADDR (&ip, 192, 168, 9, 200)
    * ip.addr = ntohl(ip.addr);
    * EOE_ecat_set_ip();
    */
   return 0;
}

/* Callback to act on a received set IP request.
 * Avalaible IP and DNS data depends on what the master provide.
 * We only fetch what is needed to init lwIP
 */
static int appl_store_ethernet_settings (void)
{
   int ret = 0;
   ip_addr_t ip;
   ip_addr_t netmask;
   ip_addr_t gateway;

   /* Fetch received IP information, IP returned in host uint32_t format */
   if(EOE_ecat_get_ip (0, &ip.addr) == -1)
   {
      ret = -1;
   }
   else if(EOE_ecat_get_subnet (0, &netmask.addr) == -1)
   {
      ret = -1;
   }
   else if(EOE_ecat_get_gateway (0, &gateway.addr) == -1)
   {
      ret = -1;
   }
   else
   {
      ip.addr = htonl(ip.addr);
      netmask.addr = htonl(netmask.addr);
      gateway.addr = htonl(gateway.addr);
      /* Configure TCP/IP network stack. DNS server and host name are not set. */
      net_configure (found_if, &ip, &netmask, &gateway, NULL, CFG_HOSTNAME);
      net_link_up (found_if);
      if (netif_is_up (found_if))
      {
         rprintf ("netif up (%d.%d.%d.%d)\n",
                  ip4_addr1 (&found_if->ip_addr),
                  ip4_addr2 (&found_if->ip_addr),
                  ip4_addr3 (&found_if->ip_addr),
                  ip4_addr4 (&found_if->ip_addr));
      }
      else
      {
         rprintf ("netif down\n");
      }
   }
   return ret;
}

/* Callback from the stack to handled a completed Ethernet frame. */
static void appl_handle_recv_buffer (uint8_t port, eoe_pbuf_t * ebuf)
{
   struct pbuf * p = ebuf->pbuf;
   p->len = p->tot_len = ebuf->len;
   if(ebuf->pbuf != NULL)
   {
      /* Dummy code for bounce back a L2 frame of type 0x88A4U back to
       * the EtherCAT master.
       */
      struct eth_hdr *ethhdr;
      uint16_t type;
      ethhdr = p->payload;
      type = htons(ethhdr->type);
      if (type == 0x88A4U)
      {
         if(mbox_post_tmo(pbuf_mbox, p, 0))
         {
            pbuf_free (p);
            rprintf("transmit_frame timeout full?\n");
         }
         else
         {
            sem_signal(ecat_isr_sem);
         }
      }
      /* Normal procedure to pass the Ethernet frame to lwIP to handle */
      else if (found_if->input (p, found_if) != 0)
      {
         pbuf_free (p);
      }
   }
}

/* Callback from the stack to fetch a posted Ethernet frame to send to the
 * Master over EtherCAT
 */
static int appl_fetch_send_buffer (uint8_t port, eoe_pbuf_t * ebuf)
{
   int ret;
   struct pbuf *p;

   if(mbox_fetch_tmo(pbuf_mbox, (void **)&p, 0))
   {
      ebuf->pbuf = NULL;
      ebuf->payload = NULL;
      ret = -1;
   }
   else
   {
      ebuf->pbuf = p;
      ebuf->payload = p->payload;
      ebuf->len = p->tot_len;
      ret = ebuf->len;
   }
   return ret;
}

/* Util function for lwIP to post Ethernet frames to be sent over dummy
 * EtherCAT network interface.
 */
static err_t transmit_frame (struct netif *netif, struct pbuf *p)
{
   /* Try posting the buffer to the EoE stack send Q, the caller will try to
    * free the buffer if we fail to post..
    */
   if(mbox_post_tmo(pbuf_mbox, p, 0))
   {
      rprintf("transmit_frame timeout full?\n");
   }
   else
   {
      /* Create a pbuf ref to keep the buf alive until it is sent over EoE */
      pbuf_ref(p);
      sem_signal(ecat_isr_sem);
   }
   return ERR_OK;
}

/* Create an dummy lwIP EtherCAT interface */
err_t eoe_netif_init (struct netif * netif)
{
   rprintf("EOE eoe_netif_init called\n");

   /* Initialise netif */
   netif->name[0]    = 'e';
   netif->name[1]    = 'c';
   netif->output     = etharp_output;
   netif->linkoutput = transmit_frame;
   netif->mtu = 1500;   /* maximum transfer unit */
   netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
   netif->hwaddr_len = ETHARP_HWADDR_LEN;
   memcpy (netif->hwaddr, mac_address, sizeof(netif->hwaddr));

   return ERR_OK;
}

/* Callback on fragment sent event, we trigger a stack cycle to handle mailbox traffic
 * if we might have more fragements in queue.
 */
void eoe_frame_sent (void)
{
   sem_signal(ecat_isr_sem);
}

int main (void)
{
   static esc_cfg_t config =
   {
      .user_arg = NULL,
      .use_interrupt = 1,
      .watchdog_cnt = INT32_MAX, /* Use HW SM watchdog instead */
      .set_defaults_hook = NULL,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = cb_state_change,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = ESC_interrupt_enable,
      .esc_hw_interrupt_disable = ESC_interrupt_disable,
      .esc_hw_eep_handler = ESC_eep_handler,
      .esc_check_dc_handler = NULL
   };

   /* Configuration parameters for EoE
    * Function callbacks to interact with an TCP/IP stack
    */
   static eoe_cfg_t eoe_config =
   {
      .get_buffer = appl_get_buffer,
      .free_buffer = appl_free_buffer,
      .load_eth_settings = appl_load_eth_settings,
      .store_ethernet_settings = appl_store_ethernet_settings,
      .handle_recv_buffer = appl_handle_recv_buffer,
      .fetch_send_buffer = appl_fetch_send_buffer,
      .fragment_sent_event = eoe_frame_sent,
   };

   /* Create an mailbox for interprocess communication between TCP/IP stack and
    * EtherCAT stack.
    */
   pbuf_mbox = mbox_create (10);
   /* Set up dummy IF */
   found_if = net_add_interface(eoe_netif_init);
   if(found_if == NULL)
   {
      rprintf("OBS! Failed to create an EtherCAT network interface\n");
   }
   /* Init EoE */
   EOE_config(&eoe_config);

   rprintf ("Hello world\n");
   ecat_slv_init (&config);

   /* The stack is run from interrupt and a worker thread in esc_hw.c */

   return 0;
}
