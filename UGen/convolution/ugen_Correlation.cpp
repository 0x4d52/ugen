// $Id: ugen_SimpleConvolution.cpp 215 2010-12-03 10:59:26Z 0x4d52 $
// $HeadURL: http://ugen.googlecode.com/svn/trunk/UGen/convolution/ugen_SimpleConvolution.cpp $

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

//#if defined(UGEN_CONVOLUTION) && UGEN_CONVOLUTION
//
//#if !defined(WIN32) && !defined(UGEN_IPHONE) && !defined(UGEN_ANDROID)
//	#include <Accelerate/Accelerate.h>
//	#include <CoreServices/CoreServices.h>
//#endif
//#include "../core/ugen_StandardHeader.h"
//
//BEGIN_UGEN_NAMESPACE
//
//#include "ugen_Correlation.h"
//#include "../basics/ugen_BinaryOpUGens.h"
//#include "../core/ugen_UGenArray.h"
//
//
//CorrelationUGenInternal::CorrelationUGenInternal(UGen const& inputA, 
//                                                 UGen const& inputB, 
//                                                 const int length,
//                                                 const int initialDelay) throw() // Will I need the delay?
//:	UGenInternal(NumInputs),
//    length_(length),
//	bufferSize_(length_ * 2),
//	initialDelay_(initialDelay),
//	buffers(BufferSpec(bufferSize_, NumBuffers, true)),
//    window(Buffer::hannWindow(length_)),
//    score(Buffer::newClear(length_, 1, true)),
//	bufferIndex(initialDelay <= 0 ? 0 : initialDelay >= bufferSize_ ? 0 : bufferSize_ - initialDelay),
//    lockedIndexOfMax(-1)
//{
//	inputs[InputA] = inputA;
//	inputs[InputB] = inputB;
//    
//}
//
//CorrelationUGenInternal::~CorrelationUGenInternal()
//{
//}
//
//UGenInternal* CorrelationUGenInternal::getChannel(const int channel) throw()
//{
//	return new CorrelationUGenInternal(inputs[InputA].getChannel(channel), 
//                                       inputs[InputB].getChannel(channel), 
//                                       length_, 
//                                       initialDelay_);
//}
//
//void CorrelationUGenInternal::outputIndex(float* outputSamples, int numSamplesToProcess)
//{
//    float fIndexOfMax = lockedIndexOfMax;
//    for (int i = 0; i < numSamplesToProcess; i++)
//        outputSamples[i] = fIndexOfMax;    
//}
//
//void CorrelationUGenInternal::outputBuffer(float* outputSamples, int numSamplesToProcess)
//{
//    float* const outputBufferSamples = buffers.getDataUnchecked(OutputBuffer);
//    memcpy(outputSamples, outputBufferSamples, numSamplesToProcess * sizeof(float));
//    outputSamples[numSamplesToProcess-1] = -length_;
//}
//
//void CorrelationUGenInternal::outputScore(float* outputSamples, int numSamplesToProcess)
//{
//    float* const scoreSamples = score.getDataUnchecked(0);
//    memcpy(outputSamples, scoreSamples, numSamplesToProcess * sizeof(float));
//    outputSamples[numSamplesToProcess-1] = -length_;
//}
//
//
//void CorrelationUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
//{
//	const int blockSize = uGenOutput.getBlockSize();
//	int numSamplesToProcess = blockSize;
//	
//	float* inputASamples = inputs[InputA].processBlock(shouldDelete, blockID, channel);
//	float* inputBSamples = inputs[InputB].processBlock(shouldDelete, blockID, channel);
//	float* outputSamples = uGenOutput.getSampleData();
//	
//    float* const windowsSamples = window.getDataUnchecked(0);
//    float* const scoreSamples = score.getDataUnchecked(0);
//	float* const bufferASamples = buffers.getDataUnchecked(InputBufferA);
//	float* const bufferBSamples = buffers.getDataUnchecked(InputBufferB);
//	float* const outputBufferSamples = buffers.getDataUnchecked(OutputBuffer);
//	
//	while(numSamplesToProcess > 0)
//	{
//		int bufferSamplesToProcess = length_ - bufferIndex;
//	
//		if(bufferSamplesToProcess > numSamplesToProcess)
//		{			
//			// buffer the inputs
//			memcpy(bufferASamples + bufferIndex, inputASamples, numSamplesToProcess * sizeof(float));
//			memcpy(bufferBSamples + bufferIndex, inputBSamples, numSamplesToProcess * sizeof(float));
//			
//            bufferIndex += numSamplesToProcess;
//			inputASamples += numSamplesToProcess;
//			inputBSamples += numSamplesToProcess;
//
//			// ...and the output - output the lockedIndexOfMax into the outputSamples 
//            outputIndex(outputSamples, numSamplesToProcess);
////            outputBuffer(outputSamples, numSamplesToProcess);
////            outputScore(outputSamples, numSamplesToProcess);
//            
//			outputSamples += numSamplesToProcess;
//			numSamplesToProcess = 0;
//		}
//		else
//		{
//			numSamplesToProcess -= bufferSamplesToProcess;
//
//			memcpy(bufferASamples + bufferIndex, inputASamples, bufferSamplesToProcess * sizeof(float));
//			memcpy(bufferBSamples + bufferIndex, inputBSamples, bufferSamplesToProcess * sizeof(float));
//			memset(outputBufferSamples, 0, (length_ + length_) * sizeof(float));
//            
//            //apply windows
//            
//            for (int i = 0; i < length_; i++)
//            {
//                bufferASamples[i] *= windowsSamples[i];
//                bufferBSamples[i] *= windowsSamples[i];
//            }
//            
//			bufferIndex += bufferSamplesToProcess;
//			inputASamples += bufferSamplesToProcess;
//			inputBSamples += bufferSamplesToProcess;
//			
//			vDSP_conv (bufferASamples, 1, 
//                       bufferBSamples, 1,			
//					   outputBufferSamples, 1, 
//					   length_, length_);
//                        
//            for (int i = 0; i < length_; i++)
//                scoreSamples[i] *= 0.9f;
//            
//            indexOfMax = findPeak(outputBufferSamples, length_);
//            
//            if ((indexOfMax >= 0) && (indexOfMax < length_))
//            {
//                if (lockedIndexOfMax >= 0)
//                {
//                    int diff = indexOfMax - lockedIndexOfMax;
//                    
//                    if (diff < 0)
//                        diff = -diff;
//                    
//                    int maximum = length_ / 4;
//                    diff = ugen::min(diff, maximum);
//                    
//                    int score = maximum - diff;
//                    
//                    float fScore = (float)score / maximum;
//                    fScore = ugen::cubed(fScore);
//                    
//                    scoreSamples[indexOfMax] += fScore;
//                }
//                else
//                {
//                    scoreSamples[indexOfMax] += 1.f;
//                }
//            }
//                        
//            lockedIndexOfMax = findPeak(scoreSamples, length_);            
//            
//            // output the lockedIndexOfMax into the outputSamples 
//            outputIndex(outputSamples, bufferSamplesToProcess);
////            outputBuffer(outputSamples, bufferSamplesToProcess);
////            outputScore(outputSamples, bufferSamplesToProcess);
//
//            outputSamples += bufferSamplesToProcess;
//			bufferSamplesToProcess = 0;
//			bufferIndex = 0;
//		}
//	}
//}
//
//int CorrelationUGenInternal::findPeak(float* buffer, int length)
//{
//    int indexOfMax = -1; 
//    float currentMax = 0.f;	
//    
//    for(int i = length - 1; i >= 0; i--) 
//    {
//        float sample = abs(buffer[i]); // saves doing abs() twice
//        
//        if (sample >= currentMax)
//        {
//            currentMax = sample;
//            indexOfMax = i;
//        }
//    }
//    
//    return indexOfMax;
//}
//
//Correlation::Correlation(UGen const& inputA, 
//                         UGen const& inputB, 
//                         const int length, 
//                         const int initialDelay) throw()
//{	
//	const int lengthChecked = ugen::clip(length, 1, 2044);
//
//	const int numInputChannels = ugen::max(inputA.getNumChannels(), inputB.getNumChannels());
//	initInternal(numInputChannels);
//	
//	for(int i = 0; i < numInternalUGens; i++)
//	{
//		internalUGens[i] = new CorrelationUGenInternal(inputA, 
//                                                       inputB, 
//                                                       lengthChecked, 
//                                                       initialDelay);
//	}
//}
//
////OverlapCorrelation::OverlapCorrelation(UGen const& inputA, UGen const& inputB, const int fftSize, const int overlap) throw()
////{	
////	const int fftSizeChecked = Bits::isPowerOf2(fftSize) ? fftSize : Bits::nextPowerOf2(fftSize);
////	const int overlapChecked = Bits::isPowerOf2(overlap) ? overlap : Bits::nextPowerOf2(overlap);
////	
////	UGenArray overlappingCorrelations;
////	const int delay = fftSize / overlapChecked;
////			
////	for(int i = 0; i < overlap; i++)
////	{
////		overlappingCorrelations <<= Correlation::AR(inputA, inputB, fftSizeChecked, delay * i);
////	}
////		
////	UGen mixer = overlappingCorrelations.mix();
////	
////	const int numInputChannels = ugen::max(inputA.getNumChannels(), inputB.getNumChannels());
////	initInternal(numInputChannels);
////	
////	for(int i = 0; i < numInternalUGens; i++)
////	{
////		internalUGens[i] = new BinaryMultiplyUGenInternal(mixer, 1.0 / overlapChecked);
////	}
////}
//
//
//END_UGEN_NAMESPACE
//
//#endif // UGEN_CONVOLUTION
