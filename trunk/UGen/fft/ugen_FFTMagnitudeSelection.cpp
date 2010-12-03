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

#if !defined(WIN32) && !defined(UGEN_IPHONE) && !defined(UGEN_ANDROID)
	#include <Accelerate/Accelerate.h>
	#include <CoreServices/CoreServices.h>
#endif
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_FFTMagnitudeSelection.h"
#include "ugen_FFTEngine.h"
#include "../core/ugen_Bits.h"

FFTMagnitudeSelectionUGenInternal::FFTMagnitudeSelectionUGenInternal(UGen const& input, 
																	 FFTEngine const& fft, 
																	 const int overlap, 
																	 IntArray const& _bins) throw()
:	ProxyOwnerUGenInternal(NumInputs, _bins.length()-1),
	fftEngine(fft),
	fftSize(fftEngine.size()),
	fftSizeHalved(fftSize / 2),	
	overlap_(overlap < 1 ? 1 : overlap),
	hopSize(fftSize / overlap),
	overlapSize(fftSize - hopSize),
	maxNumBins(fftSizeHalved + 1),
	bins(_bins),
	inputBuffer(BufferSpec(fftSize, 1, true)),
	outputBuffer(BufferSpec(fftSize, 1, true)),
	bufferIndex(fftSize - hopSize),
	magnitudes(BufferSpec(bins.length(), 1, true)),
	outputSampleData(new float*[bins.length()])
{
	ugen_assert(overlap == overlap_);	// should be > 0
	ugen_assert(bins.length() > 0);
	
	inputs[Input] = input;
}

FFTMagnitudeSelectionUGenInternal::~FFTMagnitudeSelectionUGenInternal()
{
	delete [] outputSampleData;
}

inline static void splat(float *buffer, float value, int n)
{
	ugen_assert(buffer != 0);
	ugen_assert(n > 0);

	while(n--)
		*buffer++ = value;
}

void FFTMagnitudeSelectionUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	int numSamplesToProcess = blockSize;
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);
	float* bufferSamples = inputBuffer.getData();
	
	// get output pointers
	for(int channel = 0; channel < bins.length(); channel++)
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
			
			magnitudes = fftEngine.rawToMagnitude(outputBuffer, 0, maxNumBins);
			FloatArray floatMag = magnitudes.toArray<float>(0).at(bins);
			magnitudes = Buffer(floatMag);
			
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
		
		for(int channel = 0; channel < bins.length(); channel++)
		{
			float magnitude = magnitudes.getSampleUnchecked(channel);
			
			splat(outputSampleData[channel], magnitude, outputSamplesToProcess);
			
			outputSampleData[channel] += outputSamplesToProcess;
		}	
		
	}
	
}

FFTMagnitudeSelection::FFTMagnitudeSelection(UGen const& input, 
											 FFTEngine const& fft, 
											 const int overlap, 
											 IntArray const& bins) throw()
{
	int overlapChecked = Bits::isPowerOf2(overlap) ? overlap : Bits::nextPowerOf2(overlap);
	
	ugen_assert(overlap != overlapChecked); // should be power of 2

	FFTMagnitudeSelectionUGenInternal *fftMag = new FFTMagnitudeSelectionUGenInternal(input.mix(), 
																					  fft, 
																					  overlapChecked, 
																					  bins);
		
	initInternal(fftMag->getNumBins());
	generateFromProxyOwner(fftMag);
}

END_UGEN_NAMESPACE
