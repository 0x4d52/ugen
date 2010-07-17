#include "uwemax.h"

// MSP
#include "z_dsp.h"
//typedef long t_int32_atomic; // now seems fixed in SDK5.1.1
//#include "buffer.h"

#include "math.h"


//#define MAKE_MSPOBJECT_NAME(name, tilde) (name tilde)
#define MSPOBJECT_NAME MAXOBJECT_NAME "~"
//MAKE_MSPOBJECT_NAME(MAXOBJECT_NAME, "~")