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

#ifndef UGEN_NOEXTGPL

#include "../../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_LFPulse.h"


LFPulseUGenInternal::LFPulseUGenInternal(LFPulse_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs),
	currentPhase(0.f)
{
	inputs[Freq] = freq;
	inputs[Duty] = duty;
	
	initValue(1.f);
}

UGenInternal* LFPulseUGenInternal::getChannel(const int channel) throw()
{
	return new LFPulseUGenInternal(inputs[Freq].getChannel(channel),
								   inputs[Duty].getChannel(channel));
}	

UGenInternal* LFPulseUGenInternal::getKr() throw()
{ 
	return new LFPulseUGenInternalK(inputs[Freq].kr(), inputs[Duty].kr()); 
}

void LFPulseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	double reciprocalSampleRate = UGen::getReciprocalSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	float* dutySamples = inputs[Duty].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float out;
		
		if(currentPhase >= 1.f)	{
			currentPhase -= 1.f;
			out = *dutySamples++ < 0.5f ? 1.f : 0.f;
		} else {
			out = currentPhase < *dutySamples++ ? 1.f : 0.f;
		}
		
		currentPhase += *freqSamples++ * reciprocalSampleRate;
		*outputSamples++ = out;
	}
}

double LFPulseUGenInternal::getDuration() const throw()
{ 
	return 1.0;
}

double LFPulseUGenInternal::getPosition() const throw()
{
	return (double)currentPhase;
}

bool LFPulseUGenInternal::setPosition(const double newPosition) throw()
{
	currentPhase = (float)ugen::clip(newPosition, 0.0, 1.0);
	return true;
}

LFPulseUGenInternalK::LFPulseUGenInternalK(UGen const& freq, UGen const& duty) throw()
:	LFPulseUGenInternal(freq, duty),
	value(1.f)
{
	rate = ControlRate;
}

void LFPulseUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	float* dutySamples = inputs[Duty].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)			
		{
			float out;
			
			if(currentPhase >= 1.f)	{
				currentPhase -= 1.f;
				out = *dutySamples < 0.5f ? 1.f : 0.f;
			} else {
				out = currentPhase < *dutySamples ? 1.f : 0.f;
			}
			
			currentPhase += *freqSamples * krBlockSizeOverSampleRate;
			nextValue = out;
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		
		blockPosition		+= numKrSamples;
		freqSamples			+= numKrSamples;
		dutySamples			+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();
			
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


LFPulse::LFPulse(LFPulse_InputsWithTypesOnly) throw()
{	
	UGen inputs[] = { LFPulse_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new LFPulseUGenInternal(LFPulse_InputsNoTypes);
	}
}

END_UGEN_NAMESPACE

#endif // gpl
