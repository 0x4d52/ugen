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

#include "ugen_Impulse.h"


ImpulseUGenInternal::ImpulseUGenInternal(Impulse_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs),
	currentPhase(1.f)
{
	inputs[Freq] = freq;
	initValue(1.f);
}

UGenInternal* ImpulseUGenInternal::getChannel(const int channel) throw()
{	
	return new ImpulseUGenInternal(inputs[Freq].getChannel(channel));
}	

UGenInternal* ImpulseUGenInternal::getKr() throw()
{ 
	return new ImpulseUGenInternalK(inputs[Freq].kr()); 
}

void ImpulseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	double reciprocalSampleRate = UGen::getReciprocalSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float out;
		
		if(currentPhase >= 1.f)	{
			currentPhase -= 1.f;
			out = 1.f;
		} else {
			out = 0.f;
		}
		
		currentPhase += *freqSamples++ * reciprocalSampleRate;
		*outputSamples++ = out;
	}
}

ImpulseUGenInternalK::ImpulseUGenInternalK(Impulse_InputsWithTypesOnly) throw()
:	ImpulseUGenInternal(Impulse_InputsNoTypes),
	value(1.f)
{
	rate = ControlRate;
}

void ImpulseUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		if(numKrSamples == 0)			
		{
			float out;
			
			if(currentPhase >= 1.f)	{
				currentPhase -= 1.f;
				out = 1.f;
			} else {
				out = 0.f;
			}
			
			currentPhase += *freqSamples * krBlockSizeOverSampleRate;
			value = out;
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		
		blockPosition		+= numKrSamples;
		freqSamples			+= numKrSamples;
		
		while(numSamplesToProcess && numKrSamples)
		{
			*outputSamples++ = value;
			--numSamplesToProcess;
			--numKrSamples;
		}
	}
}


Impulse::Impulse(Impulse_InputsWithTypesOnly) throw()
{	
	UGen inputs[] = { Impulse_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new ImpulseUGenInternal(Impulse_InputsNoTypes);
	}
}

END_UGEN_NAMESPACE

#endif
