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


// NB same names as SC but no derived code

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Delay.h"
#include "../basics/ugen_Temporary.h"
#include "../basics/ugen_InlineUnaryOps.h"


DelayBaseUGenInternal::DelayBaseUGenInternal(const int numInputs,
											 UGen const& input, 
											 UGen const& delayTime, 
											 Buffer const& delayBuffer,
											 const bool isMultiTap) throw()
:	ProxyOwnerUGenInternal(numInputs, isMultiTap ? delayTime.getNumChannels()-1 : 0),
	delayBuffer_(delayBuffer),
	delayBufferSize(delayBuffer_.size()),
	bufferSamples(delayBuffer_.getData(0)),
	bufferWritePos(0)
{
	inputs[Input] = input;
	inputs[DelayTime] = delayTime;
}

DelayNUGenInternal::DelayNUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw()
:	DelayBaseUGenInternal(NumInputs, input, delayTime, delayBuffer, false)
{ 
}

UGenInternal* DelayNUGenInternal::getChannel(const int channel) throw()
{
	return new DelayNUGenInternal(inputs[Input].getChannel(channel), 
								  inputs[DelayTime].getChannel(channel), 
								  Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void DelayNUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;
		
		while(numFramesThisTime--)
		{
			bufferSamples[bufferWritePos] = *inputSamples++;
			
			int bufferReadPos = bufferWritePos - (int)(*delayTimeSamples++ * sampleRate);
			if(bufferReadPos < 0)
				bufferReadPos += (int)delayBufferSize;
			
			*outputSamples++ = bufferSamples[bufferReadPos];
			
			bufferWritePos++;
		}
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}	
	
	LOCAL_COPY(bufferWritePos);
}

DelayNMultiUGenInternal::DelayNMultiUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw()
:	DelayBaseUGenInternal(NumInputs, input, delayTime, delayBuffer, true)
{ 
}

UGenInternal* DelayNMultiUGenInternal::getChannel(const int channel) throw()
{
	return new DelayNMultiUGenInternal(inputs[Input].getChannel(channel), 
									   inputs[DelayTime].getChannel(channel), 
									   delayBuffer_);
}

void DelayNMultiUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const float sampleRate = UGen::getSampleRate();
	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, 0);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	int bufferWritePos = this->bufferWritePos;
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;

		while(numFramesThisTime--)
		{
			bufferSamples[bufferWritePos] = *inputSamples++;
			
			int bufferReadPos = bufferWritePos - (int)(*delayTimeSamples++ * sampleRate);
			if(bufferReadPos < 0)
				bufferReadPos += (int)delayBufferSize;
			
			*outputSamples++ = bufferSamples[bufferReadPos];
			
			bufferWritePos++;
		}
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}	
	
	const int numChannels = getNumChannels();
	for(int channel = 1; channel < numChannels; channel++)
	{
		int numSamplesToProcess = uGenOutput.getBlockSize();
		float* outputSamples = proxies[channel]->getSampleData();
		float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
		int bufferWritePos = this->bufferWritePos; // fresh copy
		
		while(numSamplesToProcess > 0)
		{			
			int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
			
			int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
			numSamplesToProcess -= numFramesThisTime;

			while(numFramesThisTime--)
			{
				int bufferReadPos = bufferWritePos - (int)(*delayTimeSamples++ * sampleRate);
				if(bufferReadPos < 0)
					bufferReadPos += (int)delayBufferSize;
				
				*outputSamples++ = bufferSamples[bufferReadPos];
				
				bufferWritePos++;
			}
			
			if(bufferWritePos >= delayBufferSize)
				bufferWritePos = 0;
		}			
	}
	
	this->bufferWritePos = bufferWritePos;
}


DelayLUGenInternal::DelayLUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw()
:	DelayBaseUGenInternal(NumInputs, input, delayTime, delayBuffer, false)
{ 
}

