// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_BEQ.h"
#include "../core/ugen_Constants.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"



BEQBaseUGenInternal::BEQBaseUGenInternal(UGen const& input, UGen const& freq, UGen const& control, UGen const& gain) throw()
:	UGenInternal(NumInputs),
	y1(0.f), y2(0.f), a0(0.f), a1(0.f), a2(0.f), b1(0.f), b2(0.f),
	currentFreq(0.f), currentControl(0.f), currentGain(0.f)
{
	inputs[Input] = input;
	inputs[Freq] = freq;
	inputs[Control] = control;
	inputs[Gain] = gain;
}


//void BEQBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
//{
//	int numSamplesToProcess = uGenOutput.getBlockSize();
//	float* outputSamples = uGenOutput.getSampleData();
//	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
//	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
//	float* controlSamples = inputs[Control].processBlock(shouldDelete, blockID, channel);
//	float* gainSamples = inputs[Gain].processBlock(shouldDelete, blockID, channel);
//	float newFreq = *freqSamples;
//	float newControl = *controlSamples;
//	float newGain = *gainSamples;
//	float y0;
//				
//	if((currentFreq != newFreq) || (currentControl != newControl) || (currentGain != newGain))
//	{		
//		float slope = 1.f / numSamplesToProcess;
//		
//		float freqSlope	= (newFreq - currentFreq) * slope;
//		float controlSlope = (newControl - currentControl) * slope;
//		float gainSlope	= (newGain - currentGain) * slope;
//		
//		while(numSamplesToProcess--)
//		{
//			calculateCoeffs(currentFreq, currentControl, currentGain);
//			
//			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
//			*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
//			y2 = y1; 
//			y1 = y0;
//			
//			currentFreq += freqSlope;
//			currentControl += controlSlope;
//			currentGain += gainSlope;
//		}
//		
//		calculateCoeffs(newFreq, newControl, newGain);
//				
//		currentFreq = newFreq;
//		currentControl = newControl;
//		currentGain = newGain;
//	}
//	else
//	{		
//		while(numSamplesToProcess--)
//		{
//			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
//			*outputSamples++ = a0 * y0 + a1 * y1 + a2 * y2;
//			
//			y2 = y1; 
//			y1 = y0;
//		}
//	}
//	
//	y1 = zap(y1);
//	y2 = zap(y2);
//}

void BEQBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* freqSamples = inputs[Freq].processBlock(shouldDelete, blockID, channel);
	float* controlSamples = inputs[Control].processBlock(shouldDelete, blockID, channel);
	float* gainSamples = inputs[Gain].processBlock(shouldDelete, blockID, channel);
	float newFreq = *freqSamples;
	float newControl = *controlSamples;
	float newGain = *gainSamples;
	float y0;
	
	if((currentFreq != newFreq) || (currentControl != newControl) || (currentGain != newGain))
	{				
		while(numSamplesToProcess--)
		{
			calculateCoeffs(*freqSamples++, *controlSamples++, *gainSamples++);
			
			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
			*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
			y2 = y1; 
			y1 = y0;			
		}
				
		currentFreq = *(freqSamples-1);
		currentControl = *(controlSamples-1);
		currentGain = *(gainSamples-1);
	}
	else
	{		
		while(numSamplesToProcess--)
		{
			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
			*outputSamples++ = a0 * y0 + a1 * y1 + a2 * y2;
			
			y2 = y1; 
			y1 = y0;
		}
	}
	
	y1 = zap(y1);
	y2 = zap(y2);
}


void BEQBaseUGenInternal::initValue(const float value) throw()
{
	float checkedValue = zap(value);
	UGenInternal::initValue(checkedValue);
	y1 = y2 = checkedValue;
}

BLowPassUGenInternal::BLowPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f)
{	
}

UGenInternal* BLowPassUGenInternal::getChannel(const int channel) throw()
{
	return new BLowPassUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalQ].getChannel(channel));
}

