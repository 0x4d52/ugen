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

MetaDataSender::MetaDataSender() throw()
{
}

MetaDataSender::~MetaDataSender()
{
}

void MetaDataSender::addMetaDataReceiver(MetaDataReceiver* const receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	if(receivers.contains(receiver)) return;
	
	receivers.add(receiver);		
}

void MetaDataSender::removeMetaDataReceiver(MetaDataReceiver* const receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	
	receivers = receivers.removeItem(receiver);		
}

void MetaDataSender::sendMetaData(Buffer const& buffer, MetaData const& metaData, MetaData::Type type, int channel, int index)
{
	const int size = receivers.size();
	for(int i = 0; i < size; i++)
	{
		receivers[i]->handleMetaData(buffer, metaData, type, channel, index);
	}			
}

PlayBufUGenInternal::PlayBufUGenInternal(Buffer const& buffer, 
										 UGen const& rate, 
										 UGen const& trig, 
										 UGen const& offset, 
										 UGen const& loop, 
										 const UGen::DoneAction doneAction,
										 MetaData const& metaDataToUse) throw()
:	ProxyOwnerUGenInternal(NumInputs, buffer.getNumChannels() - 1),
	buffer_(buffer),
	bufferPos(0.0),
	lastTrig(0.f),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone),
	metaData(metaDataToUse),
	prevPosArray(buffer_.getNumChannels() > 1 ? DoubleArray::series(buffer_.getNumChannels(), -1.0, 0.0) : DoubleArray(-1)) // fill with -1
{
	inputs[Rate] = rate;
	inputs[Trig] = trig;
	inputs[Offset] = offset;
	inputs[Loop] = loop;	
}

PlayBufUGenInternal::~PlayBufUGenInternal()
{
//	printf("PlayBufUGenInternal deleted %p\n", this);
}

// don't do this? no need?
UGenInternal* PlayBufUGenInternal::getChannel(const int channel) throw()
{	
	PlayBufUGenInternal* internal = new PlayBufUGenInternal(buffer_.getChannel(channel), 
															inputs[Rate], 
															inputs[Trig], 
															inputs[Offset], 
															inputs[Loop], 
															doneAction_,
															metaData);
	internal->bufferPos = bufferPos;
	internal->lastTrig = lastTrig;
	return internal;
	
	// what about MetaDataSender?
}

void PlayBufUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	senderUserData = userData;
	if(isDone()) sendDoneInternal();
}

void PlayBufUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	const int numCuesPoints = metaData.getNumCuePoints();

	const int blockSize = uGenOutput.getBlockSize();
	const int bufferSize = buffer_.size();
	const double lastBufferPosition = bufferSize-1;
	
	double channelBufferPos;
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		int numSamplesToProcess = blockSize;
		channelBufferPos = bufferPos;
		float* outputSamples = proxies[channel]->getSampleData();
		float* rateSamples = inputs[Rate].processBlock(shouldDelete, blockID, 0);
		float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
		float* offsetSamples = inputs[Offset].processBlock(shouldDelete, blockID, 0);
		float* loopSamples = inputs[Loop].processBlock(shouldDelete, blockID, 0);
						
		double prevPos = prevPosArray[channel];
		
		while(numSamplesToProcess) 
		{				
			float thisTrig = *trigSamples++;
			
			if(thisTrig > 0.f && lastTrig <= 0.f)
				channelBufferPos = 0.0;
			
			double offset = *offsetSamples++;
			double position = offset + channelBufferPos;
			
			if(*loopSamples++ >= 0.5f) 
			{
				if(position >= bufferSize)
				{
					position -= bufferSize;
				} 
				else if(position < 0)
				{
					position += bufferSize;
				}
				
				*outputSamples++ = buffer_.getSample(channel, position);
				channelBufferPos += *rateSamples++;
			
				if(channelBufferPos >= bufferSize)
				{
					sendMetaData(buffer_, metaData, MetaData::ReachedEnd, channel);
					channelBufferPos -= bufferSize;
					sendMetaData(buffer_, metaData, MetaData::ReachedStart, channel);
					if(numCuesPoints > 0) checkMetaDataCuePoints(channelBufferPos, -1.0, channel, numCuesPoints, true);
					prevPos = channelBufferPos;
				}
				else if(channelBufferPos < 0)
				{
					sendMetaData(buffer_, metaData, MetaData::ReachedEnd, channel);
					channelBufferPos += bufferSize;
					sendMetaData(buffer_, metaData, MetaData::ReachedStart, channel);
					if(numCuesPoints > 0) checkMetaDataCuePoints(channelBufferPos, bufferSize, channel, numCuesPoints, false);
					prevPos = channelBufferPos;
				}
				else if(numCuesPoints > 0) 
				{
					checkMetaDataCuePoints(channelBufferPos, prevPos, channel, numCuesPoints, prevPos <= channelBufferPos);
					prevPos = channelBufferPos;
				}
			}
			else
			{
				if(numCuesPoints > 0) checkMetaDataCuePoints(channelBufferPos, prevPos, channel, numCuesPoints, prevPos <= channelBufferPos);
				prevPos = channelBufferPos;

				if((position <= 0.0) || (position > lastBufferPosition))
				{	
					*outputSamples++ = 0.0;
				}
				else
				{
					*outputSamples++ = buffer_.getSampleUnchecked(channel, position);
				}
				
				channelBufferPos += *rateSamples++;
			}
					
			--numSamplesToProcess;
			lastTrig = thisTrig;
		}		
		
		prevPosArray[channel] = prevPos;
	}
	
	bufferPos = channelBufferPos;
	
	if(bufferPos >= buffer_.size())
	{
		shouldDelete = shouldDelete ? true : shouldDeleteValue;
		setIsDone();
		sendMetaData(buffer_, metaData, MetaData::ReachedEnd);
	}
	else if(bufferPos < 0)
	{
		shouldDelete = shouldDelete ? true : shouldDeleteValue;
		setIsDone();
		sendMetaData(buffer_, metaData, MetaData::ReachedStart);		
	}
}

