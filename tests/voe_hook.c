/*
 * Standalone VoE application-hook tests.
 * Licensed under the GNU General Public License version 2 with exceptions.
 */

#include <stdio.h>
#include <string.h>

#include "esc.h"
#include "esc_voe.h"

#define MBX_SIZE 128

uint8_t MBX[MBXBUFFERS * MBX_SIZE];
_MBXcontrol MBXcontrol[MBXBUFFERS];
_ESCvar ESCvar;

static uint16_t last_error;
static int calls;
static void *seen_arg;
static uint32_t seen_vendor;
static uint16_t seen_type;
static uint8_t seen_request[MBX_SIZE];
static uint32_t seen_length;

uint8_t ESC_claimbuffer(void)
{
   int n;

   for (n = MBXBUFFERS - 1; n > 0; --n)
   {
      if (MBXcontrol[n].state == MBXstate_idle)
      {
         MBXcontrol[n].state = MBXstate_outclaim;
         return (uint8_t)n;
      }
   }
   return 0;
}

void MBX_error(uint16_t error)
{
   last_error = error;
}

static int32_t handler(void *arg, uint32_t vendor, uint16_t type,
      const uint8_t *request, uint32_t request_len,
      uint8_t *reply, uint32_t reply_capacity)
{
   ++calls;
   seen_arg = arg;
   seen_vendor = vendor;
   seen_type = type;
   seen_length = request_len;
   if (request_len <= sizeof(seen_request))
   {
      memcpy(seen_request, request, request_len);
   }

   switch (vendor)
   {
      case 0xbeef:
         if (reply_capacity < 4)
         {
            return -2;
         }
         memcpy(reply, "pong", 4);
         return 4;
      case 0x5678:
         return 0;
      case 0x9999:
         memset(reply, 0x5a, reply_capacity);
         return (int32_t)reply_capacity;
      case 0x1234:
         return (int32_t)(reply_capacity + 1);
      default:
         return -1;
   }
}

static void reset(void)
{
   int n;

   memset(MBX, 0, sizeof(MBX));
   for (n = 0; n < MBXBUFFERS; ++n)
   {
      MBXcontrol[n].state = MBXstate_idle;
   }
   ESCvar.MBXrun = 1;
   ESCvar.xoe = 0;
   ESCvar.activembxsize = MBX_SIZE;
   last_error = 0xffff;
   calls = 0;
   seen_arg = NULL;
   seen_vendor = 0;
   seen_type = 0;
   seen_length = 0;
   memset(seen_request, 0, sizeof(seen_request));
}

static void request(uint8_t mbxtype, uint32_t vendor, uint16_t type,
      const uint8_t *payload, uint16_t payload_len)
{
   _MBXh *in = (_MBXh *)MBX;
   uint32_t v = htoel(vendor);
   uint16_t t = htoes(type);

   memset(MBX, 0, sizeof(MBX));
   in->length = htoes((uint16_t)(6 + payload_len));
   in->mbxtype = mbxtype;
   memcpy(MBX + ESC_MBXHSIZE, &v, sizeof(v));
   memcpy(MBX + ESC_MBXHSIZE + 4, &t, sizeof(t));
   if (payload_len != 0)
   {
      memcpy(MBX + ESC_MBXHSIZE + 6, payload, payload_len);
   }
   MBXcontrol[0].state = MBXstate_inclaim;
}

static _MBXh *response(void)
{
   int n;

   for (n = 1; n < MBXBUFFERS; ++n)
   {
      if (MBXcontrol[n].state == MBXstate_outreq)
      {
         return (_MBXh *)&MBX[n * MBX_SIZE];
      }
   }
   return NULL;
}

static int fail(const char *message)
{
   fprintf(stderr, "FAIL: %s\n", message);
   return 1;
}

