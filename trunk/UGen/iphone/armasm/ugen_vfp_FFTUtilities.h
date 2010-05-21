// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-10 by Martin Robinson www.miajo.co.uk
 
 ------------------------------------------------------------------------------
 
 UGEN++ can be redistributed and/or modified under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.
 
 UGEN++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with UGEN++; if not, visit www.gnu.org/licenses or write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA
 
 The idea for this project and code in the UGen implementations is
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_vfp_FFTUtilities_H_
#define _UGEN_ugen_vfp_FFTUtilities_H_

#include "ugen_vfp_Utilities.h"


inline vFloat VEC_MUL_OP(vFloat const& leftVec, vFloat const& rightVec)
{
	vFloat returnVec;
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(3);
	VFP_FIXED_4_VECTOR_OP3(VFP_OP_MUL, &leftVec, &rightVec, &returnVec);
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
	return returnVec;
}

inline vFloat VEC_ADD_OP(vFloat const& leftVec, vFloat const& rightVec)
{
	vFloat returnVec;
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(3);
	VFP_FIXED_4_VECTOR_OP3(VFP_OP_ADD, &leftVec, &rightVec, &returnVec);
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
	return returnVec;
}

inline vFloat VEC_SUB_OP(vFloat const& leftVec, vFloat const& rightVec)
{
	vFloat returnVec;
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(3);
	VFP_FIXED_4_VECTOR_OP3(VFP_OP_SUB, &leftVec, &rightVec, &returnVec);
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
	return returnVec;
}

//inline void MultAndAdd(DSPSplitComplex& In1, DSPSplitComplex& In2, DSPSplitComplex& Out, const int VecLength4) throw()
//{
//	const int VecLength = VecLength4 >> 1;
//	
//	vFloat8 *InReal1 = (vFloat8 *) In1.realp;
//	vFloat8 *InImag1 = (vFloat8 *) In1.imagp;
//	vFloat8 *InReal2 = (vFloat8 *) In2.realp;
//	vFloat8 *InImag2 = (vFloat8 *) In2.imagp;
//	vFloat8 *OutReal = (vFloat8 *) Out.realp;
//	vFloat8 *OutImag = (vFloat8 *) Out.imagp;
//	
//	float Nyquist1, Nyquist2;
//	Nyquist1 = In1.imagp[0];
//	Nyquist2 = In2.imagp[0];
//	
//	In1.imagp[0] = 0.f;
//	In2.imagp[0] = 0.f;
//	
//	Out.imagp[0] += Nyquist1 * Nyquist2;
//	
//	VFP_SWITCH_TO_ARM_ASM;
//	VFP_VECTOR_LENGTH_ASM(7);
//
//	for (int i = 0; i < VecLength; i++)
//	{
//		OutReal[i] = VFP::add8(OutReal[i], VFP::sub8(VFP::mul8(InReal1[i], InReal2[i]), VFP::mul8(InImag1[i], InImag2[i])));
//		OutImag[i] = VFP::add8(OutImag[i], VFP::add8(VFP::mul8(InReal1[i], InImag2[i]), VFP::mul8(InImag1[i], InReal2[i])));
//	}
//	
//	VFP_VECTOR_LENGTH_ZERO_ASM;
//	VFP_SWITCH_TO_THUMB_ASM;
//	
//	In1.imagp[0] = Nyquist1;
//	In2.imagp[0] = Nyquist2;
//}


