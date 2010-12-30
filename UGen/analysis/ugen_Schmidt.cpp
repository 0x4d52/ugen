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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Schmidt.h"

SchmidtUGenInternal::SchmidtUGenInternal(UGen const& input, UGen const& lo, UGen const& hi) throw()
:	UGenInternal(NumInputs)
{
	inputs[Input] = input;
	inputs[Lo] = lo;
	inputs[Hi] = hi;
	state = 0.f;
}

UGenInternal* SchmidtUGenInternal::getChannel(const int channel) throw()
{
	SchmidtUGenInternal* internal = new SchmidtUGenInternal(inputs[Input].getChannel(channel), 
															inputs[Lo].getChannel(channel),
															inputs[Hi].getChannel(channel));
	internal->state = state;
	return internal;
}


void SchmidtUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* loSamples = inputs[Lo].processBlock(shouldDelete, blockID, channel);
	float* hiSamples = inputs[Hi].processBlock(shouldDelete, blockID, channel);
	
	LOCAL_DECLARE(float, state);
	
	while(numSamplesToProcess--)
	{
		float input = *inputSamples++;
		float lo = *loSamples++;
		float hi = *hiSamples++;
		
		if(state > 0.f)
		{
			if(input < lo) state = 0.f;
		}
		else
		{
			if(input > hi) state = 1.f;
		}
		
		*outputSamples++ = state;
	}
	
	LOCAL_COPY(state);
}

Schmidt::Schmidt(UGen const& input, UGen const& lo, UGen const& hi) throw()
{	
	UGen inputs[] = { input, lo, hi };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);	
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new SchmidtUGenInternal(input, lo, hi);
	}
}

END_UGEN_NAMESPACE

#endif // gpl
