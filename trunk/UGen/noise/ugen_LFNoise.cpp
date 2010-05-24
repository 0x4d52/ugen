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

#include "ugen_LFNoise.h"
#include "../basics/ugen_InlineBinaryOps.h"

LFNoise0UGenInternal::LFNoise0UGenInternal(UGen const& freq) throw()
:	UGenInternal(NumInputs),
	//random((unsigned int)this * 123463463UL + 423815L + rand(92557)),
	random(rand(0x7fffffff)),
	currentValue(random.nextBiFloat()),
	counter(0)
{
	inputs[Freq] = freq;
	
	initValue(currentValue);
}

UGenInternal* LFNoise0UGenInternal::getChannel(const int channel) throw()
{
	return new LFNoise0UGenInternal(inputs[Freq].getChannel(channel));
}

void LFNoise0UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float currentFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	
	unsigned long s1, s2, s3;
	random.get(s1, s2, s3);
	
	do {
		if (counter <= 0) {
			counter = int(UGen::getSampleRate() / max(currentFreq, 0.001f));
			counter = max(1, counter);
			currentValue = Ran088::nextBiFloat(s1,s2,s3);
		}
		int samplesThisTime = min(numSamplesToProcess, counter);
		numSamplesToProcess -= samplesThisTime;
		counter -= samplesThisTime;
		
		while(samplesThisTime--)
		{
			*outputSamples++ = currentValue;
		}
		
	} while (numSamplesToProcess);
	
	random.set(s1, s2, s3);
}

LFNoise1UGenInternal::LFNoise1UGenInternal(UGen const& freq) throw()
:	UGenInternal(NumInputs),
	//random((unsigned int)this * 123463463UL + 423815L + rand(54288)),
	random(rand(0x7fffffff)),
	currentValue(random.nextBiFloat()),
	slope(0.f),
	counter(0)
{
	inputs[Freq] = freq;
	
	initValue(currentValue);
}

UGenInternal* LFNoise1UGenInternal::getChannel(const int channel) throw()
{
	return new LFNoise1UGenInternal(inputs[Freq].getChannel(channel));
}

void LFNoise1UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float currentFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	
	unsigned long s1, s2, s3;
	random.get(s1, s2, s3);
	
	do {
		if (counter <= 0) {
			counter = int(UGen::getSampleRate() / max(currentFreq, 0.001f));
			counter = max(1, counter);
			float nextValue = Ran088::nextBiFloat(s1,s2,s3);
			slope = (nextValue - currentValue) / counter;
		}
		int samplesThisTime = min(numSamplesToProcess, counter);
		numSamplesToProcess -= samplesThisTime;
		counter -= samplesThisTime;
		
		while(samplesThisTime--)
		{
			*outputSamples++ = currentValue;
			currentValue += slope;
		}
		
	} while (numSamplesToProcess);
	
	random.set(s1, s2, s3);
}

LFNoise2UGenInternal::LFNoise2UGenInternal(UGen const& freq) throw()
:	UGenInternal(NumInputs),
	//random((unsigned int)this * 123463463UL + 423815L + rand(8277)),
	random(rand(0x7fffffff)),
	currentValue(random.nextBiFloat()),
	nextValue(random.nextBiFloat()),
	nextMidPoint(nextValue * 0.5f),
	curve(0.f),
	slope(0.f),
	counter(0)
{
	inputs[Freq] = freq;
	
	initValue(currentValue);
}

UGenInternal* LFNoise2UGenInternal::getChannel(const int channel) throw()
{
	return new LFNoise2UGenInternal(inputs[Freq].getChannel(channel));
}

void LFNoise2UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float currentFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	
	unsigned long s1, s2, s3;
	random.get(s1, s2, s3);
	
	do {
		if (counter <= 0) {
			float value = nextValue;
			nextValue = Ran088::nextBiFloat(s1,s2,s3);
			currentValue = nextMidPoint;
			nextMidPoint = (nextValue + value) * 0.5f;
			
			counter = int(UGen::getSampleRate() / max(currentFreq, 0.001f));
			counter = max(2, counter);
			float fseglen = (float)counter;
			curve = 2.f * (nextMidPoint - currentValue - fseglen * slope) / (fseglen * fseglen + fseglen);
		}
		int samplesThisTime = min(numSamplesToProcess, counter);
		numSamplesToProcess -= samplesThisTime;
		counter -= samplesThisTime;
		
		while(samplesThisTime--)
		{
			*outputSamples++ = currentValue;
			slope += curve;
			currentValue += slope;
		}
		
	} while (numSamplesToProcess);
	
	random.set(s1, s2, s3);
}

LFNoise0::LFNoise0(UGen const& freq) throw()
{
	initInternal(freq.getNumChannels());
	for(int i = 0; i < numInternalUGens; i++)
		internalUGens[i] = new LFNoise0UGenInternal(freq);
}

LFNoise1::LFNoise1(UGen const& freq) throw()
{
	initInternal(freq.getNumChannels());
	for(int i = 0; i < numInternalUGens; i++)
		internalUGens[i] = new LFNoise1UGenInternal(freq);
}

LFNoise2::LFNoise2(UGen const& freq) throw()
{
	initInternal(freq.getNumChannels());
	for(int i = 0; i < numInternalUGens; i++)
		internalUGens[i] = new LFNoise2UGenInternal(freq);
}

END_UGEN_NAMESPACE
