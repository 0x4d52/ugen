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

#ifndef UGEN_NOEXTGPL

#include "../../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_FSinOsc.h"
#include "../../basics/ugen_InlineUnaryOps.h"


FSinOscUGenInternal::FSinOscUGenInternal(UGen const& freq, const float initialPhase, const int channel) throw()
:	UGenInternal(NumInputs),
	currentFreq(0.0),
	b1(0.0),
	y1(0.0),
	y2(0.0)
{	
	inputs[Freq] = freq;
	
	if(inputs[Freq].isScalar(channel))
		currentFreq = inputs[Freq].getValue(channel);
	
	double w = currentFreq * twoPi * UGen::getReciprocalSampleRate();
	
#ifndef UGEN_ANDROID
	b1 = 2. * std::cos(w);
	y1 = std::sin(initialPhase);
	y2 = std::sin(initialPhase-w);
#else
	b1 = 2. * cos(w);
	y1 = sin(initialPhase);
	y2 = sin(initialPhase-w);	
#endif
	
	initValue(y1);
}

UGenInternal* FSinOscUGenInternal::getChannel(const int channel) throw()
{	
	return new FSinOscUGenInternal((float)currentFreq, 0.f, channel);
}	

//UGenInternal* FSinOscUGenInternal::getKr() throw()
//{ 
//	return new FSinOscUGenInternalK(inputs[Freq].kr()); 
//}

#ifndef UGEN_ANDROID
void FSinOscUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	float* outputSamples = uGenOutput.getSampleData();
	float newFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	double y0;
	
	LOCAL_DECLARE(double, b1);
	LOCAL_DECLARE(double, y1);
	LOCAL_DECLARE(double, y2);
	
	if(newFreq != currentFreq)
	{
		currentFreq = newFreq;
		
		double initialPhase;
		
		if((1.0-std::abs(y1)) < 0.00001)
		{
			initialPhase = y1 > 0.0 ? piOverTwo : -piOverTwo;
		}
		else
		{
			initialPhase = std::asin(y1);
			// based on the trajectory predict which solution of asin(y1) is correct..
			if(y2 >= y1)
			{
				double piVersion = y1 > 0.0 ? pi : -pi;			
				initialPhase = piVersion - initialPhase;
			}
		}

		double w = currentFreq * twoPi * UGen::getReciprocalSampleRate();
		
		b1 = zap(2. * std::cos(w));
		y1 = zap(std::sin(initialPhase));
		y2 = zap(std::sin(initialPhase-w));
	}
	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	for(int i = 0; i < numSamplesToProcess; ++i)
	{
		y0 = b1 * y1 - y2;
		outputSamples[i] = y0;// = b1 * y1 - y2; 
		y2 = y1; 
		y1 = y0;
	}
		
	y1 = zap(y1);
	y2 = zap(y2);
	LOCAL_COPY(b1);
	LOCAL_COPY(y1);
	LOCAL_COPY(y2);
}
#else
//Android
void FSinOscUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	float* outputSamples = uGenOutput.getSampleData();
	float newFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	double y0;
	
	LOCAL_DECLARE(double, b1);
	LOCAL_DECLARE(double, y1);
	LOCAL_DECLARE(double, y2);
	
	if(newFreq != currentFreq)
	{
		currentFreq = newFreq;
		
		double initialPhase;
		
		if((1.0-abs(y1)) < 0.00001)
		{
			initialPhase = y1 > 0.0 ? piOverTwo : -piOverTwo;
		}
		else
		{
			initialPhase = asin(y1);
			// based on the trajectory predict which solution of asin(y1) is correct..
			if(y2 >= y1)
			{
				double piVersion = y1 > 0.0 ? pi : -pi;			
				initialPhase = piVersion - initialPhase;
			}
		}
		
		double w = currentFreq * twoPi * UGen::getReciprocalSampleRate();
		
		b1 = zap(2. * cos(w));
		y1 = zap(sin(initialPhase));
		y2 = zap(sin(initialPhase-w));
	}
	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	for(int i = 0; i < numSamplesToProcess; ++i)
	{
		y0 = b1 * y1 - y2;
		outputSamples[i] = y0;// = b1 * y1 - y2; 
		y2 = y1; 
		y1 = y0;
	}
	
	y1 = zap(y1);
	y2 = zap(y2);
	LOCAL_COPY(b1);
	LOCAL_COPY(y1);
	LOCAL_COPY(y2);
}
#endif
//FSinOscUGenInternalK::FSinOscUGenInternalK(FSinOsc_InputsWithTypesOnly) throw()
//:	FSinOscUGenInternal(FSinOsc_InputsNoTypes),
//	value(0.f)
//{
//	rate = ControlRate;
//}

//void FSinOscUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
//{	
//	const int krBlockSize = UGen::getControlRateBlockSize();
//	unsigned int blockPosition = blockID % krBlockSize;
//	double krBlockSizeOverSampleRate = UGen::getReciprocalSampleRate() * krBlockSize;
//	int numSamplesToProcess = uGenOutput.getBlockSize();
//	float* outputSamples = uGenOutput.getSampleData();
//	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
//	
//	int numKrSamples = blockPosition % krBlockSize;
//	
//	while(numSamplesToProcess > 0)
//	{
//		if(numKrSamples == 0)			
//		{
//			float out;
//			
//			if(currentPhase >= 1.f)	{
//				currentPhase -= 1.f;
//				out = 1.f;
//			} else {
//				out = 0.f;
//			}
//			
//			currentPhase += *freqSamples * krBlockSizeOverSampleRate;
//			value = out;
//		}
//		
//		numKrSamples = krBlockSize - numKrSamples;
//		
//		blockPosition		+= numKrSamples;
//		freqSamples			+= numKrSamples;
//		
//		while(numSamplesToProcess && numKrSamples)
//		{
//			*outputSamples++ = value;
//			--numSamplesToProcess;
//			--numKrSamples;
//		}
//	}
//}


FSinOsc::FSinOsc(UGen const& freq) throw()
{	
	initInternal(freq.getNumChannels());
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new FSinOscUGenInternal(freq, 0.f, i);
	}
}

END_UGEN_NAMESPACE

#endif
