// $Id: ugen_LeakDC.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/filters/ugen_LeakDC.cpp $

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

#include "ugen_LeakDC.h"
#include "../core/ugen_Constants.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"

LeakDCUGenInternal::LeakDCUGenInternal(UGen const& input, UGen const& coeff) throw()
:	UGenInternal(NumInputs),
	b1(0.f),
	y1(0.f),
	x1(0.f)
{
	inputs[Input] = input;
	inputs[Coeff] = coeff;
}

UGenInternal* LeakDCUGenInternal::getKr() throw()
{
	return new LeakDCUGenInternalK(inputs[Input].kr(), inputs[Coeff].kr()); 
}

UGenInternal* LeakDCUGenInternal::getChannel(const int channel) throw()
{
	return new LeakDCUGenInternal(inputs[Input].getChannel(channel), 
								  inputs[Coeff].getChannel(channel));
}

void LeakDCUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* b1Samples = inputs[Coeff].processBlock(shouldDelete, blockID, channel);
	float newb1 = *b1Samples;
	
	if(newb1 != b1)
	{
		float b1_slope = (newb1 - b1) * UGen::getSlopeFactor();
		
		while(numSamplesToProcess--)
		{
			float x0 = *inputSamples++; 
			*outputSamples++ = y1 = x0 - x1 + b1 * y1; 
			x1 = x0;
			b1 += b1_slope;			
		}
		
		b1 = newb1;
	}
	else
	{
		while(numSamplesToProcess--)
		{
			float x0 = *inputSamples++; 
			*outputSamples++ = y1 = x0 - x1 + b1 * y1; 
			x1 = x0;
		}
	}
	
	y1 = zap(y1); // denormal!
}

void LeakDCUGenInternal::initValue(const float value) throw()
{
	float checkedValue = zap(value);
	UGenInternal::initValue(checkedValue);
	y1 = checkedValue;
}

void LeakDCUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* b1Samples = inputs[Coeff].processBlock(shouldDelete, blockID, channel);
	float newb1 = *b1Samples;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	if(newb1 != b1)
	{
		float b1_slope = (newb1 - b1) * UGen::getSlopeFactor() * krBlockSize;
						
		while(numSamplesToProcess > 0)
		{
			if(numKrSamples == 0)			
			{
				b1 += b1_slope;	
				float x0 = *inputSamples; 
				value = y1 = x0 - x1 + b1 * y1; 
				x1 = x0;
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = value;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		
		b1 = newb1;
		
	}
	else
	{
		while(numSamplesToProcess > 0)
		{
			if(numKrSamples == 0)			
			{
				float x0 = *inputSamples; 
				value = y1 = x0 - x1 + b1 * y1; 
				x1 = x0;
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = value;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
	}
	
	y1 = zap(y1);
}

LeakDC::LeakDC(UGen const& input, UGen const& coeff) throw()
{
	UGen inputs[] = { input, coeff };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);		
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new LeakDCUGenInternal(input, coeff);
		internalUGens[i]->initValue(input.getValue(i));
	}
}


END_UGEN_NAMESPACE
