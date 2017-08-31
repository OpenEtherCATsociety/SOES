#ifndef __SLAVE_H__
#define __SLAVE_H__

#include "utypes.h"

/**
 * This function reads physical input values and assigns the corresponding members
 * of Rb.Buttons
 */
void cb_get_Buttons();

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */
void cb_set_LEDs();

/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex);

/**
 * This function sets an application loop callback function.
 */
void set_application_loop_hook(void (*callback)(void));

/**
 * Main function for SOES application
 */
void soes (void);

/**
 * Initialize the SOES stack
 */
void soes_init (void);

#endif /* __SLAVE_H__ */
