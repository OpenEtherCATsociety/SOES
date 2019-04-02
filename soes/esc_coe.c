/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * CAN over EtherCAT (CoE) module.
 *
 * SDO read / write and SDO service functions
 */

#include <stddef.h>
#include <string.h>
#include <cc.h>
#include "esc.h"
#include "esc_coe.h"

#define BITS2BYTES(b) ((b + 7) >> 3)

/* Fetch value from object dictionary */
#define OBJ_VALUE_FETCH(v, o) \
   ((o).data ? *(__typeof__ (v) *)(o).data : (__typeof__ (v))(o).value)

/** Search for an object sub-index.
 *
 * @param[in] nidx   = local array index of object we want to find sub-index to
 * @param[in] subindex   = value on sub-index of object we want to locate
 * @return local array index if we succeed, -1 if we didn't find the index.
 */
int16_t SDO_findsubindex (int16_t nidx, uint8_t subindex)
{
   const _objd *objd;
   int16_t n = 0;
   uint8_t maxsub;
   objd = SDOobjects[nidx].objdesc;
   maxsub = SDOobjects[nidx].maxsub;
   while (((objd + n)->subindex < subindex) && (n < maxsub))
   {
      n++;
   }
   if ((objd + n)->subindex != subindex)
   {
      return -1;
   }
   return n;
}

/** Search for an object index matching the wanted value in the Object List.
 *
 * @param[in] index   = value on index of object we want to locate
 * @return local array index if we succeed, -1 if we didn't find the index.
 */
int32_t SDO_findobject (uint16_t index)
{
   int32_t n = 0;
   while (SDOobjects[n].index < index)
   {
      n++;
   }
   if (SDOobjects[n].index != index)
   {
      return -1;
   }
   return n;
}

/**
 * Calculate the size in Bytes of RxPDO or TxPDOs by adding the
 * objects in SyncManager SDO 1C1x.
 *
 * @param[in] index = SM index
 * @param[out] nmappings = number of mapped objects in SM, or -1 if
 *   mapping is invalid
 * @param[out] mappings = list of mapped objects in SM
 * @param[out] max_mappings = max number of mapped objects in SM
 * @return size of RxPDO or TxPDOs in Bytes.
 */
uint16_t sizeOfPDO (uint16_t index, int * nmappings,_SMmap * mappings,
                    int max_mappings)
{
   uint16_t offset = 0, hobj;
   uint8_t si, sic, c;
   int16_t nidx;
   const _objd *objd;
   const _objd *objd1c1x;
   int mapIx = 0;

   if ((index != RX_PDO_OBJIDX) && (index != TX_PDO_OBJIDX))
   {
      return 0;
   }

   nidx = SDO_findobject (index);
   if(nidx < 0)
   {
      return 0;
   }

   objd1c1x = SDOobjects[nidx].objdesc;

   si = OBJ_VALUE_FETCH (si, objd1c1x[0]);
   if (si)
   {
      for (sic = 1; sic <= si; sic++)
      {
         hobj = OBJ_VALUE_FETCH (hobj, objd1c1x[sic]);
         nidx = SDO_findobject (hobj);
         if (nidx >= 0)
         {
            uint8_t maxsub;

            objd = SDOobjects[nidx].objdesc;
            maxsub = OBJ_VALUE_FETCH (maxsub, objd[0]);

            for (c = 1; c <= maxsub; c++)
            {
               uint32_t value = OBJ_VALUE_FETCH (value, objd[c]);
               uint16_t index = value >> 16;
               uint8_t subindex = (value >> 8) & 0xFF;
               uint8_t bitlength = value & 0xFF;
               const _objd * mapping;

               if (mapIx == max_mappings)
               {
                  /* Too many mapped objects */
                  *nmappings = -1;
                  return 0;
               }

               DPRINT ("%04x:%02x @ %d\n", index, subindex, offset);
               nidx = SDO_findobject (index);
               if (nidx >= 0)
               {
                  int16_t nsub;

                  nsub = SDO_findsubindex (nidx, subindex);
                  if (nsub < 0)
                  {
                     mapping = NULL;
                  }

                  mapping = &SDOobjects[nidx].objdesc[nsub];
               }
               else
               {
                  mapping = NULL;
               }

               mappings[mapIx].obj = mapping;
               mappings[mapIx++].offset = offset;

               offset += bitlength;
            }
         }
      }
   }
   *nmappings = mapIx;
   return BITS2BYTES (offset);
}

/** Copy to mailbox.
 *
 * @param[in] source = pointer to source
 * @param[in] dest   = pointer to destination
 * @param[in] size   = Size to copy
 */
void copy2mbx (void *source, void *dest, uint16_t size)
{
   memcpy (dest, source, size);
}

