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

#ifndef _UGEN_ugen_vfp_Utilities_H_
#define _UGEN_ugen_vfp_Utilities_H_

#include "ugen_vfp_macros.h"
#include "../../basics/ugen_InlineUnaryOps.h"
#include "../../basics/ugen_InlineBinaryOps.h"


//-falign-loops-max-skip=15 -falign-jumps-max-skip=15 -falign-loops=16 -falign-jumps=16 -falign-functions=16

typedef struct _vFloat8
{
	float f[8];
} vFloat8;


class VFP
{
public:
	
	// unary ops - bersions for 8 or 16 vectors
	
	static inline void clear(float *outputSamples, unsigned int numSamples) throw()
	{
		static const float zeros[8] UGEN_ALIGN = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		VFP_FIXED_8_VECTOR_SPLAT_LOAD(zeros);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_SPLAT_SAVE(outputSamples);
		}
		
		if(numScalars > 0)
			memset(outputSamples, 0, numScalars * sizeof(float));
				
		VFP_VECTOR_LENGTH_ZERO_ASM;
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void clear8(float *outputSamples, unsigned int numSamples) throw()
	{
		clear(outputSamples, numSamples);
	}

	static inline void clear16(float *outputSamples, unsigned int numSamples) throw()
	{
		static const float zeros[8] UGEN_ALIGN = {	0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		VFP_FIXED_8_VECTOR_SPLAT_LOAD(zeros);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_SPLAT_SAVE(outputSamples);
		}
		
		if(numScalars > 0)
			memset(outputSamples, 0, numScalars * sizeof(float));
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		VFP_SWITCH_TO_THUMB_ASM;
	}

	static inline void splat_normal(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		while(numSamples--)
			*outputSamples++ = val;
	}
	
