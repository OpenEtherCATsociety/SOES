#include <kern.h>
#include "slave.h"

/**
 * This function reads physical input values and assigns the corresponding members
 * of Rb.Buttons
 */
void cb_get_Buttons()
{

}

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */


void cb_set_LEDs()
{

}


/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex)
{

}

void main_run(void * arg)
{
  soes_init();

  while(1) {
    soes();
  }
}

int main(void)
{
   rprintf("Hello Main\n");
   task_spawn ("soes", main_run, 8, 2048, NULL);

   return 0;
}

