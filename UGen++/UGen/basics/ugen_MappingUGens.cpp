// $Id: ugen_MappingUGens.cpp 998 2010-03-08 10:33:23Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_MappingUGens.cpp $

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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_MappingUGens.h"
#include "../core/ugen_Constants.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"

MappingBaseUGenInternal::MappingBaseUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs)
{
	UGen inputArgs[] = { MappingBase_InputsNoTypes };
	for(int i = 0; i < NumInputs; i++)
	{
		inputs[i] = inputArgs[i];
	}
}

LinExpSignalUGenInternal::LinExpSignalUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinExpSignalUGenInternal::getChannel(const int channel) throw()
{
	return new LinExpSignalUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinExpSignalUGenInternal::getKr() throw()
{ 
	return new LinExpSignalUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

void LinExpSignalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = linexp(*inputSamples++, 
								  *inLowSamples++, *inHighSamples++,
								  *outLowSamples++, *outHighSamples++);
	}
}


void LinExpSignalUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{
			nextValue = linexp(*inputSamples, 
							   *inLowSamples, *inHighSamples,
							   *outLowSamples, *outHighSamples);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		inLowSamples	+= numKrSamples;
		inHighSamples	+= numKrSamples;
		outLowSamples	+= numKrSamples;
		outHighSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

LinExpScalarUGenInternal::LinExpScalarUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinExpScalarUGenInternal::getChannel(const int channel) throw()
{
	return new LinExpScalarUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinExpScalarUGenInternal::getKr() throw()
{ 
	return new LinExpScalarUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

void LinExpScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float outRatio = outHighValue / outLowValue;
	float reciprocalInRange = 1.f / (inHighValue - inLowValue);
	float negInLowOverInRange = reciprocalInRange * -inLowValue;
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = linexp2(*inputSamples++, 
								   reciprocalInRange, negInLowOverInRange,
								   outLowValue, outRatio);
	}
}



void LinExpScalarUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float outRatio = outHighValue / outLowValue;
	float reciprocalInRange = 1.f / (inHighValue - inLowValue);
	float negInLowOverInRange = reciprocalInRange * -inLowValue;
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{
			nextValue = linexp2(*inputSamples, 
								reciprocalInRange, negInLowOverInRange, 
								outLowValue, outRatio);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

LinLinSignalUGenInternal::LinLinSignalUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinLinSignalUGenInternal::getChannel(const int channel) throw()
{
	return new LinLinSignalUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinLinSignalUGenInternal::getKr() throw()
{ 
	return new LinLinSignalUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void LinLinSignalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = linlin(*inputSamples++,
								  *inLowSamples++, *inHighSamples++,
								  *outLowSamples++, *outHighSamples++);
	}
}
#endif

void LinLinSignalUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{			
			nextValue = linlin(*inputSamples,
							   *inLowSamples, *inHighSamples,
							   *outLowSamples, *outHighSamples);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		inLowSamples	+= numKrSamples;
		inHighSamples	+= numKrSamples;
		outLowSamples	+= numKrSamples;
		outHighSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

LinLinScalarUGenInternal::LinLinScalarUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinLinScalarUGenInternal::getChannel(const int channel) throw()
{
	return new LinLinScalarUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinLinScalarUGenInternal::getKr() throw()
{ 
	return new LinLinScalarUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

#if !defined(UGEN_VFP) && !defined(UGEN_NEON)
void LinLinScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	float outRange = outHighValue - outLowValue;
	
	while(numSamplesToProcess--)
	{		
		*outputSamples++ = linlin2(*inputSamples++, inLowValue, inRange, outLowValue, outRange);
	}
}
#endif

void LinLinScalarUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	float outRange = outHighValue - outLowValue;
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;

	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{
			nextValue = linlin2(*inputSamples, inLowValue, inRange, outLowValue, outRange);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

LinSinSignalUGenInternal::LinSinSignalUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinSinSignalUGenInternal::getChannel(const int channel) throw()
{
	return new LinSinSignalUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinSinSignalUGenInternal::getKr() throw()
{ 
	return new LinSinSignalUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

void LinSinSignalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = linsin(*inputSamples++, 
								  *inLowSamples++, *inHighSamples++,
								  *outLowSamples++, *outHighSamples++);
	}
}

void LinSinSignalUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	float* outLowSamples = inputs[OutLow].processBlock(shouldDelete, blockID, channel);
	float* outHighSamples = inputs[OutHigh].processBlock(shouldDelete, blockID, channel);
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{
			nextValue = linsin(*inputSamples, 
							   *inLowSamples, *inHighSamples,
							   *outLowSamples, *outHighSamples);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		inLowSamples	+= numKrSamples;
		inHighSamples	+= numKrSamples;
		outLowSamples	+= numKrSamples;
		outHighSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

LinSinScalarUGenInternal::LinSinScalarUGenInternal(MappingBase_InputsWithTypesOnly) throw()
:	MappingBaseUGenInternal(MappingBase_InputsNoTypes)
{
}

UGenInternal* LinSinScalarUGenInternal::getChannel(const int channel) throw()
{
	return new LinSinScalarUGenInternal(inputs[Input].getChannel(channel),
										inputs[InLow].getChannel(channel),
										inputs[InHigh].getChannel(channel),
										inputs[OutLow].getChannel(channel),
										inputs[OutHigh].getChannel(channel));
}

UGenInternal* LinSinScalarUGenInternal::getKr() throw()
{ 
	return new LinSinScalarUGenInternalK(inputs[Input].kr(), 
										 inputs[InLow].kr(), 
										 inputs[InHigh].kr(), 
										 inputs[OutLow].kr(), 
										 inputs[OutHigh].kr()); 
}

void LinSinScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	float outRange = outHighValue - outLowValue;
	
	while(numSamplesToProcess--)
	{		
		*outputSamples++ = linsin2(*inputSamples++, 
								   inLowValue, inRange,
								   outLowValue, outRange);
	}
}

void LinSinScalarUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float outLowValue = *(inputs[OutLow].processBlock(shouldDelete, blockID, channel));
	float outHighValue = *(inputs[OutHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	float outRange = outHighValue - outLowValue;
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{			
			nextValue = linsin2(*inputSamples, 
								inLowValue, inRange,
								outLowValue, outRange);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

MapTableBaseUGenInternal::MapTableBaseUGenInternal(MapTableBase_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs),
	table_(table),
	tableSize1(table_.size()-1)
{
	inputs[Input] = input;
	inputs[InLow] = inLow;
	inputs[InHigh] = inHigh;
}

MapTableSignalUGenInternal::MapTableSignalUGenInternal(MapTableBase_InputsWithTypesOnly) throw()
:	MapTableBaseUGenInternal(MapTableBase_InputsNoTypes)
{
}

UGenInternal* MapTableSignalUGenInternal::getChannel(const int channel) throw()
{
	return new MapTableSignalUGenInternal(inputs[Input].getChannel(channel), 
										  inputs[InLow].getChannel(channel), 
										  inputs[InHigh].getChannel(channel), 
										  table_.getChannel(channel % table_.getNumChannels()));
}

UGenInternal* MapTableSignalUGenInternal::getKr() throw()
{
	return new MapTableSignalUGenInternalK(inputs[Input].kr(), 
										   inputs[InLow].kr(), 
										   inputs[InHigh].kr(), 
										   table_);
}

void MapTableSignalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		float inLowValue = *inLowSamples++;
		float inHighValue = *inHighSamples++;
		float inRange = inHighValue - inLowValue;
		
		float input = *inputSamples++;
		float tableIndex = (input - inLowValue) * tableSize1 / inRange;
		
		*outputSamples++ = table_.getSampleUnchecked(channel, tableIndex);
	}
}

void MapTableSignalUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* inLowSamples = inputs[InLow].processBlock(shouldDelete, blockID, channel);
	float* inHighSamples = inputs[InHigh].processBlock(shouldDelete, blockID, channel);
	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)
		{
			float inLowValue = *inLowSamples;
			float inHighValue = *inHighSamples;
			float inRange = inHighValue - inLowValue;
			
			float input = *inputSamples;
			float tableIndex = (input - inLowValue) * tableSize1 / inRange;
			
			nextValue = table_.getSampleUnchecked(channel, tableIndex);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		inLowSamples	+= numKrSamples;
		inHighSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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

MapTableScalarUGenInternal::MapTableScalarUGenInternal(MapTableBase_InputsWithTypesOnly) throw()
:	MapTableBaseUGenInternal(MapTableBase_InputsNoTypes)
{
}

UGenInternal* MapTableScalarUGenInternal::getChannel(const int channel) throw()
{
	return new MapTableScalarUGenInternal(inputs[Input].getChannel(channel), 
										  inputs[InLow].getChannel(channel), 
										  inputs[InHigh].getChannel(channel), 
										  table_.getChannel(channel % table_.getNumChannels()));
}

UGenInternal* MapTableScalarUGenInternal::getKr() throw()
{
	return new MapTableScalarUGenInternalK(inputs[Input].kr(), 
										   inputs[InLow].kr(), 
										   inputs[InHigh].kr(), 
										   table_);
}

void MapTableScalarUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	const int tableChannel = channel % table_.getNumChannels();
	
	while(numSamplesToProcess--)
	{
		float input = *inputSamples++;
		float tableIndex = (input - inLowValue) * tableSize1 / inRange;
		
		*outputSamples++ = table_.getSampleUnchecked(tableChannel, tableIndex);
	}
}

void MapTableScalarUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float inLowValue = *(inputs[InLow].processBlock(shouldDelete, blockID, channel));
	float inHighValue = *(inputs[InHigh].processBlock(shouldDelete, blockID, channel));
	float inRange = inHighValue - inLowValue;
	const int tableChannel = channel % table_.getNumChannels();

	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		if(numKrSamples == 0)
		{
			float input = *inputSamples;
			float tableIndex = (input - inLowValue) * tableSize1 / inRange;
			
			nextValue = table_.getSampleUnchecked(tableChannel, tableIndex);
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition	+= numKrSamples;
		inputSamples	+= numKrSamples;
		
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
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
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



LinExp::LinExp(MappingBase_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { MappingBase_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
		
	bool hasScalarRanges =	inLow.isScalar() && 
							inHigh.isScalar() && 
							outLow.isScalar() && 
							outHigh.isScalar();
			
	if(hasScalarRanges == true)
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinExpScalarUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linexp(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));
		}		
	}
	else
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinExpSignalUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linexp(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));			
		}
	}
}

LinLin::LinLin(MappingBase_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { MappingBase_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	bool hasScalarRanges =	inLow.isScalar() && 
							inHigh.isScalar() && 
							outLow.isScalar() && 
							outHigh.isScalar();
	
	if(hasScalarRanges == true)
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinLinScalarUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linlin(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));
			
		}		
	}
	else
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinLinSignalUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linlin(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));
			
		}
	}
}

