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

/** Get the value for requested SDO 0x1C32 or 0x1C33 sub index
 *
 * @param[in] index    = value on index of object we want to locate
 * @param[in] subindex = value on subindex of object we want to locate
 * @param[out] buf     = buf to copy value to
 * @param[in] datatype = EtherCAT datatype of buf
 * @return 1 if value was found, else 0.
 */
int COE_getSyncMgrPara (uint16_t index, uint8_t subindex, void * buf, uint16_t datatype)
{
   int result = 0;
   int32_t nidx;
   int32_t snidx;
   const _objd *objd;

   nidx = SDO_findobject(index);

   if(nidx < 0)
   {
      return result;
   }
   else if((index != 0x1c32) && (index != 0x1c33) && (index != 0x10F1))
   {
      return result;
   }

   snidx = SDO_findsubindex(nidx, subindex);

   if(snidx >= 0)
   {
      objd = SDOobjects[nidx].objdesc;

      if((objd[snidx].data != NULL) &&
         (objd[snidx].datatype == datatype))
      {
         memcpy(buf, objd[snidx].data, objd[snidx].bitlength / 8 );
         result = 1;
      }
      else
      {
         if((datatype == DTYPE_UNSIGNED32) &&
            (objd[snidx].datatype == datatype))
         {
            *(uint32_t *)buf = objd[snidx].value;
            result = 1;
         }
         else if((datatype == DTYPE_UNSIGNED16) &&
                 (objd[snidx].datatype == datatype))
         {
            *(uint16_t *)buf = (uint16_t)objd[snidx].value;
            result = 1;

         }
         else if((datatype == DTYPE_UNSIGNED8) &&
                 (objd[snidx].datatype == datatype))
         {
            *(uint8_t *)buf = (uint8_t)objd[snidx].value;
            result = 1;
         }
      }
   }

   return result;
}

/** Init default values for SDO Sync Objects
 *
 */
void COE_initDefaultSyncMgrPara (void)
{
   uint32_t i,j;
   const _objd *objd;
   int32_t n = 0;

   /* 1C3x */
   for(i = 0x1C32; i <= 0x1C33; i ++)
   {
      /* Look if index is present */
      n = SDO_findobject(i);
      if(n < 0)
      {
         continue;
      }

      /* Load default values */
      objd = SDOobjects[n].objdesc;
      for(j = 1; j <= SDOobjects[n].maxsub; j++ )
      {
         if(objd[j].data != NULL)
         {
            *(uint32_t *)objd[j].data = objd[j].value;
         }
         if(objd[j].subindex >= SDOobjects[n].maxsub)
         {
            break;
         }
      }
   }

   /* Look if index is present */
   n = SDO_findobject(0x10F1);
   if(n >= 0)
   {
      /* Load default values */
      objd = SDOobjects[n].objdesc;
      for(j = 1; j <= objd[0].value; j++ )
      {
         if(objd[j].data != NULL)
         {
            *(uint32_t *)objd[j].data = objd[j].value;
         }
      }

   }
}

/** Calculate the size in Bytes of RxPDO or TxPDOs by adding the objects
 * in SyncManager
 * SDO 1C1x.
 *
 * @return size of RxPDO or TxPDOs in Bytes.
 */
uint16_t sizeOfPDO (uint16_t index)
{
   uint16_t size = 0, hobj, l, si, c, sic;
   int16_t nidx;
   const _objd *objd;
   const _objd *objd1c1x;

   nidx = SDO_findobject (index);

   if(nidx < 0)
   {
      return size;
   }
   else if((index != RX_PDO_OBJIDX) && (index != TX_PDO_OBJIDX))
   {
      return size;
   }

   objd1c1x = objd = SDOobjects[nidx].objdesc;

   if (objd1c1x[0].data)
   {
      si = *((uint8_t *) objd1c1x[0].data);
   }
   else
   {
      si = (uint8_t) objd1c1x[0].value;
   }
   if (si)
   {
      for (sic = 1; sic <= si; sic++)
      {
         if (objd1c1x[sic].data)
         {
            hobj = *((uint16_t *) objd1c1x[sic].data);
            hobj = htoes(hobj);
         }
         else
         {
            hobj = (uint16_t) objd1c1x[sic].value;
         }
         nidx = SDO_findobject (hobj);
         if (nidx > 0)
         {
            objd = SDOobjects[nidx].objdesc;
            l = (uint8_t) objd->value;
            for (c = 1; c <= l; c++)
            {
               size += ((objd + c)->value & 0xff);
            }
         }
      }
   }
   return BITS2BYTES (size);
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
         if (((objd + nsub)->access == ATYPE_RW) ||
             (((objd + nsub)->access == ATYPE_RWpre)
              && ((ESCvar.ALstatus & 0x0f) == ESCpreop)))
         {
            /* expedited? */
            if (coesdo->command & COE_EXPEDITED_INDICATOR)
            {
               size = 4 - ((coesdo->command & 0x0c) >> 2);
               mbxdata = &(coesdo->size);
            }
            else
            {
               /* normal upload */
               size = (etohl (coesdo->size) & 0xffff);
               mbxdata = (&(coesdo->size)) + 1;
            }
            actsize = ((objd + nsub)->bitlength + 7) >> 3;
            if (actsize == size)
            {
               if (ESC_pre_objecthandler (index, subindex))
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
                 ESC_objecthandler (index, subindex);
               }
            }
            else
            {
               SDO_abort (index, subindex, ABORT_TYPEMISMATCH);
            }
         }
         else
         {
            if ((objd + nsub)->access == ATYPE_RWpre)
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
      coeres->index = htoel (abortcode);
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
         }
         else if (SDOobjects[nidx].objtype == OTYPE_ARRAY)
         {
            int32_t nsub = SDO_findsubindex (nidx, 0);
            const _objd *objd = SDOobjects[nidx].objdesc;
            coel->datatype = htoes ((objd + nsub)->datatype);
         }
         else
         {
            coel->datatype = htoes (0);
         }
         coel->maxsub = SDOobjects[nidx].maxsub;
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
            coel->access = htoes ((objd + nsub)->access);
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
