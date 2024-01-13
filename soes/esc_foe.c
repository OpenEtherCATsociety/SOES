/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * File over EtherCAT (FoE) module.
 */


#include <cc.h>
#include "esc.h"
#include "esc_foe.h"
#include <string.h>

 /** \file
 * \brief
 * File over EtherCAT (FoE) module.
 *
 * FOE read / write and FOE service functions
 */

//define if FOE_read should be supported
//#define FOE_READ_SUPPORTED

/** Variable holding current filename read at FOE Open.
 */
static char foe_file_name[FOE_FN_MAX + 1];


/** Main FoE configuration pointer data array. Structure is allocated and filled
 * by the application defining what preferences it requires.
 */
static foe_cfg_t * foe_cfg;
/** Pointer to current file configuration item used by FoE.
 */
static foe_file_cfg_t * foe_file;
/** Main FoE status data array. Structure gets filled with current status
 * variables during FoE usage.
 */
static _FOEvar FOEvar;

/** Validate a write or read request by checking filename and password.
 *
 * @param[in] name      = Filename
 * @param[in] num_chars = Length of filename
 * @param[in] pass      = Numeric variable of password
 * @param[in] op        = Request op-code
 * @return 0= if we succeed, FOE_ERR_NOTFOUND something wrong with filename or
 * password
 */
static uint32_t FOE_fopen (char *name, uint8_t num_chars, uint32_t pass, uint8_t op)
{
   uint32_t i;

   /* Unpack the file name into characters we can look at. */
   if (num_chars > FOE_FN_MAX)
   {
      num_chars = FOE_FN_MAX;
   }

   for (i = 0; i < num_chars; i++)
   {
      foe_file_name[i] = name[i];
   }
   foe_file_name[i] = '\0';

   /* Figure out what file they're talking about. */
   for (i = 0; i < foe_cfg->n_files; i++)
   {
      if (0 == strncmp (foe_file_name, foe_cfg->files[i].name, num_chars))
      {
         if (pass != foe_cfg->files[i].filepass)
         {
            return FOE_ERR_NORIGHTS;
         }

         if (op == FOE_OP_WRQ &&
             (foe_cfg->files[i].write_only_in_boot) &&
             (ESCvar.ALstatus != ESCboot))
         {
            return FOE_ERR_NOTINBOOTSTRAP;
         }

         foe_file = &foe_cfg->files[i];
         foe_file->address_offset = 0;
         foe_file->total_size = 0;
         switch (op)
         {
            case FOE_OP_RRQ:
            {
               FOEvar.fposition = 0;
               FOEvar.fend = foe_cfg->files[i].max_data;
               return 0;
            }
            case FOE_OP_WRQ:
            {
               FOEvar.fposition = 0;
               FOEvar.fend = foe_cfg->files[i].max_data;
               return 0;
            }
         }
      }
   }

   return FOE_ERR_NOTFOUND;
}

#ifdef FOE_READ_SUPPORTED
/** Function writing local data to mailbox buffer to be sent as next FoE frame.
 * It will try to fill the Mailbox buffer available if there is enough data
 * left to read.
 *
 * @param[in] data      = pointer to buffer
 * @param[in] maxlength = max length of data possible to read, controlled by
 * Mailbox - FoE and Mailbox frame headers.

 * @return Number of copied bytes.
 */
static uint32_t FOE_fread (uint8_t * data, uint32_t maxlength)
{
   uint32_t ncopied = 0;

   while (maxlength && (FOEvar.fend - FOEvar.fposition))
   {
      maxlength--;
      *(data++) = foe_cfg->fbuffer[FOEvar.fposition++];
      ncopied++;
   }

   return ncopied;
}
#endif

/** Function reading mailbox buffer to local buffer to be handled by
 * application write hook. Ex. flash routine used by software update.
 * It will consume the buffer and call the write hook every time the configured
 * flush buffer limit is reached.
 *
 *
 * @param[in] data   = Pointer to buffer
 * @param[in] length = Length of data to read

 * @return Number of copied bytes.
 */
static uint32_t FOE_fwrite (uint8_t *data, uint32_t length)
{
    uint32_t ncopied = 0;
    uint32_t failed = 0;

    DPRINT("FOE_fwrite\n");
    FOEvar.fprevposition = FOEvar.fposition;
    while (length && (FOEvar.fend - FOEvar.fposition) && !failed)
    {
       length--;
       foe_cfg->fbuffer[FOEvar.fbufposition++] = *(data++);
       if(FOEvar.fbufposition >= foe_cfg->buffer_size)
       {
          failed = foe_file->write_function (foe_file, foe_cfg->fbuffer, FOEvar.fbufposition);
          FOEvar.fbufposition = 0;
          foe_file->address_offset += foe_cfg->buffer_size;
       }
       FOEvar.fposition++;
       if(failed)
       {
          DPRINT("Failed FOE_fwrite ncopied=%"PRIu32"\n", ncopied);
       }
       else
       {
          ncopied++;
       }
    }

    foe_file->total_size += ncopied;

    DPRINT("FOE_fwrite END with : %"PRIu32"\n",ncopied);
    return ncopied;
}