UGenInternal* DelayLUGenInternal::getChannel(const int channel) throw()
{
	return new DelayLUGenInternal(inputs[Input].getChannel(channel), 
								  inputs[DelayTime].getChannel(channel), 
								  Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void DelayLUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;

		while(numFramesThisTime--)
		{
			bufferSamples[bufferWritePos] = *inputSamples++;
			
			float bufferReadPos = (float)bufferWritePos - *delayTimeSamples++ * sampleRate;
			if(bufferReadPos < 0.f)
				bufferReadPos += (float)delayBufferSize;
			
			*outputSamples++ = lookupIndexL(bufferReadPos);
			
			bufferWritePos++;
		}
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}
	
	LOCAL_COPY(bufferWritePos);
}

DelayLMultiUGenInternal::DelayLMultiUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw()
:	DelayBaseUGenInternal(NumInputs, input, delayTime, delayBuffer, true)
{ 
}

UGenInternal* DelayLMultiUGenInternal::getChannel(const int channel) throw()
{
	return new DelayLMultiUGenInternal(inputs[Input].getChannel(channel), 
									   inputs[DelayTime].getChannel(channel), 
									   delayBuffer_);
}

void DelayLMultiUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, 0);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	int bufferWritePos = this->bufferWritePos;
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;

		while(numFramesThisTime--)
		{
			bufferSamples[bufferWritePos] = *inputSamples++;
			
			float bufferReadPos = (float)bufferWritePos - *delayTimeSamples++ * sampleRate;
			if(bufferReadPos < 0.f)
				bufferReadPos += (float)delayBufferSize;
			
			*outputSamples++ = lookupIndexL(bufferReadPos);
			
			bufferWritePos++;
		}

		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}
	
	const int numChannels = getNumChannels();
	for(int channel = 1; channel < numChannels; channel++)
	{
		int numSamplesToProcess = uGenOutput.getBlockSize();
		float* outputSamples = proxies[channel]->getSampleData();
		float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
		int bufferWritePos = this->bufferWritePos; // fresh copy
		
		//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
		
		while(numSamplesToProcess > 0)
		{			
			int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
			
			int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
			numSamplesToProcess -= numFramesThisTime;

			while(numFramesThisTime--)
			{
				float bufferReadPos = (float)bufferWritePos - *delayTimeSamples++ * sampleRate;
				if(bufferReadPos < 0.f)
					bufferReadPos += (float)delayBufferSize;
				
				*outputSamples++ = lookupIndexL(bufferReadPos);
				
				bufferWritePos++;
			}
			
			if(bufferWritePos >= delayBufferSize)
				bufferWritePos = 0;
		}
	}
	
	this->bufferWritePos = bufferWritePos;	
}

RecircBaseUGenInternal::RecircBaseUGenInternal(UGen const& input, 
											   UGen const& delayTime, 
											   UGen const& decayTime, 
											   Buffer const& delayBuffer) throw()
:	DelayBaseUGenInternal(NumInputs, input, delayTime, delayBuffer, false),
	currentDelay(0.f),
	currentDecay(0.f),
	feedback(0.f)
{
	inputs[DecayTime] = decayTime;
}

CombNUGenInternal::CombNUGenInternal(UGen const& input, 
									 UGen const& delayTime, 
									 UGen const& decayTime, 
									 Buffer const& delayBuffer) throw()
:	RecircBaseUGenInternal(input, delayTime, decayTime, delayBuffer)
{
}

UGenInternal* CombNUGenInternal::getChannel(const int channel) throw()
{
	return new CombNUGenInternal(inputs[Input].getChannel(channel), 
								 inputs[DelayTime].getChannel(channel),
								 inputs[DecayTime].getChannel(channel),
								 Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void CombNUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	float* decayTimeSamples = inputs[DecayTime].processBlock(shouldDelete, blockID, channel);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	LOCAL_DECLARE(float, currentDecay);
	LOCAL_DECLARE(float, feedback);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	//ugen_assert(*decayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	if(*decayTimeSamples != currentDecay)
	{
		currentDecay = *decayTimeSamples;
		feedback = calcFeedback(*delayTimeSamples, currentDecay);
		LOCAL_COPY(currentDecay);
		LOCAL_COPY(feedback);
	}
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;
		
		while(numFramesThisTime--)
		{
			int bufferReadPos = bufferWritePos - (int)(*delayTimeSamples++ * sampleRate);
			if(bufferReadPos < 0)
				bufferReadPos += (int)delayBufferSize;
			
			float value = bufferSamples[bufferReadPos];
			
			bufferSamples[bufferWritePos] = value * feedback + *inputSamples++;
			
			*outputSamples++ = value;
			
			bufferWritePos++;
		}
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}	
	
	LOCAL_COPY(bufferWritePos);
}