/** Function for sending an SDO Abort reply.
 *
 * @param[in] index      = index of object causing abort reply
 * @param[in] sub-index  = sub-index of object causing abort reply
 * @param[in] abortcode  = abort code to send in reply
 */
void SDO_abort (uint16_t index, uint8_t subindex, uint32_t abortcode)
{
   uint8_t MBXout;
   _COEsdo *coeres;
   MBXout = ESC_claimbuffer ();
   if (MBXout)
   {
      coeres = (_COEsdo *) &MBX[MBXout * ESC_MBXSIZE];
      coeres->mbxheader.length = htoes (COE_DEFAULTLENGTH);
      coeres->mbxheader.mbxtype = MBXCOE;
      coeres->coeheader.numberservice =
         htoes ((0 & 0x01f) | (COE_SDOREQUEST << 12));
      coeres->index = htoes (index);
      coeres->subindex = subindex;
      coeres->command = COE_COMMAND_SDOABORT;
      coeres->size = htoel (abortcode);
      MBXcontrol[MBXout].state = MBXstate_outreq;
   }
}

/** Function for responding on requested SDO Upload, sending the content
 *  requested in a free Mailbox buffer. Depending of size of data expedited,
 *  normal or segmented transfer is used. On error an SDO Abort will be sent.
 */
void SDO_upload (void)
{
   _COEsdo *coesdo, *coeres;
   uint16_t index;
   uint8_t subindex;
   int16_t nidx, nsub;
   uint8_t MBXout;
   uint32_t size;
   uint8_t dss;
   const _objd *objd;
   coesdo = (_COEsdo *) &MBX[0];
   index = etohs (coesdo->index);
   subindex = coesdo->subindex;
   nidx = SDO_findobject (index);
   if (nidx >= 0)
   {
      nsub = SDO_findsubindex (nidx, subindex);
      if (nsub >= 0)
      {
         objd = SDOobjects[nidx].objdesc;
         MBXout = ESC_claimbuffer ();
         if (MBXout)
         {
            coeres = (_COEsdo *) &MBX[MBXout * ESC_MBXSIZE];
            coeres->mbxheader.length = htoes (COE_DEFAULTLENGTH);
            coeres->mbxheader.mbxtype = MBXCOE;
            coeres->coeheader.numberservice =
               htoes ((0 & 0x01f) | (COE_SDORESPONSE << 12));
            size = (objd + nsub)->bitlength;
            /* expedited bits used calculation */
            dss = 0x0c;
            if (size > 8)
            {
               dss = 0x08;
            }
            if (size > 16)
            {
               dss = 0x04;
            }
            if (size > 24)
            {
               dss = 0x00;
            }
            coeres->index = htoes (index);
            coeres->subindex = subindex;
            if (size <= 32)
            {
               /* expedited response i.e. length<=4 bytes */
               coeres->command = COE_COMMAND_UPLOADRESPONSE +
                  COE_SIZE_INDICATOR + COE_EXPEDITED_INDICATOR + dss;
               if ((objd + nsub)->data == NULL)
               {
                  /* use constant value */
                  coeres->size = htoel ((objd + nsub)->value);
               }
               else
               {
                  /* convert bits to bytes */
                  size = (size + 7) >> 3;
                  /* use dynamic data */
                  copy2mbx ((objd + nsub)->data, &(coeres->size), size);
               }
            }
            else
            {
               /* normal response i.e. length>4 bytes */
               coeres->command = COE_COMMAND_UPLOADRESPONSE +
                  COE_SIZE_INDICATOR;
               /* convert bits to bytes */
               size = (size + 7) >> 3;
               coeres->size = htoel (size);
               if ((size + COE_HEADERSIZE) > ESC_MBXDSIZE)
               {
                  /* segmented transfer needed */
                  /* set total size in bytes */
                  ESCvar.frags = size;
                  /* limit to mailbox size */
                  size = ESC_MBXDSIZE - COE_HEADERSIZE;
                  /* number of bytes done */
                  ESCvar.fragsleft = size;
                  /* signal segmented transfer */
                  ESCvar.segmented = MBXSEU;
                  ESCvar.data = (objd + nsub)->data;
               }
               else
               {
                  ESCvar.segmented = 0;
               }
               coeres->mbxheader.length = htoes (COE_HEADERSIZE + size);
               /* use dynamic data */
               copy2mbx ((objd + nsub)->data, (&(coeres->size)) + 1, size);
            }
            MBXcontrol[MBXout].state = MBXstate_outreq;
         }
      }
      else
      {
         SDO_abort (index, subindex, ABORT_NOSUBINDEX);
      }
   }
   else
   {
      SDO_abort (index, subindex, ABORT_NOOBJECT);
   }
   MBXcontrol[0].state = MBXstate_idle;
   ESCvar.xoe = 0;
}

/** Function for handling the following SDO Upload if previous SDOUpload
 * response was flagged it needed to be segmented.
 *
 */