void PlayBufUGenInternal::checkMetaDataCuePoints(const double currentPosition, 
												 const double previousPosition, 
												 const int channel, 
												 const int numCuePoints,
												 const bool forwards) throw()
{	
	CuePointArray& cuePoints = metaData.getCuePoints();
	CuePoint* cuePointArray = cuePoints.getArray();
	
	if(forwards)
	{
		for(int i = 0; i < numCuePoints; i++)
		{
			double cuePosition = cuePointArray[i].getSampleOffset();
			
			if((previousPosition < cuePosition) && (currentPosition >= cuePosition))
			{
				sendMetaData(buffer_, metaData, MetaData::CuePointInfo, channel, i);		
			}
		}
	}
	else
	{
		for(int i = 0; i < numCuePoints; i++)
		{
			double cuePosition = cuePointArray[i].getSampleOffset();
			
			if((previousPosition > cuePosition) && (currentPosition <= cuePosition))
			{
				sendMetaData(buffer_, metaData, MetaData::CuePointInfo, channel, i);		
			}
		}		
	}
}

double PlayBufUGenInternal::getDuration() const throw()
{
	return buffer_.duration();
}

double PlayBufUGenInternal::getPosition() const throw()
{
	return bufferPos * UGen::getReciprocalSampleRate();
}

bool PlayBufUGenInternal::setPosition(const double newPosition) throw()
{
	bufferPos = ugen::max(0.0, newPosition) * UGen::getSampleRate();
	return true;
}


PlayBuf::PlayBuf(Buffer const& buffer, 
				 UGen const& rate, 
				 UGen const& trigger, 
				 UGen const& startPos, 
				 UGen const& loop, 
				 const UGen::DoneAction doneAction,
				 MetaData const& metaData) throw()
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
													   doneAction,
													   metaData));

		for(int i = 0; i < numChannels; i++)
		{
			internalUGens[i]->initValue(buffer.getSample(i, startPosChecked.getValue(0)));
		}
	}	
	else
	{ 
		//?? what was I thinking here?
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


RecordBufUGenInternal::RecordBufUGenInternal(UGen const& input,
											 Buffer const& buffer, 
											 UGen const& recLevel,
											 UGen const& preLevel,
											 UGen const& loop, 
											 const UGen::DoneAction doneAction) throw()
:	ProxyOwnerUGenInternal(NumInputs, ugen::max(buffer.getNumChannels(), input.getNumChannels()) - 1),
	buffer_(buffer),
	bufferPos(0),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	inputs[Input] = input;
	inputs[RecLevel] = recLevel;
	inputs[PreLevel] = preLevel;
	inputs[Loop] = loop;
}

UGenInternal* RecordBufUGenInternal::getChannel(const int channel) throw()
{
	return new RecordBufUGenInternal(inputs[Input].getChannel(channel),
									 buffer_.getChannel(channel), 
									 inputs[RecLevel].getChannel(channel), 
									 inputs[PreLevel].getChannel(channel), 
									 inputs[Loop], 
									 doneAction_);
}

void RecordBufUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	senderUserData = userData;
	if(isDone()) sendDoneInternal();
}

void RecordBufUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	
	int channelBufferPos;
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		int numSamplesToProcess = blockSize;
		const int bufferSize = buffer_.size();
		channelBufferPos = bufferPos;
		float* outputSamples = proxies[channel]->getSampleData();
		float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
		float* preLevelSamples = inputs[PreLevel].processBlock(shouldDelete, blockID, channel);
		float* recLevelSamples = inputs[RecLevel].processBlock(shouldDelete, blockID, channel);
		float* loopSamples = inputs[Loop].processBlock(shouldDelete, blockID, 0);
		float* bufferSamples = buffer_.getData(channel);
		
		while(numSamplesToProcess) 
		{										
			if(*loopSamples++ >= 0.5f) 
			{
				if(channelBufferPos >= bufferSize)
					channelBufferPos = 0;
				
				float recLevel = *recLevelSamples++;
				float rec = *inputSamples++ * recLevel;
				float preLevel = *preLevelSamples++;
				float pre = bufferSamples[channelBufferPos] * preLevel;
				float out = pre + rec;
				*outputSamples++ = out;
				bufferSamples[channelBufferPos] = out;
				channelBufferPos++;
				
				if(channelBufferPos >= bufferSize)
					channelBufferPos = 0;
			}
			else
			{
				float rec = *inputSamples++ * *recLevelSamples++;
				float pre = bufferSamples[channelBufferPos] * *preLevelSamples++;
				float out = pre + rec;
				*outputSamples++ = out;
				bufferSamples[channelBufferPos] = out;
				channelBufferPos++;
			}
			
			--numSamplesToProcess;
		}		
	}
	
	bufferPos = channelBufferPos;
	
	if(bufferPos >= buffer_.size())
	{
		shouldDelete = shouldDelete ? true : shouldDeleteValue;
		setIsDone();
	}	
}

double RecordBufUGenInternal::getDuration() const throw()
{
	return buffer_.duration();
}

double RecordBufUGenInternal::getPosition() const throw()
{
	return bufferPos * UGen::getReciprocalSampleRate();
}

bool RecordBufUGenInternal::setPosition(const double newPosition) throw()
{
	bufferPos = ugen::max(0.0, newPosition) * UGen::getSampleRate();
	return true;
}

RecordBuf::RecordBuf(UGen const& input,
					 Buffer const& buffer, 
					 UGen const& recLevel,
					 UGen const& preLevel,
					 UGen const& loop, 
					 const UGen::DoneAction doneAction) throw()
{
	ugen_assert(buffer.size() > 0);
	ugen_assert(buffer.getNumChannels() > 0);

	const int numChannels = ugen::max(buffer.getNumChannels(), input.getNumChannels());
	initInternal(numChannels);
	generateFromProxyOwner(new RecordBufUGenInternal(input, buffer, recLevel, preLevel, loop.mix(), doneAction));
}
	
LoopPointsUGenInternal::LoopPointsUGenInternal(Buffer const& buffer, 
											   UGen const& rate, 
											   UGen const& start, 
											   UGen const& end,
											   UGen const& loop, 
											   UGen const& startAtZero,
											   UGen const& playToEnd,
											   const UGen::DoneAction doneAction,
											   MetaData const& metaDataToUse) throw()
:	UGenInternal(NumInputs),
	b(buffer),
	currentValue((startAtZero.getValue() >= 0.5f) ? 0.f : start.getValue() * b.size()),
	lastLoop(false),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone),
	metaData(metaDataToUse),
	prevValue((rate.getValue() >= 0.f) ? currentValue - 1.f : b.size())
{
	inputs[Rate] = rate;
	inputs[Start] = start;
	inputs[End] = end;
	inputs[Loop] = loop;
	inputs[StartAtZero] = startAtZero;
	inputs[PlayToEnd] = playToEnd;
}

void LoopPointsUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	senderUserData = userData;
	if(isDone()) sendDoneInternal();
}

void LoopPointsUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numCuesPoints = metaData.getNumCuePoints();

	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* rateSamples = inputs[Rate].processBlock(shouldDelete, blockID, 0);
	float* startSamples = inputs[Start].processBlock(shouldDelete, blockID, 0);
	float* endSamples = inputs[End].processBlock(shouldDelete, blockID, 0);
	float* loopSamples = inputs[Loop].processBlock(shouldDelete, blockID, 0);
	float* startAtZeroSamples = inputs[StartAtZero].processBlock(shouldDelete, blockID, 0);
	float* playToEndSamples = inputs[PlayToEnd].processBlock(shouldDelete, blockID, 0);
		
	while(numSamplesToProcess--)
	{
		float rate = *rateSamples;
		float start = *startSamples * b.size();
		float end = *endSamples * b.size();
		bool loop = *loopSamples >= 0.5f;
		
		if(start > end)
		{
			float temp = start;
			start = end;
			end = temp;
			rate = -rate;
		}
		
		currentValue += rate;
		
		if(loop)
		{            
			if((rate > 0.f) && (currentValue >= end))
			{
				if(lastLoop == false)
				{
					bool fromZero = *startAtZeroSamples >= 0.5f;
					
					currentValue = fromZero ? 0.f : start;
				}
				else
				{
					currentValue -= (end - start);
				}
				
				prevValue = currentValue-1.f;
			}
			else if((rate < 0.f) && (currentValue < start))
			{
				if(lastLoop == false)
				{
					bool fromEnd = *startAtZeroSamples >= 0.5f;

					currentValue = fromEnd ? (b.size()-1) : end;
				}
				else
				{            
					currentValue += (end - start);
				}
				
				prevValue = currentValue+1.f;
			}
		}
		else if(*playToEndSamples < 0.5f)
		{
			if((rate > 0.f) && (currentValue >= end))
			{
				currentValue = (float)b.size();
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				setIsDone();
				
			}
			else if((rate < 0.f) && (currentValue < start))
			{
				currentValue = -1.f;
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				setIsDone();
			}			
		}
		else
		{
			if((rate > 0.f) && (currentValue >= (float)b.size()))
			{
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				setIsDone();
			}
			else if((rate < 0.f) && (currentValue < 0.f))
			{
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				setIsDone();
			}						
		}
		
		*outputSamples = currentValue;
		
		if(numCuesPoints) checkMetaDataCuePoints(currentValue, prevValue, numCuesPoints, rate >= 0.f);
		
		prevValue = currentValue;
		
		rateSamples++;
		startSamples++;
		endSamples++;
		loopSamples++;
		startAtZeroSamples++;
		playToEndSamples++;
		outputSamples++;
		
		lastLoop = loop;
	}
}

void LoopPointsUGenInternal::checkMetaDataCuePoints(const float currentPosition, 
													const float previousPosition, 
													const int numCuePoints,
													const bool forwards) throw()
{	
	CuePointArray& cuePoints = metaData.getCuePoints();
	CuePoint* cuePointArray = cuePoints.getArray();
	
	if(forwards)
	{
		for(int i = 0; i < numCuePoints; i++)
		{
			float cuePosition = cuePointArray[i].getSampleOffset();
			
			if((previousPosition < cuePosition) && (currentPosition >= cuePosition))
			{
				sendMetaData(b, metaData, MetaData::CuePointInfo, -1, i);		
			}
		}
	}
	else
	{
		for(int i = 0; i < numCuePoints; i++)
		{
			float cuePosition = cuePointArray[i].getSampleOffset();
			
			if((previousPosition > cuePosition) && (currentPosition <= cuePosition))
			{
				sendMetaData(b, metaData, MetaData::CuePointInfo, -1, i);		
			}
		}		
	}
}


double LoopPointsUGenInternal::getDuration() const throw()
{
	return b.duration();
}

double LoopPointsUGenInternal::getPosition() const throw()
{
	return currentValue * b.duration();
}
	
bool LoopPointsUGenInternal::setPosition(const double newPosition) throw()
{
	double normalisedPos = newPosition / b.duration();
	
	currentValue = ugen::clip(normalisedPos, 0.0, 1.0);
	
	return true;
}

LoopPoints::LoopPoints(Buffer const& buffer, 
					   UGen const& rate, 
					   UGen const& start, 
					   UGen const& end,
					   UGen const& loop, 
					   UGen const& startAtZero,
					   UGen const& playToEnd,
					   const UGen::DoneAction doneAction,
					   MetaData const& metaData) throw()
{
	initInternal(1);
	internalUGens[0] = new LoopPointsUGenInternal(buffer, 
												  rate.mix(), 
												  start.mix(), 
												  end.mix(), 
												  loop.mix(), 
												  startAtZero.mix(), 
												  playToEnd.mix(),
												  doneAction,
												  metaData);
}

END_UGEN_NAMESPACE
