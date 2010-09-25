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

#include "../../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_LFSaw.h"


LFSawUGenInternal::LFSawUGenInternal(UGen const& freq, 
									 const float initialPhase) throw()
:	UGenInternal(NumInputs),
	currentPhase(ugen::clip2(initialPhase, 1.f))
{
	ugen_assert(initialPhase == currentPhase); // checking it was in range
				 
	inputs[Freq] = freq;
	initValue(currentPhase);
}

UGenInternal* LFSawUGenInternal::getChannel(const int channel) throw()
{
	return new LFSawUGenInternal(inputs[Freq].getChannel(channel), currentPhase);
}	

UGenInternal* LFSawUGenInternal::getKr() throw()
{ 
	return new LFSawUGenInternalK(inputs[Freq].kr(), currentPhase); 
}

void LFSawUGenInternal::processBlock(bool& shouldDelete, 
									 const unsigned int blockID, 
									 const int channel) throw()
{	
	double twoOverSampleRate = UGen::getReciprocalSampleRate() * 2.0;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = currentPhase;
		currentPhase += *freqSamples++ * twoOverSampleRate;
		
		if(currentPhase >= 1.f)			currentPhase -= 2.f;
		else if(currentPhase < -1.f)	currentPhase += 2.f;
	}
}

double LFSawUGenInternal::getDuration() const throw()
{
	return 1.0;
}

double LFSawUGenInternal::getPosition() const throw()
{
	return (double)(currentPhase * 0.5f + 0.5f);
}

void LFSawUGenInternal::setPosition(const double newPosition) throw()
{
	currentPhase = (float)ugen::clip2(newPosition * 2.0 - 1.0, 1.0);
}

LFSawUGenInternalK::LFSawUGenInternalK(UGen const& freq, const float initialPhase) throw()
:	LFSawUGenInternal(freq, initialPhase),
	value(currentPhase)
{
	rate = ControlRate;
}

void LFSawUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	double blockSizeOverSampleRatex2 = 2.0 * UGen::getReciprocalSampleRate() * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)			
		{
			nextValue = currentPhase;
			
			currentPhase += *freqSamples * blockSizeOverSampleRatex2;
			
			if(currentPhase >= 1.f)			currentPhase -= 2.f;
			else if(currentPhase < -1.f)	currentPhase += 2.f;
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		
		blockPosition		+= numKrSamples;
		freqSamples			+= numKrSamples;
		
		if(nextValue == value)
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


LFSaw::LFSaw(UGen const& freq, Buffer const& initialPhase) throw()
{	
	int numChannels = ugen::max(freq.getNumChannels(), initialPhase.size());
	
	initInternal(numChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new LFSawUGenInternal(freq, initialPhase.wrapAt(i));
	}
}

END_UGEN_NAMESPACE