//void BLowPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
//{
//	(void)gain; // not used in BLowPass
//	
//	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
//	BEQ_CALC_TYPE cosw0 = cos(w0); 
//	BEQ_CALC_TYPE i = 1.0 - cosw0; 
//	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq; 
//	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alpha); 
//	a0 = i * 0.5 * b0_temp; 
//	a1 = i * b0_temp; 
//	a2 = a0; 
//	b1 = cosw0 * 2.0 * b0_temp; 
//	b2 = (1.0 - alpha) * -b0_temp; 
//}

//void BLowPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
//{
//	(void)gain; // not used in BLowPass
//	
//	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
//	BEQ_CALC_TYPE cosw0 = cos(w0); 
//	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq; 
//	BEQ_CALC_TYPE temp = 1.0 / (1.0 + alpha); 
//	a0 = (1.0 - cosw0) * 0.5 * temp; 
//	a1 = (1.0 - cosw0) * temp; 
//	a2 = a0; 
//	b1 = cosw0 * 2.0 * temp; 
//	b2 = (1.0 - alpha) * -temp; 
//}


//void BLowPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
//{
//	(void)gain; // not used in BLowPass
//	
//	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
//	BEQ_CALC_TYPE cosw0 = cos(w0); 
//	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq; 
//	a0 = (1.0 - cosw0) * 0.5 * (1.0 / (1.0 + alpha)); 
//	a1 = (1.0 - cosw0) * (1.0 / (1.0 + alpha)); 
//	a2 = a0; 
//	b1 = cosw0 * 2.0 * (1.0 / (1.0 + alpha)); 
//	b2 = (1.0 - alpha) * -(1.0 / (1.0 + alpha)); 
//}


void BLowPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
{
	(void)gain;
	
	const float w0 = twoPi * freq * UGen::getReciprocalSampleRate();
	const float cos_w0 = cos(w0);
	const float sin_w0 = sin(w0);		
	const float alpha = sin_w0 * 0.5f * rq;
	const float temp1 = 1.f / (1.f + alpha);
	const float temp2 = 1.f - cos_w0;
	
	a0 = (temp2 * 0.5f) * temp1;
	a1 = temp2 * temp1;
	a2 = a0;
	b1 = (2.f * cos_w0) * temp1;
	b2 = (1.f - alpha) * -temp1;
}




BLowPass::BLowPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
			
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowPassUGenInternal(input, freq, rq);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BLowPass4::BLowPass4(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowPassUGenInternal(BLowPass::AR(input, freq, rq), 
															   freq, 
															   rq);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiPassUGenInternal::BHiPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f)
{	
}

UGenInternal* BHiPassUGenInternal::getChannel(const int channel) throw()
{
	return new BHiPassUGenInternal(inputs[Input].getChannel(channel),
								   inputs[Freq].getChannel(channel),
								   inputs[ReciprocalQ].getChannel(channel));
}

void BHiPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
{
	(void)gain; // not used in BHiPass
	
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE cosw0 = cos(w0); 
	BEQ_CALC_TYPE i = 1.0 + cosw0; 
	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq; 
	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alpha); 
	a0 = i * 0.5 * b0_temp; 
	a1 = -i * b0_temp; 
	a2 = a0; 
	b1 = cosw0 * 2.0 * b0_temp; 
	b2 = (1.0 - alpha) * -b0_temp; 
}

BHiPass::BHiPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiPassUGenInternal(input, freq, rq);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiPass4::BHiPass4(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiPassUGenInternal(BHiPass::AR(input, freq, rq), 
												   freq, 
												   rq);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BBandPassUGenInternal::BBandPassUGenInternal(UGen const& input, UGen const& freq, UGen const& bw) throw()
:	BEQBaseUGenInternal(input, freq, bw, 0.f)
{	
}

UGenInternal* BBandPassUGenInternal::getChannel(const int channel) throw()
{
	return new BBandPassUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[BW].getChannel(channel));
}

