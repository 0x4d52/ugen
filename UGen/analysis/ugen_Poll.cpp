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

UGenInternal* PollUGenInternal::getChannel(const int channel) throw()
{
	PollUGenInternal* internal = new PollUGenInternal(inputs[Input].getChannel(channel),
													  inputs[Trig].getChannel(channel));
	internal->lastTrig = lastTrig;
	internal->receivers = receivers;
	return internal;
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
