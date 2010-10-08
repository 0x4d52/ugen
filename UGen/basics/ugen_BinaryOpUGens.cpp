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

// need to check gpl 

#include <math.h>
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "../core/ugen_UGenArray.h"
#include "../buffers/ugen_Buffer.h"
#include "ugen_BinaryOpUGens.h"
#include "ugen_UnaryOpUGens.h"

BinaryOpUGenInternal::BinaryOpUGenInternal(UGen const& leftOperand, UGen const& rightOperand, int channel) throw()
:	UGenInternal(NumInputs)
{	
	if(channel < 0)
	{
		inputs[LeftOperand] = leftOperand;
		inputs[RightOperand] = rightOperand;
	}
	else
	{
		int numLeftChannels = leftOperand.getNumChannels();
		int numRightChannels = rightOperand.getNumChannels();
		UGenInternal* leftInternal = leftOperand.getInternalUGen(channel % numLeftChannels);
		UGenInternal* rightInternal = rightOperand.getInternalUGen(channel % numRightChannels);
		
		inputs[LeftOperand] = UGen(leftInternal->getChannelInternal(channel), channel);
		inputs[RightOperand] = UGen(rightInternal->getChannelInternal(channel), channel);
	}
}

BinaryOpValueInternal::BinaryOpValueInternal(Value const& leftOperand, Value const& rightOperand) throw()
:	leftOperand_(leftOperand), rightOperand_(rightOperand)
{
}

// using vector ops these might be defined elsewhere...
#if defined(UGEN_VFP) || defined(UGEN_NEON) || defined(UGEN_VDSP)
BinaryOpSymbolUGenDefinitionNoProcessBlock(Add,				+,	+);
BinaryOpSymbolUGenDefinitionNoProcessBlock(Subtract,		-,	-);
BinaryOpSymbolUGenDefinitionNoProcessBlock(Multiply,		*,	*);
#else
BinaryOpSymbolUGenDefinition(Add,					+,	+);
BinaryOpSymbolUGenDefinition(Subtract,				-,	-);
BinaryOpSymbolUGenDefinition(Multiply,				*,	*);
#endif

BinaryOpSymbolUGenDefinition(LessThan,				<,	<);
BinaryOpSymbolUGenDefinition(GreaterThan,			>,	>);
BinaryOpSymbolUGenDefinition(LessThanOrEquals,		<=, <=);
BinaryOpSymbolUGenDefinition(GreaterThanOrEquals,	>=, >=);
//BinaryOpSymbolUGenDefinition(Equal,					==, ==);
//BinaryOpSymbolUGenDefinition(NotEqual,				!=, !=);

BinaryOpFunctionUGenDefinition(IsEqualTo,		isEqualTo,		isEqualTo);
BinaryOpFunctionUGenDefinition(IsNotEqualTo,	isNotEqualTo,	isNotEqualTo);

BinaryOpFunctionUGenDefinition(Pow,			pow,		pow);
BinaryOpFunctionUGenDefinition(Hypot,		hypot,		hypot);
BinaryOpFunctionUGenDefinition(Atan2,		atan2,		atan2);
BinaryOpFunctionUGenDefinition(Min,			min,		min);
BinaryOpFunctionUGenDefinition(Max,			max,		max);

BinaryOpFunctionUGenDefinition(Clip2,		clip2,		clip2);

#ifndef UGEN_NOEXTGPL
BinaryOpFunctionUGenDefinition(Wrap,		wrap,		wrap);
BinaryOpFunctionUGenDefinition(Wrap2,		wrap2,		wrap2);
BinaryOpFunctionUGenDefinition(Fold,		fold,		fold);
BinaryOpFunctionUGenDefinition(Fold2,		fold2,		fold2)
BinaryOpFunctionUGenDefinition(Ring1,		ring1,		ring1);
BinaryOpFunctionUGenDefinition(Ring2,		ring2,		ring2);
BinaryOpFunctionUGenDefinition(Ring3,		ring3,		ring3);
BinaryOpFunctionUGenDefinition(Ring4,		ring4,		ring4)
BinaryOpFunctionUGenDefinition(Round,		round,		round);
BinaryOpFunctionUGenDefinition(Trunc,		trunc,		trunc);
BinaryOpFunctionUGenDefinition(SumSqr,		sumsqr,		sumsqr);
BinaryOpFunctionUGenDefinition(DifSqr,		difsqr,		difsqr);
BinaryOpFunctionUGenDefinition(SqrSum,		sqrsum,		sqrsum);
BinaryOpFunctionUGenDefinition(SqrDif,		sqrdif,		sqrdif);
BinaryOpFunctionUGenDefinition(AbsDif,		absdif,		absdif);
BinaryOpFunctionUGenDefinition(Thresh,		thresh,		thresh);
#endif // gpl

// special case for division... partly to demonstrate what these marcos output but
// also to avoid some divide by zero warnings

