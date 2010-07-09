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

#ifndef _UGEN_ugen_FFTMagnitude_H_
#define _UGEN_ugen_FFTMagnitude_H_

// don't forget to add this header file to UGen.h

#include "../core/ugen_UGen.h"
#include "ugen_FFTEngine.h"

class FFTMagnitudeUGenInternal : public ProxyOwnerUGenInternal
{
public:
	FFTMagnitudeUGenInternal(UGen const& input, FFTEngine const& fft, const int overlap, const int firstBin, const int numBins) throw();
	~FFTMagnitudeUGenInternal();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
	inline int getFirstBin() throw() { return firstBin_; }
	inline int getNumBins() throw() { return numBins_; }
	
protected:
	FFTEngine fftEngine;
	const int fftSize;
	const int fftSizeHalved;
	const int overlap_;
	const int hopSize;
	const int overlapSize;
	const int firstBin_;
	const int maxNumBins;
	const int numBins_;
	Buffer inputBuffer, outputBuffer;
	int bufferIndex;
	Buffer magnitudes;
	float** outputSampleData;
};

#define FFTMagnitude_Docs	@param input		The input signal to perform the FFT on, this should be mono		\
							@param fft			The FFTEngine to use. This can be just an int which will be		\
												used to create an FFTEngine or you can pass in an existing		\
												FFTEngine object. The FFT size should be a power of 2			\
												(if not, it will be rounded up to the next power of 2).			\
							@param overlap		Overlap factor for the FFT. E.g., if overlap is 2 and FFT size	\
												is 1024, an FFT will be performed every 512 samples. This		\
												should be a small power of 2 (e.g., 1, 2, 4, 8, 16 etc).		\
							@param firstBin		The first bin index to report, 0 is the DC bin and the bin at	\
												index equal to the FFT size is the Nyquist bin.					\
							@param numBins		The number of bins to report. The maximum if firstBin is zero	\
												is equal to one greater than half the FFT size (e.g., if FFT	\
												size is	128 the maximum is 65 assuming firstBin is zero).

/** Get the magnitude from one or more bins in an FFT.
 
 This creates a multichannel UGen with a number of channels equal to the number of bins. The magnitude
 of each bin is output on its own channel. The rate at which the magnitudes are updated depends on
 the FFT size and the overlap size i.e., every @f$ \frac{fftSize}{overlap} @f$ samples.
 
 @ingroup FFTUGens
 */
UGenSublcassDeclarationNoDefault(FFTMagnitude, 
								 (input, fft, overlap, firstBin, numBins), 
								 (UGen const& input, FFTEngine const& fft, const int overlap, const int firstBin = 0, const int numBins = 0), 
								COMMON_UGEN_DOCS FFTMagnitude_Docs);



#endif // _UGEN_ugen_FFTMagnitude_H_
