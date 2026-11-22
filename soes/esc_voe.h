/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for esc_voe.c
 */

#ifndef __esc_voe__
#define __esc_voe__

#include <cc.h>

/** Application handler for one VoE request.
 *
 * The vendor and type values are the first six payload bytes. The request
 * points at the opaque payload after that header. The reply points at the
 * outgoing payload after its own six-byte header.
 *
 * Return the number of reply payload bytes, or a negative value to reject the
 * request. A zero-length reply is valid and sends only the header.
 */
typedef int32_t (*voe_handler_t) (void * user_arg,
                                  uint32_t vendor,
                                  uint16_t type,
                                  const uint8_t * request,
                                  uint32_t request_len,
                                  uint8_t * reply,
                                  uint32_t reply_capacity);

typedef struct voe_cfg
{
   /** Passed back to the handler unchanged. */
   void * user_arg;
   /** Handler called for every incoming VoE mailbox request. */
   voe_handler_t handler;
} voe_cfg_t;

/** Install the application VoE configuration. The structure is caller-owned. */
void VOE_config (voe_cfg_t * cfg);

/** Process one claimed VoE mailbox request, if any. */
void ESC_voeprocess (void);

#endif