void SDO_uploadsegment (void)
{
   _COEsdo *coesdo, *coeres;
   uint8_t MBXout;
   uint32_t size, offset;
   coesdo = (_COEsdo *) &MBX[0];
   MBXout = ESC_claimbuffer ();
   if (MBXout)
   {
      coeres = (_COEsdo *) &MBX[MBXout * ESC_MBXSIZE];
      offset = ESCvar.fragsleft;
      size = ESCvar.frags - ESCvar.fragsleft;
      coeres->mbxheader.mbxtype = MBXCOE;
      coeres->coeheader.numberservice =
         htoes ((0 & 0x01f) | (COE_SDORESPONSE << 12));
      coeres->command = COE_COMMAND_UPLOADSEGMENT + (coesdo->command & COE_TOGGLEBIT);  // copy toggle bit
      if ((size + COE_SEGMENTHEADERSIZE) > ESC_MBXDSIZE)
      {
         /* more segmented transfer needed */
         /* limit to mailbox size */
         size = ESC_MBXDSIZE - COE_SEGMENTHEADERSIZE;
         /* number of bytes done */
         ESCvar.fragsleft += size;
         coeres->mbxheader.length = htoes (COE_SEGMENTHEADERSIZE + size);
      }
      else
      {
         /* last segment */
         ESCvar.segmented = 0;
         ESCvar.frags = 0;
         ESCvar.fragsleft = 0;
         coeres->command += COE_COMMAND_LASTSEGMENTBIT;
         if (size >= 7)
         {
            coeres->mbxheader.length = htoes (COE_SEGMENTHEADERSIZE + size);
         }
         else
         {
            coeres->command += (7 - size) << 1;
            coeres->mbxheader.length = htoes (COE_DEFAULTLENGTH);
         }
      }
      copy2mbx ((uint8_t *) ESCvar.data + offset, (&(coeres->command)) + 1, size);        //copy to mailbox

      MBXcontrol[MBXout].state = MBXstate_outreq;
   }
   MBXcontrol[0].state = MBXstate_idle;
   ESCvar.xoe = 0;
}


/** Function for handling incoming requested SDO Download, validating the
 * request and sending an response. On error an SDO Abort will be sent.
 */
void SDO_download (void)
{
   _COEsdo *coesdo, *coeres;
   uint16_t index;
   uint8_t subindex;
   int16_t nidx, nsub;
   uint8_t MBXout;
   uint16_t size, actsize;
   const _objd *objd;
   uint32_t *mbxdata;
   uint32_t abort;

   coesdo = (_COEsdo *) &MBX[0];
   index = etohs (coesdo->index);
   subindex = coesdo->subindex;
   nidx = SDO_findobject (index);
   if (nidx >= 0)
   {
      nsub = SDO_findsubindex (nidx, subindex);
      if (nsub >= 0)
      {
         objd = SDOobjects[nidx].objdesc;
         uint8_t access = (objd + nsub)->flags & 0x3f;
         uint8_t state = ESCvar.ALstatus & 0x0f;
         if (access == ATYPE_RW ||
             (access == ATYPE_RWpre && state == ESCpreop))
         {
            /* expedited? */
            if (coesdo->command & COE_EXPEDITED_INDICATOR)
            {
               size = 4 - ((coesdo->command & 0x0c) >> 2);
               mbxdata = &(coesdo->size);
            }
            else
            {
               /* normal download */
               size = (etohl (coesdo->size) & 0xffff);
               mbxdata = (&(coesdo->size)) + 1;
            }
            actsize = ((objd + nsub)->bitlength + 7) >> 3;
            if (actsize == size)
            {
               abort = ESC_pre_objecthandler (
                  index,
                  subindex,
                  mbxdata,
                  size,
                  (objd + nsub)->flags
               );
               if (abort == 0)
               {
                  copy2mbx (mbxdata, (objd + nsub)->data, size);
                  MBXout = ESC_claimbuffer ();
                  if (MBXout)
                  {
                     coeres = (_COEsdo *) &MBX[MBXout * ESC_MBXSIZE];
                     coeres->mbxheader.length = htoes (COE_DEFAULTLENGTH);
                     coeres->mbxheader.mbxtype = MBXCOE;
                     coeres->coeheader.numberservice =
                        htoes ((0 & 0x01f) | (COE_SDORESPONSE << 12));
                     coeres->index = htoes (index);
                     coeres->subindex = subindex;
                     coeres->command = COE_COMMAND_DOWNLOADRESPONSE;
                     coeres->size = htoel (0);
                     MBXcontrol[MBXout].state = MBXstate_outreq;
                  }
                  /* external object write handler */
                  ESC_objecthandler (index, subindex, (objd + nsub)->flags);
               }
               else
               {
                  SDO_abort (index, subindex, abort);
               }
            }
            else
            {
               SDO_abort (index, subindex, ABORT_TYPEMISMATCH);
            }
         }
         else
         {
            if (access == ATYPE_RWpre)
            {
               SDO_abort (index, subindex, ABORT_NOTINTHISSTATE);
            }
            else
            {
               SDO_abort (index, subindex, ABORT_READONLY);
            }
         }
      }
      else
      {
         SDO_abort (index, subindex, ABORT_NOSUBINDEX);
      }
   }
   else
   {
      SDO_abort (index, subindex, ABORT_NOOBJECT);
   }
   MBXcontrol[0].state = MBXstate_idle;
   ESCvar.xoe = 0;
}

