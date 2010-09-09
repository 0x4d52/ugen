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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#if !defined(WIN32) && !defined(UGEN_IPHONE)
	#include <Accelerate/Accelerate.h>
	#include <CoreServices/CoreServices.h>
#endif

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Scope.h"
#include "../fft/ugen_FFTEngineInternal.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../basics/ugen_BinaryOpUGens.h"

ScopeGUI::ScopeGUI(const ScopeStyles style)
:	isBipolar(true),
	yMaximum(1.f),
	lowerMargin(0.f),
	style_(style),
	scaleX(LabelXNone),
	scaleY(LabelYNone),
	labelFirstX(true),
	markSpacingY(0.1),
	decimalPlacesY(1),
	labelHopY(2),
	markSpacingX(4.0),
	labelHopX(4),
	markXHeight(3),
	markYWidth(3),
	offsetSamples(0.0),
	fftSize(-1),
	bufferWrap(0.0),
	textSizeX(9.f),
	textSizeY(9.f),
	textSizeChannel(11.f),
	labelChannels(true),
	channelLabelOffset(0)
{
	colours[Background] =	RGBAColour(0.2, 0.2, 0.2);
	colours[TopLine] =		RGBAColour(0.6, 0.6, 0.6);
	colours[ZeroLine] =		RGBAColour(0.4, 0.4, 0.4);
	colours[LabelMarks] =	RGBAColour(0.5, 0.9, 0.5); 
	colours[TextX] =		RGBAColour(0.5, 0.9, 0.5);
	colours[TextY] =		RGBAColour(0.5, 0.9, 0.5);
	colours[TextChannel] =	RGBAColour(0.9, 0.6, 0.5, 0.5);
	colours[Trace] =		RGBAColour(1.0, 1.0, 1.0);
}

void ScopeGUI::setPolarity(const bool isBipolar) throw()
{
	lock();
	this->isBipolar = isBipolar;
	unlock();
	
	updateGUI();
}

void ScopeGUI::setYMaximum(const float maximum) throw()
{
	lock();
	yMaximum = maximum;
	unlock();
	
	updateGUI();
}

void ScopeGUI::setLowerMargin(const float margin) throw()
{
	lock();
	lowerMargin = margin;
	unlock();
	
	updateGUI();
}

void ScopeGUI::setScaleX(Scales scale, const double markSpacing, const int labelHop, const bool labelFirst) throw()
{	
	lock();
	switch(scale)
	{
		case LabelXNone:
		case LabelXMarks:
		case LabelXSamples:
		case LabelXTime:
		case LabelXFrequency:
			scaleX = scale;
			break;
		default:
			;
	}
	
	if(markSpacing > 0.0)
		markSpacingX = markSpacing;
	
	if(labelHop > 0)
		labelHopX = labelHop;
	
	labelFirstX = labelFirst;
	
	unlock();
	updateGUI();
}

void ScopeGUI::setScaleY(Scales scale, const double markSpacing, const int labelHop, const int decimalPlaces) throw()
{
	lock();
	switch(scale)
	{
		case LabelYNone:
		case LabelYMarks:
		case LabelYAmplitude:
			scaleY = scale;
			break;
		default:
			;
	}
	
	if(markSpacing > 0.0)
		markSpacingY = markSpacing;
	
	if(labelHop > 0)
		labelHopY = labelHop;
	
	if(decimalPlaces >= 0)
		decimalPlacesY = decimalPlaces;
	
	unlock();
	updateGUI();
}

void ScopeGUI::initBuffers() throw()
{
	const int drawBufferSize = getDisplayBufferSize();
	
	if(drawBufferSize > 0 && audioBuffer.size() > 0)
	{
		minDrawBuffer = Buffer(BufferSpec(drawBufferSize, audioBuffer.getNumChannels(), true));
		maxDrawBuffer = Buffer(BufferSpec(drawBufferSize, audioBuffer.getNumChannels(), true));
	}
}

