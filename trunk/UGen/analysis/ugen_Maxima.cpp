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

#include "ugen_Maxima.h"

MaximaUGenInternal::MaximaUGenInternal(UGen const& input) throw()
:	UGenInternal(NumInputs) // Initialise the UGenIntenal's UGen inputs array using the enum size.
{
	inputs[Input] = input;
	didIncreaseLastTime = false;
	lastValue = 0.f;
}

UGenInternal* MaximaUGenInternal::getChannel(const int channel) throw()
{
	MaximaUGenInternal* internal = new MaximaUGenInternal(inputs[Input].getChannel(channel));
	internal->didIncreaseLastTime = didIncreaseLastTime;
	internal->lastValue = lastValue;
	return internal;
}

void MaximaUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	// Get the number of samples to process this block.
	int numSamplesToProcess = uGenOutput.getBlockSize();
	
	// Get a pointer to the output sample array.
	float* outputSamples = uGenOutput.getSampleData();
	
	// Get a pointer to the input sample array.
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	
	// The dsp loop:
	while(numSamplesToProcess--)
	{
		float input = *inputSamples;	// Pull a sample out of the input array.
		
		bool didIncreaseThisTime;
		
		if(input > lastValue)
			didIncreaseThisTime = true;
		else
			didIncreaseThisTime = false;
		
		float output;
		
		if((didIncreaseThisTime == false) && (didIncreaseLastTime == true))
			output = 1.f;
		else
			output = 0.f;
		
		lastValue = input;
		didIncreaseLastTime = didIncreaseThisTime;
		
		*outputSamples = output;		// Put a sample into the output array.
		
		// Increment the array pointers for the next loop iteration.
		inputSamples++;		
		outputSamples++;
	}
}

Maxima::Maxima(UGen const& input) throw()
{
	initInternal(input.getNumChannels());
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new MaximaUGenInternal(input);
	}
}

END_UGEN_NAMESPACE