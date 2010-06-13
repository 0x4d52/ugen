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

#include "ugen_MixUGen.h"


MixUGenInternal::MixUGenInternal(UGen const& array, bool shouldAllowAutoDelete) throw()
:	UGenInternal(1),
	shouldAllowAutoDelete_(shouldAllowAutoDelete)
{	
	inputs[0] = array;
	
	float value = 0.f;
	for(int i = 0; i < array.getNumChannels(); i++)
	{
		value += array.getValue(i);
	}
	
	initValue(value);
}

MixUGenInternal::MixUGenInternal(UGen* array, bool shouldAllowAutoDelete) throw()
:	UGenInternal(array), // numInputs is one using this special constructor
	shouldAllowAutoDelete_(shouldAllowAutoDelete)
{	
	float value = 0.f;
	for(int i = 0; i < array->getNumChannels(); i++)
	{
		value += array->getValue(i);
	}
	
	initValue(value);
}


#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void MixUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	int channel = 0;
	
	bool shouldDeleteLocal = false;
	bool& shouldDeleteToPass = shouldAllowAutoDelete_ ? shouldDelete : shouldDeleteLocal;	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* const outputSamplesBase = uGenOutput.getSampleData();
	const float* channelSamples = inputs->processBlock(shouldDeleteToPass, blockID, channel);
	
	float* outputSamples = outputSamplesBase;
	while(numSamplesToProcess--)
	{
		*outputSamples++ = *channelSamples++;
	}
	
	channel++;
	 
	int numChannels = inputs->getNumChannels();
	
	for(/* leave channel value alone */; channel < numChannels; channel++)
	{
		shouldDeleteLocal = false;
		int numSamplesToProcess = uGenOutput.getBlockSize();
		outputSamples = outputSamplesBase;
		const float* channelSamples = inputs->processBlock(shouldDeleteToPass, blockID, channel);
				
		while(numSamplesToProcess--)
		{
			*outputSamples++ += *channelSamples++;
		}
	}
	
}
#endif

MixArrayUGenInternal::MixArrayUGenInternal(UGenArray const& array, bool shouldAllowAutoDelete) throw()
:	ProxyOwnerUGenInternal(0, array.findMaxNumChannels() - 1),
	array_(array),
	arrayPtr(&array_),
	arrayRef(*arrayPtr),
	shouldAllowAutoDelete_(shouldAllowAutoDelete)
{	
}

MixArrayUGenInternal::MixArrayUGenInternal(UGenArray* array, bool shouldAllowAutoDelete) throw()
:	ProxyOwnerUGenInternal(0, array->findMaxNumChannels() - 1),
	arrayPtr(array),
	arrayRef(*arrayPtr),
	shouldAllowAutoDelete_(shouldAllowAutoDelete)
{	
}

void MixArrayUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw()
{
	const int size = arrayRef.size();
	for(int i = 0; i < size; i++)
	{
		arrayRef[i].prepareForBlock(actualBlockSize, blockID);
	}
}

void MixArrayUGenInternal::releaseInternal() throw()
{
	UGenInternal::releaseInternal();
	const int size = arrayRef.size();
	for(int i = 0; i < size; i++)
	{
		arrayRef[i].release();
	}
}

void MixArrayUGenInternal::stealInternal() throw()
{
	UGenInternal::releaseInternal();
	const int size = arrayRef.size();
	for(int i = 0; i < size; i++)
	{
		arrayRef[i].steal(false);
	}
}

float MixArrayUGenInternal::getValue(const int channel) const throw()
{
	float value = 0.f;
	
	for(int i = 0; i < arrayRef.size(); i++)
	{
		value += arrayRef[i].getValue(channel);
	}
	
	return value;
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void MixArrayUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	bool shouldDeleteLocal;
	bool& shouldDeleteToPass = shouldAllowAutoDelete_ ? shouldDelete : shouldDeleteLocal;	
	int numOutputChannels = getNumChannels();
	int blockSizeBytes = uGenOutput.getBlockSize() * sizeof(float);
	const int arraySize = arrayRef.size();
	
	for(int channel = 0; channel < numOutputChannels; channel++)
	{
		float * const outputSamplesBase = proxies[channel]->getSampleData();
		memset(outputSamplesBase, 0, blockSizeBytes);

		for(int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			shouldDeleteLocal = false;
			int numSamplesToProcess = uGenOutput.getBlockSize();
			float* outputSamples = outputSamplesBase;//proxies[channel]->getSampleData();
			const float* channelSamples = arrayRef[arrayIndex].processBlock(shouldDeleteToPass, blockID, channel);
									
			for(int i = 0; i < numSamplesToProcess; ++i)
			{
				outputSamples[i] += channelSamples[i];
			}
		}
	}
}
#endif




Mix::Mix(UGen const& array, bool shouldAllowAutoDelete) throw()
{
	initInternal(1);
	internalUGens[0] = new MixUGenInternal(array, shouldAllowAutoDelete);
	
	float value = 0.f;
	for(int i = 0; i < array.getNumChannels(); i++)
	{
		value += array.getValue(i);
	}
	
	internalUGens[0]->initValue(value);
}

Mix::Mix(UGen* array, bool shouldAllowAutoDelete) throw()
{
	initInternal(1);
	internalUGens[0] = new MixUGenInternal(array, shouldAllowAutoDelete);
	
	float value = 0.f;
	for(int i = 0; i < array->getNumChannels(); i++)
	{
		value += array->getValue(i);
	}
	
	internalUGens[0]->initValue(value);
}

Mix::Mix(UGenArray const& array, bool shouldAllowAutoDelete) throw()
{
	constructMixArrayWithProxies(new MixArrayUGenInternal(array, shouldAllowAutoDelete));
}

Mix::Mix(UGenArray* array, bool shouldAllowAutoDelete) throw()
{
	constructMixArrayWithProxies(new MixArrayUGenInternal(array, shouldAllowAutoDelete));
}

void Mix::constructMixArrayWithProxies(MixArrayUGenInternal* internal)
{
	int numProxies = internal->getNumProxies();
	initInternal(numProxies + 1);
	
	internalUGens[0] = internal;
	internalUGens[0]->initValue(internal->getValue(0));
	
	if(numProxies > 0)
	{
		for(int i = 1; i <= numProxies; i++)
		{
			internalUGens[i] = internal->getProxy(i);
			internalUGens[i]->initValue(internal->getValue(i));
		}
	}	
}

END_UGEN_NAMESPACE
