#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "esc_eoe.h"
#include "slave.h"
#include "esc_hw.h"
#include "config.h"

#include <lwip/sys.h>
#include <netif/etharp.h>
#include <string.h>

#define CFG_HOSTNAME "xmc48relax"
static struct netif * found_if;
static mbox_t * pbuf_mbox;
static tmr_t * ecat_timer;
static uint8_t mac_address[6] = {0x1E, 0x30, 0x6C, 0xA2, 0x45, 0x5E};

static void appl_get_buffer (eoe_pbuf_t * ebuf);
static void appl_free_buffer (eoe_pbuf_t * ebuf);
static int appl_load_eth_settings (void);
static int appl_store_ethernet_settings (void);
static void appl_handle_recv_buffer (uint8_t port, eoe_pbuf_t * ebuf);
static int appl_fetch_send_buffer (uint8_t port, eoe_pbuf_t * ebuf);

flags_t * ecat_events;

/**
 * This function gets input values and updates Rb.Button1
 */
void cb_get_Button1()
{
   Rb.Button1.B = gpio_get(GPIO_BUTTON1);
}

/**
 * This function gets input values and updates Rb.Button2
 */
void cb_get_Button2()
{
   Rb.Button2.B = gpio_get(GPIO_BUTTON2);
}

/**
 * This function sets output values according to Wb.LEDgroup1
 */
void cb_set_LEDgroup1()
{
   gpio_set(GPIO_LED1_B, Wb.LEDgroup1.LED);
}

/**
 * This function sets output values according to Wb.LEDgroup2
 */
void cb_set_LEDgroup2()
{
   gpio_set(GPIO_LED2_B, Wb.LEDgroup2.LED);
}

/**
 * This function sets output values according to Wb.LEDgroup3
 */
void cb_set_LEDgroup3()
{
   gpio_set(GPIO_LED3_B, Wb.LEDgroup3.LED);
}

/**
 * This function sets output values according to Wb.LEDgroup4
 */
void cb_set_LEDgroup4()
{
   gpio_set(GPIO_LED4_B, Wb.LEDgroup4.LED);
}

/**
 * This function sets output values according to Wb.LEDgroup5
 */
void cb_set_LEDgroup5()
{
   gpio_set(GPIO_LED5_B, Wb.LEDgroup5.LED5);
   gpio_set(GPIO_LED6_B, Wb.LEDgroup5.LED678 & 0x1);
   gpio_set(GPIO_LED7_B, Wb.LEDgroup5.LED678 & 0x2);
   gpio_set(GPIO_LED8_B, Wb.LEDgroup5.LED678 & 0x4);
}

/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex)
{

}

/**
 * This function is called after a SDO write of the object Cb.variableRW.
 */
void cb_post_write_variableRW(int subindex)
{

}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void pre_state_change_hook (uint8_t * as, uint8_t * an)
{
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
};
/* Configuration parameters for SOES
 * SM and Mailbox parameters comes from the
 * generated config.h
 */
static esc_cfg_t config =
{
   .user_arg = NULL,
   .use_interrupt = 1,
   .watchdog_cnt = 1000,
   .mbxsize = MBXSIZE,
   .mbxsizeboot = MBXSIZEBOOT,
   .mbxbuffers = MBXBUFFERS,
   .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
   .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
   .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
   .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
   .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
   .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},
   .pre_state_change_hook = pre_state_change_hook,
   .post_state_change_hook = NULL,
   .application_hook = NULL,
   .safeoutput_override = NULL,
   .pre_object_download_hook = NULL,
   .post_object_download_hook = NULL,
   .rxpdo_override = NULL,
   .txpdo_override = NULL,
   .esc_hw_interrupt_enable = ESC_interrupt_enable,
   .esc_hw_interrupt_disable = ESC_interrupt_disable,
   .esc_hw_eep_handler = ESC_eep_handler
};

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
            flags_set(ecat_events, EVENT_TX);
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
      flags_set(ecat_events, EVENT_TX);
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

/* Periodic EtherCAT timer */
static void soes_timer (tmr_t * timer, void * arg)
{
   flags_t * ev = arg;
   flags_set(ev, EVENT_PERIODIC);
}

/* Main EtherCAT loop */
void main_run(void * arg)
{
   uint32_t mask = EVENT_ISR | EVENT_TX | EVENT_PERIODIC;
   uint32_t flags;

   /* Create an mailbox for interprocess communication between TCP/IP stack and
    * EtherCAT stack.
    */
   pbuf_mbox = mbox_create (10);
   /* Create periodic events to run the EtherCAT stack and application  */
   ecat_events = flags_create (0);
   ecat_timer = tmr_create (tick_from_ms (1), soes_timer, ecat_events, TMR_CYCL);
   tmr_start (ecat_timer);
   /* Set up dummy IF */
   found_if = net_add_interface(eoe_netif_init);
   if(found_if == NULL)
   {
      rprintf("OBS! Failed to create an EtherCAT network interface\n");
   }
   /* Init EoE */
   EOE_config(&eoe_config);
   EOE_init();
   /* Init EtherCAT slave stack */
   ecat_slv_init(&config);
   for(;;)
   {
      if(config.use_interrupt != 0)
      {
         /* Wait for incoming event from application or EtherCAT stack */
         flags_wait_any (ecat_events, mask, &flags);
         /* Cyclic event */
         if(flags & EVENT_PERIODIC)
         {
            DIG_process(DIG_PROCESS_WD_FLAG);
            ecat_slv_poll();
            ESC_eoeprocess_tx();
            flags_clr(ecat_events, EVENT_PERIODIC);
         }
         /* Low prio interrupt from the ESC */
         if(flags & EVENT_ISR)
         {
            ecat_slv_poll();
            ESC_eoeprocess_tx();
            flags_clr(ecat_events, EVENT_ISR);
            ESC_interrupt_enable(ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
                  | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1 | ESCREG_ALEVENT_EEP);

         }
         /* The TCP/IP stack have posted a TX job */
         if(flags & EVENT_TX)
         {
            ESC_eoeprocess_tx();
            flags_clr(ecat_events, EVENT_TX);
         }
      }
      else
      {
         ecat_slv();
         ESC_eoeprocess_tx();
      }
   }
}

/* Start the main application loop */
int main(void)
{
   rprintf("Hello Main\n");
   task_spawn ("soes", main_run, 4, 2048, NULL);

   return 0;
}


