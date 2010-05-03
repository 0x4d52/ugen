#include "uwemax.h"

// MSP
#include "z_dsp.h"
typedef long t_int32_atomic; // hackery to get this to compile with the new SDK (must check how this is supposed to be done)
#include "buffer.h"

#include "math.h"


//#define MAKE_MSPOBJECT_NAME(name, tilde) (name tilde)
#define MSPOBJECT_NAME MAXOBJECT_NAME "~"
//MAKE_MSPOBJECT_NAME(MAXOBJECT_NAME, "~")