void BBandPassUGenInternal::calculateCoeffs(const float freq, const float bw, const float gain)
{
	(void)gain; // not used in BBandPass
	
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE sinw0 = sin(w0);
	BEQ_CALC_TYPE alpha = sinw0 * (sinh((logSqrt2 * (BEQ_CALC_TYPE)bw * w0) / sinw0));
	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alpha); 
	a0 = alpha * b0_temp; 
	a1 = 0.0;
	a2 = -a0; 
	b1 = cos(w0) * 2.0 * b0_temp; 
	b2 = (1.0 - alpha) * -b0_temp; 
}

BBandPass::BBandPass(UGen const& input, UGen const& freq, UGen const& bw) throw()
{
	UGen inputs[] = { input, freq, bw };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BBandPassUGenInternal(input, freq, bw);
		filter->calculateCoeffs(freq.getValue(i), bw.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BBandStopUGenInternal::BBandStopUGenInternal(UGen const& input, UGen const& freq, UGen const& bw) throw()
:	BEQBaseUGenInternal(input, freq, bw, 0.f)
{	
}

UGenInternal* BBandStopUGenInternal::getChannel(const int channel) throw()
{
	return new BBandStopUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[BW].getChannel(channel));
}

void BBandStopUGenInternal::calculateCoeffs(const float freq, const float bw, const float gain)
{
	(void)gain; // not used in BBandStop
	
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE sinw0 = sin(w0);
	BEQ_CALC_TYPE alpha = sinw0 * (sinh((logSqrt2 * (BEQ_CALC_TYPE)bw * w0) / sinw0));
	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alpha); 	
	b1 = 2.0 * b0_temp * cos(w0); 
	a0 = b0_temp; 
	a1 = -b1;
	a2 = b0_temp; 
	b2 = (1.0 - alpha) * -b0_temp; 
}

BBandStop::BBandStop(UGen const& input, UGen const& freq, UGen const& bw) throw()
{
	UGen inputs[] = { input, freq, bw };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BBandStopUGenInternal(input, freq, bw);
		filter->calculateCoeffs(freq.getValue(i), bw.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BPeakEQUGenInternal::BPeakEQUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, UGen const& gain) throw()
:	BEQBaseUGenInternal(input, freq, rq, gain)
{	
}

UGenInternal* BPeakEQUGenInternal::getChannel(const int channel) throw()
{
	return new BPeakEQUGenInternal(inputs[Input].getChannel(channel),
								   inputs[Freq].getChannel(channel),
								   inputs[ReciprocalQ].getChannel(channel),
								   inputs[Gain].getChannel(channel));
}

void BPeakEQUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
{	
	BEQ_CALC_TYPE a = pow(10., (BEQ_CALC_TYPE)gain * (1.0 / 40.0));
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq;
	BEQ_CALC_TYPE alphaBy_a = alpha * a;
	BEQ_CALC_TYPE alphaOver_a = alpha / a;
	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alphaOver_a);	
	b1 = 2.0 * b0_temp * cos(w0); 
	a0 = (1.0 + alphaBy_a) * b0_temp; 
	a1 = -b1;
	a2 = (1.0 - alphaBy_a) * b0_temp; 
	b2 = (1.0 - alphaOver_a) * -b0_temp; 
}

BPeakEQ::BPeakEQ(UGen const& input, UGen const& freq, UGen const& rq, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rq, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BPeakEQUGenInternal(input, freq, rq, gain);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BLowShelfUGenInternal::BLowShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
:	BEQBaseUGenInternal(input, freq, rs, gain)
{	
}

UGenInternal* BLowShelfUGenInternal::getChannel(const int channel) throw()
{
	return new BLowShelfUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[ReciprocalS].getChannel(channel),
									 inputs[Gain].getChannel(channel));
}