CombLUGenInternal::CombLUGenInternal(UGen const& input, 
									 UGen const& delayTime, 
									 UGen const& decayTime, 
									 Buffer const& delayBuffer) throw()
:	RecircBaseUGenInternal(input, delayTime, decayTime, delayBuffer)
{
}

UGenInternal* CombLUGenInternal::getChannel(const int channel) throw()
{
	return new CombLUGenInternal(inputs[Input].getChannel(channel), 
								 inputs[DelayTime].getChannel(channel),
								 inputs[DecayTime].getChannel(channel),
								 Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void CombLUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	float* decayTimeSamples = inputs[DecayTime].processBlock(shouldDelete, blockID, channel);
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	LOCAL_DECLARE(float, currentDecay);
	LOCAL_DECLARE(float, feedback);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	//ugen_assert(*decayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	if(*decayTimeSamples != currentDecay)
	{
		currentDecay = *decayTimeSamples;
		feedback = calcFeedback(*delayTimeSamples, currentDecay);
		LOCAL_COPY(currentDecay);
		LOCAL_COPY(feedback);
	}
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		int numSamplesThisTime = min(bufferSamplesRemaining, numSamplesToProcess);
		
		numSamplesToProcess -= numSamplesThisTime;
		
		while(numSamplesThisTime--)
		{
			float bufferReadPos = (float)bufferWritePos - *delayTimeSamples++ * sampleRate;
			if(bufferReadPos < 0.f)
				bufferReadPos += (float)delayBufferSize;
			
			float value = lookupIndexL(bufferReadPos);
			
			bufferSamples[bufferWritePos] = value * feedback + *inputSamples++;
			
			*outputSamples++ = value;
			
			bufferWritePos++;
		}	
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}
	
	LOCAL_COPY(bufferWritePos);
}

AllpassNUGenInternal::AllpassNUGenInternal(UGen const& input, 
										   UGen const& delayTime, 
										   UGen const& decayTime, 
										   Buffer const& delayBuffer) throw()
:	RecircBaseUGenInternal(input, delayTime, decayTime, delayBuffer)
{
}

UGenInternal* AllpassNUGenInternal::getChannel(const int channel) throw()
{
	return new AllpassNUGenInternal(inputs[Input].getChannel(channel), 
									inputs[DelayTime].getChannel(channel),
									inputs[DecayTime].getChannel(channel),
									Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void AllpassNUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	float* decayTimeSamples = inputs[DecayTime].processBlock(shouldDelete, blockID, channel);
	bool doesNotNeedToCalculate = true;
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	LOCAL_DECLARE(float, currentDelay);
	LOCAL_DECLARE(float, currentDecay);
	LOCAL_DECLARE(float, feedback);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	//ugen_assert(*decayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	if(*decayTimeSamples != currentDecay)	
	{
		currentDecay = *decayTimeSamples;
		doesNotNeedToCalculate = false;
		LOCAL_COPY(currentDecay);
	}
	
	float nextDelay = *delayTimeSamples * sampleRate;
	
	if(nextDelay != currentDelay)	
	{
		currentDelay = nextDelay;
		doesNotNeedToCalculate = false;
		LOCAL_COPY(currentDelay);
	}
	//// NEED TO SLOPE THE DELAY TIME IF CHANGED, NEED TO DO DECAY- SC DOES!!! ////
	
	if(doesNotNeedToCalculate == false)
	{
		feedback = calcFeedback(*delayTimeSamples, currentDecay);
		LOCAL_COPY(feedback);
	}
	
	float currentDelayMax1 = std::max(1.f, currentDelay);
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		
		int numFramesThisTime = ugen::min(bufferSamplesRemaining, numSamplesToProcess);
		numSamplesToProcess -= numFramesThisTime;

		while(numFramesThisTime--)
		{
			int bufferReadPos = bufferWritePos - (int)(currentDelayMax1);
			if(bufferReadPos < 0)
				bufferReadPos += (int)delayBufferSize;
			
			float inValue = bufferSamples[bufferReadPos];
			float outValue = inValue * feedback + *inputSamples++;
			
			bufferSamples[bufferWritePos] = outValue;
			
			*outputSamples++ = inValue - feedback * outValue;
			
			bufferWritePos++;
		}
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}	
	
	LOCAL_COPY(bufferWritePos);
}

