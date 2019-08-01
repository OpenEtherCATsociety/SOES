/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for esc_foe.c
 */

#ifndef __esc_foe__
#define __esc_foe__

#include <cc.h>

/** Maximum number of characters allowed in a file name. */
#define FOE_FN_MAX      15

typedef struct foe_writefile_cfg foe_writefile_cfg_t;
struct foe_writefile_cfg
{
   /** Name of file to receive from master */
   const char * name;
   /** Size of file,sizeof data we can recv */
   uint32_t       max_data;
   /** Where to store the data initially */
   uint32_t       dest_start_address;
   /** Current address during write of file */
   uint32_t       address_offset;
   /** Calculated size of file received */
   uint32_t       total_size;
   /** FoE password */
   uint32_t       filepass;
   /** Pointer to application foe write function */
   uint32_t       (*write_function) (foe_writefile_cfg_t * self, uint8_t * data, size_t length);
};

typedef struct foe_cfg
{
   /** Allocate static in caller func to fit buffer_size */
   uint8_t * fbuffer;
   /** Buffer size before we flush to destination */
   uint32_t  buffer_size;
   /** Number of files used in firmware update */
   uint32_t  n_files;
   /** Pointer to files configured to be used by FoE */
   foe_writefile_cfg_t * files;
} foe_cfg_t;

typedef struct CC_PACKED
{
   /** Current FoE state, ex. Waiting for ACK, Waiting for DATA */
   uint8_t  foestate;
   /** Current file buffer position, evaluated against foe file buffer size
    * when to flush
    */
   uint16_t fbufposition;
   /** Frame number in read or write sequence */
   uint32_t foepacket;
   /** Current position in file to be handled by FoE request */
   uint32_t fposition;
   /** Previous position in file to be handled by FoE request */
   uint32_t fprevposition;
   /** End position of allocated disk space for FoE requested file  */
   uint32_t fend;
} _FOEvar;

/* Initializes FoE state. */
void FOE_config (foe_cfg_t * cfg);
void FOE_init (void);
void ESC_foeprocess (void);

#endif
