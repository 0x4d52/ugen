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

#include "../../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_TableOsc.h"
#include "../../core/ugen_Constants.h"


TableOscUGenInternal::TableOscUGenInternal(UGen const& freq, 
										   const float initialPhase, 
										   Buffer const& table) throw()
:	UGenInternal(NumInputs),
	table_(table),
	wavetableSize(table_.size()),
	wavetable(table_.getData(0)),
	currentPhase((initialPhase < 0.f) || (initialPhase >= 1.f) ? 0.f : initialPhase * wavetableSize)
{
	ugen_assert(initialPhase >= 0.f && initialPhase <= 1.f);
	
	inputs[Freq] = freq;
	initValue(lookupIndex(currentPhase));
}

UGenInternal* TableOscUGenInternal::getChannel(const int channel) throw()
{
	return new TableOscUGenInternal(inputs[Freq].getChannel(channel),
									currentPhase,
									table_);
}	

UGenInternal* TableOscUGenInternal::getKr() throw() 
{ 
	return new TableOscUGenInternalK(inputs[Freq].kr(), currentPhase, table_); 
}

void TableOscUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	float tableSizeOverSampleRate = UGen::getReciprocalSampleRate() * wavetableSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	LOCAL_DECLARE(float, currentPhase);
	LOCAL_DECLARE(float, wavetableSize);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = lookupIndex(currentPhase);
		currentPhase += *freqSamples++ * tableSizeOverSampleRate;
		
		if(currentPhase >= wavetableSize)	currentPhase -= wavetableSize;
		else if(currentPhase < 0.f)			currentPhase += wavetableSize;
	}
	
	LOCAL_COPY(currentPhase);
}

TableOscUGenInternalK::TableOscUGenInternalK(UGen const& freq, 
											 const float initialPhase, 
											 Buffer const& table) throw()
:	TableOscUGenInternal(freq, initialPhase, table),
	value(lookupIndex(initialPhase))
{
	rate = ControlRate;
}

void TableOscUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)			
		{
			nextValue = lookupIndex(currentPhase);
			
			currentPhase += *freqSamples * krBlockSizeOverSampleRate * wavetableSize;
			
			while(currentPhase >= wavetableSize)	currentPhase -= wavetableSize;
			while(currentPhase < 0.f)				currentPhase += wavetableSize;
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		
		blockPosition		+= numKrSamples;
		freqSamples			+= numKrSamples;
		
		if(nextValue == value)
		{
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = nextValue;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		else
		{
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = value;
				value += valueSlope;
				--numSamplesToProcess;
				--numKrSamples;
			}
			
			value = nextValue;
		}
	}
}


TableOsc::TableOsc(Buffer const& table, UGen const& freq, Buffer const& initialPhase) throw()
{	
	// could check here that the table is padded appropriately and use a less efficient version if not
	
	int numChannels = ugen::max(freq.getNumChannels(), initialPhase.size());
	
	initInternal(numChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new TableOscUGenInternal(freq, initialPhase.wrapAt(i), table);
	}
}

HarmonicOsc::HarmonicOsc(Buffer const& harmonicWeights, UGen const& freq, Buffer const& initialPhase) throw()
{	
	int numChannels = ugen::max(freq.getNumChannels(), initialPhase.size());
	
	initInternal(numChannels);
	
	Buffer table(BufferSpec(8193, 1, true));
	
	for(int harmonic = 1; harmonic <= harmonicWeights.size(); harmonic++)
	{
		float amp = harmonicWeights.getSampleUnchecked(0, harmonic-1);
		table += Buffer::line(8193, 0.0, harmonic * twoPi).sin() * amp;
	}
	
	table = table.shrinkSize();
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new TableOscUGenInternal(freq, initialPhase.wrapAt(i), table);
	}
}

SinOsc::SinOsc(UGen const& freq, Buffer const& initialPhase) throw()
{	
	int numChannels = ugen::max(freq.getNumChannels(), initialPhase.size());
	
	initInternal(numChannels);
		
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new TableOscUGenInternal(freq, initialPhase.wrapAt(i), Buffer::getTableSine8192());
	}
}


END_UGEN_NAMESPACE