void ScopeGUI::setAudioBuffer(Buffer const& audioBufferToUse, const double offset, const int fftSizeOfSource) throw()
{	
	offsetSamples = offset; // this will be a time offset in samples or the first bin for an FFT
	
	if(fftSizeOfSource > 0)
		fftSize = fftSizeOfSource;
	
	if(tryLock() == true)
	{
		if((smoothingBuffer.size() > 0) && 
		   (audioBuffer.size() == audioBufferToUse.size()) &&
		   (audioBuffer.getNumChannels() == audioBufferToUse.getNumChannels())
		) {
			// smoothingBuffer should be interpolation coeffs for each channel
			// so Buffer(0.25, 1.0) should smooth even channels but not odd channels
			
			Buffer newBuffer;
			
			for(int i = 0; i < audioBuffer.getNumChannels(); i++)
			{
				const float coeff = smoothingBuffer.wrapAt(i);
				
				if(coeff > -1.f && coeff < 1.f)
				{
					newBuffer <<= audioBuffer.getChannel(i).blend(audioBufferToUse.getChannel(i), coeff);
				}
				else
				{
					newBuffer <<= audioBufferToUse.getChannel(i);
				}
				
				
//				if(coeff > 0.f && coeff < 1.f)
//				{
//					newBuffer <<= audioBufferToUse.getChannel(i) * coeff + audioBuffer.getChannel(i) * (1.f-coeff);
//				}
//				else if(coeff < 0.f && coeff > -1.f)
//				{
//					// peak hold
//					const int size = audioBufferToUse.size();
//					Buffer peakHoldBuffer = Buffer::newClear(size, 1, false);
//					float* peakHoldBufferSamples = peakHoldBuffer.getData();
//					const float* audioBufferToUseSamples = audioBufferToUse.getData(i);
//					const float* audioBufferSamples = audioBuffer.getData(i);
//					const float newCoeff = -coeff;
//					const float oldCoeff = 1.f-newCoeff;
//					
//					for(int sample = 0; sample < size; sample++)
//					{
//						if(audioBufferToUseSamples[sample] > audioBufferSamples[sample])
//						{
//							peakHoldBufferSamples[sample] = audioBufferToUseSamples[sample];
//						}
//						else
//						{
//							float newSample = audioBufferToUseSamples[sample] * newCoeff;
//							float oldSample = audioBufferSamples[sample] * oldCoeff;
//							peakHoldBufferSamples[sample] = newSample + oldSample;
//						}
//					}
//					
//					newBuffer <<= peakHoldBuffer;
//				}
//				else
//				{
//					newBuffer <<= audioBufferToUse.getChannel(i);
//				}
			}
			
			audioBuffer = newBuffer;
		}
		else
			audioBuffer = audioBufferToUse.copy();
		
		initBuffers();
		unlock();
		
		updateGUI();
	}
}

void ScopeGUI::handleBuffer(Buffer const& buffer, const double offset, const int fftSize) throw()
{
	setAudioBuffer(buffer, offset, fftSize);
}

void ScopeGUI::setWrap(const double amount) throw()
{
	lock();
//	bufferWrap = ugen::wrap(amount, 0.0, 1.0);
	
	double wrappedAmount = amount;
	
	if(amount < 0.0)
	{
		while(amount < 0.0)
			wrappedAmount += 1.0;
	}
	else
	{
		while(amount > 1.0)
			wrappedAmount -= 1.0;
	}
	
	bufferWrap = wrappedAmount;

	unlock();
	
	updateGUI();
}

void ScopeGUI::setScopeColour(const ScopeColours which, RGBAColour const& colour) throw()
{
	if((which >= 0) && (which < NumScopeColours))
	{
		lock();
		colours[which] = colour;
		unlock();
		
		updateGUI();
	}
}

const RGBAColour& ScopeGUI::getScopeColour(const ScopeColours which) const throw()
{
	if((which < 0) || (which >= NumScopeColours)) 
		return RGBAColour::getNull();
	else
		return colours[which];
}

