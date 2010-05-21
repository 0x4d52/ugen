// $Id: ugen_TSpawn.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/spawn/ugen_TSpawn.cpp $

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

#include "ugen_TSpawn.h"
#include "../basics/ugen_MixUGen.h"

TSpawnUGenInternal::TSpawnUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats) throw()
:	SpawnBaseUGenInternal(NumInputs, numChannels, maxRepeats),
	lastTrig(0.f)
{
	inputs[Trig] = trig;  // should have already been ensured to be a single channel
}

//void TSpawnUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
//{
//	// render the current voices
//	SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
//	
//	if(reachedMaxRepeats() == false)
//	{
//		// now overdub new voices this block
//		const int blockSize = uGenOutput.getBlockSize();	
//		int numSamplesToProcess = blockSize;	
//		int blockSample = 0;
//		const int numChannels = getNumChannels();
//		float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
//		bool didTriggerThisBlock = false;
//		while(numSamplesToProcess)
//		{
//			float currentTrig = *trigSamples++;
//			if(lastTrig <= 0.f && currentTrig > 0.f)
//			{
//				didTriggerThisBlock = true;
//				UGen newVoice = spawnEvent(*this, currentEventIndex++);
//				
//				newVoice.prepareForBlock(blockSize, blockID); // prepare for full size (allocates the output buffers)
//				
//				UGen::setBlockSize(numSamplesToProcess);
//
//				const int currentBlockSampleID = blockID + blockSample;
//				if(blockSample > 0)
//					newVoice.prepareForBlock(numSamplesToProcess, currentBlockSampleID); // prepare for sub block
//				
//				for(int channel = 0; channel < numChannels; channel++)
//				{
//					bool shouleDeleteLocal = false;
//					float *voiceSamples = newVoice.processBlock(shouleDeleteLocal, currentBlockSampleID, channel);
//					accumulateSamples(bufferData[channel] + blockSample, voiceSamples, numSamplesToProcess);
//				}
//				
//				events <<= newVoice;
//			}
//			
//			lastTrig = currentTrig;	
//			--numSamplesToProcess;
//			++blockSample;
//		}
//		
//		if(didTriggerThisBlock) 
//		{
//			UGen::setBlockSize(blockSize);
//			mixer = Mix(&events, false);
//		}
//	}
//}


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
			float currentTrig = *trigSamples++;
						
			if(lastTrig <= 0.f && currentTrig > 0.f)
			{
				if(numSamples > 0)
				{
					// render samples from startSample to startSample+numSample
					UGen::setBlockSize(numSamples);
					
					for(int channel = 0; channel < numChannels; channel++)
					{
						bufferData[channel] = proxies[channel]->getSampleData() + startSample;
					}
					
					mixer.prepareForBlock(numSamples, blockID + startSample);
					mixer.setOutputs(bufferData, numSamples, numChannels);
					mixer.processBlock(shouldDelete, blockID + startSample, -1);
				}
				
				startSample += numSamples;
				numSamples = 1;
				
				// add the new voice and re init the mixer
				UGen newVoice = spawnEvent(*this, currentEventIndex++);
				events <<= newVoice;
				mixer = Mix(&events, false);
			}
			else
			{
				++numSamples;
			}
			
			lastTrig = currentTrig;	
			--numSamplesToProcess;
		}
		
		UGen::setBlockSize(numSamples);
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			bufferData[channel] = proxies[channel]->getSampleData() + startSample;
		}
		
		mixer.prepareForBlock(numSamples, blockID + startSample);
		mixer.setOutputs(bufferData, numSamples, numChannels);
		mixer.processBlock(shouldDelete, blockID + startSample, -1);
		
		UGen::setBlockSize(blockSize);
	}
}


void TSpawnUGenInternal::trigger(void* extraArgs) throw()
{
	if(reachedMaxRepeats() == false)
	{
		UGen newVoice = spawnEvent(*this, currentEventIndex++, extraArgs);
		events <<= newVoice;
	}
}

END_UGEN_NAMESPACE
