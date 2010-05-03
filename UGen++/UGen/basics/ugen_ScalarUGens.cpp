// $Id: ugen_ScalarUGens.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_ScalarUGens.cpp $

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
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#include <string.h>

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_ScalarUGens.h"

NullUGenInternal::NullUGenInternal() throw()
:	ScalarBaseUGenInternal(0.f)
{
}

UGenInternal* NullUGenInternal::getKr() throw()
{
	NullUGenInternal* newNullKr = new NullUGenInternal();
	newNullKr->rate = ControlRate;
	return newNullKr;
}

void NullUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	memset(outputSamples, 0, numSamplesToProcess * sizeof(float));
}

ScalarUGenInternal::ScalarUGenInternal(const float value) throw()
:	ScalarBaseUGenInternal(value)
{
}


#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void ScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{		
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	LOCAL_DECLARE(float, value_);
	
	// this is faster on the Mac with compiler optimisations on the Mac sice it
	// gets replaced by vectorised operations!
	memset(outputSamples, 0, numSamplesToProcess * sizeof(float));
	for(int i = 0; i < numSamplesToProcess; ++i)
		outputSamples[i] += value_;
}
#endif



FloatPtrUGenInternal::FloatPtrUGenInternal(float const *valuePtr) throw()
:	ScalarBaseUGenInternal(*valuePtr),
	ptr(valuePtr) 
{
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void FloatPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	PtrUGenProcessBlock();
}
#endif

DoublePtrUGenInternal::DoublePtrUGenInternal(double const *valuePtr) throw()
:	ScalarBaseUGenInternal(*valuePtr),
	ptr(valuePtr) 
{	
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void DoublePtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	PtrUGenProcessBlock();
}
#endif


IntPtrUGenInternal::IntPtrUGenInternal(int const *valuePtr) throw()
:	ScalarBaseUGenInternal(*valuePtr),
	ptr(valuePtr) 
{
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void IntPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	PtrUGenProcessBlock();
}
#endif


BoolPtrUGenInternal::BoolPtrUGenInternal(bool const *valuePtr) throw()
:	ScalarBaseUGenInternal(*valuePtr),
	ptr(valuePtr) 
{
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void BoolPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float nextValue = (float)(*ptr != 0);
	
	memset(outputSamples, 0, numSamplesToProcess * sizeof(float));
	if(nextValue == value_)	{
		for(int i = 0; i < numSamplesToProcess; ++i)
			outputSamples[i] += nextValue;
	} else {
		int numKrSamplesToProcess = UGen::getControlRateBlockSize();
		float valueSlope = (nextValue - value_) * UGen::getControlSlopeFactor();
		numSamplesToProcess -= numKrSamplesToProcess;
		for(int i = 0; i < numKrSamplesToProcess; ++i) {
			*outputSamples++ += value_;
			value_ += valueSlope;
		}
		if(numSamplesToProcess > 0) {
			for(int i = 0; i < numSamplesToProcess; ++i)
				outputSamples[i] += nextValue;
		}
		value_ = nextValue;
	}	
}
#endif



END_UGEN_NAMESPACE

