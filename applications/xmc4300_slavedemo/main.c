#include <string.h>

#include "xmc_gpio.h"

#include "soes.h"

int main(void)
{
  soes_init();

  while(1) {
    soes_task();
  }
}

