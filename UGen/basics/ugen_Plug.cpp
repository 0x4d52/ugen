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

// same name a SC Plug but no derived code

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Plug.h"


PlugUGenInternal::PlugUGenInternal(UGen const& source, bool shouldAllowAutoDelete) throw()
:	ProxyOwnerUGenInternal(0, source.getNumChannels() - 1),
	currentSourceIndex(-1),
	fadeSourceIndex(-1),
	shouldAllowAutoDelete_(shouldAllowAutoDelete)
{	
	setSource(source, true);
}

void PlugUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw()
{
	const int size = sources.size();
	for(int i = 0; i < size; i++)
	{
		sources[i].prepareForBlock(actualBlockSize, blockID);
	}
}

void PlugUGenInternal::releaseInternal() throw()
{
	UGenInternal::releaseInternal();
	const int size = sources.size();
	for(int i = 0; i < size; i++)
	{
		sources[i].release();
	}
}

void PlugUGenInternal::stealInternal() throw()
{
	UGenInternal::releaseInternal();
	const int size = sources.size();
	for(int i = 0; i < size; i++)
	{
		sources[i].steal(false);
	}
}

void PlugUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	bool shouldDeleteLocal;
	bool& shouldDeleteToPass = shouldAllowAutoDelete_ ? shouldDelete : shouldDeleteLocal;	
	
	const int numSources = sources.size();
	for(int sourceIndex = 0; sourceIndex < numSources; sourceIndex++)
	{		
		if(currentSourceIndex == sourceIndex)
		{
			if(fadeSourceIndex != -1)
			{
				float fadeSourceFadeLevel = 0.f, currentSourceFadeLevel = 0.f;
				const int numChannels = getNumChannels();
				
				for(int channel = 0; channel < numChannels; channel++)
				{
					shouldDeleteLocal = false;
					int numSamples = uGenOutput.getBlockSize();
					float *currentSourceSamples = sources[currentSourceIndex].processBlock(shouldDeleteToPass, blockID, channel);
					float *fadeSourceSamples = sources[fadeSourceIndex].processBlock(shouldDeleteToPass, blockID, channel);
					float *outputSamples = proxies[channel]->getSampleData();
					fadeSourceFadeLevel = this->fadeSourceFadeLevel;  // need local copies because we mave multiple channels
					currentSourceFadeLevel = this->currentSourceFadeLevel;
					
					while(numSamples--)
					{
						*outputSamples++ =  (*currentSourceSamples++ * currentSourceFadeLevel) +
											(*fadeSourceSamples++ * fadeSourceFadeLevel);
						
						currentSourceFadeLevel += deltaFade;
						fadeSourceFadeLevel -= deltaFade;
						
						if(fadeSourceFadeLevel <= 0.f)
						{
							fadeSourceFadeLevel = 0.f;
							currentSourceFadeLevel = 1.f;
						}
					}
				}
				
				if(fadeSourceFadeLevel <= 0.f)
				{
					fadeSourceIndex = -1;
					if(releasePreviousSourcesAfterFade)
					{
						sources.clear();
						sources = tempSource;
						currentSourceIndex = 0;
						tempSource = UGenArray();
					}
				}
				else
				{
					this->fadeSourceFadeLevel = fadeSourceFadeLevel;
					this->currentSourceFadeLevel = currentSourceFadeLevel;
				}
			}
			else
			{
				const int memcpySize = uGenOutput.getBlockSize() * sizeof(float);
				const int numChannels = getNumChannels();
				
				for(int channel = 0; channel < numChannels; channel++)
				{
					shouldDeleteLocal = false;
					float *sourceSamples = sources[currentSourceIndex].processBlock(shouldDeleteToPass, blockID, channel);
					float *outputSamples = proxies[channel]->getSampleData();
					
					memcpy(outputSamples, sourceSamples, memcpySize);
				}
			}
		}
		else
		{
			shouldDeleteLocal = false;
			sources[sourceIndex].processBlock(shouldDeleteToPass, blockID, -1); // process previous sources
		}
	}	
	
}


void PlugUGenInternal::setSource(UGen const& source, const bool releasePreviousSources, const float fadeTime)
{
	ugen_assert(fadeTime >= 0.f);
	
	if(releasePreviousSources == true && fadeTime <= 0.f)
	{
		fadeSourceIndex = -1;
		sources = UGenArray(source);
		currentSourceIndex = 0;
	}
	else
	{		
		int indexOfExistingSource = sources.indexOf(source);
				
		if(currentSourceIndex != -1 && fadeTime > 0.f)
		{
			fadeSourceIndex = currentSourceIndex;
			fadeSourceFadeLevel = 1.f;
			currentSourceFadeLevel = 0.f;
			deltaFade = (float)UGen::getReciprocalSampleRate() / fadeTime;
		}
		else
			fadeSourceIndex = -1;
		
		if(indexOfExistingSource == -1)
		{
			//sources = UGenArray(sources, source, false);
			sources.add(source);
			currentSourceIndex++;
		}
		else
			currentSourceIndex = indexOfExistingSource;
		
		releasePreviousSourcesAfterFade = releasePreviousSources;
		if(releasePreviousSourcesAfterFade == true && fadeTime > 0.f)
			tempSource = UGenArray(source);
	}
}

UGen& PlugUGenInternal::getSource()
{
	return sources[currentSourceIndex];
}

Plug::Plug(UGen const& source, bool shouldAllowAutoDelete) throw()
{	
	int numChannels = source.getNumChannels();
	
	ugen_assert(numChannels > 0);
	
	initInternal(numChannels);
	generateFromProxyOwner(new PlugUGenInternal(source, shouldAllowAutoDelete));
	
	for(int i = 0; i < numChannels; i++)
	{
		internalUGens[i]->initValue(source.getValue(i));
	}
}


END_UGEN_NAMESPACE
