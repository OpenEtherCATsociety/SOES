#include <stddef.h>
#include "utypes.h"
#include "config.h"
#include "esc.h"

/* Global variables used by the stack */
uint8_t     MBX[MBXBUFFERS * MAX(MBXSIZE,MBXSIZEBOOT)];
_MBXcontrol MBXcontrol[MBXBUFFERS];
_ESCvar     ESCvar;

/* Application variables */
_Rbuffer    Rb;
_Wbuffer    Wb;
_Cbuffer    Cb;
_Mbuffer    Mb;