AllpassLUGenInternal::AllpassLUGenInternal(UGen const& input, 
										   UGen const& delayTime, 
										   UGen const& decayTime, 
										   Buffer const& delayBuffer) throw()
:	RecircBaseUGenInternal(input, delayTime, decayTime, delayBuffer)
{
}

UGenInternal* AllpassLUGenInternal::getChannel(const int channel) throw()
{
	return new AllpassLUGenInternal(inputs[Input].getChannel(channel), 
									inputs[DelayTime].getChannel(channel),
									inputs[DecayTime].getChannel(channel),
									Buffer(BufferSpec(delayBuffer_.size(), 1, true)));
}

void AllpassLUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
	float* decayTimeSamples = inputs[DecayTime].processBlock(shouldDelete, blockID, channel);
	bool doesNotNeedToCalculate = true;
	LOCAL_DECLARE(float * const, bufferSamples);
	LOCAL_DECLARE(const double, delayBufferSize);
	LOCAL_DECLARE(int, bufferWritePos);
	LOCAL_DECLARE(float, currentDelay);
	LOCAL_DECLARE(float, currentDecay);
	LOCAL_DECLARE(float, feedback);
	
	//ugen_assert(*delayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	//ugen_assert(*decayTimeSamples >= 0.f); // not foolproof but better than checking every sample
	
	if(*decayTimeSamples != currentDecay)	
	{
		currentDecay = *decayTimeSamples;
		doesNotNeedToCalculate = false;
		LOCAL_COPY(currentDecay);
	}
	
	float nextDelay = *delayTimeSamples * sampleRate;
	
	if(nextDelay != currentDelay)	
	{
		currentDelay = nextDelay;
		doesNotNeedToCalculate = false;
		LOCAL_COPY(currentDelay);
	}
	//// NEED TO SLOPE THE DELAY TIME IF CHANGED, NEED TO DO DECAY- SC DOES!!! ////
	
	if(doesNotNeedToCalculate == false)
	{
		feedback = calcFeedback(*delayTimeSamples, currentDecay);
		LOCAL_COPY(feedback);
	}
	
	float currentDelayMax1 = std::max(1.f, currentDelay);
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesRemaining = delayBufferSize - bufferWritePos;
		int numSamplesThisTime = min(bufferSamplesRemaining, numSamplesToProcess);
		
		numSamplesToProcess -= numSamplesThisTime;
		
		while(numSamplesThisTime--)
		{
			float bufferReadPos = bufferWritePos - currentDelayMax1;
			if(bufferReadPos < 0.f)
				bufferReadPos += (float)delayBufferSize;
			
			float inValue = lookupIndexL(bufferReadPos);
			float outValue = inValue * feedback + *inputSamples++;
			
			bufferSamples[bufferWritePos]  = outValue;
			
			*outputSamples++ = inValue - feedback * outValue;
			
			bufferWritePos++;
		}	
		
		if(bufferWritePos >= delayBufferSize)
			bufferWritePos = 0;
	}
	
	LOCAL_COPY(bufferWritePos);
}

