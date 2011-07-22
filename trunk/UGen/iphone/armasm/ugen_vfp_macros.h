/*
VFP math library for the iPhone / iPod touch

Copyright (c) 2007-2008 Wolfgang Engel and Matthias Grundmann
http://code.google.com/p/vfpmathlibrary/

 
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __UGEN_VFP_MACROS_H__
#define __UGEN_VFP_MACROS_H__

#include "ugen_vfp_clobbers.h"

// Usage for any VFP routine is:

/*
    asm volatile (VFP_SWITCH_TO_ARM

                  <--  your code goes here

                  VFP_SWITCH_TO_THUMB
                  : <list of modified INPUT registers>
                  : <list of input registers>
                  : "r0", <additional registers you used>
                  );
*/

// NOTE: Lots of VFP macros overwrite register r0, therefore you have to make sure 
//       to include "r0" in the list of registers used, as in above example.

//#define USE_THUMB
  


//#define ASM_WRAPPER_R0(x)		asm volatile (x : : : "r0" );
//#define ASM_WRAPPER_R0_PC(x)	asm volatile (x : : : "r0", "pc" );
	

// Switches to from THUMB to ARM mode.
#ifdef USE_THUMB
	#define VFP_SWITCH_TO_ARM ".align 4               \n\t" \
							  "mov     r0, pc         \n\t" \
							  "bx      r0             \n\t" \
							  ".arm                   \n\t" 
	#define VFP_SWITCH_TO_ARM_ASM asm volatile(VFP_SWITCH_TO_ARM : : : "r0", "pc");
#else
	#define VFP_SWITCH_TO_ARM  ""
	#define VFP_SWITCH_TO_ARM_ASM asm volatile(VFP_SWITCH_TO_ARM : : : VFP_CLOBBER_S0_S31)
#endif


// Switches from ARM to THUMB mode.
#ifdef USE_THUMB
	#define VFP_SWITCH_TO_THUMB "ssadd     r0, pc, #1     \n\t" \
								"bx      r0             \n\t" \
								".thumb                 \n\t" 
	#define VFP_SWITCH_TO_THUMB_ASM asm volatile(VFP_SWITCH_TO_THUMB : : : "r0", "pc");
#else
	#define VFP_SWITCH_TO_THUMB  ""
	#define VFP_SWITCH_TO_THUMB_ASM asm volatile(VFP_SWITCH_TO_THUMB : : : VFP_CLOBBER_S0_S31)
#endif


// NOTE: Both VFP_VECTOR_LENGTH* macros will stall the FP unit, 
//       until all currently processed operations have been executed.
//       Call wisely.
//       FP operations (except load/stores) will interpret a command like
//	 fadds s8, s16, s24
//	 AS
//	 fadds {s8-s11}, {s16-s19}, {s24-s27} in case length is set to zero.


// Sets length and stride to 0.
#define VFP_VECTOR_LENGTH_ZERO "fmrx    r0, fpscr            \n\t" \
                               "bic     r0, r0, #0x00370000  \n\t" \
                               "fmxr    fpscr, r0            \n\t" 
#define VFP_VECTOR_LENGTH_ZERO_ASM asm volatile (VFP_VECTOR_LENGTH_ZERO : : : "r0");
  


// Set vector length. VEC_LENGTH has to be bitween 0 for length 1 and 3 for length 4.
#define VFP_VECTOR_LENGTH(VEC_LENGTH) "fmrx    r0, fpscr                         \n\t" \
                                      "bic     r0, r0, #0x00370000               \n\t" \
                                      "orr     r0, r0, #0x000" #VEC_LENGTH "0000 \n\t" \
                                      "fmxr    fpscr, r0                         \n\t"
#define VFP_VECTOR_LENGTH_ASM(VEC_LENGTH) asm volatile (VFP_VECTOR_LENGTH(VEC_LENGTH) : : : "r0");


