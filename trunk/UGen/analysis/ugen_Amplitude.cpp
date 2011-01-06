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

#include "ugen_Amplitude.h"


AmplitudeBaseUGenInternal::AmplitudeBaseUGenInternal(UGen const& input, 
													 const float durationToUse) throw()
:	UGenInternal(NumInputs),
	duration(durationToUse)
{
	ugen_assert(durationToUse > 0.f);
	
	inputs[Input] = input;
	
	measureLength = ugen::max(1, (int)(UGen::getSampleRate() * duration));
	measuredItems = 0;
	maximum = 0.f;
	oldMaximum = 0.f;
	currentAmplitude = 0.f;
}

void AmplitudeBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float input = *inputSamples;
		
		if(abs(input) > maximum) 
			maximum = abs(input);
		
		measuredItems++;
		
		if(measuredItems == measureLength)
		{
			currentAmplitude = maximum;
			measuredItems = 0;
			maximum = 0.f;
		}
		
		float output = currentAmplitude;
		
		*outputSamples = output;
		
		outputSamples++;
		inputSamples++;
	}
}

AmplitudeUGenInternal::AmplitudeUGenInternal(UGen const& input, 
											 const float duration) throw()
:	AmplitudeBaseUGenInternal(input, duration)
{
}

UGenInternal* AmplitudeUGenInternal::getChannel(const int channel) throw()
{
	AmplitudeUGenInternal* internal = new AmplitudeUGenInternal(inputs[Input].getChannel(channel), 
																duration);
	internal->measuredItems = measuredItems;
	internal->maximum = maximum;
	internal->oldMaximum = oldMaximum;
	internal->currentAmplitude = currentAmplitude;
	return internal;
}

Amplitude::Amplitude(UGen const& input, const float duration) throw()
{
	initInternal(input.getNumChannels());
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new AmplitudeUGenInternal(input, duration);
		internalUGens[i]->initValue(input.getValue(i));
	}
}

DetectSilenceUGenInternal::DetectSilenceUGenInternal(UGen const& input, 
													 const float duration) throw()
:	AmplitudeBaseUGenInternal(input, duration),
	started(false)
{
}

UGenInternal* DetectSilenceUGenInternal::getChannel(const int channel) throw()
{
	DetectSilenceUGenInternal* internal = new DetectSilenceUGenInternal(inputs[Input].getChannel(channel), 
																		duration);
	internal->measuredItems = measuredItems;
	internal->maximum = maximum;
	internal->oldMaximum = oldMaximum;
	internal->currentAmplitude = currentAmplitude;
	internal->started = started;
	return internal;
}

void DetectSilenceUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	AmplitudeBaseUGenInternal::processBlock(shouldDelete, blockID, channel);
	
	if(currentAmplitude > 0.f)
	{
		started = true;
	}
	else if(started)
	{
		shouldDelete = true;
	}
}

DetectSilence::DetectSilence(UGen const& input, const float duration) throw()
{
	initInternal(input.getNumChannels());
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new DetectSilenceUGenInternal(input, duration);
		internalUGens[i]->initValue(input.getValue(i));
	}
}


END_UGEN_NAMESPACE
