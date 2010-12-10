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

#include "ugen_FFTEngine.h"
#include "ugen_FFTEngineInternal.h"
#include "../core/ugen_Arrays.h"

FFTEngine::FFTEngine(const int fftSize) throw()
{
	if(fftSize <= 0)
	{
		// deault to 4096
		internal = new FFTEngineInternal(4096);
	}
	else if(fftSize < 16)
	{
		// less than 16 the fftSize is specified as a power of 2
		internal = new FFTEngineInternal(1 << fftSize);
	}
	else
	{
		// use fftSize directly, the internal rounds it up to a power of 2 if necessary
		internal = new FFTEngineInternal(fftSize);
	}
}

FFTEngine::FFTEngine(FFTEngine const& copy) throw()
{
	internal = copy.internal;
	internal->incrementRefCount();
}

FFTEngine& FFTEngine::operator= (FFTEngine const& other) throw()
{
	if (this != &other)
    {		
		internal->decrementRefCount();
		internal = other.internal;
		internal->incrementRefCount();
    }
	
    return *this;
}

int FFTEngine::size() const throw()							{ return internal->size();			}
Buffer& FFTEngine::getFFTWindow() throw()					{ return internal->getFFTWindow();	}
Buffer& FFTEngine::getIFFTWindow() throw()					{ return internal->getIFFTWindow(); }
void FFTEngine::setFFTWindow(Buffer const& window) throw()	{ internal->setFFTWindow(window);	}
void FFTEngine::setIFFTWindow(Buffer const& window) throw() { internal->setIFFTWindow(window);	}

void FFTEngine::fft(Buffer const& outputBuffer, 
					Buffer const& inputBuffer, 
					const bool applyWindow,
					const int outputChannel,
					const int inputChannel) throw()
{
	internal->fft(outputBuffer, inputBuffer, applyWindow, outputChannel, inputChannel);
}

void FFTEngine::fft(Buffer const& outputBuffer, 
					Buffer const& inputBuffer, 
					const bool applyWindow,
					const int outputChannel,
					const int inputChannel,
					const int outputOffset,
					const int inputOffset) throw()
{
	internal->fft(outputBuffer, inputBuffer, applyWindow, outputChannel, inputChannel, outputOffset, inputOffset);
}

void FFTEngine::ifft(Buffer const& outputBuffer, 
					 Buffer const& inputBuffer, 
					 const bool applyWindow, 
					 const bool applyScaling,
					 const int outputChannel,
					 const int inputChannel) throw()
{
	internal->ifft(outputBuffer, inputBuffer, applyWindow, applyScaling, outputChannel, inputChannel);
}

void FFTEngine::ifft(Buffer const& outputBuffer, 
					 Buffer const& inputBuffer, 
					 const bool applyWindow, 
					 const bool applyScaling,
					 const int outputChannel,
					 const int inputChannel,
					 const int outputOffset,
					 const int inputOffset) throw()
{
	internal->ifft(outputBuffer, inputBuffer, applyWindow, applyScaling, outputChannel, inputChannel, outputOffset, inputOffset);
}

Buffer FFTEngine::rawToRealImagRawSplit(Buffer const& raw) throw()
{
	return internal->rawToRealImagRawSplit(raw);
}

Buffer FFTEngine::rawToRealImagUnpacked(Buffer const& raw) throw()
{
	return internal->rawToRealImagUnpacked(raw);
}

Buffer FFTEngine::rawToRealImagUnpacked(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	return internal->rawToRealImagUnpacked(raw, firstBin, numBins);
}

Buffer FFTEngine::rawToRealImagUnpackedSplit(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	return internal->rawToRealImagUnpackedSplit(raw, firstBin, numBins);
}

Buffer FFTEngine::rawToMagnitudePhase(Buffer const& raw, 
								  const int firstBin, 
								  const int numBins, 
								  const bool magnitudeAssumesWindowedFFT) throw()
{
	return internal->rawToMagnitudePhase(raw, firstBin, numBins, magnitudeAssumesWindowedFFT);
}

Buffer FFTEngine::rawToMagnitudePhaseSplit(Buffer const& raw, 
									   const int firstBin, 
									   const int numBins, 
									   const bool magnitudeAssumesWindowedFFT) throw()
{
	return internal->rawToMagnitudePhaseSplit(raw, firstBin, numBins, magnitudeAssumesWindowedFFT);
}

Buffer FFTEngine::rawToMagnitude(Buffer const& raw, 
							 const int firstBin, 
							 const int numBins,
							 const bool magnitudeAssumesWindowedFFT) throw()
{
	return internal->rawToMagnitude(raw, firstBin, numBins, magnitudeAssumesWindowedFFT);
}

Buffer FFTEngine::rawToPhase(Buffer const& raw, const int firstBin, const int numBins) throw()
{
	return internal->rawToPhase(raw, firstBin, numBins);
}

Buffer FFTEngine::generatePhaseShiftResponse(FloatArray const& phases) throw()
{
	if(phases.length() < 1) return Buffer();
	
	const int size = internal->size();
	const int halfSize = size >> 1;

	Buffer spectrum = Buffer::withSize(size, phases.length());
	Buffer response = Buffer::withSize(size, phases.length());

	for(int channel = 0; channel < phases.length(); channel++)
	{
		float phase = phases[channel];
		float real = cos(phase);
		float imag = sin(phase);
		
		spectrum.setSampleUnchecked(channel, 0, 0.f); // real DC
				
		// real
		for(int i = 1; i < halfSize; i++)
		{
			spectrum.setSampleUnchecked(channel, i, real);
		}
		
		spectrum.setSampleUnchecked(channel, halfSize, 0.f); // real Nyquist
		
		// imag
		for(int i = halfSize+1; i < size; i++)
		{
			spectrum.setSampleUnchecked(channel, i, imag);
		}
			
		// calculate
		ifft(response, spectrum, false, true, channel, channel);
	}
	
	// rotate response by halfSize samples
	response = response.getRegion(halfSize, size-1).append(response.getRegion(0, halfSize-1));
	return response;
}



END_UGEN_NAMESPACE