/** Function handling the final FOE_fwrite when we close up regardless
 * if we have filled the buffers or not.
 *
 * @return Number of copied bytes on success, 0= if failed.
 */
static uint32_t FOE_fclose (void)
{
   uint32_t failed = 0;

   DPRINT("FOE_fclose\n");
   
   failed = foe_file->write_function (foe_file, foe_cfg->fbuffer, FOEvar.fbufposition);
   foe_file->address_offset += FOEvar.fbufposition;
   FOEvar.fbufposition = 0;

   return failed;
}

/** Initialize by clearing all current status variables.
 *
 */
void FOE_init ()
{
   DPRINT("FOE_init\n");
   FOEvar.foepacket = 0;
   FOEvar.foestate = FOE_READY;
   FOEvar.fposition = 0;
   FOEvar.fprevposition = 0;
   FOEvar.fbufposition = 0;
}

/** Function for sending an FOE abort frame.
 *
 * @param[in] code   = abort code
 */
static void FOE_abort (uint32_t code)
{
   _FOE *foembx;
   uint8_t mbxhandle;

   if (code)
   {
      /* Send back an error packet. */
      mbxhandle = ESC_claimbuffer ();
      if (mbxhandle)
      {
         foembx = (_FOE *) &MBX[mbxhandle * ESC_MBXSIZE];
         foembx->mbxheader.length = htoes (ESC_FOEHSIZE);   /* Don't bother with error text for now. */
         foembx->mbxheader.mbxtype = MBXFOE;
         foembx->foeheader.opcode = FOE_OP_ERR;
         foembx->foeheader.errorcode = htoel (code);
         MBXcontrol[mbxhandle].state = MBXstate_outreq;
      }
      /* Nothing we can do if we can't get an outbound mailbox. */
   }
   DPRINT("FOE_abort: 0x%"PRIX32"\n", code);
   FOE_init ();
}

#ifdef FOE_READ_SUPPORTED
/** Sends an FoE data frame, returning the number of data bytes
 * written or an error number.
 * Error numbers will be greater than FOE_DATA_SIZE.

 * @param[in] data   = pointer to buffer
 * @param[in] length = length of data to read

 * @return Number of data bytes written or an error number. Error numbers
 * will be greater than FOE_DATA_SIZE.
 */
static uint32_t FOE_send_data_packet ()
{
   _FOE *foembx;
   uint32_t data_len;
   uint8_t mbxhandle;

   mbxhandle = ESC_claimbuffer ();
   if (mbxhandle)
   {
      foembx = (_FOE *) &MBX[mbxhandle * ESC_MBXSIZE];
      data_len = FOE_fread (foembx->data, ESC_FOE_DATA_SIZE);
      foembx->foeheader.opcode = FOE_OP_DATA;
      foembx->foeheader.packetnumber = htoel (FOEvar.foepacket);
      FOEvar.foepacket++;
      foembx->mbxheader.length = htoes (data_len + ESC_FOEHSIZE);
      foembx->mbxheader.mbxtype = MBXFOE;
      /* Mark the outbound mailbox as filled. */
      MBXcontrol[mbxhandle].state = MBXstate_outreq;
      return data_len;
   }
   else
   {
      return FOE_ERR_PROGERROR;
   }
}
#endif

/** Sends an FoE ack data frame.

 * @return 0= or error number.
 */
static uint32_t FOE_send_ack ()
{
   _FOE *foembx;
   uint8_t mbxhandle;

   mbxhandle = ESC_claimbuffer ();
   if (mbxhandle)
   {
      DPRINT("FOE_send_ack\n");
      foembx = (_FOE *) &MBX[mbxhandle * ESC_MBXSIZE];
      foembx->mbxheader.length = htoes (ESC_FOEHSIZE);
      foembx->mbxheader.mbxtype = MBXFOE;
      foembx->foeheader.opcode = FOE_OP_ACK;
      foembx->foeheader.packetnumber = htoel (FOEvar.foepacket);
      FOEvar.foepacket++;
      MBXcontrol[mbxhandle].state = MBXstate_outreq;
      return 0;
   }
   else
   {
      DPRINT("ERROR:FOE_send_ack\n");
      return FOE_ERR_PROGERROR;
   }
}

