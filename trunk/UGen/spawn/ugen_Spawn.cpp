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

// NB same name as SC but no derived code, the helper event classes do have derived code though

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Spawn.h"
#include "../basics/ugen_MixUGen.h"

SpawnBaseUGenInternal::SpawnBaseUGenInternal(const int numInputs, const int _numChannels, const int maxRepeats) throw()
:	ProxyOwnerUGenInternal(numInputs, _numChannels-1),
	numChannels(_numChannels),
	currentEventIndex(0),
	maxRepeats_(maxRepeats),
	bufferData(new float*[numChannels]),
	stopEvents(false)
{
	ugen_assert(numChannels > 0);
	ugen_assert(maxRepeats >= 0);
	initEvents();
}

SpawnBaseUGenInternal::~SpawnBaseUGenInternal()// throw()
{
	delete [] bufferData;
}

void SpawnBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{		
	if(reachedMaxRepeats() == true && events.sizeNotNull() == 0) shouldDelete = true;
	
	if(shouldStopAllEvents() == true) initEvents();
	
	const int numSamplesToProcess = uGenOutput.getBlockSize();	
	const int numChannels = getNumChannels();
	for(int channel = 0; channel < numChannels; channel++)
	{
		bufferData[channel] = proxies[channel]->getSampleData();
	}
	
	mixer.prepareForBlock(numSamplesToProcess, blockID);
	mixer.setOutputs(bufferData, numSamplesToProcess, numChannels);
	mixer.processBlock(shouldDelete, blockID, -1);
}

void SpawnBaseUGenInternal::releaseInternal() throw()
{
	UGenInternal::releaseInternal();
	mixer.release(); 
}

void SpawnBaseUGenInternal::stealInternal() throw()
{
	UGenInternal::stealInternal();
	mixer.steal(false); 
}

void SpawnBaseUGenInternal::initEvents() throw()
{
	events = UGen::emptyChannels(numChannels);
	mixer = Mix(&events, false);
	stopEvents = false;
}

void SpawnBaseUGenInternal::stopAllEvents() throw()
{
	stopEvents = true;
}

SpawnUGenInternal::SpawnUGenInternal(const int numChannels, const double nextTime_, const int maxRepeats) throw()
:	SpawnBaseUGenInternal(0, numChannels, maxRepeats),
	nextTime(nextTime_),
	nextTimeSamples(0)
{
	ugen_assert(nextTime >= 0.0)
}

void SpawnUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	// render the current voices
	SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
	
	const unsigned int blockSize = uGenOutput.getBlockSize();
	const unsigned int nextBlockID = blockID + blockSize;
	
	if(nextTimeSamples < nextBlockID && reachedMaxRepeats() == false)
	{
		// now overdub new voices this block
			
		if(nextTimeSamples < blockID)
			nextTimeSamples = blockID;
		
		const int blockOffset = nextTimeSamples - blockID;
		int numSamplesToProcess = blockSize - blockOffset; 	
		const int numChannels = getNumChannels();
		
		if(blockOffset > 0)
		{
			for(int channel = 0; channel < numChannels; channel++)
			{
				bufferData[channel] += blockOffset;
			}
		}
		
		do
		{
			UGen newVoice = spawnEvent(*this, currentEventIndex++);
			newVoice.prepareForBlock(blockSize, blockID); // prepare for full size (allocates the output buffers)
			
			UGen::setBlockSize(numSamplesToProcess);
			newVoice.prepareForBlock(numSamplesToProcess, nextTimeSamples); // prepare for sub block
			
			unsigned int nextTimeSamplesDelta = (unsigned int)(nextTime * UGen::getSampleRate());
			
			for(int channel = 0; channel < numChannels; channel++)
			{
				bool shouleDeleteLocal = false;
				float *voiceSamples = newVoice.processBlock(shouleDeleteLocal, nextTimeSamples, channel);
				accumulateSamples(bufferData[channel], voiceSamples, numSamplesToProcess);
				bufferData[channel] += nextTimeSamplesDelta;
			}
			
			events <<= newVoice;
			
			numSamplesToProcess -= nextTimeSamplesDelta;
			nextTimeSamples += nextTimeSamplesDelta;
		} 
		while(nextTimeSamples <	nextBlockID && reachedMaxRepeats() == false);
		
		UGen::setBlockSize(blockSize); // reset
		
		mixer = Mix(&events, false);		
	}
}




END_UGEN_NAMESPACE