/** Function for sending an SDO Info Error reply.
 *
 * @param[in] abortcode  = = abort code to send in reply
 */
void SDO_infoerror (uint32_t abortcode)
{
   uint8_t MBXout;
   _COEobjdesc *coeres;
   MBXout = ESC_claimbuffer ();
   if (MBXout)
   {
      coeres = (_COEobjdesc *) &MBX[MBXout * ESC_MBXSIZE];
      coeres->mbxheader.length = htoes ((uint16_t) 0x0a);
      coeres->mbxheader.mbxtype = MBXCOE;
      coeres->coeheader.numberservice =
         htoes ((0 & 0x01f) | (COE_SDOINFORMATION << 12));
      /* SDO info error request */
      coeres->infoheader.opcode = COE_INFOERROR;
      coeres->infoheader.incomplete = 0;
      coeres->infoheader.reserved = 0x00;
      coeres->infoheader.fragmentsleft = 0;
      coeres->index = (uint16_t)htoel (abortcode);
      coeres->datatype = (uint16_t)(htoel (abortcode) >> 16);
      MBXcontrol[MBXout].state = MBXstate_outreq;
      MBXcontrol[0].state = MBXstate_idle;
      ESCvar.xoe = 0;
   }
}

#define ODLISTSIZE  ((ESC_MBX1_sml - ESC_MBXHSIZE - sizeof(_COEh) - sizeof(_INFOh) - 2) & 0xfffe)

/** Function for handling incoming requested SDO Get OD List, validating the
 * request and sending an response. On error an SDO Info Error will be sent.
 */
void SDO_getodlist (void)
{
   uint16_t frags;
   uint8_t MBXout = 0;
   uint16_t entries = 0;
   uint16_t i, n;
   uint16_t *p;
   _COEobjdesc *coel, *coer;

   while (SDOobjects[entries].index != 0xffff)
   {
      entries++;
   }
   ESCvar.entries = entries;
   frags = ((entries << 1) + ODLISTSIZE - 1);
   frags /= ODLISTSIZE;
   coer = (_COEobjdesc *) &MBX[0];
   /* check for unsupported opcodes */
   if (etohs (coer->index) > 0x01)
   {
      SDO_infoerror (ABORT_UNSUPPORTED);
   }
   else
   {
      MBXout = ESC_claimbuffer ();
   }
   if (MBXout)
   {
      coel = (_COEobjdesc *) &MBX[MBXout * ESC_MBXSIZE];
      coel->mbxheader.mbxtype = MBXCOE;
      coel->coeheader.numberservice =
         htoes ((0 & 0x01f) | (COE_SDOINFORMATION << 12));
      coel->infoheader.opcode = COE_GETODLISTRESPONSE;
      /* number of objects request */
      if (etohs (coer->index) == 0x00)
      {
         coel->index = htoes ((uint16_t) 0x00);
         coel->infoheader.incomplete = 0;
         coel->infoheader.reserved = 0x00;
         coel->infoheader.fragmentsleft = htoes ((uint16_t) 0);
         MBXcontrol[0].state = MBXstate_idle;
         ESCvar.xoe = 0;
         ESCvar.frags = frags;
         ESCvar.fragsleft = frags - 1;
         p = &(coel->datatype);
         *p = htoes (entries);
         p++;
         *p = 0;
         p++;
         *p = 0;
         p++;
         *p = 0;
         p++;
         *p = 0;
         coel->mbxheader.length = htoes (0x08 + (5 << 1));
      }
      /* only return all objects */
      if (etohs (coer->index) == 0x01)
      {
         if (frags > 1)
         {
            coel->infoheader.incomplete = 1;
            ESCvar.xoe = MBXCOE + MBXODL;
            n = ODLISTSIZE >> 1;
         }
         else
         {
            coel->infoheader.incomplete = 0;
            MBXcontrol[0].state = MBXstate_idle;
            ESCvar.xoe = 0;
            n = entries;
         }
         coel->infoheader.reserved = 0x00;
         ESCvar.frags = frags;
         ESCvar.fragsleft = frags - 1;
         coel->infoheader.fragmentsleft = htoes (ESCvar.fragsleft);
         coel->index = htoes ((uint16_t) 0x01);

         p = &(coel->datatype);
         for (i = 0; i < n; i++)
         {
            *p = htoes (SDOobjects[i].index);
            p++;
         }

         coel->mbxheader.length = htoes (0x08 + (n << 1));
      }
      MBXcontrol[MBXout].state = MBXstate_outreq;
   }
}
/** Function for continuing sending left overs from previous requested
 * SDO Get OD List, validating the request and sending an response.
 *
 */
