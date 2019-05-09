#include <stddef.h>
#include "utypes.h"
#include "ecat_options.h"
#include "esc.h"

/* Global variables used by the stack */
uint8_t     MBX[MBXBUFFERS * MAX(MBXSIZE,MBXSIZEBOOT)];
_MBXcontrol MBXcontrol[MBXBUFFERS];
_ESCvar     ESCvar;
_SMmap      SMmap2[MAX_MAPPINGS_SM2];
_SMmap      SMmap3[MAX_MAPPINGS_SM3];

/* Application variables */
_Objects    _Obj;