void BLowShelfUGenInternal::calculateCoeffs(const float freq, const float rs, const float gain)
{	
	BEQ_CALC_TYPE a = pow(10., (BEQ_CALC_TYPE)gain * (1.0 / 40.0));
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE sinw0 = sin(w0);
	BEQ_CALC_TYPE cosw0 = cos(w0);
	BEQ_CALC_TYPE alpha = sinw0 * 0.5 * sqrt((a + (1.0/a)) * ((BEQ_CALC_TYPE)rs - 1.0) + 2.0);
	BEQ_CALC_TYPE aPlus1 = a + 1.0;
	BEQ_CALC_TYPE aMinus1 = a - 1.0;
	BEQ_CALC_TYPE i = aPlus1 * cosw0;
	BEQ_CALC_TYPE j = aMinus1 * cosw0;
	BEQ_CALC_TYPE k = 2.0 * sqrt(a) * alpha;
	BEQ_CALC_TYPE aPlus1Minusk = aPlus1 - k;
	BEQ_CALC_TYPE b0_temp = 1.0 / (aPlus1 + j + k);	
	
	a0 = a * (aPlus1 - j + k) * b0_temp;
	a1 = 2.0 * a * (aMinus1 - i) * b0_temp;
	a2 = a * (aPlus1Minusk - j) * b0_temp;
	b1 = 2.0 * (aMinus1 + i) * b0_temp;
	b2 = (aPlus1Minusk + j) * -b0_temp; 
}

BLowShelf::BLowShelf(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rs, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowShelfUGenInternal(input, freq, rs, gain);
		filter->calculateCoeffs(freq.getValue(i), rs.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiShelfUGenInternal::BHiShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
:	BEQBaseUGenInternal(input, freq, rs, gain)
{	
}

UGenInternal* BHiShelfUGenInternal::getChannel(const int channel) throw()
{
	return new BHiShelfUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalS].getChannel(channel),
									inputs[Gain].getChannel(channel));
}

void BHiShelfUGenInternal::calculateCoeffs(const float freq, const float rs, const float gain)
{	
	BEQ_CALC_TYPE a = pow(10., (BEQ_CALC_TYPE)gain * 0.025);
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE sinw0 = sin(w0);
	BEQ_CALC_TYPE cosw0 = cos(w0);
	BEQ_CALC_TYPE alpha = sinw0 * 0.5 * sqrt((a + (1.0/a)) * ((BEQ_CALC_TYPE)rs - 1.0) + 2.0);
	BEQ_CALC_TYPE aPlus1 = a + 1.0;
	BEQ_CALC_TYPE aMinus1 = a - 1.0;
	BEQ_CALC_TYPE i = aPlus1 * cosw0;
	BEQ_CALC_TYPE j = aMinus1 * cosw0;
	BEQ_CALC_TYPE k = 2.0 * sqrt(a) * alpha;
	BEQ_CALC_TYPE aPlus1Minusk = aPlus1 - k;
	BEQ_CALC_TYPE b0_temp = 1.0 / (aPlus1 - j + k);	
	
	a0 = a * (aPlus1 + j + k) * b0_temp;
	a1 = -2.0 * a * (aMinus1 + i) * b0_temp;
	a2 = a * (aPlus1Minusk + j) * b0_temp;
	b1 = -2.0 * (aMinus1 - i) * b0_temp;
	b2 = (aPlus1Minusk - j) * -b0_temp; 
}

BHiShelf::BHiShelf(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rs, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiShelfUGenInternal(input, freq, rs, gain);
		filter->calculateCoeffs(freq.getValue(i), rs.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BAllPassUGenInternal::BAllPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f)
{	
}

UGenInternal* BAllPassUGenInternal::getChannel(const int channel) throw()
{
	return new BAllPassUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalQ].getChannel(channel));
}

void BAllPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain)
{
	(void)gain; // not used in BAllPass
	
	BEQ_CALC_TYPE w0 = twoPi * (BEQ_CALC_TYPE)freq * UGen::getReciprocalSampleRate(); 
	BEQ_CALC_TYPE alpha = sin(w0) * 0.5 * (BEQ_CALC_TYPE)rq; 
	BEQ_CALC_TYPE b0_temp = 1.0 / (1.0 + alpha); 	
	b1 = 2.0 * b0_temp * cos(w0); 
	a0 = (1.0 - alpha) * b0_temp; 
	a1 = -b1;
	a2 = 1.0; 
	b2 = -a0; 
}

BAllPass::BAllPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BAllPassUGenInternal(input, freq, rq);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), 1.f);
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}


END_UGEN_NAMESPACE

#endif // gpl