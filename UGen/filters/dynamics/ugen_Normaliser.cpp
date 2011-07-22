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

#ifndef UGEN_NOEXTGPL

#include "../../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Normaliser.h"

NormaliserUGenInternal::NormaliserUGenInternal(UGen const& input, UGen const& level, const float duration) throw()
:	UGenInternal(NumInputs),
	numBuffersProcessed(0),
	bufferPosition(0),
	bufferSize((int)ugen::ceil(duration * UGen::getSampleRate())),
	slope(0.f),
	level(1.f),
	currentMaximum(0.f),
	prevMaximum(0.f),
	slopeFactor(1.f/ bufferSize),
	inputBuffer(Buffer::newClear(bufferSize, 1)),
	midBuffer(Buffer::newClear(bufferSize, 1)),
	outputBuffer(Buffer::newClear(bufferSize, 1))
{
	inputs[Input] = input;
	inputs[Level] = level;
}

UGenInternal* NormaliserUGenInternal::getChannel(const int channel) throw()
{
	return new NormaliserUGenInternal(inputs[Input].getChannel(channel),
									  inputs[Level].getChannel(channel),
									  bufferSize * UGen::getReciprocalSampleRate());
}

void NormaliserUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	
	int bufferRemaining = bufferSize - bufferPosition;
	
	float value;
	float amp = *(inputs[Level].processBlock(shouldDelete, blockID, channel));
	
	while (numSamplesToProcess > 0) 
	{
		int numSamplesThisTime = ugen::min(numSamplesToProcess, bufferRemaining);
		float* inputBufferSamples = inputBuffer.getData(0) + bufferPosition;
		float* outputBufferSamples = outputBuffer.getData(0) + bufferPosition;
		
		bufferPosition += numSamplesThisTime;
		numSamplesToProcess -= numSamplesThisTime;
		
		if (numBuffersProcessed >= 2) 
		{
			while(numSamplesThisTime--)
			{
				value = *inputSamples++;
				*inputBufferSamples++ = value;
				*outputSamples++ = level * *outputBufferSamples++;
				level += slope;
				value = std::abs(value);
				
				if (value > currentMaximum) 
					currentMaximum = value;
			}
		} 
		else 
		{
			while(numSamplesThisTime--)
			{
				value = *inputSamples++;
				*inputBufferSamples++ = value;
				*outputSamples++ = 0.f;
				level += slope;
				value = std::abs(value);
				
				if (value > currentMaximum) 
					currentMaximum = value;
			}
		}
		
		
		
		if (bufferPosition >= bufferSize) 
		{
			bufferPosition = 0;
			bufferRemaining = bufferSize;
			
			float nextMaximum = ugen::max(prevMaximum, currentMaximum);
			prevMaximum = currentMaximum;
			currentMaximum = 0.f;
			
			float nextLevel = nextMaximum <= 0.00001f ? 100000.f * amp : amp / nextMaximum;
			
			slope = (nextLevel - level) * slopeFactor;
			
			Buffer temp = outputBuffer;
			outputBuffer = midBuffer;
			midBuffer = inputBuffer;
			inputBuffer = temp;
			
			numBuffersProcessed++;
		}
	}
	
}

Normaliser::Normaliser(UGen const& input, UGen const& level, const float duration) throw()
{
	initInternal(ugen::max(input.getNumChannels(), level.getNumChannels()));
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new NormaliserUGenInternal(input, level, duration);
	}
}


END_UGEN_NAMESPACE

#endif // gpl