DelayN::DelayN(UGen const& input, const float maximumDelayTime, UGen const& delayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	const int numInputChannels = input.getNumChannels();
	const int numDelayTimeChannels = delayTime.getNumChannels();
	
	if(numInputChannels == 1 && numDelayTimeChannels > 1)
	{
		initInternal(numDelayTimeChannels);
		Buffer delayBuffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true));
		generateFromProxyOwner(new DelayNMultiUGenInternal(input, delayTime, delayBuffer));
	}
	else if(numDelayTimeChannels > numInputChannels)
	{
		initInternal(numDelayTimeChannels);
		Buffer delayBuffers(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 
									   numInputChannels, 
									   true));
		
		int inputChannel = 0;
		ProxyOwnerUGenInternal* proxyOwner = 0;
		for(int delayChannel = 0; delayChannel < numDelayTimeChannels; delayChannel++)
		{
			if(inputChannel == 0)
			{
				proxyOwner = new DelayNMultiUGenInternal(input[inputChannel], 
														 delayTime[delayChannel], 
														 delayBuffers[inputChannel]);
				internalUGens[delayChannel] = proxyOwner;
			}
			else
			{
				internalUGens[delayChannel] = proxyOwner->getProxy(delayChannel);
			}
			
			inputChannel++;
			if(inputChannel == numInputChannels)
				inputChannel = 0;
		}
	}
	else
	{
		initInternal(ugen::max(numInputChannels, numDelayTimeChannels));
		
		for(int i = 0; i < numInternalUGens; i++)
		{
			Buffer delayBuffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true));
			internalUGens[i] = new DelayNUGenInternal(input, delayTime, delayBuffer);
		}	
	}
}

DelayL::DelayL(UGen const& input, const float maximumDelayTime, UGen const& delayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	const int numInputChannels = input.getNumChannels();
	const int numDelayTimeChannels = delayTime.getNumChannels();
	
	if(numInputChannels == 1 && numDelayTimeChannels > 1)
	{
		initInternal(numDelayTimeChannels);
		Buffer delayBuffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true));
		generateFromProxyOwner(new DelayLMultiUGenInternal(input, delayTime, delayBuffer));
	}
	else if(numDelayTimeChannels > numInputChannels)
	{
		initInternal(numDelayTimeChannels);
		Buffer delayBuffers(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 
									   numInputChannels, 
									   true));
		
		int inputChannel = 0;
		ProxyOwnerUGenInternal* proxyOwner = 0;
		for(int delayChannel = 0; delayChannel < numDelayTimeChannels; delayChannel++)
		{
			if(inputChannel == 0)
			{
				proxyOwner = new DelayLMultiUGenInternal(input[inputChannel], 
														 delayTime[delayChannel], 
														 delayBuffers[inputChannel]);
				internalUGens[delayChannel] = proxyOwner;
			}
			else
			{
				internalUGens[delayChannel] = proxyOwner->getProxy(delayChannel);
			}
			
			inputChannel++;
			if(inputChannel == numInputChannels)
				inputChannel = 0;
		}
	}
	else
	{
		initInternal(ugen::max(numInputChannels, numDelayTimeChannels));
		
		for(int i = 0; i < numInternalUGens; i++)
		{
			Buffer delayBuffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true));
			internalUGens[i] = new DelayLUGenInternal(input, delayTime, delayBuffer);
		}	
	}
}

CombN::CombN(UGen const& input, const float maximumDelayTime, UGen const& delayTime, UGen const& decayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	UGen inputs[] = { input, delayTime, decayTime };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);	
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new CombNUGenInternal(input, 
												 delayTime,//Clip(delayTime, 0.f, maximumDelayTime),
												 decayTime,
												 Buffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true)));
	}	
}

CombL::CombL(UGen const& input, const float maximumDelayTime, UGen const& delayTime, UGen const& decayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	UGen inputs[] = { input, delayTime, decayTime };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);	
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new CombLUGenInternal(input, 
												 delayTime,//Clip(delayTime, 0.f, maximumDelayTime),
												 decayTime,
												 Buffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true)));
	}	
}

