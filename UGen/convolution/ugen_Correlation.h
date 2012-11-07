// $Id: ugen_SimpleConvolution.h 89 2010-07-09 16:20:21Z 0x4d52 $
// $HeadURL: http://ugen.googlecode.com/svn/trunk/UGen/convolution/ugen_SimpleConvolution.h $

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

#ifndef _UGEN_ugen_Correlation_H_
#define _UGEN_ugen_Correlation_H_

#include "../core/ugen_UGen.h"
#include "../fft/ugen_FFTEngineInternal.h"

//class CorrelationUGenInternal : public UGenInternal
//{
//public:
//	CorrelationUGenInternal(UGen const& inputA, 
//                            UGen const& inputB, 
//                            const int length, 
//                            const int initialDelay) throw();
//	~CorrelationUGenInternal();
//	UGenInternal* getChannel(const int channel) throw();
//	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
//	
//    void outputIndex(float* outputSamples, int numSamplesToProcess);
//    void outputBuffer(float* outputSamples, int numSamplesToProcess);
//    void outputScore(float* outputSamples, int numSamplesToProcess);
//    
//    static int findPeak(float* buffer, int length);
//    
//	enum Inputs { InputA, InputB, NumInputs };
//	enum Buffers { InputBufferA, InputBufferB, OutputBuffer, NumBuffers };
//	
//	
//protected:
//    const int length_;
//	const int bufferSize_;
//	const int initialDelay_;
//	Buffer buffers;
//    Buffer window;
//    Buffer score;
//	int bufferIndex;
//    int indexOfMax;
//    int lockedIndexOfMax;
//};
//
//
//UGenSublcassDeclaration(Correlation, 
//						(inputA, inputB, length, initialDelay),
//						(UGen const& inputA, UGen const& inputB, const int length = 512, const int initialDelay = 0), 
//						COMMON_UGEN_DOCS);
//
////UGenSublcassDeclaration(OverlapCorrelation, 
////						(inputA, inputB, fftSize, overlap),
////						(UGen const& inputA, UGen const& inputB, const int length = 512, const int overlap = 2), 
////						COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_Correlation_H_
