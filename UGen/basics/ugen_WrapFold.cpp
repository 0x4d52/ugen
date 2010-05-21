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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_WrapFold.h"
#include "ugen_BinaryOpUGens.h"

WrapFoldBaseUGenInternal::WrapFoldBaseUGenInternal(WrapFold_InputsWithTypesAndDefaults) throw()
:	UGenInternal(NumInputs)
{
	inputs[Input] = input;
	inputs[Lower] = lower;
	inputs[Upper] = upper;
}

WrapUGenInternal::WrapUGenInternal(WrapFold_InputsWithTypesAndDefaults) throw()
:	WrapFoldBaseUGenInternal(WrapFold_InputsNoTypes)
{
}

UGenInternal* WrapUGenInternal::getChannel(const int channel) throw()
{
	return new WrapUGenInternal(inputs[Input].getChannel(channel),
								inputs[Lower].getChannel(channel),
								inputs[Upper].getChannel(channel));	
}

void WrapUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lowerSamples = inputs[Lower].processBlock(shouldDelete, blockID, channel);
	float* upperSamples = inputs[Upper].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = wrap(*inputSamples++, *lowerSamples++, *upperSamples++);
	}
}

FoldUGenInternal::FoldUGenInternal(WrapFold_InputsWithTypesAndDefaults) throw()
:	WrapFoldBaseUGenInternal(WrapFold_InputsNoTypes)
{
}

UGenInternal* FoldUGenInternal::getChannel(const int channel) throw()
{
	return new FoldUGenInternal(inputs[Input].getChannel(channel),
								inputs[Lower].getChannel(channel),
								inputs[Upper].getChannel(channel));		
}

void FoldUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float* lowerSamples = inputs[Lower].processBlock(shouldDelete, blockID, channel);
	float* upperSamples = inputs[Upper].processBlock(shouldDelete, blockID, channel);
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ = fold(*inputSamples++, *lowerSamples++, *upperSamples++);
	}	
}

Wrap::Wrap(WrapFold_InputsWithTypesOnly) throw()
{
	if((upper.getNumChannels() == 1) && upper.isConst(0) && (upper.getValue(0) == 0.f))
	{
		UGen const& rightOperand = lower; // is now actually upper!
		UGen inputs[] = { input, rightOperand };
		const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
		initInternal(numInputChannels);
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new BinaryWrapUGenInternal(input, rightOperand);
			internalUGens[i]->initValue(ugen::wrap(input.getValue(i), rightOperand.getValue(i)));
		}				
	}
	else
	{
		UGen inputs[] = { WrapFold_InputsNoTypes };
		const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
		initInternal(numInputChannels);
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new WrapUGenInternal(WrapFold_InputsNoTypes);
			internalUGens[i]->initValue(ugen::wrap(input.getValue(i), 
												   lower.getValue(i),
												   upper.getValue(i)));
		}
	}
}

Fold::Fold(WrapFold_InputsWithTypesOnly) throw()
{
	if((upper.getNumChannels() == 1) && upper.isConst(0) && (upper.getValue(0) == 0.f))
	{
		UGen const& rightOperand = lower; // is now actually upper!
		UGen inputs[] = { input, rightOperand };
		const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
		initInternal(numInputChannels);
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new BinaryFoldUGenInternal(input, rightOperand);
			internalUGens[i]->initValue(ugen::fold(input.getValue(i), rightOperand.getValue(i)));
		}		
	}
	else
	{		
		UGen inputs[] = { WrapFold_InputsNoTypes };
		const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
		initInternal(numInputChannels);
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = new FoldUGenInternal(WrapFold_InputsNoTypes);
			internalUGens[i]->initValue(ugen::fold(input.getValue(i), 
												   lower.getValue(i),
												   upper.getValue(i)));			
		}
	}
}


END_UGEN_NAMESPACE