AllpassN::AllpassN(UGen const& input, const float maximumDelayTime, UGen const& delayTime, UGen const& decayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	UGen inputs[] = { input, delayTime, decayTime };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);	
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new AllpassNUGenInternal(input, 
													delayTime,//Clip(delayTime, 0.f, maximumDelayTime),
													decayTime,
													Buffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true)));
	}	
}

AllpassL::AllpassL(UGen const& input, const float maximumDelayTime, UGen const& delayTime, UGen const& decayTime) throw()
{
	ugen_assert(maximumDelayTime >= 0.f);
	
	UGen inputs[] = { input, delayTime, decayTime };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);	
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new AllpassLUGenInternal(input, 
													delayTime,//Clip(delayTime, 0.f, maximumDelayTime),
													decayTime,
													Buffer(BufferSpec(int(UGen::getSampleRate() * maximumDelayTime) + 1, 1, true)));
	}	
}


TapInUGenInternal::TapInUGenInternal(UGen const& input,
									 Buffer const& buffer, 
									 UGen const& recLevel,
									 UGen const& preLevel) throw()
:	ProxyOwnerUGenInternal(NumInputs, ugen::max(buffer.getNumChannels(), input.getNumChannels())-1),
	buffer_(buffer)
{
	inputs[Input] = input;
	inputs[RecLevel] = recLevel;
	inputs[PreLevel] = preLevel;
}

UGenInternal* TapInUGenInternal::getChannel(const int channel) throw()
{
	return new TapInUGenInternal(inputs[Input].getChannel(channel),
								 buffer_.getChannel(channel), 
								 inputs[RecLevel].getChannel(channel), 
								 inputs[PreLevel].getChannel(channel));	
}

void TapInUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
		
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		int numSamplesToProcess = blockSize;
		const int bufferSize = buffer_.size();
		float* outputSamples = proxies[channel]->getSampleData();
		float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
		float* preLevelSamples = inputs[PreLevel].processBlock(shouldDelete, blockID, channel);
		float* recLevelSamples = inputs[RecLevel].processBlock(shouldDelete, blockID, channel);
		float* bufferSamples = buffer_.getData(channel);
		int channelBufferPos = buffer_.getCircularHead(blockID, channel);
		
		while(numSamplesToProcess) 
		{							
			float recLevel = *recLevelSamples++;
			float rec = *inputSamples++ * recLevel;
			float preLevel = *preLevelSamples++;
			float pre = bufferSamples[channelBufferPos] * preLevel;
			float out = pre + rec;
			*outputSamples++ = out;
			bufferSamples[channelBufferPos] = out;
			channelBufferPos++;
			
			if(channelBufferPos >= bufferSize)
				channelBufferPos = 0;
			
			--numSamplesToProcess;
		}		
		
		buffer_.setCircularHead(blockID, channel, channelBufferPos);
	}
}

TapIn::TapIn(UGen const& input,
			 Buffer const& _buffer, 
			 UGen const& recLevel,
			 UGen const& preLevel) throw()
{
	ugen_assert(_buffer.size() > 0);
	ugen_assert(_buffer.getNumChannels() > 0);
	
	Buffer buffer = _buffer;
	
	for(int i = 0; i < buffer.getNumChannels(); i++)
	{
		buffer.setCircularHead(0, i, 0);
	}
	
	const int numChannels = ugen::max(buffer.getNumChannels(), input.getNumChannels());
	initInternal(numChannels);
	generateFromProxyOwner(new TapInUGenInternal(input, buffer, recLevel, preLevel));	
}

TapOutNUGenInternal::TapOutNUGenInternal(Buffer const& buffer, UGen const& delayTime) throw()
:	ProxyOwnerUGenInternal(NumInputs, ugen::max(buffer.getNumChannels(), delayTime.getNumChannels())-1),	
	buffer_(buffer)
{
	inputs[DelayTime] = delayTime;
}