void ScopeGUI::setMarkXHeight(const int size) throw()
{
	if(size >= 0)
	{
		lock();
		markXHeight = size;
		unlock();
		
		updateGUI();
	}
	else
		ugen_assertfalse;
}

void ScopeGUI::setMarkYWidth(const int size) throw()
{
	if(size >= 0)
	{
		lock();
		markYWidth = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setMarkSizes(const int size) throw()
{
	if(size >= 0)
	{
		lock();
		markXHeight = size;
		markYWidth = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setTextSizeX(const float size) throw()
{
	if(size > 0.f)
	{
		lock();
		textSizeX = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setTextSizeY(const float size) throw()
{
	if(size > 0.f)
	{
		lock();
		textSizeY = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setTextSizeChannel(const float size) throw()
{
	if(size > 0.f)
	{
		lock();
		textSizeChannel = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setTextSizes(const float size) throw()
{
	if(size > 0.f)
	{
		lock();
		textSizeX = textSizeY = textSizeChannel = size;
		unlock();
		
		updateGUI();
	}	
	else
		ugen_assertfalse;
}

void ScopeGUI::setChannelLabels(TextArray const& labels, const int offset) throw()
{
	lock();
	channelLabels = labels;
	channelLabelOffset = offset;
	unlock();
}

void ScopeGUI::calculateBuffers()
{	
	if(tryLock() == true)
	{
		const int audioBufferSize = audioBuffer.size();
		const int drawBufferSize = getDisplayBufferSize();
		
		if(audioBufferSize > 0 && minDrawBuffer.size() > 0 && maxDrawBuffer.size() > 0)
		{
			minDrawBuffer.clear();
			maxDrawBuffer.clear();
			
			const double drawIndexInc = min(1.0, (double)drawBufferSize / (double)audioBufferSize);
			const double audioIndexInc = min(1.0, (double)audioBufferSize / (double)drawBufferSize);
			
			for(int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
			{		
				if(drawIndexInc < 1.0)
				{
					double drawIndex = 0.0;			
					int previousDrawIndex = -1;
					float minimum, maximum;
					
					for(int sample = 0; sample < audioBufferSize; sample++, drawIndex += drawIndexInc)
					{
						const int iDrawIndex = (int)drawIndex;
						float value = audioBuffer.getSampleUnchecked(channel, sample);
						if(previousDrawIndex != iDrawIndex)
						{
							if(previousDrawIndex >= 0)
							{
								minDrawBuffer.setSampleUnchecked(channel, previousDrawIndex, minimum);
								maxDrawBuffer.setSampleUnchecked(channel, previousDrawIndex, maximum);
							}
							
							minimum = maximum = value;
							previousDrawIndex = iDrawIndex;
						}
						else
						{
							minimum = min(minimum, value);
							maximum = max(maximum, value);
						}
					}
				}
				else
				{
					double audioIndex = 0.0;
					
					for(int horiz = 0; horiz < drawBufferSize; horiz++, audioIndex += audioIndexInc)
					{
						float value = audioBuffer.getSampleUnchecked(channel, (int)audioIndex);
						minDrawBuffer.setSampleUnchecked(channel, horiz, value);
						maxDrawBuffer.setSampleUnchecked(channel, horiz, value);
					}
				}
				
			}
		}
		
		unlock();
	}
}

void ScopeGUI::resizedGUI()
{	
	initBuffers();
	
	if(audioBuffer.size() > 0)
	{
		calculateBuffers();
	}
}

ScopeUGenInternal::ScopeUGenInternal(ScopeGUIPtrPtr scopeGUI, UGen const& input, UGen const& duration) throw()
:	UGenInternal(NumInputs),
	scopeGUIref(scopeGUI),
	audioBuffer(BufferSpec((int)(UGen::getSampleRate()), input.getNumChannels(), true)),
	bufferIndex(0),
	audioBufferSizeUsed(0),
	samplesProcessed(0)
{	
	inputs[Input] = input;
	inputs[Duration] = duration;
	
	audioBufferSizeUsed = max(1, (int)(duration.getValue() * UGen::getSampleRate() + 0.5));
}

void ScopeUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	if(scopeGUIref != 0)
	{
		// only works if ScopeGUI is the first in the in list of base classes!
		ScopeGUI *scopeGUI = *( static_cast<ScopeGUI**> (scopeGUIref) ); 
				
		if(scopeGUI && scopeGUI->isValid())
		{
			float duration = *(inputs[Duration].processBlock(shouldDelete, blockID, 0));
			ugen_assert(duration > 0.f);
			
			int audioBufferSizeRequired = max(1, (int)(duration * UGen::getSampleRate() + 0.5));
			int audioBufferAllocatedSize = audioBuffer.size();			  
			
			if(audioBufferSizeRequired > audioBufferAllocatedSize)
			{
				if((audioBufferAllocatedSize > 1) && (bufferIndex >= audioBufferAllocatedSize))
				{
					scopeGUI->setAudioBuffer(audioBuffer.getRegion(0, audioBufferAllocatedSize-1), 
											 samplesProcessed);
					//samplesProcessed += audioBufferAllocatedSize;
				}
				
				audioBuffer = Buffer(audioBufferSizeRequired, inputs[Input].getNumChannels(), false);
				audioBufferSizeUsed = audioBufferSizeRequired;
				bufferIndex = 0;
			}
			else if(audioBufferSizeRequired < audioBufferAllocatedSize)
			{
				audioBufferSizeUsed = audioBufferSizeRequired;
				
				if(bufferIndex >= audioBufferSizeUsed)
				{
					scopeGUI->setAudioBuffer(audioBuffer.getRegion(0, audioBufferSizeUsed-1),
											 samplesProcessed);
					//samplesProcessed += audioBufferSizeUsed;
					bufferIndex = 0;
				}
			}
			
			int channelBufferIndex;
			
			for(int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
			{
				int numSamplesToProcess = uGenOutput.getBlockSize();
				float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
				float* bufferSamples = audioBuffer.getData(channel);
				channelBufferIndex = bufferIndex; // need a copy of this so it's the same at the start of processing each channel
				
				int bufferSamplesToProcess = audioBufferSizeUsed - channelBufferIndex;
				
				if(bufferSamplesToProcess > numSamplesToProcess)
				{				
					while(numSamplesToProcess--)
						bufferSamples[channelBufferIndex++] = *inputSamples++;
				}
				else
				{
					while(bufferSamplesToProcess--)
						bufferSamples[channelBufferIndex++] = *inputSamples++;
					
					channelBufferIndex = 0;
				}
			}
			
			bufferIndex = channelBufferIndex; // store for next block
			
			if(bufferIndex == 0)
			{
				scopeGUI->setAudioBuffer(audioBuffer.getRegion(0, audioBufferSizeUsed-1),
										 samplesProcessed);
				//samplesProcessed += audioBufferSizeUsed;
			}
		}
	}
	
	samplesProcessed += uGenOutput.getBlockSize(); // make sure we increment the sample counter for EVERY block! even if we skip "frames"
}

Scope::Scope(ScopeGUIPtrPtr scopeGUI, UGen const& input, UGen const& duration) throw()
{
	initInternal(1);
	internalUGens[0] = new ScopeUGenInternal(scopeGUI, input, duration.mix());
}

BufferSenderUGenInternal::BufferSenderUGenInternal(UGen const& input, UGen const& duration) throw()
:	UGenInternal(NumInputs),
	audioBuffer(BufferSpec((int)(UGen::getSampleRate()), input.getNumChannels(), true)),
	bufferIndex(0),
	audioBufferSizeUsed(0),
	samplesProcessed(0)
{	
	inputs[Input] = input;
	inputs[Duration] = duration;
	
	audioBufferSizeUsed = max(1, (int)(duration.getValue() * UGen::getSampleRate() + 0.5));
}

void BufferSenderUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	float duration = *(inputs[Duration].processBlock(shouldDelete, blockID, 0));
	ugen_assert(duration > 0.f);
	
	int audioBufferSizeRequired = max(1, (int)(duration * UGen::getSampleRate() + 0.5));
	int audioBufferAllocatedSize = audioBuffer.size();			  
	
	if(audioBufferSizeRequired > audioBufferAllocatedSize)
	{
		if((audioBufferAllocatedSize > 1) && (bufferIndex >= audioBufferAllocatedSize))
		{
			sendBuffer(audioBuffer.getRegion(0, audioBufferAllocatedSize-1), samplesProcessed);
		}
		
		audioBuffer = Buffer(audioBufferSizeRequired, inputs[Input].getNumChannels(), false);
		audioBufferSizeUsed = audioBufferSizeRequired;
		bufferIndex = 0;
	}
	else if(audioBufferSizeRequired < audioBufferAllocatedSize)
	{
		audioBufferSizeUsed = audioBufferSizeRequired;
		
		if(bufferIndex >= audioBufferSizeUsed)
		{
			sendBuffer(audioBuffer.getRegion(0, audioBufferSizeUsed-1), samplesProcessed);
			bufferIndex = 0;
		}
	}
	
	int numSamplesRemaining = uGenOutput.getBlockSize();
	int offset = 0;
	
	while(numSamplesRemaining > 0)
	{
		const int bufferSamplesToProcess = audioBufferSizeUsed - bufferIndex;
		const int numSamplesThisTime = min(bufferSamplesToProcess, numSamplesRemaining);
		
		for(int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
		{
			int numSamplesToProcess = numSamplesThisTime;
			const float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel) + offset;
			float* bufferSamples = audioBuffer.getData(channel) + bufferIndex;
					
			while(numSamplesToProcess--)
				*bufferSamples++ = *inputSamples++;
		}
		
		numSamplesRemaining -= numSamplesThisTime;
		bufferIndex += numSamplesThisTime;
		samplesProcessed += numSamplesThisTime;
		offset += numSamplesThisTime;
		
		if(bufferIndex >= audioBufferSizeUsed)
		{
			sendBuffer(audioBuffer.getRegion(0, audioBufferSizeUsed-1), samplesProcessed);
			bufferIndex = 0;
		}	
	}	
}

Sender::Sender(UGen const& input, UGen const& duration) throw()
{
	initInternal(1);
	internalUGens[0] = new BufferSenderUGenInternal(input, duration.mix());
}

SpectralScopeUGenInternal::SpectralScopeUGenInternal(ScopeGUIPtrPtr scopeGUI, 
													 UGen const& input, 
													 FFTEngine::FFTModes mode,
													 FFTEngine const& fft, 
													 const int overlap,
													 const int firstBin,
													 const int numBins) throw()
:	UGenInternal(NumInputs),	
	scopeGUIref(scopeGUI),
	mode_(mode),
	fftEngine(fft),
	fftSize(fftEngine.size()),
	fftSizeHalved(fftSize / 2),
	overlap_(overlap < 1 ? 1 : overlap),
	firstBin_(firstBin < fftSizeHalved ? firstBin : fftSizeHalved),
	maxNumBins(fftSizeHalved - firstBin_ + 1),
	numBins_(numBins ? (numBins < maxNumBins ? numBins : maxNumBins) : maxNumBins),
	inputBuffer(BufferSpec(fftSize, input.getNumChannels(), true)),
	outputBuffer(BufferSpec(fftSize, input.getNumChannels(), true)),
	bufferIndex(0)
{
	ugen_assert(overlap == overlap_);	// should be > 0
	ugen_assert(firstBin == firstBin_);	// should be in range
//	ugen_assert(numBins == numBins_);	// should be in range	
	
	inputs[Input] = input;	
}

void SpectralScopeUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	if(scopeGUIref == 0) return;
	
	// only works if ScopeGUI is the first in the in list of base classes!
	ScopeGUI *scopeGUI = *( static_cast<ScopeGUI**> (scopeGUIref) ); 
	
	if(scopeGUI && scopeGUI->isValid())
	{
		int channelBufferIndex;
		
		// keep filling the buffer until it has enough samples...
		for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
		{
			int numSamplesToProcess = uGenOutput.getBlockSize();
			float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float* bufferSamples = inputBuffer.getData(channel);
			channelBufferIndex = bufferIndex; // need a copy of this so it's the same at the start of processing each channel
			
			int bufferSamplesToProcess = fftSize - channelBufferIndex;
			
			if(bufferSamplesToProcess > numSamplesToProcess)
			{				
				while(numSamplesToProcess--)
					bufferSamples[channelBufferIndex++] = *inputSamples++;
			}
			else
			{
				while(bufferSamplesToProcess--)
					bufferSamples[channelBufferIndex++] = *inputSamples++;
				
				channelBufferIndex = 0;
			}
		}
		
		bufferIndex = channelBufferIndex; // store for next block
		
		// was the buffer filled ?
		if(bufferIndex == 0)
		{
			// do fft and send
			for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
			{				
				fftEngine.fft(outputBuffer, inputBuffer, true, channel, channel);
			}
						
			switch(mode_)
			{
				case FFTEngine::RealImagRaw:
					scopeGUI->setAudioBuffer(outputBuffer, 
											 0, 
											 fftSize);
					break;
				case FFTEngine::RealImagRawSplit:
					scopeGUI->setAudioBuffer(fftEngine.rawToRealImagRawSplit(outputBuffer),
											 0,
											 fftSize);
					break;
				case FFTEngine::RealImagUnpacked:
					scopeGUI->setAudioBuffer(fftEngine.rawToRealImagUnpacked(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;
				case FFTEngine::RealImagUnpackedSplit:
					scopeGUI->setAudioBuffer(fftEngine.rawToRealImagUnpackedSplit(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;
				case FFTEngine::MagnitudePhase:
					scopeGUI->setAudioBuffer(fftEngine.rawToMagnitudePhase(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;
				case FFTEngine::MagnitudePhaseSplit:
					scopeGUI->setAudioBuffer(fftEngine.rawToMagnitudePhaseSplit(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;					
				case FFTEngine::Magnitude: 
					scopeGUI->setAudioBuffer(fftEngine.rawToMagnitude(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;
				case FFTEngine::Phase: 
					scopeGUI->setAudioBuffer(fftEngine.rawToPhase(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
					break;
				default:
					scopeGUI->setAudioBuffer(fftEngine.rawToMagnitude(outputBuffer, firstBin_, numBins_),
											 firstBin_,
											 fftSize);
			}
			
			// keep overlapping samples for next FFT
			if((overlap_ > 1) && (uGenOutput.getBlockSize() < fftSize))
			{
				const int hopSize = fftSize / overlap_;
				const int copySize = fftSize - hopSize;
				
				for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
				{
					float *bufferSamples = inputBuffer.getData(channel);
					memcpy(bufferSamples, bufferSamples + hopSize, copySize * sizeof(float));
				}
				
				bufferIndex = copySize;
			}
		}
	}
}

SpectralScope::SpectralScope(ScopeGUIPtrPtr scopeGUI, 
							 UGen const& input, 
							 FFTEngine::FFTModes mode,
							 FFTEngine const& fft, 
							 const int overlap,
							 const int firstBin,
							 const int numBins) throw()
{
	initInternal(1);
	internalUGens[0] = new SpectralScopeUGenInternal(scopeGUI, input, mode, fft, overlap, firstBin, numBins);
}

FFTSenderUGenInternal::FFTSenderUGenInternal(UGen const& input, 
											 FFTEngine::FFTModes mode,
											 FFTEngine const& fft, 
											 const int overlap,
											 const int firstBin,
											 const int numBins) throw()
:	UGenInternal(NumInputs),	
	mode_(mode),
	fftEngine(fft),
	fftSize(fftEngine.size()),
	fftSizeHalved(fftSize / 2),
	overlap_(overlap < 1 ? 1 : overlap),
	firstBin_(firstBin < fftSizeHalved ? firstBin : fftSizeHalved),
	maxNumBins(fftSizeHalved - firstBin_ + 1),
	numBins_(numBins ? (numBins < maxNumBins ? numBins : maxNumBins) : maxNumBins),
	inputBuffer(BufferSpec(fftSize, input.getNumChannels(), true)),
	outputBuffer(BufferSpec(fftSize, input.getNumChannels(), true)),
	bufferIndex(0)
{
	ugen_assert(overlap == overlap_);	// should be > 0
	ugen_assert(firstBin == firstBin_);	// should be in range
//	ugen_assert(numBins == numBins_);	// should be in range	
	
	inputs[Input] = input;	
}

void FFTSenderUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	int channelBufferIndex;

	// keep filling the buffer until it has enough samples...
	for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
	{
		int numSamplesToProcess = uGenOutput.getBlockSize();
		float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
		float* bufferSamples = inputBuffer.getData(channel);
		channelBufferIndex = bufferIndex; // need a copy of this so it's the same at the start of processing each channel
		
		int bufferSamplesToProcess = fftSize - channelBufferIndex;
		
		if(bufferSamplesToProcess > numSamplesToProcess)
		{				
			while(numSamplesToProcess--)
				bufferSamples[channelBufferIndex++] = *inputSamples++;
		}
		else
		{
			while(bufferSamplesToProcess--)
				bufferSamples[channelBufferIndex++] = *inputSamples++;
			
			channelBufferIndex = 0;
		}
	}

	bufferIndex = channelBufferIndex; // store for next block

	// was the buffer filled ?
	if(bufferIndex == 0)
	{
		// do fft and send
		for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
		{				
			fftEngine.fft(outputBuffer, inputBuffer, true, channel, channel);
		}
		
		switch(mode_)
		{
			case FFTEngine::RealImagRaw:
				sendBuffer(outputBuffer, 0, fftSize);
				break;
			case FFTEngine::RealImagRawSplit:
				sendBuffer(fftEngine.rawToRealImagRawSplit(outputBuffer),
						   0, fftSize);
				break;
			case FFTEngine::RealImagUnpacked:
				sendBuffer(fftEngine.rawToRealImagUnpacked(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;
			case FFTEngine::RealImagUnpackedSplit:
				sendBuffer(fftEngine.rawToRealImagUnpackedSplit(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;
			case FFTEngine::MagnitudePhase:
				sendBuffer(fftEngine.rawToMagnitudePhase(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;
			case FFTEngine::MagnitudePhaseSplit:
				sendBuffer(fftEngine.rawToMagnitudePhaseSplit(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;					
			case FFTEngine::Magnitude: 
				sendBuffer(fftEngine.rawToMagnitude(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;
			case FFTEngine::Phase: 
				sendBuffer(fftEngine.rawToPhase(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
				break;
			default:
				sendBuffer(fftEngine.rawToMagnitude(outputBuffer, firstBin_, numBins_),
						   firstBin_, fftSize);
		}
		
		// keep overlapping samples for next FFT
		if((overlap_ > 1) && (uGenOutput.getBlockSize() < fftSize))
		{
			const int hopSize = fftSize / overlap_;
			const int copySize = fftSize - hopSize;
			
			for(int channel = 0; channel < inputBuffer.getNumChannels(); channel++)
			{
				float *bufferSamples = inputBuffer.getData(channel);
				memcpy(bufferSamples, bufferSamples + hopSize, copySize * sizeof(float));
			}
			
			bufferIndex = copySize;
		}
	}
}

FFTSender::FFTSender(UGen const& input, 
					 FFTEngine::FFTModes mode,
					 FFTEngine const& fft, 
					 const int overlap,
					 const int firstBin,
					 const int numBins) throw()
{
	initInternal(1);
	internalUGens[0] = new FFTSenderUGenInternal(input, mode, fft, overlap, firstBin, numBins);
}


END_UGEN_NAMESPACE