void SDO_getodlistcont (void)
{
   uint8_t MBXout;
   uint16_t i, n, s;
   uint16_t *p;
   _COEobjdesc *coel;

   MBXout = ESC_claimbuffer ();
   if (MBXout)
   {
      coel = (_COEobjdesc *) &MBX[MBXout * ESC_MBXSIZE];
      coel->mbxheader.mbxtype = MBXCOE;
      coel->coeheader.numberservice =
         htoes ((0 & 0x01f) | (COE_SDOINFORMATION << 12));
      coel->infoheader.opcode = COE_GETODLISTRESPONSE;
      s = (ESCvar.frags - ESCvar.fragsleft) * (ODLISTSIZE >> 1);
      if (ESCvar.fragsleft > 1)
      {
         coel->infoheader.incomplete = 1;
         n = s + (ODLISTSIZE >> 1);
      }
      else
      {
         coel->infoheader.incomplete = 0;
         MBXcontrol[0].state = MBXstate_idle;
         ESCvar.xoe = 0;
         n = ESCvar.entries;
      }
      coel->infoheader.reserved = 0x00;
      ESCvar.fragsleft--;
      coel->infoheader.fragmentsleft = htoes (ESCvar.fragsleft);
      /* pointer 2 bytes back to exclude index */
      p = &(coel->index);
      for (i = s; i < n; i++)
      {
         *p = htoes (SDOobjects[i].index);
         p++;
      }
      coel->mbxheader.length = htoes (0x06 + ((n - s) << 1));
      MBXcontrol[MBXout].state = MBXstate_outreq;
   }
}

/** Function for handling incoming requested SDO Get Object Description,
 * validating the request and sending an response. On error an
 * SDO Info Error will be sent.
 */
void SDO_getod (void)
{
   uint8_t MBXout;
   uint16_t index;
   int32_t nidx;
   uint8_t *d;
   const uint8_t *s;
   uint8_t n = 0;
   _COEobjdesc *coer, *coel;
   coer = (_COEobjdesc *) &MBX[0];
   index = etohs (coer->index);
   nidx = SDO_findobject (index);
   if (nidx >= 0)
   {
      MBXout = ESC_claimbuffer ();
      if (MBXout)
      {
         coel = (_COEobjdesc *) &MBX[MBXout * ESC_MBXSIZE];
         coel->mbxheader.mbxtype = MBXCOE;
         coel->coeheader.numberservice =
            htoes ((0 & 0x01f) | (COE_SDOINFORMATION << 12));
         coel->infoheader.opcode = COE_GETODRESPONSE;
         coel->infoheader.incomplete = 0;
         coel->infoheader.reserved = 0x00;
         coel->infoheader.fragmentsleft = htoes (0);
         coel->index = htoes (index);
         if (SDOobjects[nidx].objtype == OTYPE_VAR)
         {
            int32_t nsub = SDO_findsubindex (nidx, 0);
            const _objd *objd = SDOobjects[nidx].objdesc;
            coel->datatype = htoes ((objd + nsub)->datatype);
            coel->maxsub = SDOobjects[nidx].maxsub;
         }
         else if (SDOobjects[nidx].objtype == OTYPE_ARRAY)
         {
            int32_t nsub = SDO_findsubindex (nidx, 0);
            const _objd *objd = SDOobjects[nidx].objdesc;
            coel->datatype = htoes ((objd + nsub)->datatype);
            coel->maxsub = SDOobjects[nidx].objdesc->value;
         }
         else
         {
            coel->datatype = htoes (0);
            coel->maxsub = SDOobjects[nidx].objdesc->value;
         }
         coel->objectcode = SDOobjects[nidx].objtype;
         s = (uint8_t *) SDOobjects[nidx].name;
         d = (uint8_t *) &(coel->name);
         while (*s && (n < (ESC_MBXDSIZE - 0x0c)))
         {
            *d = *s;
            n++;
            s++;
            d++;
         }
         *d = *s;
         coel->mbxheader.length = htoes ((uint16_t) 0x0c + n);
         MBXcontrol[MBXout].state = MBXstate_outreq;
         MBXcontrol[0].state = MBXstate_idle;
         ESCvar.xoe = 0;
      }
   }
   else
   {
      SDO_infoerror (ABORT_NOOBJECT);
   }
}

