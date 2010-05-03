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

#include "ugen_FFTEngineInternal.h"

FFTEngineInternal::FFTEngineInternal(const int fftSizeToUse) throw()
:	fftSize(Bits::isPowerOf2(fftSizeToUse) ? fftSizeToUse : Bits::nextPowerOf2(fftSizeToUse)),
	fftSizeHalved(fftSize>>1),
	fftSizeBytes(fftSize * sizeof(float)),
	fftSizeHalvedBytes(fftSizeHalved * sizeof(float)),
	ifftScaling(1.f/(float)fftSize),
#ifdef UGEN_FFTW
	transformBuffer(BufferSpec(fftSize + 2, 1, false)),
#else
	transformBuffer(BufferSpec(fftSize, 1, false)),
#endif
	transformBufferSamples(transformBuffer.getData()),
	fftWindow(Buffer::hannWindow(fftSize)),
	ifftWindow(fftWindow),
	fftWindowSamples(fftWindow.getData()),
	ifftWindowSamples(ifftWindow.getData()),
	fftWindowFactor(fftWindow.sum(0) / fftSize),
	windowingBuffer(BufferSpec(fftSize, 1, false)),
	windowingBufferSamples(windowingBuffer.getData())
{
	static int announced = 0;
	
	ugen_assert(fftSizeToUse == fftSize); // fftSizeToUse should be a power of 2
	
	transformBufferSplit.realp = transformBufferSamples;
	transformBufferSplit.imagp = transformBufferSamples + fftSizeHalved;
	
#ifdef UGEN_FFTW
	if(!announced) printf("FFTEngine using FFTW\n"); announced = 1;
	fftwPlan = fftwf_plan_dft_r2c_1d(fftSize, transformBufferSamples, (fftwf_complex*) transformBufferSamples, FFTW_ESTIMATE);
	ifftwPlan = fftwf_plan_dft_c2r_1d(fftSize, (fftwf_complex*) transformBufferSamples, transformBufferSamples, FFTW_ESTIMATE);
#elif defined(UGEN_FFTREAL)
	if(!announced) printf("FFTEngine using FFTReal\n"); announced = 1;
	fftReal = new FFTReal<float>(fftSize);
#else
	if(!announced) printf("FFTEngine using vDSP\n"); announced = 1;
	fftSizeLog2 = 4;
	while((1 << fftSizeLog2) < fftSize)
		fftSizeLog2++;
	fftvDSP = create_fftsetup (fftSizeLog2, 0);
#endif
}

FFTEngineInternal::~FFTEngineInternal()
{
#ifdef UGEN_FFTW
	fftwf_destroy_plan(fftwPlan);
	fftwf_destroy_plan(ifftwPlan);
#elif defined(UGEN_FFTREAL)
	delete fftReal;
#else
	destroy_fftsetup(fftvDSP);
#endif		
}

void FFTEngineInternal::dispose()
{
	//delete this; // just leak until I find why it crashes in the desctuctor occasionally!
}

void FFTEngineInternal::fft(Buffer const& outputBuffer, 
							Buffer const& inputBuffer, 
							const bool applyWindow,
							const int outputChannel,
							const int inputChannel) throw()
{
	if((outputBuffer.size() < fftSize) || (inputBuffer.size() < fftSize))
	{
		printf("FFTEngineInternal::fft() - buffer(s) too small\n");
		return;
	}
	
	DSPSplitComplex fftBuffer;
	fftBuffer.realp = Buffer(outputBuffer).getData(outputChannel);
	fftBuffer.imagp = fftBuffer.realp + fftSizeHalved;
	
	//fft(fftBuffer, inputBuffer.getDataReadOnly(inputChannel), applyWindow);
	fft(fftBuffer, inputBuffer.getData(inputChannel), applyWindow);
}
 