BinaryDivideUGenInternal::BinaryDivideUGenInternal(UGen const& leftOperand, UGen const& rightOperand) throw() 
:	BinaryOpUGenInternal(leftOperand, rightOperand) 
{ 
} 

UGenInternal* BinaryDivideUGenInternal::getChannel(const int channel) throw() 
{ 
	return new BinaryDivideUGenInternal(inputs[LeftOperand].getChannel(channel),
										inputs[RightOperand].getChannel(channel)); 
} 

UGenInternal* BinaryDivideUGenInternal::getKr() throw() 
{ 
	return new BinaryDivideUGenInternalK(inputs[LeftOperand].kr(), inputs[RightOperand].kr()); 
} 

BinaryDivideUGenInternalK::BinaryDivideUGenInternalK(UGen const& leftOperand, UGen const& rightOperand) throw() 
:	BinaryDivideUGenInternal(leftOperand, rightOperand), 
	value(0.f) 
{ 
	rate = ControlRate; 
} 

BinaryDivideUGen::BinaryDivideUGen(UGen const& leftOperand, UGen const& rightOperand) throw() 
{ 	
	const int numRightChannels = rightOperand.getNumChannels();
	const int numLeftChannels = leftOperand.getNumChannels();
	
	if(numLeftChannels > numRightChannels) 
		initInternal(numLeftChannels); 
	else 
		initInternal(numRightChannels); 
	
	for(int i = 0; i < numInternalUGens; i++) 
	{ 
		const int rightIndex = i % numRightChannels;
		if(rightOperand.isConst(rightIndex))
		{
			// special case where the right operand is a scalar
			// use multiplication by the reciprocal instead
			float reciprocalRightOperand = 1.f / rightOperand.getValue(rightIndex);
			internalUGens[i] = new BinaryMultiplyUGenInternal(leftOperand, reciprocalRightOperand); 
		}
		else
			internalUGens[i] = new BinaryDivideUGenInternal(leftOperand, rightOperand); 
	}
} 

#if !defined(UGEN_VFP) && !defined(UGEN_NEON) && !defined(UGEN_VDSP)
void BinaryDivideUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID, channel); 
	float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID, channel); 
	
	while(numSamplesToProcess--) 
	{ 
		// maybe should "zap" the output?
		*outputSamples++ = *leftOperandSamples++ / *rightOperandSamples++; 
	}
} 
#endif

void BinaryDivideUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int krBlockSize = UGen::getControlRateBlockSize(); 
	unsigned int blockPosition = blockID % krBlockSize; 
	int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 
	float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID, channel); 
	float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID, channel); 
	int numKrSamples = blockPosition % krBlockSize; 
	
	while(numSamplesToProcess > 0) 
	{ 
		if(numKrSamples == 0) 
			value = *leftOperandSamples / *rightOperandSamples; // maybe should "zap" the output?
		
		numKrSamples		=  krBlockSize - numKrSamples; 
		blockPosition		+= numKrSamples; 
		leftOperandSamples	+= numKrSamples; 
		rightOperandSamples += numKrSamples; 
	
		while(numSamplesToProcess && numKrSamples) 
		{ 
			*outputSamples++ = value; 
			--numSamplesToProcess; 
			--numKrSamples; 
		} 
	}
} 

UGen operator / (const float leftOperand, UGen const& rightOperand) throw() 
{ 
	if(rightOperand.isNull()) 
	{
		ugen_assertfalse;
		return UGen(leftOperand);
	}
	else 
		return UGen(leftOperand) / rightOperand; 
} 

UGen UGen::operator / (UGen const& rightOperand) const throw() 
{ 
	if(isNull())
	{
		ugen_assertfalse;
		return rightOperand; 
	}
	else 
		return BinaryDivideUGen(*this, rightOperand); 
} 

UGenArray UGenArray::operator / (UGenArray const& rightOperand) const throw() 
{ 
	int size; 
	if(internal->size() > rightOperand.internal->size()) 
		size = internal->size(); 
	else 
		size = rightOperand.internal->size(); 

	UGenArray newArray(size); 

	for(int i = 0; i < size; i++) 
	{ 
		newArray.internal->getArray()[i] = wrapAt(i) / rightOperand.wrapAt(i); 
	} 

	return newArray; 
} 