// Fixed vector operation for vectors of length 1, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_1_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (	VFP_SWITCH_TO_ARM \
					"fldmias  %1, {s8}        \n\t" \
					"fldmias  %2, {s16}       \n\t" \
					VFP_OP  " s8, s8, s16   \n\t" \
					"fstmias  %0, {s8}        \n\t" \
					VFP_SWITCH_TO_THUMB \
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "pc", "memory", "s8", "s16" \
					);

#define VFP_FIXED_1_UNARY_OP3(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1, {s8}        \n\t" \
					VFP_OP  " s8, s8		  \n\t" \
					"fstmias  %0, {s8}        \n\t" \
					: \
					: "r" (P_DST), "r" (P_SRC_1) \
					: "memory", "s8" \
					);

#define VFP_FIXED_1_UNARY_OP4(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8}        \n\t" \
					VFP_OP  " s8, s8		  \n\t" \
					"fstmias  %0!, {s8}        \n\t" \
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "memory", "s8" \
					);



// Fixed vector operation for vectors of length 2, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_2_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s9}   \n\t" \
                "fldmias  %2, {s16-s17} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                "fstmias  %0, {s8-s9}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0", "pc", "memory" \
                );

// Fixed vector operation for vectors of length 3, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_3_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s10}  \n\t" \
                "fldmias  %2, {s16-s18} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                VFP_OP  " s10, s10, s18 \n\t" \
                "fstmias  %0, {s8-s10}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0", "cc", "pc", "memory", VFP_CLOBBER_S8_S10, VFP_CLOBBER_S16_S18 \
                );

// Fixed vector operation for vectors of length 4, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_4_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s11}  \n\t" \
                "fldmias  %2, {s16-s19} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                VFP_OP  " s10, s10, s18 \n\t" \
                VFP_OP  " s11, s11, s19 \n\t" \
                "fstmias  %0, {s8-s11}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0", "cc", "pc", "memory", VFP_CLOBBER_S8_S11, VFP_CLOBBER_S16_S19 \
                );

// suffixes:
// OP2 doesn't call VFP_OP multiple times
// OP3 as OP2 but also doesn't set/reset the thumb/arm status
// OP4 as OP3 but also increments the buffer pointers
// so e.g., VFP_FIXED_8_VECTOR_OP4 is fastest here

#define VFP_FIXED_4_VECTOR_OP2(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	VFP_SWITCH_TO_ARM				\
					"fldmias  %1, {s8-s11}  \n\t"	\
					"fldmias  %2, {s16-s19} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0, {s8-s11}   \n\t"	\
					VFP_SWITCH_TO_THUMB				\
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "pc", "memory", VFP_CLOBBER_S8_S11, VFP_CLOBBER_S16_S19 \
					);

#define VFP_FIXED_4_VECTOR_OP3(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1, {s8-s11}  \n\t"	\
					"fldmias  %2, {s16-s19} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0, {s8-s11}   \n\t"	\
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S11, VFP_CLOBBER_S16_S19 \
					);

#define VFP_FIXED_4_VECTOR_OP4(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s11}  \n\t"	\
					"fldmias  %2!, {s16-s19} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0!, {s8-s11}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1), "=r" (P_SRC_2) \
					: "0" (P_DST), "1" (P_SRC_1), "2" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S11, VFP_CLOBBER_S16_S19 \
					);