void FFTEngineInternal::fft(DSPSplitComplex& outputBuffer, const float* const inputBuffer, const bool applyWindow) throw()
{		
	if(applyWindow)
	{
		int numSamples = fftSize;
		const float *inputSamples = inputBuffer;
		float *tempSamples = windowingBufferSamples;
		const float *windowSamples = this->fftWindowSamples;
		while(numSamples--)
		{
			*tempSamples++ = *inputSamples++ * *windowSamples++;
		}
		fft(outputBuffer, windowingBufferSamples);
	}
	else
		fft(outputBuffer, inputBuffer);
	
}

void FFTEngineInternal::ifft(Buffer const& outputBuffer, 
							 Buffer const& inputBuffer, 
							 const bool applyWindow, 
							 const bool applyScaling,
							 const int outputChannel,
							 const int inputChannel) throw()
{
	if((outputBuffer.size() < fftSize) || (inputBuffer.size() < fftSize))
	{
		ugen_assertfalse; // buffer(s) too small
		return;
	}
	
	DSPSplitComplex fftBuffer;
	//fftBuffer.realp = (float*)inputBuffer.getDataReadOnly(inputChannel);
	fftBuffer.realp = (float*)inputBuffer.getData(inputChannel);
	fftBuffer.imagp = fftBuffer.realp + fftSizeHalved;
	
	ifft(Buffer(outputBuffer).getData(outputChannel), fftBuffer, applyWindow, applyScaling);
}	

void FFTEngineInternal::ifft(float* const outputBuffer, DSPSplitComplex const& inputBuffer, const bool applyWindow, const bool applyScaling) throw()
{
	ifft(outputBuffer, inputBuffer);
	
	if(applyWindow && applyScaling)
	{
		int numSamples = fftSize;
		float *outputSamples = outputBuffer;
		float *windowSamples = this->ifftWindowSamples;
		while(numSamples--)
		{
			*outputSamples++ *= *windowSamples++ * ifftScaling;
		}			
	}
	else if(applyWindow)
	{
		int numSamples = fftSize;
		float *outputSamples = outputBuffer;
		float *windowSamples = this->ifftWindowSamples;
		while(numSamples--)
		{
			*outputSamples++ *= *windowSamples++;
		}
	}
	else if(applyScaling)
	{
		int numSamples = fftSize;
		float *outputSamples = outputBuffer;
		while(numSamples--)
		{
			*outputSamples++ *= ifftScaling;
		}	
	}
}

Buffer FFTEngineInternal::rawToRealImagRawSplit(Buffer const& raw) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}

	Buffer splitBuffer;
	for(int i = 0; i < raw.getNumChannels(); i++)
	{
		splitBuffer <<= raw.getRegion(0, fftSizeHalved-1, i, i);
		splitBuffer <<= raw.getRegion(fftSizeHalved, fftSize-1, i, i);
	}
	
	return splitBuffer;
}

Buffer FFTEngineInternal::rawToRealImagUnpacked(Buffer const& raw) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	Buffer zeros(BufferSpec(1, raw.getNumChannels(), true));
	return (raw.getRegion(0, fftSizeHalved),zeros, raw.getRegion(fftSizeHalved+1, fftSize-1), zeros);
}

Buffer FFTEngineInternal::rawToRealImagUnpacked(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	Buffer unpacked = rawToRealImagUnpacked(raw);
	
	if(firstBin != 0 || (firstBin + numBins) < fftSizeHalved)
	{
		Buffer temp = unpacked;
		unpacked = temp.getRegion(firstBin, firstBin + numBins - 1);		
		unpacked = (unpacked, temp.getRegion(firstBin + fftSizeHalved + 1, firstBin + fftSizeHalved + numBins));
	}
	
	return unpacked;
}

Buffer FFTEngineInternal::rawToRealImagUnpackedSplit(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	ugen_assert(firstBin >= 0);
	ugen_assert(numBins > 0);
	
	Buffer splitBuffer;
	
	for(int i = 0; i < raw.getNumChannels(); i++)
	{
		splitBuffer <<= raw.getRegion(0, fftSizeHalved, i, i);
		splitBuffer <<= (B(0.f), raw.getRegion(fftSizeHalved+1, fftSize-1, i, i), B(0.f));
	}
	
	return splitBuffer.getRegion(firstBin, firstBin + numBins - 1);
}

