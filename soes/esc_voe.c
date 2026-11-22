/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Vendor over EtherCAT (VoE) mailbox dispatcher.
 *
 * VoE carries a six-byte vendor/type header followed by an opaque application
 * payload. The protocol is intentionally generic: this module only validates
 * the mailbox framing, calls the configured application handler and builds the
 * reply. Vendor-specific identities and payloads belong to the application.
 */

#include <cc.h>
#include "esc.h"
#include "esc_voe.h"
#include <string.h>

#if USE_VOE

/** Application configuration, installed by VOE_config(). */
static voe_cfg_t * voe_cfg;

void VOE_config (voe_cfg_t * cfg)
{
   voe_cfg = cfg;
}

/** Handle one claimed VoE request.
 *
 * The request is dropped and an error reply is sent when the header is
 * malformed or the application rejects it. When no handler is configured the
 * mailbox is left untouched so the normal unsupported-protocol path can answer.
 */
void ESC_voeprocess (void)
{
   _MBXh * in;
   _MBXh * out;
   uint8_t * payload;
   uint8_t handle;
   uint16_t length;
   uint32_t vendor;
   uint16_t type;
   uint32_t capacity;
   int32_t reply_len;

   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   if (ESCvar.xoe != 0)
   {
      return;
   }
   if (MBXcontrol[0].state != MBXstate_inclaim)
   {
      return;
   }

   in = (_MBXh *)&MBX[0];
   if (in->mbxtype != MBXVOE)
   {
      return;
   }
   if ((voe_cfg == NULL) || (voe_cfg->handler == NULL))
   {
      return;
   }

   length = etohs (in->length);
   if (length < 6)
   {
      MBXcontrol[0].state = MBXstate_idle;
      MBX_error (MBXERR_SIZETOOSHORT);
      return;
   }
   if (length > (ESC_MBXSIZE - ESC_MBXHSIZE))
   {
      MBXcontrol[0].state = MBXstate_idle;
      MBX_error (MBXERR_INVALIDSIZE);
      return;
   }

   payload = MBX + ESC_MBXHSIZE;
   memcpy (&vendor, payload, sizeof (vendor));
   memcpy (&type, payload + 4, sizeof (type));
   vendor = etohl (vendor);
   type = etohs (type);

   handle = ESC_claimbuffer ();
   if (handle == 0)
   {
      MBXcontrol[0].state = MBXstate_idle;
      MBX_error (MBXERR_NOMOREMEMORY);
      return;
   }

   capacity = ESC_MBXSIZE - ESC_MBXHSIZE - 6;
   reply_len = voe_cfg->handler (voe_cfg->user_arg, vendor, type,
         payload + 6, length - 6,
         MBX + handle * ESC_MBXSIZE + ESC_MBXHSIZE + 6, capacity);
   if (reply_len < 0)
   {
      MBXcontrol[handle].state = MBXstate_idle;
      MBXcontrol[0].state = MBXstate_idle;
      MBX_error (MBXERR_UNSUPPORTEDPROTOCOL);
      return;
   }
   if ((uint32_t)reply_len > capacity)
   {
      MBXcontrol[handle].state = MBXstate_idle;
      MBXcontrol[0].state = MBXstate_idle;
      MBX_error (MBXERR_INVALIDSIZE);
      return;
   }

   out = (_MBXh *)&MBX[handle * ESC_MBXSIZE];
   memcpy (MBX + handle * ESC_MBXSIZE + ESC_MBXHSIZE, payload, 6);
   out->length = htoes ((uint16_t)(6 + (uint32_t)reply_len));
   out->mbxtype = MBXVOE;
   MBXcontrol[handle].state = MBXstate_outreq;
   MBXcontrol[0].state = MBXstate_idle;
}

#endif /* USE_VOE */
