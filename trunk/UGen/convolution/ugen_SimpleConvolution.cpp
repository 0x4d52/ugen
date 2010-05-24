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

#if !defined(WIN32) && !defined(UGEN_IPHONE)
	#include <Accelerate/Accelerate.h>
	#include <CoreServices/CoreServices.h>
#endif
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_SimpleConvolution.h"
#include "../basics/ugen_BinaryOpUGens.h"
#include "../core/ugen_UGenArray.h"


SimpleConvolutionUGenInternal::SimpleConvolutionUGenInternal(UGen const& inputA, 
															 UGen const& inputB, 
															 const int fftSize,
															 const int initialDelay) throw()
:	UGenInternal(NumInputs),
	fftSize_(fftSize),
	fftSizeHalved(fftSize / 2),
	initialDelay_(initialDelay),
	fftEngine(new FFTEngineInternal(fftSize)),
	buffers(BufferSpec(fftSize_, NumBuffers, true)),
	bufferIndex(initialDelay <= 0 ? 0 : initialDelay >= fftSize_ ? 0 : fftSize_ - initialDelay)
{
	inputs[InputA] = inputA;
	inputs[InputB] = inputB;
}

SimpleConvolutionUGenInternal::~SimpleConvolutionUGenInternal()
{
	delete fftEngine;
}

UGenInternal* SimpleConvolutionUGenInternal::getChannel(const int channel) throw()
{
	return new SimpleConvolutionUGenInternal(inputs[InputA].getChannel(channel), 
											 inputs[InputB].getChannel(channel), 
											 fftSize_, 
											 initialDelay_);
}

void SimpleConvolutionUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	int numSamplesToProcess = blockSize;
	
	float* inputASamples = inputs[InputA].processBlock(shouldDelete, blockID, channel);
	float* inputBSamples = inputs[InputB].processBlock(shouldDelete, blockID, channel);
	float* outputSamples = uGenOutput.getSampleData();
	
	float* const bufferASamples = buffers.getDataUnchecked(InputA);
	float* const bufferBSamples = buffers.getDataUnchecked(InputB);
	float* const outputBufferSamples = buffers.getDataUnchecked(Output);
		
	DSPSplitComplex fftABuffer;
	fftABuffer.realp = buffers.getDataUnchecked(ProcessA);
	fftABuffer.imagp = fftABuffer.realp + fftSizeHalved;
	
	DSPSplitComplex fftBBuffer;
	fftBBuffer.realp = buffers.getDataUnchecked(ProcessB);
	fftBBuffer.imagp = fftBBuffer.realp + fftSizeHalved;
	
	DSPSplitComplex fftResultBufferSplit;
	fftResultBufferSplit.realp = buffers.getDataUnchecked(FFTResult);
	fftResultBufferSplit.imagp = fftResultBufferSplit.realp + fftSizeHalved;
	
	while(numSamplesToProcess > 0)
	{
		int bufferSamplesToProcess = fftSize_ - bufferIndex;
		
		if(bufferSamplesToProcess > numSamplesToProcess)
		{			
			// buffer the inputs
			memcpy(bufferASamples + bufferIndex, inputASamples, numSamplesToProcess * sizeof(float));
			memcpy(bufferBSamples + bufferIndex, inputBSamples, numSamplesToProcess * sizeof(float));
			
			// ...and the output
			memcpy(outputSamples, outputBufferSamples + bufferIndex, numSamplesToProcess * sizeof(float));
												
			bufferIndex += numSamplesToProcess;
			inputASamples += numSamplesToProcess;
			inputBSamples += numSamplesToProcess;
			outputSamples += numSamplesToProcess;
			numSamplesToProcess = 0;
		}
		else
		{
			numSamplesToProcess -= bufferSamplesToProcess;
			
			memcpy(bufferASamples + bufferIndex, inputASamples, bufferSamplesToProcess * sizeof(float));
			memcpy(bufferBSamples + bufferIndex, inputBSamples, bufferSamplesToProcess * sizeof(float));
			
			memcpy(outputSamples, outputBufferSamples + bufferIndex, bufferSamplesToProcess * sizeof(float));
			
			bufferIndex += bufferSamplesToProcess;
			inputASamples += bufferSamplesToProcess;
			inputBSamples += bufferSamplesToProcess;
			outputSamples += bufferSamplesToProcess;
			bufferSamplesToProcess = 0;
			
			// do fft			
			fftEngine->fft(fftABuffer, buffers.getDataUnchecked(InputA), true);
			fftEngine->fft(fftBBuffer, buffers.getDataUnchecked(InputB), true);
			
			// convolve
			memset(fftResultBufferSplit.realp, 0, fftSize_ * sizeof(float));
			MultAndAdd(fftABuffer, fftBBuffer, fftResultBufferSplit, fftSizeHalved / 4);
						
			// do ifft
			fftEngine->ifft(outputBufferSamples, fftResultBufferSplit, true, true);
												
			bufferIndex = 0;
		}
	}
	
}

SimpleConvolution::SimpleConvolution(UGen const& inputA, 
									 UGen const& inputB, 
									 const int fftSize, 
									 const int initialDelay) throw()
{	
	const int fftSizeChecked = Bits::isPowerOf2(fftSize) ? fftSize : Bits::nextPowerOf2(fftSize);

	const int numInputChannels = ugen::max(inputA.getNumChannels(), inputB.getNumChannels());
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new SimpleConvolutionUGenInternal(inputA, 
															 inputB, 
															 fftSizeChecked, 
															 initialDelay);
	}
}

OverlapConvolution::OverlapConvolution(UGen const& inputA, UGen const& inputB, const int fftSize, const int overlap) throw()
{	
	const int fftSizeChecked = Bits::isPowerOf2(fftSize) ? fftSize : Bits::nextPowerOf2(fftSize);
	const int overlapChecked = Bits::isPowerOf2(overlap) ? overlap : Bits::nextPowerOf2(overlap);
	
	UGenArray overlappingConvolutions;
	const int delay = fftSize / overlapChecked;
			
	for(int i = 0; i < overlap; i++)
	{
		overlappingConvolutions <<= SimpleConvolution::AR(inputA, inputB, fftSizeChecked, delay * i);
	}
		
	UGen mixer = overlappingConvolutions.mix();
	
	const int numInputChannels = ugen::max(inputA.getNumChannels(), inputB.getNumChannels());
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new BinaryMultiplyUGenInternal(mixer, 1.0 / overlapChecked);
	}
}


END_UGEN_NAMESPACE
