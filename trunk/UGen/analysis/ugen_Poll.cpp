/*
 *  ugen_Poll.cpp
 *  AccelGesture
 *
 *  Created by Martin Robinson on 13/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Poll.h"

PollUGenInternal::PollUGenInternal(UGen const& input, UGen const& trig) throw()
:	UGenInternal(NumInputs)
{
	inputs[Input] = input;
	inputs[Trig] = trig;
	lastTrig = 0.f;
}

void PollUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* const trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
	
	for(int sample = 0; sample < numSamplesToProcess; sample++)
	{
		float thisTrig = trigSamples[sample];
		
		if(thisTrig > 0.f && lastTrig <= 0.f)
		{
			const int numChannels = inputs[Input].getNumChannels();
			Buffer poll = Buffer::newClear(numChannels);
			float * const pollSamples = poll.getData(0);
			
			for(int channel = 0; channel < numChannels; channel++)
			{
				float* const inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
				pollSamples[channel] = inputSamples[sample];
			}
			
			sendBuffer(poll);
		}
		
		*outputSamples++ = 0.f;
		lastTrig = thisTrig;
	}
}

Poll::Poll(UGen const& input, UGen const& trig) throw()
{
	initInternal(1);
	internalUGens[0] = new PollUGenInternal(input, trig.mix());
}

END_UGEN_NAMESPACE
