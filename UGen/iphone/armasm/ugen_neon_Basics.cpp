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

#include "../../core/ugen_StandardHeader.h"

#ifdef UGEN_NEON

BEGIN_UGEN_NAMESPACE

#include "ugen_neon_Utilities.h"
#include "../../basics/ugen_ScalarUGens.h"
#include "../../basics/ugen_MixUGen.h"
#include "../../basics/ugen_MulAdd.h"
#include "../../basics/ugen_Plug.h"
#include "../../basics/ugen_MappingUGens.h"


// neon versions of some of the UGen processing functions...



void ScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{		
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();	
	Neon::splat(value_, outputSamples, numSamplesToProcess);
}

void FloatPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float nextValue = (float)*ptr; 

	value_ = nextValue;
	Neon::splat(value_, outputSamples, numSamplesToProcess);	
}

void DoublePtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float nextValue = (float)*ptr; 
	
	value_ = nextValue;
	Neon::splat(value_, outputSamples, numSamplesToProcess);
}

void IntPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float nextValue = (float)*ptr; 
	
	value_ = nextValue;
	Neon::splat(value_, outputSamples, numSamplesToProcess);	
}

void BoolPtrUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float nextValue = (float)(*ptr != 0);
	
	value_ = nextValue;
	Neon::splat(value_, outputSamples, numSamplesToProcess);	
}

void MixUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	int channel = 0;
	
	bool shouldDeleteLocal = false;
	bool& shouldDeleteToPass = shouldAllowAutoDelete_ ? shouldDelete : shouldDeleteLocal;	
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();
	float* const channelSamples = inputs->processBlock(shouldDeleteToPass, blockID, channel);
	
	memcpy(outputSamples, channelSamples, numSamplesToProcess * sizeof(float));
	
	channel++;
	
	int numChannels = inputs->getNumChannels();
	
	for(/* leave channel value alone */; channel < numChannels; channel++)
	{
		shouldDeleteLocal = false;
		float* const channelSamples = inputs->processBlock(shouldDeleteToPass, blockID, channel);
		
		Neon::add(outputSamples, channelSamples, outputSamples, numSamplesToProcess);
	}	
}


void MixArrayUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	bool shouldDeleteLocal;
	bool& shouldDeleteToPass = shouldAllowAutoDelete_ ? shouldDelete : shouldDeleteLocal;	
	const int numOutputChannels = getNumChannels();
	const int blockSizeBytes = uGenOutput.getBlockSize() * sizeof(float);
	const int arraySize = array_.size();
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	
	for(int channel = 0; channel < numOutputChannels; channel++)
	{
		float* const outputSamples = proxies[channel]->getSampleData();		
		memset(outputSamples, 0, numSamplesToProcess * sizeof(float));
		
		for(int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			UGen& ugen = array_[arrayIndex];
			
			if(ugen.isNull(channel)) continue;
			
			if(shouldWrapChannels_ || (channel < ugen.getNumChannels()))
			{
				shouldDeleteLocal = false;
				float* const channelSamples = ugen.processBlock(shouldDeleteToPass, blockID, channel);
				Neon::add(outputSamples, channelSamples, outputSamples, numSamplesToProcess);			
			}
		}
	}
}


void MulAddUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();
	float* const inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* const mulSamples = inputs[Mul].processBlock(shouldDelete, blockID, channel);
	float* const addSamples = inputs[Add].processBlock(shouldDelete, blockID, channel);	
	Neon::muladd(inputSamples, mulSamples, addSamples, outputSamples, numSamplesToProcess);
}



void LinLinScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();
	const float* const inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	const float* const inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	const float* const inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	const float* const outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	const float* const outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	Neon::linlin(inputSamples, 
				 inLowSamples, inHighSamples, 
				 outLowSamples, outHighSamples, 
				 outputSamples, numSamplesToProcess);
}


void LinLinSignalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();
	const float* const inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	const float* const inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	const float* const inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	const float* const outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	const float* const outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	Neon::linlin(inputSamples, 
				 inLowSamples, inHighSamples, 
				 outLowSamples, outHighSamples, 
				 outputSamples, numSamplesToProcess);
}


END_UGEN_NAMESPACE

#endif
