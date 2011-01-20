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

#include "ugen_OscillatorUGens.h"


PhasorUGenInternal::PhasorUGenInternal(Phasor_InputsWithTypesOnly, const float initialPhase) throw()
:	UGenInternal(NumInputs),
	currentPhase(initialPhase)
{
	UGen args[] = { Phasor_InputsNoTypes };
	for(int i = 0; i < NumInputs; i++)
	{
		inputs[i] = args[i];
	}
}

//UGenInternal* PhasorUGenInternal::getChannel(const int channel) throw()
//{	
//	UGenInternal* internals[NumInputs];
//	getInternalChannels(channel, internals);
//	
//	return new PhasorUGenInternal(UGen(internals[Freq], channel),
//								  UGen(internals[Phase], channel),
//								  currentPhase);
//}	

UGenInternal* PhasorUGenInternal::getChannel(const int channel) throw()
{		
	return new PhasorUGenInternal(inputs[Freq].getChannel(channel),
								  inputs[Phase].getChannel(channel),
								  currentPhase);
}

UGenInternal* PhasorUGenInternal::getKr() throw() 
{ 
	return new PhasorUGenInternalK(inputs[Freq].kr(), 
								   inputs[Phase].kr(), 
								   currentPhase); 
}


void PhasorUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	double reciprocalSampleRate = UGen::getReciprocalSampleRate();
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	float* phaseSamples = inputs[Phase].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		if(currentPhase >= 1.0)			currentPhase -= 1.0;
		else if(currentPhase < 0.0)		currentPhase += 1.0;
		
		double outputPhase = currentPhase + *phaseSamples++;
		currentPhase += *freqSamples++ * reciprocalSampleRate;
		
		if(outputPhase > 1.0)			outputPhase -= 1.0;
		else if(outputPhase < 0.0)		outputPhase += 1.0;
		
		*outputSamples++ = outputPhase;
	}
}

//void PhasorUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
//{
//	const int krBlockSize = UGen::getControlRateBlockSize();
//	unsigned int blockPosition = blockID % krBlockSize;
//	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
//	int numSamplesToProcess = uGenOutput.getBlockSize();
//	float* outputSamples = uGenOutput.getSampleData();
//	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
//	float* phaseSamples = inputs[Phase].processBlock(shouldDelete, blockID, channel);
//	
//	int numKrSamples = blockPosition % krBlockSize;
//	
//	while(numSamplesToProcess > 0)
//	{
//		if(numKrSamples == 0)
//		{
//			if(currentPhase >= 1.0)			currentPhase -= 1.0;
//			else if(currentPhase < 0.0)		currentPhase += 1.0;
//			
//			double outputPhase = currentPhase + *phaseSamples;
//			currentPhase += *freqSamples * krBlockSizeOverSampleRate;
//			
//			if(outputPhase > 1.0)			outputPhase -= 1.0;
//			else if(outputPhase < 0.0)		outputPhase += 1.0;
//			
//			value = outputPhase;
//		}
//		
//		numKrSamples = krBlockSize - numKrSamples;
//		
//		blockPosition		+= numKrSamples;
//		freqSamples			+= numKrSamples;
//		phaseSamples		+= numKrSamples;
//		
//		while(numSamplesToProcess && numKrSamples)
//		{
//			*outputSamples++ = value;
//			--numSamplesToProcess;
//			--numKrSamples;
//		}
//	}
//}

void PhasorUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID & (krBlockSize-1);
	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	float* phaseSamples = inputs[Phase].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition & (krBlockSize-1);
	
	if(numKrSamples != 0)
	{
		numKrSamples = krBlockSize - numKrSamples;
		
		numSamplesToProcess -= numKrSamples;
		outputSamples		+= numKrSamples;
		freqSamples			+= numKrSamples;
		phaseSamples		+= numKrSamples;
	}
	
	while(numSamplesToProcess > 0)
	{
		if(currentPhase >= 1.0)			currentPhase -= 1.0;
		else if(currentPhase < 0.0)		currentPhase += 1.0;
		
		double outputPhase = currentPhase + *phaseSamples;
		currentPhase += *freqSamples * krBlockSizeOverSampleRate;
		
		if(outputPhase > 1.0)			outputPhase -= 1.0;
		else if(outputPhase < 0.0)		outputPhase += 1.0;
		
		*outputSamples = value = outputPhase;
			
		numSamplesToProcess	-= krBlockSize;
		outputSamples		+= krBlockSize;
		freqSamples			+= krBlockSize;
		phaseSamples		+= krBlockSize;		
	} 
}


Phasor::Phasor(Phasor_InputsWithTypesOnly) throw()
{	
	UGen inputs[] = { Phasor_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new PhasorUGenInternal(Phasor_InputsNoTypes);
	}
}


END_UGEN_NAMESPACE