#define VFP_FIXED_8_VECTOR_OP3(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1, {s8-s15}  \n\t"	\
					"fldmias  %2, {s16-s23} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0, {s8-s15}   \n\t"	\
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_8_VECTOR_OP3_NV(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm          (	"fldmias  %1, {s8-s15}  \n\t"	\
					"fldmias  %2, {s16-s23} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0, {s8-s15}   \n\t"	\
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);


#define VFP_FIXED_8_VECTOR_OP4(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}  \n\t"	\
					"fldmias  %2!, {s16-s23} \n\t"	\
					VFP_OP  " s8, s8, s16   \n\t"	\
					"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1), "=r" (P_SRC_2) \
					: "0" (P_DST), "1" (P_SRC_1), "2" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_16_VECTOR_OP4(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s23}  \n\t"	\
					"fldmias  %2!, {s24-s31} \n\t"	\
					VFP_OP  " s8, s8, s24   \n\t"	\
					"fldmias  %2!, {s24-s31} \n\t"	\
					VFP_OP  " s8, s8, s24   \n\t"	\
					"fstmias  %0!, {s8-s23}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1), "=r" (P_SRC_2) \
					: "0" (P_DST), "1" (P_SRC_1), "2" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S31 \
					);


#define VFP_FIXED_8_VECTOR_MULADD_4(P_IN, P_MUL, P_ADD, P_OUT) \
	asm volatile (	"fldmias  %1!, {s8-s15}		\n\t"	\
					"fldmias  %2!, {s16-s23}	\n\t"	\
					"fldmias  %3!, {s24-s31}	\n\t"	\
					"fmacs s8, s16, s24			\n\t"	\
					"fstmias  %0!, {s8-s15}		\n\t"	\
					: "=r" (P_OUT), "=r" (P_ADD), "=r" (P_MUL), "=r" (P_IN) \
					: "0" (P_OUT), "1" (P_ADD), "2" (P_MUL), "3" (P_IN) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S31 \
					);


#define VFP_FIXED_8_UNARY_VECTOR_OP4(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}   \n\t"	\
					VFP_OP  " s8, s8          \n\t"	\
					"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_8_UNARY_VECTOR_SQUARED_4(P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}   \n\t"	\
					"fmuls s8, s8, s8          \n\t"	\
					"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_8_UNARY_VECTOR_CUBED_4(P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}   \n\t"	\
					"fmuls s16, s8, s8        \n\t" \
					"fmuls s8, s8, s16        \n\t" \
					"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_1_VECTOR_CPY_4(P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8}		\n\t"	\
					"fstmias  %0!, {s8}		\n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", "s8" \
					);


#define VFP_FIXED_8_VECTOR_CPY_4(P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}		\n\t"	\
					"fstmias  %0!, {s8-s15}		\n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_16_VECTOR_CPY_4(P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}		\n\t"	\
					"fldmias  %1!, {s16-s23}	\n\t"	\
					"fstmias  %0!, {s8-s15}		\n\t"	\
					"fstmias  %0!, {s16-s23}	\n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);


#define VFP_FIXED_8_VECTOR_SPLAT_LOAD(P_VAL) \
	asm volatile (	"fldmias  %0, {s8-s15}   \n\t"	\
					: \
					: "r" (P_VAL) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);