UGenInternal* TapOutNUGenInternal::getChannel(const int channel) throw()
{
	return new TapOutNUGenInternal(buffer_.getChannel(channel), inputs[DelayTime].getChannel(channel));	
}

void TapOutNUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	const int blockSize = uGenOutput.getBlockSize();
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		float* outputSamples = proxies[channel]->getSampleData();
		int channelBufferPos = buffer_.getCircularHead(blockID, channel);
		
		if(channelBufferPos < 0)
		{
			memset(outputSamples, 0, blockSize * sizeof(float));
		}
		else
		{		
			int numSamplesToProcess = blockSize;
			const int bufferSize = buffer_.size();
			const float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);
			float* bufferSamples = buffer_.getData(channel);

			while(numSamplesToProcess--) 
			{							
				int bufferReadPos = channelBufferPos - (int)(*delayTimeSamples++ * sampleRate);
				if(bufferReadPos < 0)
					bufferReadPos += bufferSize;
				*outputSamples++ = bufferSamples[bufferReadPos];
				channelBufferPos++;
				if(channelBufferPos >= bufferSize)
					channelBufferPos -= bufferSize;
			}	
		}
	}	
}

TapOutLUGenInternal::TapOutLUGenInternal(Buffer const& buffer, UGen const& delayTime) throw()
:	ProxyOwnerUGenInternal(NumInputs, ugen::max(buffer.getNumChannels(), delayTime.getNumChannels())-1),	
	buffer_(buffer)
{
	inputs[DelayTime] = delayTime;
}

UGenInternal* TapOutLUGenInternal::getChannel(const int channel) throw()
{
	return new TapOutLUGenInternal(buffer_.getChannel(channel), inputs[DelayTime].getChannel(channel));	
}

void TapOutLUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const float sampleRate = UGen::getSampleRate();
	const int blockSize = uGenOutput.getBlockSize();
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		const float bufferSize = buffer_.size();
		float* outputSamples = proxies[channel]->getSampleData();
		float channelBufferPos = (float)buffer_.getCircularHead(blockID, channel);
		
		if(channelBufferPos < 0.f)
		{
			memset(outputSamples, 0, blockSize * sizeof(float));
		}
		else
		{
			const float floatBlockSize = blockSize;
			int numSamplesToProcess = blockSize;
			const int bufferChannel = channel % buffer_.getNumChannels();
			const float* delayTimeSamples = inputs[DelayTime].processBlock(shouldDelete, blockID, channel);

			while(numSamplesToProcess--) 
			{				
				float delay = ugen::max(floatBlockSize, *delayTimeSamples++ * sampleRate);
				float bufferReadPos = channelBufferPos - delay;
				if(bufferReadPos < 0.f)
					bufferReadPos += bufferSize;
				*outputSamples++ = buffer_.getSampleUnchecked(bufferChannel, bufferReadPos);
				channelBufferPos += 1.f;
				if(channelBufferPos >= bufferSize)
					channelBufferPos -= bufferSize;
			}	
		}
	}	
}

TapOutN::TapOutN(Buffer const& buffer, UGen const& delayTime) throw()
{
	ugen_assert(buffer.size() > 0);
	ugen_assert(buffer.getNumChannels() > 0);
		
	const int numChannels = ugen::max(buffer.getNumChannels(), delayTime.getNumChannels());
	initInternal(numChannels);
	generateFromProxyOwner(new TapOutNUGenInternal(buffer, delayTime));	
}

TapOutL::TapOutL(Buffer const& buffer, UGen const& delayTime) throw()
{
	ugen_assert(buffer.size() > 0);
	ugen_assert(buffer.getNumChannels() > 0);
	
	const int numChannels = ugen::max(buffer.getNumChannels(), delayTime.getNumChannels());
	initInternal(numChannels);
	generateFromProxyOwner(new TapOutLUGenInternal(buffer, delayTime));	
}



END_UGEN_NAMESPACE
