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


// same name as SC PLayBuf but no derived code

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_PlayBuf.h"


PlayBufUGenInternal::PlayBufUGenInternal(Buffer const& buffer, 
										 UGen const& rate, 
										 UGen const& trig, 
										 UGen const& offset, 
										 UGen const& loop, 
										 const UGen::DoneAction doneAction) throw()
:	ProxyOwnerUGenInternal(NumInputs, buffer.getNumChannels() - 1),
	buffer_(buffer),
	bufferPos(0.0),
	lastTrig(0.f),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	inputs[Rate] = rate;
	inputs[Trig] = trig;
	inputs[Offset] = offset;
	inputs[Loop] = loop;
}

// don't do this? no need?
UGenInternal* PlayBufUGenInternal::getChannel(const int channel) throw()
{	
	return new PlayBufUGenInternal(buffer_.getChannel(channel), inputs[Rate], inputs[Trig], inputs[Offset], inputs[Loop], doneAction_);
}

void PlayBufUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	const int blockSize = uGenOutput.getBlockSize();
		
	double channelBufferPos;
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		int numSamplesToProcess = blockSize;
		const int bufferSize = buffer_.size();
		channelBufferPos = bufferPos;
		float* outputSamples = proxies[channel]->getSampleData();
		float* rateSamples = inputs[Rate].processBlock(shouldDelete, blockID, 0);
		float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
		float* offsetSamples = inputs[Offset].processBlock(shouldDelete, blockID, 0);
		float* loopSamples = inputs[Loop].processBlock(shouldDelete, blockID, 0);
				
		while(numSamplesToProcess ) 
		{				
			float thisTrig = *trigSamples++;
			
			if(thisTrig > 0.f && lastTrig <= 0.f)
				channelBufferPos = 0.0;
			
			double offset = *offsetSamples++;
			double position = offset + channelBufferPos;
			
			if(*loopSamples++ >= 0.5f) 
			{
				if(position >= bufferSize)
					position -= bufferSize;
				else if(channelBufferPos < 0)
					position += bufferSize;
				
				*outputSamples++ = buffer_.getSample(channel, position);
				channelBufferPos += *rateSamples++;
			
				if(channelBufferPos >= bufferSize)
					channelBufferPos -= bufferSize;
				else if(channelBufferPos < 0)
					channelBufferPos += bufferSize;
			}
			else
			{
				*outputSamples++ = buffer_.getSample(channel, position);
				channelBufferPos += *rateSamples++;
			}
			
			--numSamplesToProcess;
			lastTrig = thisTrig;
		}		
	}
	
	bufferPos = channelBufferPos;
	
	if(bufferPos >= buffer_.size())
	{
		shouldDelete = shouldDelete ? true : shouldDeleteValue;
		sendDone();
	}
}



PlayBuf::PlayBuf(Buffer const& buffer, 
				 UGen const& rate, 
				 UGen const& trigger, 
				 UGen const& startPos, 
				 UGen const& loop, 
				 const UGen::DoneAction doneAction) throw()
{	
	// just mix the input ugens, they should be mono
	// mix() will just return the original UGen if it has only one channel anyway
	
	const int numChannels = buffer.getNumChannels();
	
	if(numChannels > 0 && buffer.size() > 0)
	{
		initInternal(numChannels);
		
		UGen startPosChecked = startPos.mix();
		generateFromProxyOwner(new PlayBufUGenInternal(buffer, 
													   rate.mix(), 
													   trigger.mix(), 
													   startPosChecked, 
													   loop.mix(), 
													   doneAction));

		for(int i = 0; i < numChannels; i++)
		{
			internalUGens[i]->initValue(buffer.getSample(i, startPosChecked.getValue(0)));
		}
	}	
}

BufferValuesUGenInternal::BufferValuesUGenInternal(Buffer const& bufferToUse)
:	ProxyOwnerUGenInternal(NumInputs, bufferToUse.size() - 1),
	buffer(bufferToUse)
{
}

void BufferValuesUGenInternal::processBlock(bool& shouldDelete, 
											const unsigned int blockID, 
											const int /*channel*/) throw()
{
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		int numSamplesToProcess = uGenOutput.getBlockSize();
		float* outputSamples = proxies[channel]->getSampleData();
		float value = buffer[channel];
		
		while(numSamplesToProcess--) *outputSamples++ = value;
	}
}

void BufferValuesUGenInternal::handleBuffer(Buffer const& bufferReceived, const double value1, const int value2) throw()
{
	buffer = bufferReceived;
}

BufferValues::BufferValues(Buffer const& buffer) throw()
{
	ugen_assert(buffer.size() > 0);
	ugen_assert(buffer.getNumChannels() > 0);
	
	initInternal(buffer.size());
	generateFromProxyOwner(new BufferValuesUGenInternal(buffer));
	
	for(int i = 0; i < buffer.size(); i++)
	{
		internalUGens[i]->initValue(buffer.getSampleUnchecked(i));
	}
}

END_UGEN_NAMESPACE
