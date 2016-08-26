#ifndef _SOES_H_
#define _SOES_H_

#include <xmc_gpio.h>
#include <xmc_scu.h>

#define P_LED  P4_1
#define P_BTN  P3_4

extern void soes_init (void);
extern void soes_task (void);

#endif

