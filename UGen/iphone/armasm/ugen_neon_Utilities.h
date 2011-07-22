// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_neon_Utilities_H_
#define _UGEN_ugen_neon_Utilities_H_

#include <arm_neon.h>
#include "../../basics/ugen_InlineUnaryOps.h"
#include "../../basics/ugen_InlineBinaryOps.h"

typedef float32x4_t vFloat;
typedef int32x4_t vInt;

class Neon
{
public:
	// unary ops
	
	static inline void clear(float *outputSamples, unsigned int numSamples) throw()
	{
		memset(outputSamples, 0, numSamples * sizeof(float));
	}
	
	static inline void splat_normal(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		while(numSamples--)
			*outputSamples++ = val;
	}
	
	static inline void splat(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;

		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vdupq_n_f32(val);
		}
		
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = val;
		}
	}	
	
	static inline void copy(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		memcpy(outputSamples, inputSamples, numSamples * sizeof(float));
	}
		
	static inline void abs(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{	
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;

		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vabsq_f32(*inputSamplesVec++);
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = std::fabs(*inputSamples++);
		}	
	}	
	
	static inline void neg(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;

		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vnegq_f32(*inputSamplesVec++);
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = -(*inputSamples++);
		}	
	}

	static inline void sqrt(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vrecpeq_f32(vrsqrteq_f32(*inputSamplesVec++));
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = std::sqrt(*inputSamples++);
		}
	}
		
	static inline void reciprocal(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vrecpeq_f32(*inputSamplesVec++);
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = 1.f / (*inputSamples++);
		}	
	}
	
	static inline void squared(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vmulq_f32(*inputSamplesVec, *inputSamplesVec);
			inputSamplesVec++;
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::squared(*inputSamples++);
		}	
	}
	
	static inline void cubed(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			vFloat temp = vmulq_f32(*inputSamplesVec, *inputSamplesVec);
			*outputSamplesVec++ = vmulq_f32(temp, *inputSamplesVec++);
		}
		
		inputSamples = (const float *)inputSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::cubed(*inputSamples++);
		}	
	}
	
	// binary ops
	
//	static inline void add(const float *leftSamples, 
//						   const float *rightSamples, 
//						   float *outputSamples, 
//						   unsigned int numSamples) throw()
//	{		
//		unsigned int numVectors = numSamples >> 2U;
//		unsigned int numScalars = numSamples & 3U;
//				
//		vFloat *leftSamplesVec = (vFloat*)leftSamples;
//		vFloat *rightSamplesVec = (vFloat*)rightSamples;
//		vFloat *outputSamplesVec = (vFloat*)outputSamples;
//		
//		while(numVectors--)
//		{
//			*outputSamplesVec++ = vaddq_f32(*leftSamplesVec++, *rightSamplesVec++);
//		}
//		
//		leftSamples = (const float *)leftSamplesVec;
//		rightSamples = (const float *)rightSamplesVec;
//		outputSamples = (float *)outputSamplesVec;
//		
//		while(numScalars--)
//		{
//			*outputSamples++ = *leftSamples++ + *rightSamples++;
//		}
//	}

	
	static inline void add(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{		
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
				
		while(numVectors--)
		{
			asm volatile ("vldmia %1!, {q8}			\n\t"
						  "vldmia %2!, {q9}			\n\t"
						  "vadd.f32	q10, q8, q9		\n\t"
						  "vstmia %0!, {q10}			\n\t"
						  : "=r" (outputSamples), "=r" (leftSamples), "=r" (rightSamples)
						  : "0" (outputSamples), "1" (leftSamples), "2" (rightSamples)
						  : "r0", "cc", "memory", "q8", "q9", "q10"
						  );
		}
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ + *rightSamples++;
		}
	}
	
	static inline void add16(const float *leftSamples, 
							 const float *rightSamples, 
							 float *outputSamples, 
							 unsigned int numSamples) throw()
	{		
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		while(numVectors--)
		{
			asm volatile ("vldmia %1!, {q4-q7}			\n\t"
						  "vldmia %2!, {q8-q11}			\n\t"
						  "vadd.f32	q12, q4, q8			\n\t"
						  "vadd.f32	q13, q5, q9			\n\t"
						  "vadd.f32	q14, q6, q10		\n\t"
						  "vadd.f32	q15, q7, q11		\n\t"
						  "vstmia %0!, {q12-q15}		\n\t"
						  : "=r" (outputSamples), "=r" (leftSamples), "=r" (rightSamples)
						  : "0" (outputSamples), "1" (leftSamples), "2" (rightSamples)
						  : "r0", "cc", "memory", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						  );
		}
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ + *rightSamples++;
		}
	}
	
	
	
	static inline void mul(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *leftSamplesVec = (vFloat*)leftSamples;
		vFloat *rightSamplesVec = (vFloat*)rightSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vmulq_f32(*leftSamplesVec++, *rightSamplesVec++);
		}
		
		leftSamples = (const float *)leftSamplesVec;
		rightSamples = (const float *)rightSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ * *rightSamples++;
		}
	}
	
	static inline void sub(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *leftSamplesVec = (vFloat*)leftSamples;
		vFloat *rightSamplesVec = (vFloat*)rightSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vsubq_f32(*leftSamplesVec++, *rightSamplesVec++);
		}
		
		leftSamples = (const float *)leftSamplesVec;
		rightSamples = (const float *)rightSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ - *rightSamples++;
		}
	}
	