	static inline void splat(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		const float values[8] UGEN_ALIGN = { val, val, val, val, val, val, val, val };
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		VFP_FIXED_8_VECTOR_SPLAT_LOAD(values);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_SPLAT_SAVE(outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
			*outputSamples++ = val;
		
		VFP_SWITCH_TO_THUMB_ASM;
	}	
	
	static inline void splat8(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		splat(val, outputSamples, numSamples);
	}
	
	static inline void splat16(float val, float *outputSamples, unsigned int numSamples) throw()
	{
		const float values[8] UGEN_ALIGN = { val, val, val, val, val, val, val, val };
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		VFP_FIXED_8_VECTOR_SPLAT_LOAD(values);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_SPLAT_SAVE(outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
			*outputSamples++ = val;
		
		VFP_SWITCH_TO_THUMB_ASM;
	}	

	static inline void copy(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_CPY_4(inputSamples, outputSamples);
		}
		
		if(numScalars > 0)
			memcpy(outputSamples, inputSamples, numScalars * sizeof(float));
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void copy8(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		copy(inputSamples, outputSamples, numSamples);
	}

	static inline void copy16(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_CPY_4(inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		if(numScalars > 0)
			memcpy(outputSamples, inputSamples, numScalars * sizeof(float));
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void abs(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_VECTOR_LENGTH_ASM(7);
		VFP_SWITCH_TO_ARM_ASM;
		
		while(numVectors--)
		{
			VFP_FIXED_8_UNARY_VECTOR_OP4(VFP_OP_ABS, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;

		while(numScalars--)
		{
			*outputSamples++ = std::fabs(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}	

	static inline void abs8(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		abs(inputSamples, outputSamples, numSamples);
	}
	
	static inline void abs16(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_UNARY_VECTOR_OP4(VFP_OP_ABS, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = std::fabs(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}	
	
	static inline void neg(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_UNARY_VECTOR_OP4(VFP_OP_NEG, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = -(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void neg8(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		neg(inputSamples, outputSamples, numSamples);
	}
	
	static inline void neg16(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_UNARY_VECTOR_OP4(VFP_OP_NEG, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = -(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}	
	
	static inline void sqrt(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_UNARY_VECTOR_OP4(VFP_OP_SQRT, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = std::sqrt(*inputSamples++);
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void sqrt8(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		sqrt(inputSamples, outputSamples, numSamples);
	}
	
	static inline void sqrt16(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_UNARY_VECTOR_OP4(VFP_OP_SQRT, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = std::sqrt(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	// unary ops with only an 8 vector version...
	
	static inline void reciprocal(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		static const float ones[8] UGEN_ALIGN = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		VFP_FIXED_8_VECTOR_SPLAT_LOAD(ones); 
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_SPLAT_OP_S8(VFP_OP_DIV, inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = 1.f / (*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void squared(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_UNARY_VECTOR_SQUARED_4(inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::squared(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void cubed(const float *inputSamples, float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_UNARY_VECTOR_CUBED_4(inputSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::cubed(*inputSamples++);
		}	
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	// binary ops
	
	static inline void add(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{		
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_OP4(VFP_OP_ADD, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
				
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ + *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void add8(const float *leftSamples, 
							const float *rightSamples, 
							float *outputSamples, 
							unsigned int numSamples) throw()
	{
		add(leftSamples, rightSamples, outputSamples, numSamples);
	}
	
	static inline void add16(const float *leftSamples, 
							 const float *rightSamples, 
							 float *outputSamples, 
							 unsigned int numSamples) throw()
	{		
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_OP4(VFP_OP_ADD, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ + *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	

	static inline void mul(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_OP4(VFP_OP_MUL, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ * *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void mul8(const float *leftSamples, 
							const float *rightSamples, 
							float *outputSamples, 
							unsigned int numSamples) throw()
	{
		mul(leftSamples, rightSamples, outputSamples, numSamples);
	}
	
	static inline void mul16(const float *leftSamples, 
							 const float *rightSamples, 
							 float *outputSamples, 
							 unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_OP4(VFP_OP_MUL, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ * *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	static inline void sub(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_OP4(VFP_OP_SUB, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ - *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;

	}
	
	static inline void sub8(const float *leftSamples, 
							const float *rightSamples, 
							float *outputSamples, 
							unsigned int numSamples) throw()
	{
		sub(leftSamples, rightSamples, outputSamples, numSamples);
	}

	static inline void sub16(const float *leftSamples, 
							 const float *rightSamples, 
							 float *outputSamples, 
							 unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_OP4(VFP_OP_SUB, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ - *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
	}
	
	
	static inline void div(const float *leftSamples, 
						   const float *rightSamples, 
						   float *outputSamples, 
						   unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_OP4(VFP_OP_DIV, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ / *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;

	}
	
	static inline void div8(const float *leftSamples, 
							const float *rightSamples, 
							float *outputSamples, 
							unsigned int numSamples) throw()
	{
		div(leftSamples, rightSamples, outputSamples, numSamples);
	}
	
	static inline void div16(const float *leftSamples, 
							 const float *rightSamples, 
							 float *outputSamples, 
							 unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 4U;
		unsigned int numScalars = numSamples & 15U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_16_VECTOR_OP4(VFP_OP_DIV, leftSamples, rightSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *leftSamples++ / *leftSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;
		
	}
	
	// these use vFloat8s normally for FFT based ops, vector size of 8 needs to have been preset before hand
	
	static inline vFloat8 mul8(vFloat8 const& leftVec, vFloat8 const& rightVec) throw()
	{
		vFloat8 returnVec;
		VFP_FIXED_8_VECTOR_OP3(VFP_OP_MUL, &leftVec, &rightVec, &returnVec);
		return returnVec;
	}
	
	static inline vFloat8 add8(vFloat8 const& leftVec, vFloat8 const& rightVec) throw()
	{
		vFloat8 returnVec;
		VFP_FIXED_8_VECTOR_OP3(VFP_OP_ADD, &leftVec, &rightVec, &returnVec);
		return returnVec;
	}
	
	static inline vFloat8 sub8(vFloat8 const& leftVec, vFloat8 const& rightVec) throw()
	{
		vFloat8 returnVec;		
		VFP_FIXED_8_VECTOR_OP3(VFP_OP_SUB, &leftVec, &rightVec, &returnVec);
		return returnVec;
	}
		
	
	
	// acummulators
	
	static inline void muladd(const float *inputSamples, 
							  const float *mulSamples, 
							  const float *addSamples, 
							  float *outputSamples, 
							  unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_MULADD_4(inputSamples, mulSamples, addSamples, outputSamples);
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = *inputSamples++ * *mulSamples++ + *addSamples++;
		}
		
		VFP_SWITCH_TO_THUMB_ASM;

	}
	
	static inline void linlin(const float *inputSamples, 
							  const float *inLowSamples, const float *inHighSamples,
							  const float *outLowSamples, const float *outHighSamples,
							  float *outputSamples, unsigned int numSamples) throw()
	{
		unsigned int numVectors = numSamples >> 3U;
		unsigned int numScalars = numSamples & 7U;
		
		float data[24] UGEN_ALIGN;
		float *inRangeSamples = data;
		float *outRangeSamples = inRangeSamples + 8;
		float *tempSamples = outRangeSamples + 8;
				
		VFP_SWITCH_TO_ARM_ASM;
		VFP_VECTOR_LENGTH_ASM(7);
		
		while(numVectors--)
		{
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_SUB, inHighSamples, inLowSamples, inRangeSamples);
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_SUB, outHighSamples, outLowSamples, outRangeSamples);
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_SUB, inputSamples, inLowSamples, tempSamples);		
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_MUL, tempSamples, outRangeSamples, tempSamples);
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_DIV, tempSamples, inRangeSamples, tempSamples);
			VFP_FIXED_8_VECTOR_OP3(VFP_OP_ADD, tempSamples, outLowSamples, outputSamples);
			inputSamples += 8;
			inLowSamples += 8;
			inHighSamples += 8;
			outLowSamples += 8;
			outHighSamples += 8;
			outputSamples += 8;
		}
		
		VFP_VECTOR_LENGTH_ZERO_ASM;
		
		while(numScalars--)
		{
			*outputSamples++ = ugen::linlin(*inputSamples++,
											*inLowSamples++, *inHighSamples++,
											*outLowSamples++, *outHighSamples++);
		}
		
		VFP_SWITCH_TO_THUMB_ASM;

	}
	
};


#endif // _UGEN_ugen_vfp_Utilities_H_