Buffer Buffer::operator / (Buffer const& rightOperand) const throw() 
{ 
	const int newNumChannels = numChannels_ > rightOperand.numChannels_ ? numChannels_ : rightOperand.numChannels_; 
	const int maxSize = size_ > rightOperand.size_ ? size_ : rightOperand.size_; 
	const int minSize = size_ < rightOperand.size_ ? size_ : rightOperand.size_; 
	const int diffSize = maxSize-minSize; 
	
	Buffer newBuffer(maxSize, newNumChannels, false); 
	
	for(int channelIndex = 0; channelIndex < newNumChannels; channelIndex++) 
	{ 
		int numSamples; 
		float* leftSamples = channels[channelIndex % numChannels_]->data; 
		float* rightSamples = rightOperand.channels[channelIndex % rightOperand.numChannels_]->data; 
		float* outputSamples = newBuffer.channels[channelIndex]->data; 
		
		if(size_ == 1 && numChannels_ == 1) 
		{ 
			numSamples = maxSize; 
			while(--numSamples >= 0) 
			{ 
				*outputSamples++ = *leftSamples / *rightSamples++; 
			} 
		} 
		else if(rightOperand.size_ == 1 && rightOperand.numChannels_ == 1) 
		{ 
			float reciprocalRightOperand = 1.f / *rightSamples;
			numSamples = maxSize; 
			while(--numSamples >= 0) 
			{ 
				*outputSamples++ = *leftSamples++ * reciprocalRightOperand; 
			} 
		} 
		else 
		{ 
			numSamples = minSize; 
			while(--numSamples >= 0) 
			{ 
				*outputSamples++ = *leftSamples++ / *rightSamples++; 
			} 
			
			numSamples = diffSize; 
			while(--numSamples >= 0) 
			{ 
				*outputSamples++ = 0.f; 
			} 
		} 
	} 

	return newBuffer; 
} 

Buffer operator / (const float leftOperand, Buffer const& rightOperand) throw() 
{ 
	if(rightOperand.isNull()) 
	{
		ugen_assertfalse;
		return Buffer(leftOperand); 
	}
	else 
		return Buffer(leftOperand) / rightOperand; 
}

Value operator / (const double leftOperand, Value const& rightOperand) throw()
{	
	return Value(leftOperand) / rightOperand;
}

Value Value::operator / (Value const& rightOperand) const throw()
{
	Value TempValue = *this; // copy away const
	if(containsInternalType<ValueInternal>() && (TempValue.getValue() == 1.0))
	{
		if(rightOperand.containsInternalType<UnaryReciprocalValueInternal>())
			return (dynamic_cast<UnaryReciprocalValueInternal*>(rightOperand.internal))->getOperand();
		else
			return rightOperand.reciprocal();
	}
	else	
		return BinaryDivideValue(*this, rightOperand); // default
}

ValueArray operator / (const double leftOperand, ValueArray const& rightOperand) throw()
{
	return ValueArray(leftOperand) / rightOperand;
}

ValueArray ValueArray::operator / (ValueArray const& rightOperand) const throw()
{
	if(size() == 0)																									
		return rightOperand;																						
	else if(rightOperand.size() == 0)																				
		return *this;																								
	else {																										
		const int maxSize = ugen::max(size(), rightOperand.size());													
		ValueArray newArray(maxSize);																				
		for(int i = 0; i < maxSize; i++) {																			
			newArray.put(i, wrapAt(i) / rightOperand.wrapAt(i));												
		}																											
		return newArray;																							
	}	
}

BinaryDivideValueInternal::BinaryDivideValueInternal(Value const& leftOperand, Value const& rightOperand) throw()
: BinaryOpValueInternal(leftOperand, rightOperand) 
{ 
}

double BinaryDivideValueInternal::getValue() throw()
{
	return leftOperand_.getValue() / rightOperand_.getValue();
}

BinaryDivideValue::BinaryDivideValue(Value const& leftOperand, Value const& rightOperand) throw()
:	Value(new BinaryDivideValueInternal(leftOperand, rightOperand))
{
}


IngoreRightOperandUGenInternal::IngoreRightOperandUGenInternal(UGen const& leftOperand, UGen const& rightOperand) throw()
:	BinaryOpUGenInternal(leftOperand, rightOperand) 
{
}

UGenInternal* IngoreRightOperandUGenInternal::getChannel(const int channel) throw()
{
	return new IngoreRightOperandUGenInternal(inputs[LeftOperand].getChannel(channel),
											  inputs[RightOperand].getChannel(channel)); 
}

void IngoreRightOperandUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* outputSamples = uGenOutput.getSampleData(); 	
	inputs[RightOperand].processBlock(shouldDelete, blockID); 
	inputs[LeftOperand].setOutput(outputSamples, numSamplesToProcess, channel);
	inputs[LeftOperand].processBlock(shouldDelete, blockID, channel); 
	//inputs[RightOperand].processBlock(shouldDelete, blockID); 
}

IngoreRightOperandUGen::IngoreRightOperandUGen(UGen const& leftOperand, UGen const& rightOperand) throw()
{
	initInternal(leftOperand.getNumChannels()); 
	for(int i = 0; i < numInternalUGens; i++) 
	{ 
		internalUGens[i] = new IngoreRightOperandUGenInternal(leftOperand, rightOperand); 
	}	
}



END_UGEN_NAMESPACE