Buffer FFTEngineInternal::rawToMagnitudePhase(Buffer const& raw, 
											  const int firstBin, 
											  const int numBins, 
											  const bool magnitudeAssumesWindowedFFT) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	ugen_assert(firstBin >= 0);
	ugen_assert(numBins > 0);
	
	const int firstRealBin = firstBin;
	const int firstImagBin = firstBin + fftSizeHalved + 1;
	
	Buffer unpacked = rawToRealImagUnpacked(raw);
	
	Buffer real(unpacked.getRegion(firstRealBin, firstRealBin + numBins - 1));
	Buffer imag(unpacked.getRegion(firstImagBin, firstImagBin + numBins - 1));
	
	Buffer magnitude(hypot(imag, real));
	
	if(magnitudeAssumesWindowedFFT)
		magnitude = magnitude * (1.0 / (fftSizeHalved * fftWindowFactor));
	else
		magnitude = magnitude * (1.0 / fftSizeHalved);
	
	Buffer phase(atan2(imag, real));
	
	return (magnitude, phase);
}

Buffer FFTEngineInternal::rawToMagnitudePhaseSplit(Buffer const& raw, 
												   const int firstBin, 
												   const int numBins, 
												   const bool magnitudeAssumesWindowedFFT) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	ugen_assert(firstBin >= 0);
	ugen_assert(numBins > 0);
	
	const int firstRealBin = firstBin;
	const int firstImagBin = firstBin + fftSizeHalved + 1;
	
	Buffer unpacked = rawToRealImagUnpacked(raw);
	
	Buffer real(unpacked.getRegion(firstRealBin, firstRealBin + numBins - 1));
	Buffer imag(unpacked.getRegion(firstImagBin, firstImagBin + numBins - 1));
	
	Buffer magnitude(hypot(imag, real));
	
	if(magnitudeAssumesWindowedFFT)
		magnitude = magnitude * (1.0 / (fftSizeHalved * fftWindowFactor));
	else
		magnitude = magnitude * (1.0 / fftSizeHalved);
	
	Buffer phase(atan2(imag, real));
	Buffer splitBuffer;
	
	for(int i = 0; i < raw.getNumChannels(); i++)
	{
		splitBuffer <<= magnitude.getChannel(i);
		splitBuffer <<= phase.getChannel(i);
	}
	
	return splitBuffer;
}

Buffer FFTEngineInternal::rawToMagnitude(Buffer const& raw, 
										 const int firstBin, 
										 const int numBins,
										 const bool magnitudeAssumesWindowedFFT) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	ugen_assert(firstBin >= 0);
	ugen_assert(numBins > 0);
	
	const int firstRealBin = firstBin;
	const int firstImagBin = firstBin + fftSizeHalved + 1;
	
	Buffer unpacked = rawToRealImagUnpacked(raw);
	
	Buffer real(unpacked.getRegion(firstRealBin, firstRealBin + numBins - 1));
	Buffer imag(unpacked.getRegion(firstImagBin, firstImagBin + numBins - 1));
	
	if(magnitudeAssumesWindowedFFT)
		return hypot(imag, real) * (1.0 / (fftSizeHalved * fftWindowFactor));
	else
		return hypot(imag, real) * (1.0 / fftSizeHalved);
}

Buffer FFTEngineInternal::rawToPhase(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	if(raw.size() != fftSize) 
	{ 
		ugen_assertfalse; // raw data wrong size
		return Buffer();
	}
	
	ugen_assert(firstBin >= 0);
	ugen_assert(numBins > 0);
	
	const int firstRealBin = firstBin;
	const int firstImagBin = firstBin + fftSizeHalved + 1;
	
	Buffer unpacked = rawToRealImagUnpacked(raw);
	
	Buffer real(unpacked.getRegion(firstRealBin, firstRealBin + numBins - 1));
	Buffer imag(unpacked.getRegion(firstImagBin, firstImagBin + numBins - 1));
	
	return atan2(imag, real);
}






END_UGEN_NAMESPACE
