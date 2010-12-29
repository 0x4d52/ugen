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

#include "ugen_Lag.h"
#include "../../core/ugen_Constants.h"
#include "../../basics/ugen_InlineUnaryOps.h"
#include "../../basics/ugen_InlineBinaryOps.h"

LagUGenInternal::LagUGenInternal(UGen const& input, UGen const& lagTime) throw()
:	UGenInternal(NumInputs),
	b1(0.f),
	y1(0.f),
	currentLagTime(0.f)
{
	inputs[Input] = input;
	inputs[LagTime] = lagTime;
}

UGenInternal* LagUGenInternal::getKr() throw()
{
	return new LagUGenInternalK(inputs[Input].kr(), inputs[LagTime].kr()); 
}

UGenInternal* LagUGenInternal::getChannel(const int channel) throw()
{
	return new LagUGenInternal(inputs[Input].getChannel(channel), 
							   inputs[LagTime].getChannel(channel));
}

void LagUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lagSamples = inputs[LagTime].processBlock(shouldDelete, blockID, channel);
	float newLagTime = *lagSamples;
	
	ugen_assert(newLagTime >= 0.f);
	
	if(newLagTime != currentLagTime)
	{
		float next_b1 = newLagTime == 0.f ? 0.f : (float)exp(log001 / (newLagTime * UGen::getSampleRate()));
		float b1_slope = (next_b1 - b1) / (float)numSamplesToProcess;
		
		while(numSamplesToProcess--)
		{
			b1 += b1_slope;
			float y0 = *inputSamples++; 
			*outputSamples++ = y1 = (y0 + b1 * (y1 - y0));
		}
		
		b1 = next_b1;
		currentLagTime = newLagTime;
	}
	else
	{
		while(numSamplesToProcess--)
		{
			float y0 = *inputSamples++; 
			*outputSamples++ = y1 = (y0 + b1 * (y1 - y0));
		}
	}
	
	y1 = zap(y1); // denormal!
}

void LagUGenInternal::initValue(const float value) throw()
{
	float checkedValue = zap(value);
	UGenInternal::initValue(checkedValue);
	y1 = checkedValue;
}

void LagUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lagSamples = inputs[LagTime].processBlock(shouldDelete, blockID, channel);
	float newLagTime = *lagSamples;
	
	ugen_assert(newLagTime >= 0.f);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	if(newLagTime != currentLagTime)
	{
		float next_b1 = newLagTime == 0.f ? 0.f : (float)exp(log001 * krBlockSize / (newLagTime * UGen::getSampleRate()));
		float b1_slope = (next_b1 - b1) * krBlockSize / (float)numSamplesToProcess;
		currentLagTime = newLagTime; 
				
		while(numSamplesToProcess > 0)
		{
			float nextValue = value;
			if(numKrSamples == 0)			
			{
				b1 += b1_slope;
				float y0 = *inputSamples; 
				nextValue = y1 = (y0 + b1 * (y1 - y0));
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			if(nextValue == value)
			{
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = nextValue;
					--numSamplesToProcess;
					--numKrSamples;
				}
			} 
			else
			{
				float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();
				
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = value;
					value += valueSlope;
					--numSamplesToProcess;
					--numKrSamples;
				}
				
				value = nextValue;
			}
		}
		
		b1 = next_b1;
	}
	else
	{
		while(numSamplesToProcess > 0)
		{
			float nextValue = value;
			if(numKrSamples == 0)			
			{
				float y0 = *inputSamples; 
				nextValue = y1 = (y0 + b1 * (y1 - y0));
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			if(nextValue == value)
			{
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = nextValue;
					--numSamplesToProcess;
					--numKrSamples;
				}
			} 
			else
			{
				float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();
				
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = value;
					value += valueSlope;
					--numSamplesToProcess;
					--numKrSamples;
				}
				
				value = nextValue;
			}
		}
	}
	
	y1 = zap(y1);
}

Lag::Lag(UGen const& input, UGen const& lagTime) throw()
{
	UGen inputs[] = { input, lagTime };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);		
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new LagUGenInternal(input, lagTime);
		internalUGens[i]->initValue(input.getValue(i));
	}
}




LagUDUGenInternal::LagUDUGenInternal(UGen const& input, 
									 UGen const& lagTimeUp, 
									 UGen const& lagTimeDown) throw()
:	UGenInternal(NumInputs),
	b1u(0.f),
	b1d(0.f),
	y1(0.f),
	currentLagTimeUp(0.f),
	currentLagTimeDown(0.f)
{
	inputs[Input] = input;
	inputs[LagTimeUp] = lagTimeUp;
	inputs[LagTimeDown] = lagTimeDown;
}

UGenInternal* LagUDUGenInternal::getKr() throw()
{
	return new LagUDUGenInternalK(inputs[Input].kr(), 
								  inputs[LagTimeUp].kr(), 
								  inputs[LagTimeDown].kr()); 
}

UGenInternal* LagUDUGenInternal::getChannel(const int channel) throw()
{
	return new LagUDUGenInternal(inputs[Input].getChannel(channel), 
								 inputs[LagTimeUp].getChannel(channel),
								 inputs[LagTimeDown].getChannel(channel));
}

void LagUDUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lagUpSamples = inputs[LagTimeUp].processBlock(shouldDelete, blockID, channel);
	float* lagDownSamples = inputs[LagTimeDown].processBlock(shouldDelete, blockID, channel);
	float newLagTimeUp = *lagUpSamples;
	float newLagTimeDown = *lagDownSamples;
	
	ugen_assert(newLagTimeUp >= 0.f);
	ugen_assert(newLagTimeDown >= 0.f);
	
	if((newLagTimeUp != currentLagTimeUp) || (newLagTimeDown != currentLagTimeDown))
	{
		float next_b1u = newLagTimeUp == 0.f ? 0.f : (float)exp(log001 / (newLagTimeUp * UGen::getSampleRate()));
		float next_b1d = newLagTimeDown == 0.f ? 0.f : (float)exp(log001 / (newLagTimeDown * UGen::getSampleRate()));
		float slope = 1.f / (float)numSamplesToProcess;
		float b1u_slope = (next_b1u - b1u) * slope;
		float b1d_slope = (next_b1d - b1d) * slope;
		currentLagTimeUp = newLagTimeUp;
		currentLagTimeDown = newLagTimeDown;
		
		while(numSamplesToProcess--)
		{
			b1u += b1u_slope;
			b1d += b1d_slope;
			float y0 = *inputSamples++; 
			
			if (y0 > y1)
				*outputSamples++ = y1 = (y0 + b1u * (y1 - y0));
			else
				*outputSamples++ = y1 = (y0 + b1d * (y1 - y0));
		}
		
		b1u = next_b1u;
		b1d = next_b1d;
	}
	else
	{
		while(numSamplesToProcess--)
		{
			float y0 = *inputSamples++; 
			if (y0 > y1)
				*outputSamples++ = y1 = (y0 + b1u * (y1 - y0));
			else
				*outputSamples++ = y1 = (y0 + b1d * (y1 - y0));
		}
	}
	
	y1 = zap(y1); // denormal!
}

void LagUDUGenInternal::initValue(const float value) throw()
{
	float checkedValue = zap(value);
	UGenInternal::initValue(checkedValue);
	y1 = checkedValue;
}

void LagUDUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lagUpSamples = inputs[LagTimeUp].processBlock(shouldDelete, blockID, channel);
	float* lagDownSamples = inputs[LagTimeDown].processBlock(shouldDelete, blockID, channel);
	float newLagTimeUp = *lagUpSamples;
	float newLagTimeDown = *lagDownSamples;
	
	ugen_assert(newLagTimeUp >= 0.f);
	ugen_assert(newLagTimeDown >= 0.f);
	
	int numKrSamples = blockPosition % krBlockSize;
	
	if((newLagTimeUp != currentLagTimeUp) || (newLagTimeDown != currentLagTimeDown))
	{
		float next_b1u = newLagTimeUp == 0.f ? 0.f : (float)exp(log001 * krBlockSize / (newLagTimeUp * UGen::getSampleRate()));
		float next_b1d = newLagTimeDown == 0.f ? 0.f : (float)exp(log001 * krBlockSize / (newLagTimeDown * UGen::getSampleRate()));
		float slope = (float)krBlockSize / (float)numSamplesToProcess;
		float b1u_slope = (next_b1u - b1u) * slope;
		float b1d_slope = (next_b1d - b1d) * slope;
		currentLagTimeUp = newLagTimeUp;
		currentLagTimeDown = newLagTimeDown;
		
		while(numSamplesToProcess > 0)
		{
			float nextValue = value;
			if(numKrSamples == 0)			
			{
				b1u += b1u_slope;
				b1d += b1d_slope;
				float y0 = *inputSamples; 
				
				if (y0 > y1)
					nextValue = y1 = (y0 + b1u * (y1 - y0));
				else
					nextValue = y1 = (y0 + b1d * (y1 - y0));
				
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			if(nextValue == value)
			{
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = nextValue;
					--numSamplesToProcess;
					--numKrSamples;
				}
			} 
			else
			{
				float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();
				
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = value;
					value += valueSlope;
					--numSamplesToProcess;
					--numKrSamples;
				}
				
				value = nextValue;
			}
		}
		
		b1u = next_b1u;
		b1d = next_b1d;
	}
	else
	{
		while(numSamplesToProcess > 0)
		{
			float nextValue = value;
			if(numKrSamples == 0)			
			{
				float y0 = *inputSamples; 
				
				if (y0 > y1)
					nextValue = y1 = (y0 + b1u * (y1 - y0));
				else
					nextValue = y1 = (y0 + b1d * (y1 - y0));
			}
			
			numKrSamples = krBlockSize - numKrSamples;
			
			blockPosition		+= numKrSamples;
			inputSamples		+= numKrSamples;
			
			if(nextValue == value)
			{
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = nextValue;
					--numSamplesToProcess;
					--numKrSamples;
				}
			} 
			else
			{
				float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();
				
				while(numSamplesToProcess && numKrSamples)
				{
					*outputSamples++ = value;
					value += valueSlope;
					--numSamplesToProcess;
					--numKrSamples;
				}
				
				value = nextValue;
			}
		}
	}
	
	y1 = zap(y1);
}

LagUD::LagUD(UGen const& input, UGen const& lagTimeUp, UGen const& lagTimeDown) throw()
{
	UGen inputs[] = { input, lagTimeUp, lagTimeDown };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);		
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new LagUDUGenInternal(input, lagTimeUp, lagTimeDown);
		internalUGens[i]->initValue(input.getValue(i));
	}
}


END_UGEN_NAMESPACE

#endif // gpl