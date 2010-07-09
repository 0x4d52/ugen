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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_BEQ.h"
#include "../core/ugen_Constants.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"



BEQBaseUGenInternal::BEQBaseUGenInternal(UGen const& input, UGen const& freq, UGen const& control, UGen const& gain, const bool faster) throw()
:	UGenInternal(NumInputs),
	y1(0.f), y2(0.f), a0(0.f), a1(0.f), a2(0.f), b1(0.f), b2(0.f),
	currentFreq(0.f), currentControl(0.f), currentGain(0.f),
	//initialised(false),
	faster_(faster)
{
	inputs[Input] = input;
	inputs[Freq] = freq;
	inputs[Control] = control;
	inputs[Gain] = gain;
}

void BEQBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int filterLoops = UGen::getFilterLoops();
	int filterRemain = UGen::getFilterRemain();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float newFreq = *(inputs[Freq].processBlock(shouldDelete, blockID, channel));
	float newControl = *(inputs[Control].processBlock(shouldDelete, blockID, channel));
	float newGain = *(inputs[Gain].processBlock(shouldDelete, blockID, channel));
	float y0;
	
//	if(!initialised)
//	{
//		calculateCoeffs(newFreq, newControl, newGain, a0, a1, a2, b1, b2);
//		currentFreq = newFreq;
//		currentControl = newControl;
//		currentGain = newGain;
//		initialised = true;
//	}
	
	LOCAL_DECLARE(double, y1);
	LOCAL_DECLARE(double, y2);
	LOCAL_DECLARE(double, a0);
	LOCAL_DECLARE(double, a1);
	LOCAL_DECLARE(double, a2);
	LOCAL_DECLARE(double, b1);
	LOCAL_DECLARE(double, b2);

// too innefficient in the loop? perhaps put in the calculateCoeffs functions
//	ugen_assert(newFreq > 0.f);
//	ugen_assert(newFreq < (UGen::getSampleRate() * 0.5));
	
	if((currentFreq != newFreq) || (currentControl != newControl) || (currentGain != newGain))
	{		
		double oneOverFilterLoops = UGen::getReciprocalFilterLoops();
		
		if(faster_ == true) // faster but glitches during large, rapid changes
		{
			double next_a0, next_a1, next_a2, next_b1, next_b2;
			calculateCoeffs(newFreq, newControl, newGain, next_a0, next_a1, next_a2, next_b1, next_b2);
						
			double a0_slope = (next_a0 - a0) * oneOverFilterLoops;
			double a1_slope = (next_a1 - a1) * oneOverFilterLoops;
			double a2_slope = (next_a2 - a2) * oneOverFilterLoops;
			double b1_slope = (next_b1 - b1) * oneOverFilterLoops;
			double b2_slope = (next_b2 - b2) * oneOverFilterLoops;
			
			while(filterLoops--)
			{
				y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
				*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
				
				y2 = *inputSamples++ + b1 * y0 + b2 * y1; 
				*outputSamples++ = (float)(a0 * y2 + a1 * y0 + a2 * y1);
				
				y1 = *inputSamples++ + b1 * y2 + b2 * y0; 
				*outputSamples++ = (float)(a0 * y1 + a1 * y2 + a2 * y0);
				
				a0 += a0_slope;
				a1 += a1_slope;
				a2 += a2_slope;
				b1 += b1_slope;
				b2 += b2_slope;
			}
			
			a0 = next_a0;
			a1 = next_a1;
			a2 = next_a2;
			b1 = next_b1;
			b2 = next_b2;
		}
		else // more cpu intensive but cleaner for rapid changes
		{		
			double freqSlope	= (newFreq - currentFreq) * oneOverFilterLoops;
			double controlSlope = (newControl - currentControl) * oneOverFilterLoops;
			double gainSlope	= (newGain - currentGain) * oneOverFilterLoops;
			
			while(filterLoops--)
			{
				calculateCoeffs(currentFreq, currentControl, currentGain, a0, a1, a2, b1, b2);
				
				y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
				*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
				
				y2 = *inputSamples++ + b1 * y0 + b2 * y1; 
				*outputSamples++ = (float)(a0 * y2 + a1 * y0 + a2 * y1);
				
				y1 = *inputSamples++ + b1 * y2 + b2 * y0; 
				*outputSamples++ = (float)(a0 * y1 + a1 * y2 + a2 * y0);
				
				currentFreq += freqSlope;
				currentControl += controlSlope;
				currentGain += gainSlope;
			}
			
			calculateCoeffs(newFreq, newControl, newGain, a0, a1, a2, b1, b2);
		}
		
		while(filterRemain--)
		{
			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
			*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
			
			y2 = y1; 
			y1 = y0;
		}
		
		currentFreq = newFreq;
		currentControl = newControl;
		currentGain = newGain;
		LOCAL_COPY(a0);
		LOCAL_COPY(a1);
		LOCAL_COPY(a2);
		LOCAL_COPY(b1);
		LOCAL_COPY(b2);
	}
	else
	{
		while(filterLoops--)
		{
			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
			*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
			
			y2 = *inputSamples++ + b1 * y0 + b2 * y1; 
			*outputSamples++ = (float)(a0 * y2 + a1 * y0 + a2 * y1);
			
			y1 = *inputSamples++ + b1 * y2 + b2 * y0; 
			*outputSamples++ = (float)(a0 * y1 + a1 * y2 + a2 * y0);
		}
		
		while(filterRemain--)
		{
			y0 = *inputSamples++ + b1 * y1 + b2 * y2; 
			*outputSamples++ = (float)(a0 * y0 + a1 * y1 + a2 * y2);
			
			y2 = y1; 
			y1 = y0;
		}
	}
	
	y1 = zap(y1);
	y2 = zap(y2);
	LOCAL_COPY(y1);
	LOCAL_COPY(y2);
}