//	static inline void div(const float *leftSamples, 
//						   const float *rightSamples, 
//						   float *outputSamples, 
//						   unsigned int numSamples) throw()
//	{
//		// no neon divide fp op ?
//		while(numSamples--)
//		{
//			*outputSamples++ = *leftSamples++ / *rightSamples++;
//		}
//	}
	
	static inline void div(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *leftSamplesVec = (vFloat*)leftSamples;
		vFloat *rightSamplesVec = (vFloat*)rightSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vmulq_f32(*leftSamplesVec++, vrecpeq_f32(*rightSamplesVec++));
		}
		
		leftSamples = (const float *)leftSamplesVec;
		rightSamples = (const float *)rightSamplesVec;
		outputSamples = (float *)outputSamplesVec;

		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ / *rightSamples++;
		}
	}
	
	
	
	// acummulators
	
	static inline void muladd(const float *inputSamples, 
							  const float *mulSamples, 
							  const float *addSamples, 
							  float *outputSamples, 
							  unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *mulSamplesVec = (vFloat*)mulSamples;
		vFloat *addSamplesVec = (vFloat*)addSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		while(numVectors--)
		{
			*outputSamplesVec++ = vmlaq_f32(*addSamplesVec++, *inputSamplesVec++, *mulSamplesVec++);
		}
		
		inputSamples = (const float *)inputSamplesVec;
		mulSamples = (const float *)mulSamplesVec;
		addSamples = (const float *)addSamplesVec;
		outputSamples = (float *)outputSamplesVec;
		
		while(numScalars--)
		{
			*outputSamples++ = *inputSamples++ * *mulSamples++ + *addSamples++;
		}
	}
	
//	static inline vFloat linlin(vFloat input, 
//								vFloat inLow, vFloat inHigh,
//								vFloat outLow, vFloat outHigh) throw()
//	{
//		return vaddq_f32(vmulq_f32(vsubq_f32(input, inLow), 
//								   vmulq_f32(vsubq_f32(outHigh, outLow),
//											 vrecpeq_f32(vsubq_f32(inHigh, inLow)))), 
//						 outLow);
//	}
	
	static inline void linlin(const float *inputSamples, 
							  const float *inLowSamples, const float *inHighSamples,
							  const float *outLowSamples, const float *outHighSamples,
							  float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 2U;
		unsigned int numScalars = numSamples & 3U;
		
		vFloat *inputSamplesVec = (vFloat*)inputSamples;
		vFloat *inLowSamplesVec = (vFloat*)inLowSamples;
		vFloat *inHighSamplesVec = (vFloat*)inHighSamples;
		vFloat *outLowSamplesVec = (vFloat*)outLowSamples;
		vFloat *outHighSamplesVec = (vFloat*)outHighSamples;
		vFloat *outputSamplesVec = (vFloat*)outputSamples;
		
		for(int i = 0; i < numVectors; i++)
		{
			outputSamplesVec[i] = vaddq_f32(vmulq_f32(vsubq_f32(inputSamplesVec[i], inLowSamplesVec[i]), 
													  vmulq_f32(vsubq_f32(outHighSamplesVec[i], outLowSamplesVec[i]),
																vrecpeq_f32(vsubq_f32(inHighSamplesVec[i], inLowSamplesVec[i])))), 
											outLowSamplesVec[i]);
		}

		inputSamples = (const float *)(inputSamplesVec + numVectors);
		inLowSamples = (const float *)(inLowSamplesVec + numVectors);
		inHighSamples = (const float *)(inHighSamplesVec + numVectors);
		outLowSamples = (const float *)(outLowSamplesVec + numVectors);
		outHighSamples = (const float *)(outHighSamplesVec + numVectors);
		outputSamples = (float *)(outputSamplesVec + numVectors);
		
//		while(numVectors--)
//		{
//			*outputSamplesVec++ = Neon::linlin(*inputSamplesVec++,
//											   *inLowSamplesVec++, *inHighSamplesVec++,
//											   *outLowSamplesVec++, *outHighSamplesVec++);
//		}
//		
//		
//		inputSamples = (const float *)(inputSamplesVec);
//		inLowSamples = (const float *)(inLowSamplesVec);
//		inHighSamples = (const float *)(inHighSamplesVec);
//		outLowSamples = (const float *)(outLowSamplesVec);
//		outHighSamples = (const float *)(outHighSamplesVec);
//		outputSamples = (float *)(outputSamplesVec);
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::linlin(*inputSamples++,
											*inLowSamples++, *inHighSamples++,
											*outLowSamples++, *outHighSamples++);
		}
	}
	
//	static inline void linlin(const float *inputSamples, 
//							  const float *inLowSamples, const float *inHighSamples,
//							  const float *outLowSamples, const float *outHighSamples,
//							  float *outputSamples, unsigned int numSamples) throw()
//	{
//		unsigned int numScalars = numSamples;
//		
//		while(numScalars--)
//		{
//			*outputSamples++ = ugen::linlin(*inputSamples++,
//											*inLowSamples++, *inHighSamples++,
//											*outLowSamples++, *outHighSamples++);
//		}
//	}
	
	
};


#endif // _UGEN_ugen_neon_Utilities_H_