//inline void MultAndAdd(DSPSplitComplex& In1, DSPSplitComplex& In2, DSPSplitComplex& Out, const int VecLength4) throw()
//{
//	int VecLength = VecLength4 >> 1U; // we need half the number of iterations for 8!
//	
//	float *InReal1 = In1.realp;
//	float *InImag1 = In1.imagp;
//	float *InReal2 = In2.realp;
//	float *InImag2 = In2.imagp;
//	float *OutReal = Out.realp;
//	float *OutImag = Out.imagp;
//	
//	float Nyquist1, Nyquist2;
//	Nyquist1 = In1.imagp[0];
//	Nyquist2 = In2.imagp[0];
//	
//	In1.imagp[0] = 0.f;
//	In2.imagp[0] = 0.f;
//	
//	Out.imagp[0] += Nyquist1 * Nyquist2;
//	
//	//	OutReal[i] = VFP::add8(OutReal[i], VFP::sub8(VFP::mul8(InReal1[i], InReal2[i]), VFP::mul8(InImag1[i], InImag2[i])));
//	//	OutImag[i] = VFP::add8(OutImag[i], VFP::add8(VFP::mul8(InReal1[i], InImag2[i]), VFP::mul8(InImag1[i], InReal2[i])));
//	
//	VFP_SWITCH_TO_ARM_ASM;
//	VFP_VECTOR_LENGTH_ASM(7);
//	
//	while(VecLength--) {
//		asm volatile (
//					  "fldmias %4, {s8-s15}							\n\t"
//					  "fldmias %5, {s16-s23}						\n\t"
//					  "fmuls s8, s8, s16							\n\t"
//					  "fldmias %3, {s24-s31}						\n\t"
//					  "fldmias %2, {s16-s23}						\n\t"
//					  "fmuls s16, s16, s24							\n\t"
//					  "fldmias %0, {s24-s31}						\n\t"
//					  "fsubs s8, s16, s8							\n\t"
//					  "fadds s8, s8, s24							\n\t"
//					  "fstmias %0!, {s8-s15}						\n\t"
//					  
//					  "fldmias %4!, {s8-s15}						\n\t"
//					  "fldmias %3!, {s16-s23}						\n\t"
//					  "fmuls s8, s8, s16							\n\t"
//					  "fldmias %5!, {s24-s31}						\n\t"
//					  "fldmias %2!, {s16-s23}						\n\t"
//					  "fmuls s16, s16, s24							\n\t"
//					  "fldmias %1, {s24-s31}						\n\t"
//					  "fadds s8, s16, s8							\n\t"
//					  "fadds s8, s8, s24							\n\t"
//					  "fstmias %1!, {s8-s15}						\n\t"
//					 
//					  
//					  : "=r"(OutReal), "=r"(OutImag), "=r"(InReal1), "=r"(InReal2), "=r"(InImag1), "=r"(InImag2)
//					  :  "0"(OutReal), "1"(OutImag) , "2"(InReal1), "3"(InReal2), "4"(InImag1), "5"(InImag2)
//					  : "r0", "memory", "cc", VFP_CLOBBER_S8_S31
//					  );
//	}
//	
//	VFP_VECTOR_LENGTH_ZERO_ASM;
//	VFP_SWITCH_TO_THUMB_ASM;
//	
//	In1.imagp[0] = Nyquist1;
//	In2.imagp[0] = Nyquist2;
//}


inline void MultAndAdd(DSPSplitComplex& In1, DSPSplitComplex& In2, DSPSplitComplex& Out, const int VecLength4) throw()
{
	int VecLength = VecLength4 >> 1U; // we need half the number of iterations for 8!
	
	float *InReal1 = In1.realp;
	float *InImag1 = In1.imagp;
	float *InReal2 = In2.realp;
	float *InImag2 = In2.imagp;
	float *OutReal = Out.realp;
	float *OutImag = Out.imagp;
	
	float Nyquist1, Nyquist2;
	Nyquist1 = In1.imagp[0];
	Nyquist2 = In2.imagp[0];
	
	In1.imagp[0] = 0.f;
	In2.imagp[0] = 0.f;
	
	Out.imagp[0] += Nyquist1 * Nyquist2;
	
	//	OutReal[i] = VFP::add8(OutReal[i], VFP::sub8(VFP::mul8(InReal1[i], InReal2[i]), VFP::mul8(InImag1[i], InImag2[i])));
	//	OutImag[i] = VFP::add8(OutImag[i], VFP::add8(VFP::mul8(InReal1[i], InImag2[i]), VFP::mul8(InImag1[i], InReal2[i])));
	
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(7);
	
	while(VecLength--) {
		asm volatile ("fldmias %0, {s24-s31}						\n\t"
					  "fldmias %4, {s8-s15}							\n\t"
					  "fldmias %5, {s16-s23}						\n\t"
					  "fnmacs s24, s8, s16							\n\t"
					  "fldmias %3, {s8-s15}							\n\t"
					  "fldmias %2, {s16-s23}						\n\t"
					  "fmacs s24, s8, s16							\n\t"
					  "fstmias %0!, {s24-s31}						\n\t"
					  
					  "fldmias %1, {s24-s31}						\n\t"
					  "fldmias %4!, {s8-s15}						\n\t"
					  "fldmias %3!, {s16-s23}						\n\t"
					  "fmacs s24, s8, s16							\n\t"
					  "fldmias %5!, {s8-s15}						\n\t"
					  "fldmias %2!, {s16-s23}						\n\t"
					  "fmacs s24, s8, s16							\n\t"
					  "fstmias %1!, {s24-s31}						\n\t"
					  
					  : "=r"(OutReal), "=r"(OutImag), "=r"(InReal1), "=r"(InReal2), "=r"(InImag1), "=r"(InImag2)
					  :  "0"(OutReal), "1"(OutImag) , "2"(InReal1), "3"(InReal2), "4"(InImag1), "5"(InImag2)
					  : "r0", "memory", "cc", VFP_CLOBBER_S8_S31
					  );
	}
	
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
	
	In1.imagp[0] = Nyquist1;
	In2.imagp[0] = Nyquist2;
}


#endif // _UGEN_ugen_vfp_FFTUtilities_H_