void BEQBaseUGenInternal::initValue(const float value) throw()
{
	float checkedValue = zap(value);
	UGenInternal::initValue(checkedValue);
	y1 = y2 = checkedValue;
}

BLowPassUGenInternal::BLowPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f, faster)
{	
}

UGenInternal* BLowPassUGenInternal::getChannel(const int channel) throw()
{
	return new BLowPassUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalQ].getChannel(channel),
									faster_);
}

void BLowPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain, 
										   double& a0, double& a1, double& a2, double& b1, double& b2)
{
	(void)gain; // not used in BLowPass
	
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double cosw0 = cos(w0); 
	double i = 1.0 - cosw0; 
	double alpha = sin(w0) * 0.5 * (double)rq; 
	double b0rz = 1.0 / (1.0 + alpha); 
	a0 = i * 0.5 * b0rz; 
	a1 = i * b0rz; 
	a2 = a0; 
	b1 = cosw0 * 2.0 * b0rz; 
	b2 = (1.0 - alpha) * -b0rz; 
}

BLowPass::BLowPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
			
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowPassUGenInternal(input, freq, rq, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BLowPass4::BLowPass4(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowPassUGenInternal(BLowPass::AR(input, freq, rq), 
															   freq, 
															   rq, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiPassUGenInternal::BHiPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f, faster)
{	
}

UGenInternal* BHiPassUGenInternal::getChannel(const int channel) throw()
{
	return new BHiPassUGenInternal(inputs[Input].getChannel(channel),
								   inputs[Freq].getChannel(channel),
								   inputs[ReciprocalQ].getChannel(channel),
								   faster_);
}

void BHiPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain, 
										   double& a0, double& a1, double& a2, double& b1, double& b2)
{
	(void)gain; // not used in BHiPass
	
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double cosw0 = cos(w0); 
	double i = 1.0 + cosw0; 
	double alpha = sin(w0) * 0.5 * (double)rq; 
	double b0rz = 1.0 / (1.0 + alpha); 
	a0 = i * 0.5 * b0rz; 
	a1 = -i * b0rz; 
	a2 = a0; 
	b1 = cosw0 * 2.0 * b0rz; 
	b2 = (1.0 - alpha) * -b0rz; 
}

BHiPass::BHiPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiPassUGenInternal(input, freq, rq, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiPass4::BHiPass4(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiPassUGenInternal(BHiPass::AR(input, freq, rq), 
												   freq, 
												   rq, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BBandPassUGenInternal::BBandPassUGenInternal(UGen const& input, UGen const& freq, UGen const& bw, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, bw, 0.f, faster)
{	
}

UGenInternal* BBandPassUGenInternal::getChannel(const int channel) throw()
{
	return new BBandPassUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[BW].getChannel(channel),
									 faster_);
}

void BBandPassUGenInternal::calculateCoeffs(const float freq, const float bw, const float gain, 
											double& a0, double& a1, double& a2, double& b1, double& b2)
{
	(void)gain; // not used in BBandPass
	
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double sinw0 = sin(w0);
	double alpha = sinw0 * (sinh((0.34657359027997 * (double)bw * w0) / sinw0));
	double b0rz = 1.0 / (1.0 + alpha); 
	a0 = alpha * b0rz; 
	a1 = 0.0;
	a2 = -a0; 
	b1 = cos(w0) * 2.0 * b0rz; 
	b2 = (1.0 - alpha) * -b0rz; 
}

BBandPass::BBandPass(UGen const& input, UGen const& freq, UGen const& bw) throw()
{
	UGen inputs[] = { input, freq, bw };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BBandPassUGenInternal(input, freq, bw, false);
		filter->calculateCoeffs(freq.getValue(i), bw.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BBandStopUGenInternal::BBandStopUGenInternal(UGen const& input, UGen const& freq, UGen const& bw, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, bw, 0.f, faster)
{	
}

UGenInternal* BBandStopUGenInternal::getChannel(const int channel) throw()
{
	return new BBandStopUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[BW].getChannel(channel),
									 faster_);
}

void BBandStopUGenInternal::calculateCoeffs(const float freq, const float bw, const float gain, 
											double& a0, double& a1, double& a2, double& b1, double& b2)
{
	(void)gain; // not used in BBandStop
	
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double sinw0 = sin(w0);
	double alpha = sinw0 * (sinh((0.34657359027997 * (double)bw * w0) / sinw0));
	double b0rz = 1.0 / (1.0 + alpha); 	
	b1 = 2.0 * b0rz * cos(w0); 
	a0 = b0rz; 
	a1 = -b1;
	a2 = b0rz; 
	b2 = (1.0 - alpha) * -b0rz; 
}

BBandStop::BBandStop(UGen const& input, UGen const& freq, UGen const& bw) throw()
{
	UGen inputs[] = { input, freq, bw };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BBandStopUGenInternal(input, freq, bw, false);
		filter->calculateCoeffs(freq.getValue(i), bw.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}

BPeakEQUGenInternal::BPeakEQUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, UGen const& gain, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rq, gain, faster)
{	
}

UGenInternal* BPeakEQUGenInternal::getChannel(const int channel) throw()
{
	return new BPeakEQUGenInternal(inputs[Input].getChannel(channel),
								   inputs[Freq].getChannel(channel),
								   inputs[ReciprocalQ].getChannel(channel),
								   inputs[Gain].getChannel(channel),
								   faster_);
}

void BPeakEQUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain, 
											double& a0, double& a1, double& a2, double& b1, double& b2)
{	
	double a = pow(10., (double)gain * 0.025);
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double alpha = sin(w0) * 0.5 * (double)rq;
	double alphaBy_a = alpha * a;
	double alphaOver_a = alpha / a;
	double b0rz = 1.0 / (1.0 + alphaOver_a);	
	b1 = 2.0 * b0rz * cos(w0); 
	a0 = (1.0 + alphaBy_a) * b0rz; 
	a1 = -b1;
	a2 = (1.0 - alphaBy_a) * b0rz; 
	b2 = (1.0 - alphaOver_a) * -b0rz; 
}

BPeakEQ::BPeakEQ(UGen const& input, UGen const& freq, UGen const& rq, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rq, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BPeakEQUGenInternal(input, freq, rq, gain, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BLowShelfUGenInternal::BLowShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rs, gain, faster)
{	
}

UGenInternal* BLowShelfUGenInternal::getChannel(const int channel) throw()
{
	return new BLowShelfUGenInternal(inputs[Input].getChannel(channel),
									 inputs[Freq].getChannel(channel),
									 inputs[ReciprocalS].getChannel(channel),
									 inputs[Gain].getChannel(channel),
									 faster_);
}

void BLowShelfUGenInternal::calculateCoeffs(const float freq, const float rs, const float gain, 
										  double& a0, double& a1, double& a2, double& b1, double& b2)
{	
	double a = pow(10., (double)gain * 0.025);
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double sinw0 = sin(w0);
	double cosw0 = cos(w0);
	double alpha = sinw0 * 0.5 * sqrt((a + (1.0/a)) * ((double)rs - 1.0) + 2.0);
	double aPlus1 = a + 1.0;
	double aMinus1 = a - 1.0;
	double i = aPlus1 * cosw0;
	double j = aMinus1 * cosw0;
	double k = 2.0 * sqrt(a) * alpha;
	double aPlus1Minusk = aPlus1 - k;
	double b0rz = 1.0 / (aPlus1 + j + k);	
	
	a0 = a * (aPlus1 - j + k) * b0rz;
	a1 = 2.0 * a * (aMinus1 - i) * b0rz;
	a2 = a * (aPlus1Minusk - j) * b0rz;
	b1 = 2.0 * (aMinus1 + i) * b0rz;
	b2 = (aPlus1Minusk + j) * -b0rz; 
}

BLowShelf::BLowShelf(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rs, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BLowShelfUGenInternal(input, freq, rs, gain, false);
		filter->calculateCoeffs(freq.getValue(i), rs.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BHiShelfUGenInternal::BHiShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rs, gain, faster)
{	
}

UGenInternal* BHiShelfUGenInternal::getChannel(const int channel) throw()
{
	return new BHiShelfUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalS].getChannel(channel),
									inputs[Gain].getChannel(channel),
									faster_);
}

void BHiShelfUGenInternal::calculateCoeffs(const float freq, const float rs, const float gain, 
											double& a0, double& a1, double& a2, double& b1, double& b2)
{	
	double a = pow(10., (double)gain * 0.025);
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double sinw0 = sin(w0);
	double cosw0 = cos(w0);
	double alpha = sinw0 * 0.5 * sqrt((a + (1.0/a)) * ((double)rs - 1.0) + 2.0);
	double aPlus1 = a + 1.0;
	double aMinus1 = a - 1.0;
	double i = aPlus1 * cosw0;
	double j = aMinus1 * cosw0;
	double k = 2.0 * sqrt(a) * alpha;
	double aPlus1Minusk = aPlus1 - k;
	double b0rz = 1.0 / (aPlus1 - j + k);	
	
	a0 = a * (aPlus1 + j + k) * b0rz;
	a1 = -2.0 * a * (aMinus1 + i) * b0rz;
	a2 = a * (aPlus1Minusk + j) * b0rz;
	b1 = -2.0 * (aMinus1 - i) * b0rz;
	b2 = (aPlus1Minusk - j) * -b0rz; 
}

BHiShelf::BHiShelf(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw()
{
	UGen inputs[] = { input, freq, rs, gain };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);	
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BHiShelfUGenInternal(input, freq, rs, gain, false);
		filter->calculateCoeffs(freq.getValue(i), rs.getValue(i), gain.getValue(i));
		filter->initValue(input.getValue(i) * gain.getValue(i));
		internalUGens[i] = filter;
	}
}

BAllPassUGenInternal::BAllPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, const bool faster) throw()
:	BEQBaseUGenInternal(input, freq, rq, 0.f, faster)
{	
}

UGenInternal* BAllPassUGenInternal::getChannel(const int channel) throw()
{
	return new BAllPassUGenInternal(inputs[Input].getChannel(channel),
									inputs[Freq].getChannel(channel),
									inputs[ReciprocalQ].getChannel(channel),
									faster_);
}

void BAllPassUGenInternal::calculateCoeffs(const float freq, const float rq, const float gain, 
										   double& a0, double& a1, double& a2, double& b1, double& b2)
{
	(void)gain; // not used in BAllPass
	
	double w0 = twopi * (double)freq * UGen::getReciprocalSampleRate(); 
	double alpha = sin(w0) * 0.5 * (double)rq; 
	double b0rz = 1.0 / (1.0 + alpha); 	
	b1 = 2.0 * b0rz * cos(w0); 
	a0 = (1.0 - alpha) * b0rz; 
	a1 = -b1;
	a2 = 1.0; 
	b2 = -a0; 
}

BAllPass::BAllPass(UGen const& input, UGen const& freq, UGen const& rq) throw()
{
	UGen inputs[] = { input, freq, rq };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		BEQBaseUGenInternal* filter = new BAllPassUGenInternal(input, freq, rq, false);
		filter->calculateCoeffs(freq.getValue(i), rq.getValue(i));
		filter->initValue(input.getValue(i));
		internalUGens[i] = filter;
	}
}


END_UGEN_NAMESPACE

#endif // gpl