LinSin::LinSin(MappingBase_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { MappingBase_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	bool hasScalarRanges =	inLow.isScalar() && 
							inHigh.isScalar() && 
							outLow.isScalar() && 
							outHigh.isScalar();
	
	if(hasScalarRanges == true)
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinSinScalarUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linsin(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));			
		}		
	}
	else
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new LinSinSignalUGenInternal(MappingBase_InputsNoTypes);
			internalUGens[i]->initValue(ugen::linsin(input.getValue(i),
													 inLow.getValue(i), inHigh.getValue(i),
													 outLow.getValue(i), outHigh.getValue(i)));
			
		}
	}
}

Map::Map(MapTableBase_InputsWithTypesAndDefaults) throw()
{
	UGen inputs[] = { input, inLow, inHigh };
	int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	if(table.getNumChannels() > numInputChannels)
		numInputChannels = table.getNumChannels();
	
	initInternal(numInputChannels);

	bool hasScalarRanges =	inLow.isScalar() && inHigh.isScalar();
	
	if(hasScalarRanges == true)
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new MapTableScalarUGenInternal(MapTableBase_InputsNoTypes);
			
			float inLowValue	= inLow.getValue(i);
			float inRange		= inHigh.getValue(i) - inLowValue;
			float tableIndex	= (input.getValue(i) - inLowValue) * table.size() / inRange;
			
			internalUGens[i]->initValue(table.getSampleUnchecked(i, tableIndex));
		}		
	}
	else
	{
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new MapTableSignalUGenInternal(MapTableBase_InputsNoTypes);
			
			float inLowValue	= inLow.getValue(i);
			float inRange		= inHigh.getValue(i) - inLowValue;
			float tableIndex	= (input.getValue(i) - inLowValue) * table.size() / inRange;
			
			internalUGens[i]->initValue(table.getSampleUnchecked(i, tableIndex));
		}
	}
}


END_UGEN_NAMESPACE