/* Handlers for various FoE states. */

#ifdef FOE_READ_SUPPORTED
/** FoE read request handler. Starts with Initialize, Open and Sending one frame.
 * When first frame have been sent we will send data from Ack.
 * On error we will send FOE Abort.
 *
 */
static void FOE_read ()
{
   _FOE *foembx;
   uint32_t password;
   uint32_t res;
   uint8_t data_len;

   if (FOEvar.foestate != FOE_READY)
   {
      FOE_abort (FOE_ERR_ILLEGAL);
      return;
   }

   FOE_init ();
   foembx = (_FOE *) &MBX[0];
   /* Get the length of the file name in octets. */
   data_len = (uint8_t)(etohs (foembx->mbxheader.length) - ESC_FOEHSIZE);
   password = etohl (foembx->foeheader.password);

   res = FOE_fopen (foembx->filename, data_len, password, FOE_OP_RRQ);
   if (res == 0)
   {
      FOEvar.foepacket = 1;
      /*
       * Attempt to send the packet
       */
      res = FOE_send_data_packet ();
      if (res <= ESC_FOE_DATA_SIZE)
      {
         FOEvar.foestate = FOE_WAIT_FOR_ACK;
      }
      else
      {
         FOE_abort (res);
      }
   }
   else
   {
      FOE_abort (res);
   }
}

/** FoE data ack handler. Will continue sending next frame until finished.
 * On error we will send FOE Abort.
 */
static void FOE_ack ()
{
   uint32_t res;

   /* Make sure we're able to take this. */
   if (FOEvar.foestate == FOE_WAIT_FOR_FINAL_ACK)
   {
      /* Move us back to ready state. */
      FOE_init ();
      return;
   }
   else if (FOEvar.foestate != FOE_WAIT_FOR_ACK)
   {
      FOE_abort (FOE_ERR_ILLEGAL);
      return;
   }
   res = FOE_send_data_packet ();
   if (res < ESC_FOE_DATA_SIZE)
   {
      FOEvar.foestate = FOE_WAIT_FOR_FINAL_ACK;
   }
   else if (res >= FOE_ERR_NOTDEFINED)
   {
      FOE_abort (FOE_ERR_PROGERROR);
   }
}
#endif

/** FoE write request handler. Starts with Initialize, Open and Ack that we can/will
 * receive data. On error we will send FOE Abort.
 *
 */
static void FOE_write ()
{
   _FOE *foembx;
   uint32_t password;
   uint32_t res;
   uint8_t data_len;

   if (FOEvar.foestate != FOE_READY)
   {
      FOE_abort (FOE_ERR_ILLEGAL);
      return;
   }

   FOE_init ();
   foembx = (_FOE *) &MBX[0];
   data_len = (uint8_t)(etohs (foembx->mbxheader.length) - ESC_FOEHSIZE);
   password = etohl (foembx->foeheader.password);

   /* Get an address we can write the file to, if possible. */
   res = FOE_fopen (foembx->filename, data_len, password, FOE_OP_WRQ);
   DPRINT("%s %sOK, file \"%s\"\n", __func__, (res == 0) ? "" : "N", foe_file_name);
   if (res == 0)
   {
      res = FOE_send_ack ();
      if (res)
      {
         FOE_abort (res);
      }
      else
      {
         FOEvar.foestate = FOE_WAIT_FOR_DATA;
      }
   }
   else
   {
      FOE_abort (res);
   }
}
/** FoE data request handler. Validates and reads data until we're finished. Every
 * read frame followed by an Ack frame. On error we will send FOE Abort.
 *
 */
