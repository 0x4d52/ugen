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

#include "ugen_BlockDelay.h"

BlockDelayUGenInternal::BlockDelayUGenInternal(UGen const& input) throw()
:	UGenInternal(NumInputs),
	delayBuffer(BufferSpec(UGen::getEstimatedBlockSize(), 1, true)),
	delayBufferSamples(delayBuffer.getData())
{
	inputs[Input] = input;
}

UGenInternal* BlockDelayUGenInternal::getChannel(const int channel) throw()
{
	return new BlockDelayUGenInternal(inputs[Input].getChannel(channel));
}

void BlockDelayUGenInternal::prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	int previousBlockSize = uGenOutput.getBlockSize();
	
	if(actualBlockSize > delayBuffer.size())
	{
		delayBuffer = Buffer(BufferSpec(actualBlockSize, 1, true));
		delayBufferSamples = delayBuffer.getData();
	}
	else if(previousBlockSize != actualBlockSize)
	{
		delayBuffer.clear();
	}
	
	uGenOutput.prepareForBlock(actualBlockSize); // only prepare the output block
}

void BlockDelayUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	// this assumes that blocks will all be the same size, which they may not be!!
	// this will only work if block sizes are consistent
	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	memcpy(outputSamples, delayBufferSamples, numSamplesToProcess * sizeof(float));
	
	float* inputSamples = inputs[Input].processBlock(shouldDelete, lastBlockID, channel);
	
	if(inputSamples)
		memcpy(delayBufferSamples, inputSamples, numSamplesToProcess * sizeof(float));
}

void BlockDelayUGenInternal::releaseInternal() throw()
{
	// break the potential infinte loop in a release action
}

void BlockDelayUGenInternal::stealInternal() throw()
{
	// break the potential infinte loop in a steal action
}

BlockDelay::BlockDelay(UGen const& input) throw()
{
	int numChannels = input.getNumChannels();
	initInternal(numChannels);
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new BlockDelayUGenInternal(input);
	}
}

END_UGEN_NAMESPACE
