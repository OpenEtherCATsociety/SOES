/*
 * Standalone FoE read (RRQ) boundary tests.
 * Licensed under the GNU General Public License version 2 with exceptions.
 */

#include <stdio.h>
#include <string.h>

#include "esc.h"
#include "esc_foe.h"

#define MBX_SIZE 128
#define FILE_CAPACITY 512

uint8_t MBX[MBXBUFFERS * MBX_SIZE];
_MBXcontrol MBXcontrol[MBXBUFFERS];
_ESCvar ESCvar;

/* Minimal claim implementation; esc_foe.c only needs an outbox slot. */
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

static uint8_t file_data[FILE_CAPACITY];
static foe_file_cfg_t file = {
   .name = "demo.bin",
   .max_data = FILE_CAPACITY,
   .filepass = 0,
};
static foe_cfg_t config = {
   .fbuffer = file_data,
   .buffer_size = FILE_CAPACITY,
   .n_files = 1,
   .files = &file,
};

static void reset(void)
{
   int n;

   memset(MBX, 0, sizeof(MBX));
   for (n = 0; n < MBXBUFFERS; ++n)
   {
      MBXcontrol[n].state = MBXstate_idle;
   }
   ESCvar.MBXrun = 1;
   ESCvar.ALstatus = ESCpreop;
   ESCvar.xoe = 0;
   ESCvar.activembxsize = MBX_SIZE;
   file.address_offset = 0;
   file.total_size = 0;
}

static void request(uint8_t opcode, uint32_t number, const char *name)
{
   _FOE *mbx = (_FOE *)MBX;
   size_t name_len = (name != NULL) ? strlen(name) : 0;

   memset(MBX, 0, sizeof(MBX));
   mbx->mbxheader.length = htoes((uint16_t)(ESC_FOEHSIZE + name_len));
   mbx->mbxheader.mbxtype = MBXFOE;
   mbx->foeheader.opcode = opcode;
   if (opcode == FOE_OP_RRQ || opcode == FOE_OP_WRQ)
   {
      mbx->foeheader.password = htoel(number);
      if (name_len != 0)
      {
         memcpy(mbx->filename, name, name_len);
      }
   }
   else
   {
      mbx->foeheader.packetnumber = htoel(number);
   }
   MBXcontrol[0].state = MBXstate_inclaim;
   ESCvar.xoe = 0;
}

static _FOE *response(void)
{
   int n;

   for (n = 1; n < MBXBUFFERS; ++n)
   {
      if (MBXcontrol[n].state == MBXstate_outreq)
      {
         return (_FOE *)&MBX[n * MBX_SIZE];
      }
   }
   return NULL;
}

#if FOE_READ_SUPPORTED
static void release(_FOE *mbx)
{
   size_t offset = (size_t)((uint8_t *)mbx - MBX);

   MBXcontrol[offset / MBX_SIZE].state = MBXstate_idle;
}
#endif

static int fail(const char *message)
{
   fprintf(stderr, "FAIL: %s\n", message);
   return 1;
}

#if !FOE_READ_SUPPORTED
static int test_disabled(void)
{
   _FOE *reply;

   reset();
   request(FOE_OP_RRQ, 0, "demo.bin");
   ESC_foeprocess();
   reply = response();
   if (reply == NULL)
   {
      return fail("disabled RRQ produced no response");
   }
   if (reply->foeheader.opcode != FOE_OP_ERR ||
       etohl(reply->foeheader.errorcode) != FOE_ERR_NOTDEFINED)
   {
      return fail("disabled RRQ did not report FOE_ERR_NOTDEFINED");
   }
   return 0;
}
#endif

#if FOE_READ_SUPPORTED
static int test_read(uint32_t size)
{
   uint8_t expected[FILE_CAPACITY];
   uint32_t offset = 0;
   uint32_t packet = 1;
   uint32_t i;

   for (i = 0; i < FILE_CAPACITY; ++i)
   {
      file_data[i] = (uint8_t)(0xa7u ^ (i * 19u));
   }
   memcpy(expected, file_data, sizeof(expected));
   file.max_data = size;
   reset();
   request(FOE_OP_RRQ, 0, "demo.bin");
   ESC_foeprocess();

   for (;;)
   {
      _FOE *reply = response();
      uint16_t length;
      uint16_t data_len;

      if (reply == NULL)
      {
         break;
      }
      length = etohs(reply->mbxheader.length);
      if (reply->foeheader.opcode != FOE_OP_DATA ||
          etohl(reply->foeheader.packetnumber) != packet ||
          length < ESC_FOEHSIZE)
      {
         return fail("unexpected FoE read reply");
      }
      data_len = (uint16_t)(length - ESC_FOEHSIZE);
      if (data_len > ESC_FOE_DATA_SIZE || offset + data_len > size ||
          memcmp(reply->data, expected + offset, data_len) != 0)
      {
         return fail("FoE read payload mismatch");
      }
      offset += data_len;
      release(reply);
      request(FOE_OP_ACK, packet, NULL);
      ESC_foeprocess();
      ++packet;
   }

   if (offset != size)
   {
      return fail("FoE read did not transfer the whole file");
   }
   return 0;
}

static int test_errors(void)
{
   _FOE *reply;

   reset();
   request(FOE_OP_RRQ, 0, "missing.bin");
   ESC_foeprocess();
   reply = response();
   if (reply == NULL || reply->foeheader.opcode != FOE_OP_ERR ||
       etohl(reply->foeheader.errorcode) != FOE_ERR_NOTFOUND)
   {
      return fail("missing file did not report FOE_ERR_NOTFOUND");
   }

   reset();
   request(FOE_OP_RRQ, 1, "demo.bin");
   ESC_foeprocess();
   reply = response();
   if (reply == NULL || reply->foeheader.opcode != FOE_OP_ERR ||
       etohl(reply->foeheader.errorcode) != FOE_ERR_NORIGHTS)
   {
      return fail("wrong password did not report FOE_ERR_NORIGHTS");
   }

   reset();
   ((_FOE *)MBX)->mbxheader.length = htoes(ESC_FOEHSIZE - 1);
   ((_FOE *)MBX)->mbxheader.mbxtype = MBXFOE;
   MBXcontrol[0].state = MBXstate_inclaim;
   ESC_foeprocess();
   reply = response();
   if (reply == NULL || reply->foeheader.opcode != FOE_OP_ERR ||
       etohl(reply->foeheader.errorcode) != MBXERR_SIZETOOSHORT)
   {
      return fail("short header did not report MBXERR_SIZETOOSHORT");
   }
   return 0;
}
#endif

int main(void)
{
   FOE_config(&config);
   FOE_init();

#if FOE_READ_SUPPORTED
   {
      uint32_t block = MBX_SIZE - (uint32_t)(ESC_MBXHSIZE + ESC_FOEHSIZE);
      uint32_t sizes[] = {
         0, 1, block - 1, block, block + 1, 2 * block, FILE_CAPACITY,
      };
      uint32_t i;

      for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i)
      {
         if (test_read(sizes[i]) != 0)
         {
            return 1;
         }
      }
   }
   if (test_errors() != 0)
   {
      return 1;
   }
   puts("FoE read boundaries: PASS");
#else
   if (test_disabled() != 0)
   {
      return 1;
   }
   puts("FoE read disabled: PASS");
#endif
   return 0;
}