static void FOE_data ()
{
   _FOE *foembx;
   uint32_t packet;
   uint32_t data_len, ncopied;
   uint32_t res;

   if(FOEvar.foestate != FOE_WAIT_FOR_DATA)
   {
      FOE_abort(FOE_ERR_ILLEGAL);
      return;
   }

   foembx = (_FOE*)&MBX[0];
   data_len = etohs(foembx->mbxheader.length) - ESC_FOEHSIZE;
   packet = etohl(foembx->foeheader.packetnumber);

   if (packet != FOEvar.foepacket)
   {
      DPRINT("FOE_data packet error, packet: %"PRIu32", foeheader.packet: %"PRIu32"\n",
            packet,
            FOEvar.foepacket);
      FOE_abort (FOE_ERR_PACKETNO);
   }
   else if (data_len == 0)
   {
      DPRINT("FOE_data completed\n");
      FOE_fclose ();
      res = FOE_send_ack ();
      FOE_init ();
   }
   else if (FOEvar.fposition + data_len > FOEvar.fend)
   {
      DPRINT("FOE_data disk full\n");
      FOE_abort (FOE_ERR_DISKFULL);
   }
   else
   {
      ncopied = FOE_fwrite (foembx->data, data_len);
      if (!ncopied)
      {
         DPRINT("FOE_data no copied\n");
         FOE_abort (FOE_ERR_PROGERROR);
      }
      else if (data_len == ESC_FOE_DATA_SIZE)
      {
         DPRINT("FOE_data data_len == FOE_DATA_SIZE\n");
         if (ncopied != data_len)
         {
            DPRINT("FOE_data only %"PRIu32" of %"PRIu32" copied\n",ncopied, data_len);
            FOE_abort (FOE_ERR_PROGERROR);
         }
         res = FOE_send_ack ();
         if (res)
         {
            FOE_abort (res);
         }
      }
      else
      {
         if ((ncopied != data_len) || FOE_fclose ())
         {
            DPRINT("FOE_fclose failed to write extra buffer\n");
            FOE_abort (FOE_ERR_PROGERROR);
         }
         else
         {
            DPRINT("FOE_data completed\n");
            res = FOE_send_ack ();
            FOE_init ();
         }
      }
   }
}

#ifdef FOE_READ_SUPPORTED
/** FoE read request busy handler. Send an Ack of last frame again. On error
 * we will send FOE Abort.
 *
 */
static void FOE_busy ()
{
   /* Only valid if we're servicing a read request. */
   if (FOEvar.foestate != FOE_WAIT_FOR_ACK)
   {
      FOE_abort (FOE_ERR_ILLEGAL);
   }
   else
   {
      /* Send the last part again. */
      FOEvar.fposition = FOEvar.fprevposition;
      FOEvar.foepacket--;
      FOE_ack ();
   }
}
#endif

/** FoE error requesthandler. Send an FOE Abort.
 *
 */
static void FOE_error ()
{
   /* Master panic! abort the transfer. */
   FOE_abort (0);
}

/** Function copying the application configuration variable
 * to the FoE module local pointer variable.
 *
 * @param[in] cfg       = Pointer to by the Application static declared
 * configuration variable holding application specific details.
 */
void FOE_config (foe_cfg_t * cfg)
{
   foe_cfg = cfg;
}

/** Main FoE function checking the status on current mailbox buffers carrying
 * data, distributing the mailboxes to appropriate FOE functions depending
 * on requested opcode.
 * On Error an FoE Error or FoE Abort will be sent.
 */
void ESC_foeprocess (void)
{
   _MBXh *mbh;
   _FOE *foembx;

   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   if (!ESCvar.xoe && (MBXcontrol[0].state == MBXstate_inclaim))
   {
      mbh = (_MBXh *) &MBX[0];
      if (mbh->mbxtype == MBXFOE)
      {
         ESCvar.xoe = MBXFOE;
      }
   }
   if (ESCvar.xoe == MBXFOE)
   {
      foembx = (_FOE *) &MBX[0];
      /* Verify the size of the file data. */
      if (etohs (foembx->mbxheader.length) < ESC_FOEHSIZE)
      {
         FOE_abort (MBXERR_SIZETOOSHORT);
      }
      else
      {
         switch (foembx->foeheader.opcode)
         {
            case FOE_OP_WRQ:
            {
               DPRINT("FOE_OP_WRQ\n");
               FOE_write ();
               break;
            }
            case FOE_OP_DATA:
            {
               DPRINT("FOE_OP_DATA\n");
               FOE_data ();
               break;
            }
#ifdef FOE_READ_SUPPORTED
            case FOE_OP_RRQ:
            {
               DPRINT("FOE_OP_RRQ\n");
               FOE_read ();
               break;
            }
            case FOE_OP_ACK:
            {
               DPRINT("FOE_OP_ACK\n");
               FOE_ack ();
               break;
            }

            case FOE_OP_BUSY:
            {
               DPRINT("FOE_OP_BUSY\n");
               FOE_busy ();
               break;
            }
#endif
            case FOE_OP_ERR:
            {
               DPRINT("FOE_OP_ERR\n");
               FOE_error ();
               break;
            }
            default:
            {
               DPRINT("FOE_ERR_NOTDEFINED\n");
               FOE_abort (FOE_ERR_NOTDEFINED);
               break;
            }
         }
      }
      MBXcontrol[0].state = MBXstate_idle;
      ESCvar.xoe = 0;
   }
}
