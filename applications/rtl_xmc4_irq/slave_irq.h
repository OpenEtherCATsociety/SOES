#ifndef __SLAVE_H__
#define __SLAVE_H__

#include "soes/esc.h"
#include "utypes.h"

/**
 * This function gets input values and updates Rb.Buttons
 */
void cb_get_Buttons();

/**
 * This function sets output values according to Wb.LEDgroup0
 */
void cb_set_LEDgroup0();

/**
 * This function sets output values according to Wb.LEDgroup1
 */
void cb_set_LEDgroup1();

/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex);

/**
 * This function sets an application loop callback function.
 */
void set_application_loop_hook(void (*callback)(void));

/**
 * ISR function for SOES application
 */
void soes_isr (void);

/**
 * Main function for SOES application
 */
void soes_irq (void);

/**
 * Initialize the SOES stack
 */
void soes_init_irq (esc_cfg_t * config);

#endif /* __SLAVE_H__ */
