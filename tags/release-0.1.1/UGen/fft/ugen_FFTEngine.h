// $Id: ugen_FFTEngine.h 993 2010-03-03 21:47:50Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/fft/ugen_FFTEngine.h $

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

class FFTEngine
{
public:
	FFTEngine(const int fftSize) throw();
	FFTEngine(FFTEngine const& copy) throw();
	FFTEngine& operator= (FFTEngine const& other) throw();
	
	void dispose() { }
	
	int size() const throw();
	Buffer& getFFTWindow() throw();
	Buffer& getIFFTWindow() throw();
	void setFFTWindow(Buffer const& window) throw();
	void setIFFTWindow(Buffer const& window) throw();
	
	void fft(Buffer const& outputBuffer, 
			 Buffer const& inputBuffer, 
			 const bool applyWindow = false,
			 const int outputChannel = 0,
			 const int inputChannel = 0) throw();
	
	void ifft(Buffer const& outputBuffer, 
			  Buffer const& inputBuffer, 
			  const bool applyWindow = false, 
			  const bool applyScaling = false,
			  const int outputChannel = 0,
			  const int inputChannel = 0) throw();

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
	
private:
	FFTEngineInternal* internal;
};


#endif // _UGEN_ugen_FFTEngine_H_
