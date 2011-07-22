// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#if !defined(WIN32) && !defined(UGEN_IPHONE) && !defined(UGEN_ANDROID)
	#include <Accelerate/Accelerate.h>
	#include <CoreServices/CoreServices.h>
#endif
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_FFTMagnitude.h"
#include "ugen_FFTEngine.h"
#include "../core/ugen_Bits.h"

FFTMagnitudeUGenInternal::FFTMagnitudeUGenInternal(UGen const& input, 
												   FFTEngine const& fft, 
												   const int overlap, 
												   const int firstBin, 
												   const int numBins) throw()
:	ProxyOwnerUGenInternal(NumInputs, numBins-1),
	fftEngine(fft),
	fftSize(fftEngine.size()),
	fftSizeHalved(fftSize / 2),	
	overlap_(overlap < 1 ? 1 : overlap),
	hopSize(fftSize / overlap),
	overlapSize(fftSize - hopSize),
	firstBin_(firstBin < fftSizeHalved ? firstBin : fftSizeHalved),
	maxNumBins(fftSizeHalved - firstBin_ + 1),
	numBins_(numBins ? (numBins < maxNumBins ? numBins : maxNumBins) : maxNumBins),
	inputBuffer(BufferSpec(fftSize, 1, true)),
	outputBuffer(BufferSpec(fftSize, 1, true)),
	bufferIndex(fftSize - hopSize),
	magnitudes(BufferSpec(numBins_, 1, true)),
	outputSampleData(new float*[numBins_])
{
	ugen_assert(overlap == overlap_);	// should be > 0
	ugen_assert(firstBin == firstBin_);	// should be in range
	
	inputs[Input] = input;
}

FFTMagnitudeUGenInternal::~FFTMagnitudeUGenInternal()
{
	delete [] outputSampleData;
}

inline static void splat(float *buffer, float value, int n)
{
	ugen_assert(buffer == 0);
	ugen_assert(n > 0);
	
	while(n--)
		*buffer++ = value;
}



void FFTMagnitudeUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	int numSamplesToProcess = blockSize;
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);
	float* bufferSamples = inputBuffer.getData();
	
	// get output pointers
	for(int channel = 0; channel < numBins_; channel++)
	{
		outputSampleData[channel] = proxies[channel]->getSampleData();
	}	
	
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesToProcess = fftSize - bufferIndex;
		
		if(bufferSamplesToProcess > numSamplesToProcess)
		{							
			memcpy(bufferSamples + bufferIndex, inputSamples, numSamplesToProcess * sizeof(float));
			
			bufferIndex += numSamplesToProcess;
			inputSamples += numSamplesToProcess;
			numSamplesToProcess = 0;
		}
		else
		{
			numSamplesToProcess -= bufferSamplesToProcess;
									
			memcpy(bufferSamples + bufferIndex, inputSamples, bufferSamplesToProcess * sizeof(float));
			
			bufferIndex += bufferSamplesToProcess;
			inputSamples += bufferSamplesToProcess;
			bufferSamplesToProcess = 0;
			
			fftEngine.fft(outputBuffer, inputBuffer, true);
			
			magnitudes = fftEngine.rawToMagnitude(outputBuffer, firstBin_, numBins_);
			
			if(overlap_ > 1)
			{
				memcpy(inputBuffer.getData(), inputBuffer.getData() + hopSize, overlapSize * sizeof(float));
				bufferIndex = fftSize - hopSize;
			}
			else
			{
				bufferIndex = 0;
			}
		}
		
		const int outputSamplesToProcess = hopSize < blockSize ? hopSize : blockSize;
		
		for(int channel = 0; channel < numBins_; channel++)
		{
			float magnitude = magnitudes.getSampleUnchecked(channel);
			
			splat(outputSampleData[channel], magnitude, outputSamplesToProcess);
			
			outputSampleData[channel] += outputSamplesToProcess;
		}	
		
	}
	
}

FFTMagnitude::FFTMagnitude(UGen const& input, 
						   FFTEngine const& fft, 
						   const int overlap, 
						   const int firstBin, 
						   const int numBins) throw()
{
	int overlapChecked = Bits::isPowerOf2(overlap) ? overlap : Bits::nextPowerOf2(overlap);
	
	ugen_assert(overlap != overlapChecked); // should be power of 2
	
	FFTMagnitudeUGenInternal *fftMag = new FFTMagnitudeUGenInternal(input.mix(), 
																	fft, 
																	overlapChecked, 
																	firstBin, 
																	numBins);
		
	initInternal(fftMag->getNumBins());
	generateFromProxyOwner(fftMag);
}

END_UGEN_NAMESPACE