int main(void)
{
   static voe_cfg_t config = { .user_arg = &calls, .handler = handler };
   static const uint8_t payload[] = { 1, 2, 3 };
   _MBXh *reply;
   uint32_t vendor;
   uint16_t type;

   /* A different mailbox protocol is left for the normal dispatcher. */
   reset();
   VOE_config(&config);
   request(MBXCOE, 0xbeef, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   if (MBXcontrol[0].state != MBXstate_inclaim || response() != NULL || calls != 0)
   {
      return fail("non-VoE mailbox was consumed");
   }

   /* Without a handler the unsupported-protocol path must still run. */
   reset();
   VOE_config(NULL);
   request(MBXVOE, 0xbeef, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   if (MBXcontrol[0].state != MBXstate_inclaim || response() != NULL)
   {
      return fail("unconfigured VoE consumed the request");
   }

   /* Generic request and reply. */
   reset();
   VOE_config(&config);
   request(MBXVOE, 0xbeef, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   reply = response();
   if (calls != 1 || seen_arg != &calls || seen_vendor != 0xbeef ||
       seen_type != 0x2222 || seen_length != sizeof(payload) ||
       memcmp(seen_request, payload, sizeof(payload)) != 0)
   {
      return fail("handler arguments were not preserved");
   }
   if (reply == NULL || MBXcontrol[0].state != MBXstate_idle ||
       etohs(reply->length) != 10 || reply->mbxtype != MBXVOE)
   {
      return fail("generic reply framing is wrong");
   }
   memcpy(&vendor, MBX + ((uint8_t *)reply - MBX) + ESC_MBXHSIZE, sizeof(vendor));
   memcpy(&type, MBX + ((uint8_t *)reply - MBX) + ESC_MBXHSIZE + 4, sizeof(type));
   if (etohl(vendor) != 0xbeef || etohs(type) != 0x2222 ||
       memcmp(MBX + ((uint8_t *)reply - MBX) + ESC_MBXHSIZE + 6, "pong", 4) != 0)
   {
      return fail("generic reply header or payload is wrong");
   }

   /* Rejected request becomes an unsupported-protocol error. */
   reset();
   VOE_config(&config);
   request(MBXVOE, 0xdead, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   if (last_error != MBXERR_UNSUPPORTEDPROTOCOL || response() != NULL ||
       MBXcontrol[0].state != MBXstate_idle)
   {
      return fail("rejected request did not report unsupported protocol");
   }

   /* Oversized reply is rejected before it can overrun the mailbox. */
   reset();
   VOE_config(&config);
   request(MBXVOE, 0x1234, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   if (last_error != MBXERR_INVALIDSIZE || response() != NULL ||
       MBXcontrol[0].state != MBXstate_idle)
   {
      return fail("oversized reply was not rejected");
   }

   /* Zero-length replies are legal and carry only the six-byte header. */
   reset();
   VOE_config(&config);
   request(MBXVOE, 0x5678, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   reply = response();
   if (reply == NULL || etohs(reply->length) != 6 || reply->mbxtype != MBXVOE)
   {
      return fail("zero-length reply framing is wrong");
   }

   /* A reply exactly at capacity is accepted. */
   reset();
   VOE_config(&config);
   request(MBXVOE, 0x9999, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   reply = response();
   if (reply == NULL ||
       etohs(reply->length) != MBX_SIZE - ESC_MBXHSIZE ||
       reply->mbxtype != MBXVOE)
   {
      return fail("full-capacity reply framing is wrong");
   }

   /* Short and oversized request headers are rejected. */
   reset();
   VOE_config(&config);
   ((_MBXh *)MBX)->length = htoes(5);
   ((_MBXh *)MBX)->mbxtype = MBXVOE;
   MBXcontrol[0].state = MBXstate_inclaim;
   ESC_voeprocess();
   if (last_error != MBXERR_SIZETOOSHORT || MBXcontrol[0].state != MBXstate_idle)
   {
      return fail("short VoE header was not rejected");
   }

   reset();
   VOE_config(&config);
   ((_MBXh *)MBX)->length = htoes((uint16_t)(MBX_SIZE - ESC_MBXHSIZE + 1));
   ((_MBXh *)MBX)->mbxtype = MBXVOE;
   MBXcontrol[0].state = MBXstate_inclaim;
   ESC_voeprocess();
   if (last_error != MBXERR_INVALIDSIZE || MBXcontrol[0].state != MBXstate_idle)
   {
      return fail("oversized VoE header was not rejected");
   }

   /* No free outbox drops the request without calling the handler. */
   reset();
   VOE_config(&config);
   MBXcontrol[1].state = MBXstate_outreq;
   MBXcontrol[2].state = MBXstate_outreq;
   request(MBXVOE, 0xbeef, 0x2222, payload, sizeof(payload));
   ESC_voeprocess();
   if (last_error != MBXERR_NOMOREMEMORY || calls != 0 ||
       MBXcontrol[0].state != MBXstate_idle)
   {
      return fail("buffer exhaustion was not reported");
   }

   puts("VoE hook boundaries: PASS");
   return 0;
}
