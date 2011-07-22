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

#include "ugen_TrigProcess.h"

DebounceUGenInternal::DebounceUGenInternal(UGen const& input, UGen const& time) throw()
:	UGenInternal(NumInputs)
{
	inputs[Input] = input;
	inputs[Time] = time;
	count = 0;
}

UGenInternal* DebounceUGenInternal::getChannel(const int channel) throw()
{
	DebounceUGenInternal* internal = new DebounceUGenInternal(inputs[Input].getChannel(channel),
															  inputs[Time].getChannel(channel));
	internal->count = count;
	return internal;
}

void DebounceUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamples = uGenOutput.getSampleData();
	float* const inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* const timeSamples = inputs[Time].processBlock(shouldDelete, blockID, channel);
	
	LOCAL_DECLARE(int, count);
	
	for(int i = 0; i < numSamplesToProcess; i++)
	{
		// should un wrap this loop a bit..
		
		if(count > 0)
		{
			outputSamples[i] = 1.f;
			count--;
		}
		else
		{
			if(inputSamples[i] > 0.f)
			{
				outputSamples[i] = 1.f;
				count = timeSamples[i] * UGen::getSampleRate();
			}
			else
			{
				outputSamples[i] = 0;
			}
		}
	}
	
	LOCAL_COPY(count);
}

Debounce::Debounce(UGen const& input, UGen const& time) throw()
{
	initInternal(input.getNumChannels());
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new DebounceUGenInternal(input, time);
	}
}

END_UGEN_NAMESPACE