/** Function for handling incoming requested SDO Get Entry Description,
 * validating the request and sending an response. On error an
 * SDO Info Error will be sent.
 */
void SDO_geted (void)
{
   uint8_t MBXout;
   uint16_t index;
   int32_t nidx, nsub;
   uint8_t subindex;
   uint8_t *d;
   const uint8_t *s;
   const _objd *objd;
   uint8_t n = 0;
   _COEentdesc *coer, *coel;
   coer = (_COEentdesc *) &MBX[0];
   index = etohs (coer->index);
   subindex = coer->subindex;
   nidx = SDO_findobject (index);
   if (nidx >= 0)
   {
      nsub = SDO_findsubindex (nidx, subindex);
      if (nsub >= 0)
      {
         objd = SDOobjects[nidx].objdesc;
         MBXout = ESC_claimbuffer ();
         if (MBXout)
         {
            coel = (_COEentdesc *) &MBX[MBXout * ESC_MBXSIZE];
            coel->mbxheader.mbxtype = MBXCOE;
            coel->coeheader.numberservice =
               htoes ((0 & 0x01f) | (COE_SDOINFORMATION << 12));
            coel->infoheader.opcode = COE_ENTRYDESCRIPTIONRESPONSE;
            coel->infoheader.incomplete = 0;
            coel->infoheader.reserved = 0x00;
            coel->infoheader.fragmentsleft = htoes ((uint16_t) 0);
            coel->index = htoes (index);
            coel->subindex = subindex;
            coel->valueinfo = COE_VALUEINFO_ACCESS +
               COE_VALUEINFO_OBJECT + COE_VALUEINFO_MAPPABLE;
            coel->datatype = htoes ((objd + nsub)->datatype);
            coel->bitlength = htoes ((objd + nsub)->bitlength);
            coel->access = htoes ((objd + nsub)->flags);
            s = (uint8_t *) (objd + nsub)->name;
            d = (uint8_t *) &(coel->name);
            while (*s && (n < (ESC_MBXDSIZE - 0x10)))
            {
               *d = *s;
               n++;
               s++;
               d++;
            }
            *d = *s;
            coel->mbxheader.length = htoes ((uint16_t) 0x10 + n);
            MBXcontrol[MBXout].state = MBXstate_outreq;
            MBXcontrol[0].state = MBXstate_idle;
            ESCvar.xoe = 0;
         }
      }
      else
      {
         SDO_infoerror (ABORT_NOSUBINDEX);
      }
   }
   else
   {
      SDO_infoerror (ABORT_NOOBJECT);
   }
}

/** Main CoE function checking the status on current mailbox buffers carrying
 * data, distributing the mailboxes to appropriate CoE functions.
 * On Error an MBX_error or SDO Abort will be sent depending on error cause.
 */
void ESC_coeprocess (void)
{
   _MBXh *mbh;
   _COEsdo *coesdo;
   _COEobjdesc *coeobjdesc;
   uint8_t service;
   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   if (!ESCvar.xoe && (MBXcontrol[0].state == MBXstate_inclaim))
   {
      mbh = (_MBXh *) &MBX[0];
      if (mbh->mbxtype == MBXCOE)
      {
         if (etohs (mbh->length) < COE_MINIMUM_LENGTH)
         {
            MBX_error (MBXERR_INVALIDSIZE);
         }
         else
         {
            ESCvar.xoe = MBXCOE;
         }
      }
   }
   if ((ESCvar.xoe == (MBXCOE + MBXODL)) && (!ESCvar.mbxoutpost))
   {
      /* continue get OD list */
      SDO_getodlistcont ();
   }
   if (ESCvar.xoe == MBXCOE)
   {
      coesdo = (_COEsdo *) &MBX[0];
      coeobjdesc = (_COEobjdesc *) &MBX[0];
      service = etohs (coesdo->coeheader.numberservice) >> 12;
      /* initiate SDO upload request */
      if ((service == COE_SDOREQUEST)
          && (coesdo->command == COE_COMMAND_UPLOADREQUEST)
          && (etohs (coesdo->mbxheader.length) == 0x0a))
      {
         SDO_upload ();
      }
      /* SDO upload segment request */
      if ((service == COE_SDOREQUEST)
          && ((coesdo->command & 0xef) == COE_COMMAND_UPLOADSEGREQ)
          && (etohs (coesdo->mbxheader.length) == 0x0a)
          && (ESCvar.segmented == MBXSEU))
      {
         SDO_uploadsegment ();
      }
      /* initiate SDO download request */
      else
      {
         if ((service == COE_SDOREQUEST) && ((coesdo->command & 0xf1) == 0x21))
         {
            SDO_download ();
         }
         /* initiate SDO get OD list */
         else
         {
            if ((service == COE_SDOINFORMATION)
                && (coeobjdesc->infoheader.opcode == 0x01))
            {
               SDO_getodlist ();
            }
            /* initiate SDO get OD */
            else
            {
               if ((service == COE_SDOINFORMATION)
                   && (coeobjdesc->infoheader.opcode == 0x03))
               {
                  SDO_getod ();
               }
               /* initiate SDO get ED */
               else
               {
                  if ((service == COE_SDOINFORMATION)
                      && (coeobjdesc->infoheader.opcode == 0x05))
                  {
                     SDO_geted ();
                  }
                  else
                  {
                     /* COE not recognised above */
                     if (ESCvar.xoe == MBXCOE)
                     {
                        if (service == 0)
                        {
                           MBX_error (MBXERR_INVALIDHEADER);
                        }
                        else
                        {
                           SDO_abort (etohs (coesdo->index), coesdo->subindex, ABORT_UNSUPPORTED);
                        }
                        MBXcontrol[0].state = MBXstate_idle;
                        ESCvar.xoe = 0;
                     }
                  }
               }
            }
         }
      }
   }
}

