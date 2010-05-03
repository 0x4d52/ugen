// $Id: ugen_Dust.cpp 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/noise/ugen_Dust.cpp $

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

#include "ugen_Dust.h"

DustUGenInternal::DustUGenInternal(Dust_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs),
	//random((unsigned int)this * 823487UL + 18493UL + rand(1296)),
	random(rand(0x7fffffff)),
	prevDensity(0.f),
	threshold(0.f),
	scale(0.f)
{
	inputs[Density] = density;
}

UGenInternal* DustUGenInternal::getChannel(const int channel) throw()
{	
	return new DustUGenInternal(inputs[Density].getChannel(channel));
}	

void DustUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float currentDensity = *(inputs[Density].processBlock(shouldDelete, blockID, channel));
	
	unsigned long s1, s2, s3;
	random.get(s1, s2, s3);
	
	if (currentDensity != prevDensity) {
		threshold = currentDensity * UGen::getReciprocalSampleRate();
		scale  = threshold > 0.f ? 1.f / threshold : 0.f;
	}
	
	while(numSamplesToProcess--)
	{
		float value = Ran088::nextFloat(s1, s2, s3);
		if(value < threshold)
			*outputSamples++ = value * scale;
		else
			*outputSamples++ = 0.f;
	}
	
	prevDensity = currentDensity;
	random.set(s1, s2, s3);
}

Dust2UGenInternal::Dust2UGenInternal(Dust_InputsWithTypesOnly) throw()
:	DustUGenInternal(Dust_InputsNoTypes)
{
}

UGenInternal* Dust2UGenInternal::getChannel(const int channel) throw()
{	
	return new Dust2UGenInternal(inputs[Density].getChannel(channel));
}

void Dust2UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float currentDensity = *(inputs[Density].processBlock(shouldDelete, blockID, channel));
	
	unsigned long s1, s2, s3;
	random.get(s1, s2, s3);
	
	if (currentDensity != prevDensity) {
		threshold = currentDensity * UGen::getReciprocalSampleRate();
		scale  = threshold > 0.f ? 2.f / threshold : 0.f;
	}
	
	while(numSamplesToProcess--)
	{
		float value = Ran088::nextFloat(s1, s2, s3);
		if(value < threshold)
			*outputSamples++ = value * scale - 1.f;
		else
			*outputSamples++ = 0.f;
	}
	
	prevDensity = currentDensity;
	random.set(s1, s2, s3);	
}

Dust::Dust(Dust_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { Dust_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	for(int i = 0; i < numInputChannels; i++)
	{
		internalUGens[i] = new DustUGenInternal(Dust_InputsNoTypes);
	}
}

Dust2::Dust2(Dust_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { Dust_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	for(int i = 0; i < numInputChannels; i++)
	{
		internalUGens[i] = new Dust2UGenInternal(Dust_InputsNoTypes);
	}
}


END_UGEN_NAMESPACE
