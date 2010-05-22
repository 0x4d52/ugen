// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-9 by Martin Robinson www.miajo.co.uk
 
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

BEGIN_UGEN_NAMESPACE

#include "ugen_Triggers.h"


SAHUGenInternal::SAHUGenInternal(UGen const& input, UGen const& trig) throw()
:	UGenInternal(NumInputs),
	currentValue(0.f),
	lastTrig(0.f)
{
	inputs[Input] = input;
	inputs[Trig] = trig;
}

UGenInternal* SAHUGenInternal::getChannel(const int channel) throw()
{
	return new SAHUGenInternal(inputs[Input].getChannel(channel), 
							   inputs[Trig].getChannel(channel));
}

void SAHUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float thisTrig = *trigSamples++;
		
		if(lastTrig <= 0.f && thisTrig > 0.f)
			currentValue = *inputSamples++;
		else
			inputSamples++;
				
		*outputSamples++ = currentValue;
		lastTrig = thisTrig;
	}
}

SAH::SAH(UGen const& input, UGen const& trig) throw()
{
	UGen inputs[] = { input, trig };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new SAHUGenInternal(input, trig);
		if(trig.getValue(i) <= 0.f)
			internalUGens[i]->initValue(0.f);
		else
			internalUGens[i]->initValue(input.getValue(i));		
	}
}

ToggleFFUGenInternal::ToggleFFUGenInternal(UGen const& trig) throw()
:	UGenInternal(NumInputs),
	currentValue(0.f),
	lastTrig(0.f)
{
	inputs[Trig] = trig;
}

UGenInternal* ToggleFFUGenInternal::getChannel(const int channel) throw()
{
	return new ToggleFFUGenInternal(inputs[Trig].getChannel(channel));
}

void ToggleFFUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float thisTrig = *trigSamples++;
		
		if(lastTrig <= 0.f && thisTrig > 0.f)
			currentValue = currentValue > 0.f ? 0.f : 1.f;
		
		*outputSamples++ = currentValue;
		lastTrig = thisTrig;
	}
}

ToggleFF::ToggleFF(UGen const& trig) throw()
{
	initInternal(trig.getNumChannels());	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new ToggleFFUGenInternal(trig);		
	}
}


END_UGEN_NAMESPACE