/**
 * Get value from bitmap
 *
 * This function gets a value from a bitmap.
 *
 * @param[in] bitmap = bitmap containing value
 * @param[in] offset = start offset
 * @param[in] length = number of bits to get
 * @return bitslice value
 */
static uint64_t COE_bitsliceGet (uint64_t * bitmap, int offset, int length)
{
   const int word_offset = offset / 64;
   const int bit_offset = offset % 64;
   const uint64_t mask = (length == 64) ? UINT64_MAX : (1ULL << length) - 1;
   uint64_t w0;
   uint64_t w1 = 0;

   /* Get the least significant word */
   w0 = bitmap[word_offset];
   w0 = w0 >> bit_offset;

   /* Get the most significant word, if required */
   if (length + bit_offset > 64)
   {
      w1 = bitmap[word_offset + 1];
      w1 = w1 << (64 - bit_offset);
   }

   w0 = (w1 | w0);
   return (w0 & mask);
}

/**
 * Set value in bitmap
 *
 * This function sets a value in a bitmap.
 *
 * @param[in] bitmap = bitmap to contain value
 * @param[in] offset = start offset
 * @param[in] length = number of bits to set
 * @param[in] value  = value to set
 */
static void COE_bitsliceSet (uint64_t * bitmap, int offset, int length,
                             uint64_t value)
{
   const int word_offset = offset / 64;
   const int bit_offset = offset % 64;
   const uint64_t mask = (length == 64) ? UINT64_MAX : (1ULL << length) - 1;
   const uint64_t mask0 = mask << bit_offset;
   uint64_t v0 = value << bit_offset;
   uint64_t w0 = bitmap[word_offset];

   /* Set the least significant word */
   w0 = (w0 & ~mask0) | (v0 & mask0);
   bitmap[word_offset] = w0;

   /* Set the most significant word, if required */
   if (length + bit_offset > 64)
   {
      const uint64_t mask1 = mask >> (64 - bit_offset);
      uint64_t v1 = value >> (64 - bit_offset);
      uint64_t w1 = bitmap[word_offset + 1];

      w1 = (w1 & ~mask1) | (v1 & mask1);
      bitmap[word_offset + 1] = w1;
   }
}

/**
 * Get object value
 *
 * This function atomically gets an object value.
 *
 * @param[in] obj   = object description
 * @return object value
 */
static uint64_t COE_getValue (const _objd * obj)
{
   uint64_t value = 0;

   /* TODO: const data */

   switch(obj->datatype)
   {
   case DTYPE_BIT1:
   case DTYPE_BIT2:
   case DTYPE_BIT3:
   case DTYPE_BIT4:
   case DTYPE_BIT5:
   case DTYPE_BIT6:
   case DTYPE_BIT7:
   case DTYPE_BIT8:
   case DTYPE_BOOLEAN:
   case DTYPE_UNSIGNED8:
   case DTYPE_INTEGER8:
      value = *(uint8_t *)obj->data;
      break;

   case DTYPE_UNSIGNED16:
   case DTYPE_INTEGER16:
      value = *(uint16_t *)obj->data;
      break;

   case DTYPE_REAL32:
   case DTYPE_UNSIGNED32:
   case DTYPE_INTEGER32:
      value = *(uint32_t *)obj->data;
      break;

   case DTYPE_REAL64:
   case DTYPE_UNSIGNED64:
   case DTYPE_INTEGER64:
      /* FIXME: must be atomic */
      value = *(uint64_t *)obj->data;
      break;

   default:
      CC_ASSERT (0);
   }

   return value;
}

/**
 * Set object value
 *
 * This function atomically sets an object value.
 *
 * @param[in] obj   = object description
 * @param[in] value = new value
 */