#define VFP_FIXED_8_VECTOR_SPLAT_SAVE(P_DST) \
	asm volatile (	"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST)\
					: "0" (P_DST) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_16_VECTOR_SPLAT_SAVE(P_DST) \
	asm volatile (	"fstmias  %0!, {s8-s15}   \n\t"	\
					"fstmias  %0!, {s8-s15}   \n\t"	\
					: "=r" (P_DST) \
					: "0" (P_DST) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_8_VECTOR_SPLAT(P_VAL, P_DST) \
	asm volatile (	"flds s0, [%1]				\n\t" \
					"fcpys s8, s0				\n\t" \
					"fstmias  %0!, {s8-s15}		\n\t" \
					: "=r" (P_DST) \
					: "0" (P_DST), "r" (P_VAL) \
					: "r0", "cc", "memory", "s0", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_16_VECTOR_SPLAT(P_VAL, P_DST) \
	asm volatile (	"flds  s0, [%1]				\n\t" \
					"fcpys s8, s0				\n\t" \
					"fcpys s16, s0				\n\t" \
					"fstmias  %0!, {s8-s15}		\n\t" \
					"fstmias  %0!, {s16-s23}	\n\t" \
					: "=r" (P_DST), \
					: "0" (P_DST), "r" (P_VAL) \
					: "r0", "cc", "memory", "s0", VFP_CLOBBER_S8_S23 \
					);


#define VFP_FIXED_8_VECTOR_SPLAT_OP_S8(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s16-s23}	\n\t"	\
					VFP_OP " s16, s8, s16		\n\t"	\
					"fstmias  %0!, {s16-s23}	\n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);


// note 16 version do vectors of 8 twice = 16 so vector size should be set to 7 not 15
#define VFP_FIXED_16_UNARY_VECTOR_OP4(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}   \n\t"	\
					VFP_OP  " s8, s8		  \n\t"	\
					"fldmias  %1!, {s16-s23}  \n\t"	\
					"fstmias  %0!, {s8-s15}   \n\t"	\
					VFP_OP  " s16, s16		  \n\t"	\
					"fstmias  %0!, {s16-s23}  \n\t"	\
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_8_SCALAR_UNARY_OP4(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}      \n\t" \
					VFP_OP  " s8, s8			 \n\t" \
					VFP_OP  " s9, s9			 \n\t" \
					VFP_OP  " s10, s10			 \n\t" \
					VFP_OP  " s11, s11			 \n\t" \
					VFP_OP  " s12, s12			 \n\t" \
					VFP_OP  " s13, s13			 \n\t" \
					VFP_OP  " s14, s14			 \n\t" \
					VFP_OP  " s15, s15			 \n\t" \
					"fstmias  %0!, {s8-s15}      \n\t" \
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S15 \
					);

#define VFP_FIXED_16_SCALAR_UNARY_OP4(VFP_OP, P_SRC_1, P_DST) \
	asm volatile (	"fldmias  %1!, {s8-s15}      \n\t" \
					"fldmias  %1!, {s16-s23}     \n\t" \
					VFP_OP  " s8, s8			 \n\t" \
					VFP_OP  " s9, s9			 \n\t" \
					VFP_OP  " s10, s10			 \n\t" \
					VFP_OP  " s11, s11			 \n\t" \
					VFP_OP  " s12, s12			 \n\t" \
					VFP_OP  " s13, s13			 \n\t" \
					VFP_OP  " s14, s14			 \n\t" \
					VFP_OP  " s15, s15			 \n\t" \
					VFP_OP  " s16, s16			 \n\t" \
					VFP_OP  " s17, s17			 \n\t" \
					VFP_OP  " s18, s18			 \n\t" \
					VFP_OP  " s19, s19			 \n\t" \
					VFP_OP  " s20, s20			 \n\t" \
					VFP_OP  " s21, s21			 \n\t" \
					VFP_OP  " s22, s22			 \n\t" \
					VFP_OP  " s23, s23			 \n\t" \
					"fstmias  %0!, {s8-s15}      \n\t" \
					"fstmias  %0!, {s16-s23}     \n\t" \
					: "=r" (P_DST), "=r" (P_SRC_1) \
					: "0" (P_DST), "1" (P_SRC_1) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

// here src2 should contain an array of multipliers to scale the floats before int conversion
// e.g. for audio float -> short: src2[] = { 32767.f, 32767.f, 32767.f, 32767.f .... etc }
#define VFP_FIXED_8_VECTOR_MUL_FTI_3(P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1, {s8-s15}		\n\t" \
					"fldmias  %2, {s16-s23}		\n\t" \
					"fmuls s8, s8, s16			\n\t" \
					"ftosis s8, s8				\n\t" \
					"ftosis s9, s9				\n\t" \
					"ftosis s10, s10			\n\t" \
					"ftosis s11, s11			\n\t" \
					"ftosis s12, s12			\n\t" \
					"ftosis s13, s13			\n\t" \
					"ftosis s14, s14			\n\t" \
					"ftosis s15, s15			\n\t" \
					"fstmias  %0, {s8-s15}		\n\t" \
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_16_VECTOR_MUL_SCALAR_FTI(P_VAL, P_SRC, P_DSTa, P_DSTb) \
	asm volatile (	"flds  s0, [%2]			\n\t" \
					"fldmias  %3!, {s8-s15}		\n\t" \
					"fldmias  %3!, {s16-s23}	\n\t" \
					"fmuls s8, s8, s0			\n\t" \
					"fmuls s16, s16, s0			\n\t" \
					"ftosis s8, s8				\n\t" \
					"ftosis s9, s9				\n\t" \
					"ftosis s10, s10			\n\t" \
					"ftosis s11, s11			\n\t" \
					"ftosis s12, s12			\n\t" \
					"ftosis s13, s13			\n\t" \
					"ftosis s14, s14			\n\t" \
					"ftosis s15, s15			\n\t" \
					"ftosis s16, s16			\n\t" \
					"ftosis s17, s17			\n\t" \
					"ftosis s18, s18			\n\t" \
					"ftosis s19, s19			\n\t" \
					"ftosis s20, s20			\n\t" \
					"ftosis s21, s21			\n\t" \
					"ftosis s22, s22			\n\t" \
					"ftosis s23, s23			\n\t" \
					"fstmias  %0, {s8-s15}		\n\t" \
					"fstmias  %1, {s16-s23}	\n\t" \
					: \
					: "r" (P_DSTa), "r" (P_DSTb), "r" (P_VAL), "r" (P_SRC) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_16_VECTOR_MUL_SCALAR_FTI_LOAD(P_VAL) \
	asm volatile (	"flds  s0, [%0]			\n\t" \
					: \
					: "r" (P_VAL) \
					: "r0", "s0", "cc", "memory" \
					);

#define VFP_FIXED_16_VECTOR_MUL_SCALAR_FTI_SAVE(P_SRC, P_DSTa, P_DSTb) \
	asm volatile (	"fldmias  %2!, {s8-s15}		\n\t" \
					"fldmias  %2!, {s16-s23}	\n\t" \
					"fmuls s8, s8, s0			\n\t" \
					"fmuls s16, s16, s0			\n\t" \
					"ftosis s8, s8				\n\t" \
					"ftosis s9, s9				\n\t" \
					"ftosis s10, s10			\n\t" \
					"ftosis s11, s11			\n\t" \
					"ftosis s12, s12			\n\t" \
					"ftosis s13, s13			\n\t" \
					"ftosis s14, s14			\n\t" \
					"ftosis s15, s15			\n\t" \
					"ftosis s16, s16			\n\t" \
					"ftosis s17, s17			\n\t" \
					"ftosis s18, s18			\n\t" \
					"ftosis s19, s19			\n\t" \
					"ftosis s20, s20			\n\t" \
					"ftosis s21, s21			\n\t" \
					"ftosis s22, s22			\n\t" \
					"ftosis s23, s23			\n\t" \
					"fstmias  %0, {s8-s15}		\n\t" \
					"fstmias  %1, {s16-s23}	\n\t" \
					: \
					: "r" (P_DSTa), "r" (P_DSTb), "r" (P_SRC) \
					: "r0", "s0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);


// here src2 should contain an array of multipliers to scale the floats after int conversion
// e.g. for audio short -> float: src2[] = { 1.f/32767.f, 1.f/32767.f, 1.f/32767.f, 1.f/32767.f .... etc }
#define VFP_FIXED_8_VECTOR_ITF_MUL_3(P_SRC_1, P_SRC_2, P_DST) \
	asm volatile (	"fldmias  %1, {s8-s15}		\n\t" \
					"fldmias  %2, {s16-s23}		\n\t" \
					"fsitos s8, s8				\n\t" \
					"fsitos s9, s9				\n\t" \
					"fsitos s10, s10			\n\t" \
					"fsitos s11, s11			\n\t" \
					"fsitos s12, s12			\n\t" \
					"fsitos s13, s13			\n\t" \
					"fsitos s14, s14			\n\t" \
					"fsitos s15, s15			\n\t" \
					"fmuls s8, s8, s16			\n\t" \
					"fstmias  %0, {s8-s15}		\n\t" \
					: \
					: "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
					: "r0", "cc", "memory", VFP_CLOBBER_S8_S23 \
					);

#define VFP_FIXED_16_VECTOR_ITF_MUL_SCALAR(P_VAL, P_SRCa, P_SRCb, P_DST) \
asm volatile (	"flds s0, [%1]				\n\t" \
				"fldmias  %2, {s8-s15}		\n\t" \
				"fldmias  %3, {s16-s23}		\n\t" \
				"fsitos s8, s8				\n\t" \
				"fsitos s9, s9				\n\t" \
				"fsitos s10, s10			\n\t" \
				"fsitos s11, s11			\n\t" \
				"fsitos s12, s12			\n\t" \
				"fsitos s13, s13			\n\t" \
				"fsitos s14, s14			\n\t" \
				"fsitos s15, s15			\n\t" \
				"fsitos s16, s16			\n\t" \
				"fsitos s17, s17			\n\t" \
				"fsitos s18, s18			\n\t" \
				"fsitos s19, s19			\n\t" \
				"fsitos s20, s20			\n\t" \
				"fsitos s21, s21			\n\t" \
				"fsitos s22, s22			\n\t" \
				"fsitos s23, s23			\n\t" \
				"fmuls s8, s8, s0			\n\t" \
				"fmuls s16, s16, s0			\n\t" \
				"fstmias  %0!, {s8-s15}		\n\t" \
				"fstmias  %0!, {s16-s23}	\n\t" \
				: \
				: "r" (P_DST), "r" (P_VAL), "r" (P_SRCa),  "r" (P_SRCb)  \
				: "r0", "cc", "memory", "s0", VFP_CLOBBER_S8_S23 \
				);


#define VFP_FIXED_16_VECTOR_ITF_MUL_SCALAR_LOAD(P_VAL) \
	asm volatile (	"flds s0, [%0]				\n\t" \
					: \
					: "r" (P_VAL) \
					: "r0", "s0", "cc", "memory" \
					);

#define VFP_FIXED_16_VECTOR_ITF_MUL_SCALAR_SAVE(P_SRCa, P_SRCb, P_DST) \
asm volatile (	"fldmias  %1, {s8-s15}		\n\t" \
				"fldmias  %2, {s16-s23}		\n\t" \
				"fsitos s8, s8				\n\t" \
				"fsitos s9, s9				\n\t" \
				"fsitos s10, s10			\n\t" \
				"fsitos s11, s11			\n\t" \
				"fsitos s12, s12			\n\t" \
				"fsitos s13, s13			\n\t" \
				"fsitos s14, s14			\n\t" \
				"fsitos s15, s15			\n\t" \
				"fsitos s16, s16			\n\t" \
				"fsitos s17, s17			\n\t" \
				"fsitos s18, s18			\n\t" \
				"fsitos s19, s19			\n\t" \
				"fsitos s20, s20			\n\t" \
				"fsitos s21, s21			\n\t" \
				"fsitos s22, s22			\n\t" \
				"fsitos s23, s23			\n\t" \
				"fmuls s8, s8, s0			\n\t" \
				"fmuls s16, s16, s0			\n\t" \
				"fstmias  %0!, {s8-s15}		\n\t" \
				"fstmias  %0!, {s16-s23}	\n\t" \
				: \
				: "r" (P_DST), "r" (P_SRCa),  "r" (P_SRCb)  \
				: "r0", "s0", "cc", "memory", VFP_CLOBBER_S8_S23 \
				);


// NOTE: Usage example for VFP_FIXED_?_VECTOR_OP
// 	 float* src_ptr_1;
// 	 float* src_ptr_2;
// 	 float* dst_ptr; 
//	 VFP_FIXED_4_VECTOR_OP(VFP_OP_ADD, src_ptr_1, src_ptr_2, dst_ptr)

// unary
#define VFP_OP_ABS "fabss"
#define VFP_OP_SQRT "fsqrts"
#define VFP_OP_NEG "fnegs"
#define VFP_OP_CPY "fcpys"
#define VFP_OP_FTI "ftosis"
#define VFP_OP_ITF "fsitos"

// binary
#define VFP_OP_ADD "fadds"
#define VFP_OP_SUB "fsubs"
#define VFP_OP_MUL "fmuls"
#define VFP_OP_DIV "fdivs"

// accumulate:
// e.g., VFP_OP_MULADD does:
// dst = dst + src1 * src2
// so to do muladd (scale/offset) load src1 with input, dst with the offset and src2 with scale
#define VFP_OP_MULADD "fmacs"

#endif // __UGEN_VFP_MACROS_H__
