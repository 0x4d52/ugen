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
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_BinaryOpUGens.h"
#include "ugen_MulAdd.h"

MulAddUGenInternal::MulAddUGenInternal(UGen const& input, UGen const& mul, UGen const& add) throw()
:	UGenInternal(NumInputs)
{
	inputs[Input] = input;
	inputs[Mul] = mul;
	inputs[Add] = add;
}

UGenInternal* MulAddUGenInternal::getChannel(const int channel) throw()
{		
	return new MulAddUGenInternal(inputs[Input].getChannel(channel),
								  inputs[Mul].getChannel(channel),
								  inputs[Add].getChannel(channel));
}	

UGenInternal* MulAddUGenInternal::getKr() throw()
{ 
	return new MulAddUGenInternal(inputs[Input].kr(), inputs[Mul].kr(), inputs[Add].kr()); 
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void MulAddUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* mulSamples = inputs[Mul].processBlock(shouldDelete, blockID, channel);
	float* addSamples = inputs[Add].processBlock(shouldDelete, blockID, channel);
	while(numSamplesToProcess--)
	{
		*outputSamples++ = *inputSamples++ * *mulSamples++ + *addSamples++;
	}
}
#endif

void MulAddUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* mulSamples = inputs[Mul].processBlock(shouldDelete, blockID, channel);
	float* addSamples = inputs[Add].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)			
		{
			nextValue = *inputSamples * *mulSamples + *addSamples;
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		
		blockPosition		+= numKrSamples;
		inputSamples		+= numKrSamples;
		mulSamples			+= numKrSamples;
		addSamples			+= numKrSamples;
		
		if(value == nextValue)
		{
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = nextValue;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		else
		{
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = value;
				value += valueSlope;
				--numSamplesToProcess;
				--numKrSamples;
			}
			
			value = nextValue;
		}
	}
}

MulAdd::MulAdd(UGen const& input, UGen const& mul, UGen const& add) throw()
{	
	UGen inputs[] = { input, mul, add };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	
	ugen_assert(numInputChannels > 0);
	
	initInternal(numInputChannels);
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new MulAddUGenInternal(input, mul, add);
		internalUGens[i]->initValue(input.getValue(i) * mul.getValue(i) + add.getValue(i));
	}
}

END_UGEN_NAMESPACE