static void COE_setValue (const _objd * obj, uint64_t value)
{
   switch(obj->datatype)
   {
   case DTYPE_BIT1:
   case DTYPE_BIT2:
   case DTYPE_BIT3:
   case DTYPE_BIT4:
   case DTYPE_BIT5:
   case DTYPE_BIT6:
   case DTYPE_BIT7:
   case DTYPE_BIT8:
   case DTYPE_BOOLEAN:
   case DTYPE_UNSIGNED8:
   case DTYPE_INTEGER8:
      *(uint8_t *)obj->data = value & UINT8_MAX;
      break;

   case DTYPE_UNSIGNED16:
   case DTYPE_INTEGER16:
      *(uint16_t *)obj->data = value & UINT16_MAX;
      break;

   case DTYPE_REAL32:
   case DTYPE_UNSIGNED32:
   case DTYPE_INTEGER32:
      *(uint32_t *)obj->data = value & UINT32_MAX;
      break;

   case DTYPE_REAL64:
   case DTYPE_UNSIGNED64:
   case DTYPE_INTEGER64:
      /* FIXME: must be atomic */
      *(uint64_t *)obj->data = value;
      break;

   default:
      DPRINT ("ignored\n");
      break;
   }
}

/**
 * Init default values for SDO objects
 */
void COE_initDefaultValues (void)
{
   int i;
   const _objd *objd;
   int n;
   uint8_t maxsub;

   /* Set default values from object descriptor */
   for (n = 0; SDOobjects[n].index != 0xffff; n++)
   {
      objd = SDOobjects[n].objdesc;
      maxsub = SDOobjects[n].maxsub;

      i = 0;
      do
      {
         if (objd[i].data != NULL)
         {
            /* TODO: bitlength > 64 */
            COE_setValue (&objd[i], objd[i].value);
            DPRINT ("%04x:%02x = %x\n", SDOobjects[n].index, objd[i].subindex, objd[i].value);
         }
      } while (objd[i++].subindex < maxsub);
   }

   /* Let application override default values */
   if (ESCvar.set_defaults_hook != NULL)
   {
      ESCvar.set_defaults_hook();
   }
}

/**
 * Pack process data
 *
 * This function reads mapped objects and constructs the process data
 * inputs (TXPDO).
 *
 * @param[in] buffer     = input process data
 * @param[in] nmappings  = number of mappings in sync manager
 * @param[in] mappings   = list of mapped objects in sync manager
 */
void COE_pdoPack (uint8_t * buffer, int nmappings, _SMmap * mappings)
{
   int ix;

   /* Check that buffer is aligned on 64-bit boundary */
   CC_ASSERT (((uintptr_t)buffer & 0x07) == 0);

   for (ix = 0; ix < nmappings; ix++)
   {
      const _objd * obj = mappings[ix].obj;
      uint16_t offset = mappings[ix].offset;

      if (obj != NULL)
      {
         if (obj->bitlength > 64)
         {
            memcpy (
               &buffer[BITS2BYTES (offset)],
               obj->data,
               BITS2BYTES (obj->bitlength)
            );
         }
         else
         {
            /* Atomically get object value */
            uint64_t value = COE_getValue (obj);
            COE_bitsliceSet (
               (uint64_t *)buffer,
               offset,
               obj->bitlength,
               value
            );
         }
      }
   }
}

/**
 * Unpack process data
 *
 * This function unpacks process data output (RXPDO) and writes to the
 * mapped objects.
 *
 * @param[in] buffer    = output process data
 * @param[in] nmappings = number of mappings in sync manager
 * @param[in] mappings  = list of mapped objects in sync manager
 */
void COE_pdoUnpack (uint8_t * buffer, int nmappings, _SMmap * mappings)
{
   int ix;

   /* Check that buffer is aligned on 64-bit boundary */
   CC_ASSERT (((uintptr_t)buffer & 0x07) == 0);

   for (ix = 0; ix < nmappings; ix++)
   {
      const _objd * obj = mappings[ix].obj;
      uint16_t offset = mappings[ix].offset;

      if (obj != NULL)
      {
         if (obj->bitlength > 64)
         {
            memcpy (
               obj->data,
               &buffer[BITS2BYTES (offset)],
               BITS2BYTES (obj->bitlength)
            );
         }
         else
         {
            /* Atomically set object value */
            uint64_t value = COE_bitsliceGet (
               (uint64_t *)buffer,
               offset,
               obj->bitlength
            );
            COE_setValue (obj, value);
         }
      }
   }
}

/**
 * Fetch max subindex
 *
 * This function fetches the value of subindex 0 (max subindex).
 *
 * @param[in] index = object index
 */
uint8_t COE_maxSub (uint16_t index)
{
   int nidx;
   uint8_t maxsub;

   nidx = SDO_findobject (index);
   if (nidx == -1)
      return 0;

   maxsub = OBJ_VALUE_FETCH (maxsub, SDOobjects[nidx].objdesc[0]);
   return maxsub;
}
