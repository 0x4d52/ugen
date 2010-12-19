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

#ifndef _UGEN_ugen_FFTEngine_H_
#define _UGEN_ugen_FFTEngine_H_

#include "../buffers/ugen_Buffer.h"

class FFTEngineInternal;

/** FFT processing helper class.
 This relies on a choice of underlying FFT library.  */
class FFTEngine
{
public:
	/** Construct an FFTEngine with a given FFT size.
	 If this is 0 or less then a size of 4096 is used.
	 If this is 16 or greater then this size is used directly for the FFT 
	 (and rounded up to the next power of 2 if necessary). 
	 power of 2 if not. If this is less tan 16 this value is taken to the 
	 power of 2 before being used. */
	FFTEngine(const int fftSize = 0) throw();
	FFTEngine(FFTEngine const& copy) throw();
	FFTEngine& operator= (FFTEngine const& other) throw();
		
	/** Get the FFT size. */
	int size() const throw();
	FFTEngineInternal* getInternal() throw() { return internal; }
	Buffer& getFFTWindow() throw();
	Buffer& getIFFTWindow() throw();
	void setFFTWindow(Buffer const& window) throw();
	void setIFFTWindow(Buffer const& window) throw();
	
	/** Perform an FFT. */
	void fft(Buffer const& outputBuffer, 
			 Buffer const& inputBuffer, 
			 const bool applyWindow = false,
			 const int outputChannel = 0,
			 const int inputChannel = 0) throw();
	
	/** Perform an FFT at particular offsets within the input/output Buffers. */
	void fft(Buffer const& outputBuffer, 
			 Buffer const& inputBuffer, 
			 const bool applyWindow,
			 const int outputChannel,
			 const int inputChannel,
			 const int outputOffset,
			 const int inputOffset) throw();
	
	
	/** Perform an inverse FFT. */
	void ifft(Buffer const& outputBuffer, 
			  Buffer const& inputBuffer, 
			  const bool applyWindow = false, 
			  const bool applyScaling = false,
			  const int outputChannel = 0,
			  const int inputChannel = 0) throw();
	
	/** Perform an inverse FFT at particular offsets within the input/output Buffers. */
	void ifft(Buffer const& outputBuffer, 
			  Buffer const& inputBuffer, 
			  const bool applyWindow, 
			  const bool applyScaling,
			  const int outputChannel,
			  const int inputChannel,
			  const int outputOffset,
			  const int inputOffset) throw();

	Buffer rawToRealImagRawSplit(Buffer const& raw) throw();	
	Buffer rawToRealImagUnpacked(Buffer const& raw) throw();
	Buffer rawToRealImagUnpacked(Buffer const& raw, const int firstBin, const int numBins) throw();
	Buffer rawToRealImagUnpackedSplit(Buffer const& raw, const int firstBin, const int numBins) throw();
	Buffer rawToMagnitudePhase(Buffer const& raw, 
							   const int firstBin, 
							   const int numBins, 
							   const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToMagnitudePhaseSplit(Buffer const& raw, 
									const int firstBin, 
									const int numBins, 
									const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToMagnitude(Buffer const& raw, 
						  const int firstBin, 
						  const int numBins,
						  const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToPhase(Buffer const& raw, const int firstBin, const int numBins) throw();
	
	/** An enum for modes when FFT data is sent as a Buffer (e.g., to a ScopeGUI) 
		In the descriptions below:
		- R - real
		- I - imaginary
		- M - magnitude
		- P - phase */
	enum FFTModes 
	{ 
		RealImagRaw,				///< The data is packed R[0], R[1], ... R[n-1], R[n], I[1], I[2], ... I[n-1]
		RealImagRawSplit,			///< As RealImagRaw but real data is in even channels, and imag in odd channels
		RealImagUnpacked,			///< The data is unpacked: R[0], R[1], ... R[n-1], R[n], I[0], I[1], ... I[n]
		RealImagUnpackedSplit,		///< As RealImagUnpackedSplit but real data is in even channels, and imag in odd channels
		MagnitudePhase,				///< The data is  M[0], M[1], ... M[n-1], M[n], P[0], P[1], ... P[n]
		MagnitudePhaseSplit,		///< As MagnitudePhase but magnitude data is in even channels, and phase in odd channels
		Magnitude,					///< The data is  M[0], M[1], ... M[n-1], M[n]
		Phase,						///< The data is  P[0], P[1], ... P[n-1], P[n]
		NumScopeModes 
	};
	
	/** Generate impulse responses for creating filters for specific phase shifts. */
	Buffer generatePhaseShiftResponse(FloatArray const& phases) throw();
	
private:
	FFTEngineInternal* internal;
};


#endif // _UGEN_ugen_FFTEngine_H_
