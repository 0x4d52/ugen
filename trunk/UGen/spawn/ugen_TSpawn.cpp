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

#include "ugen_TSpawn.h"
#include "../basics/ugen_MixUGen.h"

TSpawnUGenInternal::TSpawnUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats) throw()
:	SpawnBaseUGenInternal(NumInputs, numChannels, maxRepeats),
	currentTrig(0.f), lastTrig(0.f)
{
	inputs[Trig] = trig;  // should have already been ensured to be a single channel
}

void TSpawnUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	if(shouldStopAllEvents() == true) initEvents();
	
	if(reachedMaxRepeats() == true)
	{
		SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
	}
	else
	{
		const int blockSize = uGenOutput.getBlockSize();	
		int numSamplesToProcess = blockSize;	
		int startSample = 0;
		int numSamples = 0;
		
		const int numChannels = getNumChannels();
		
		float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
		
		while(numSamplesToProcess)
		{
			currentTrig = *trigSamples++;
						
			if(lastTrig <= 0.f && currentTrig > 0.f)
			{
				if(numSamples > 0)
				{
					// render samples from startSample to startSample+numSample					
					for(int channel = 0; channel < numChannels; channel++)
					{
						bufferData[channel] = proxies[channel]->getSampleData() + startSample;
					}
					
					mixer.prepareForBlock(numSamples, blockID + startSample, -1);
					mixer.setOutputs(bufferData, numSamples, numChannels);
					mixer.processBlock(shouldDelete, blockID + startSample, -1);
				}
				
				startSample += numSamples;
				numSamples = 1;
				
				// add the new voice
				UGen newVoice = spawnEvent(*this, currentEventIndex++);
				events.add(newVoice);
			}
			else
			{
				++numSamples;
			}
			
			lastTrig = currentTrig;	
			--numSamplesToProcess;
		}
				
		for(int channel = 0; channel < numChannels; channel++)
		{
			bufferData[channel] = proxies[channel]->getSampleData() + startSample;
		}
		
		mixer.prepareForBlock(numSamples, blockID + startSample, -1);
		mixer.setOutputs(bufferData, numSamples, numChannels);
		mixer.processBlock(shouldDelete, blockID + startSample, -1);
		
		events.removeNulls();
	}
}


bool TSpawnUGenInternal::trigger(void* extraArgs) throw()
{	
	if(reachedMaxRepeats() == false)
	{
		currentTrig = 1.f;
		UGen newVoice = spawnEvent(*this, currentEventIndex++, extraArgs);
		events.add(newVoice);
	}
	
	return true;
}

END_UGEN_NAMESPACE
