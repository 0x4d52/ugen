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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_BasicPan.h"


//=============================== LinPan2 ======================================


LinPan2UGenInternal::LinPan2UGenInternal(UGen const& input, UGen const& position, UGen const& level) throw()
:	ProxyOwnerUGenInternal(NumInputs, 1)
{
	inputs[Input] = input;
	inputs[Position] = position;
	inputs[Level] = level;
}

void LinPan2UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples0 = proxies[0]->getSampleData();
	float* outputSamples1 = proxies[1]->getSampleData();
	float* positionSamples = inputs[Position].processBlock(shouldDelete, blockID, channel);
	float* levelSamples = inputs[Level].processBlock(shouldDelete, blockID, channel);
	
	ugen_assert(*positionSamples >= -1.f && *positionSamples <= 1.f);
	
	if(inputs[Input].getNumChannels() <= 1)
	{
		float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);

		while(numSamplesToProcess--)
		{
			float pos1 = *positionSamples++ * 0.5f + 0.5f;
			float pos0 = 1.f - pos1;
			float inputValue = *inputSamples++ * *levelSamples++;
			
			*outputSamples0++ = inputValue * pos0;
			*outputSamples1++ = inputValue * pos1;
		}
	}
	else
	{
		float* inputSamples0 = inputs[Input].processBlock(shouldDelete, blockID, 0);
		float* inputSamples1 = inputs[Input].processBlock(shouldDelete, blockID, 1);
		
		while(numSamplesToProcess--)
		{
			float pos1 = *positionSamples++ * 0.5f + 0.5f;
			float pos0 = 1.f - pos1;
			float level = *levelSamples++;
			float inputValue0 = *inputSamples0++ * level;
			float inputValue1 = *inputSamples1++ * level;
			
			*outputSamples0++ = inputValue0 * pos0;
			*outputSamples1++ = inputValue1 * pos1;
		}		
	}
}


LinPan2::LinPan2(UGen const& input, UGen const& position, UGen const& level) throw()
{	
	initInternal(2);
	
	UGen positionChecked = position.mix();
	UGen levelChecked = level.mix();	
	
	LinPan2UGenInternal*  internal = new LinPan2UGenInternal(input, positionChecked, levelChecked);
	internalUGens[0] = internal;
	internalUGens[1] = internal->getProxy(1);
	
	float pos1 = positionChecked.getValue(0) * 0.5f + 0.5f;
	float pos0 = 1.f - pos1;
	
	if(input.getNumChannels() <= 1)
	{
		float inputValue = input.getValue(0) * levelChecked.getValue(0);
		internalUGens[0]->initValue(inputValue * pos0);
		internalUGens[1]->initValue(inputValue * pos1);	
	}
	else
	{
		float inputValue0 = input.getValue(0) * levelChecked.getValue(0);
		float inputValue1 = input.getValue(1) * levelChecked.getValue(0);
		internalUGens[0]->initValue(inputValue0 * pos0);
		internalUGens[1]->initValue(inputValue1 * pos1);	
	}
	
}


//=============================== Pan2 ======================================

Pan2UGenInternal::Pan2UGenInternal(UGen const& input, 
								   UGen const& position, 
								   UGen const& level) throw()
:	ProxyOwnerUGenInternal(NumInputs, 1),
	panTable(Buffer::getTableConstantPan512()),
	lastPos(position.getValue(0) * 0.5f + 0.5f),
	last0(panTable.lookup(0, lastPos)),
	last1(panTable.lookup(1, lastPos))
{
	inputs[Input] = input;
	inputs[Position] = position;
	inputs[Level] = level;	
}

void Pan2UGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{		
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples0 = proxies[0]->getSampleData();
	float* outputSamples1 = proxies[1]->getSampleData();
	float pos = *(inputs[Position].processBlock(shouldDelete, blockID, 0)) * 0.5f + 0.5f;
	float* levelSamples = inputs[Level].processBlock(shouldDelete, blockID, 0);
		
	if(pos == lastPos)
	{
		if(inputs[Input].getNumChannels() <= 1)
		{
			float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);
			
			while(numSamplesToProcess--)
			{
				float inputValue = *inputSamples++ * *levelSamples++;
				
				*outputSamples0++ = inputValue * last0;
				*outputSamples1++ = inputValue * last1;
			}
		}
		else 
		{
			float* inputSamples0 = inputs[Input].processBlock(shouldDelete, blockID, 0);
			float* inputSamples1 = inputs[Input].processBlock(shouldDelete, blockID, 1);
			
			while(numSamplesToProcess--)
			{
				float level = *levelSamples++;
				float inputValue0 = *inputSamples0++ * level;
				float inputValue1 = *inputSamples1++ * level;
				
				*outputSamples0++ = inputValue0 * last0;
				*outputSamples1++ = inputValue1 * last1;
			}
		}

	}
	else
	{
		float posInc = (pos - lastPos) / (float)numSamplesToProcess;
		
		if(inputs[Input].getNumChannels() <= 1)
		{
			float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, 0);

			while(numSamplesToProcess--)
			{
				lastPos += posInc;
				float inputValue = *inputSamples++ * *levelSamples++;
				
				last0 = panTable.lookup(0, lastPos);
				last1 = panTable.lookup(1, lastPos);
				
				*outputSamples0++ = inputValue * last0;
				*outputSamples1++ = inputValue * last1;
			}
		}
		else
		{
			float* inputSamples0 = inputs[Input].processBlock(shouldDelete, blockID, 0);
			float* inputSamples1 = inputs[Input].processBlock(shouldDelete, blockID, 1);
			
			while(numSamplesToProcess--)
			{
				lastPos += posInc;
				float level = *levelSamples++;
				float inputValue0 = *inputSamples0++ * level;
				float inputValue1 = *inputSamples1++ * level;
				
				last0 = panTable.lookup(0, lastPos);
				last1 = panTable.lookup(1, lastPos);
				
				*outputSamples0++ = inputValue0 * last0;
				*outputSamples1++ = inputValue1 * last1;
			}
		}
		
		lastPos = pos;
	}
}


Pan2::Pan2(UGen const& input, UGen const& position, UGen const& level) throw()
{		
	initInternal(2);
	
	UGen positionChecked = position.mix();
	UGen levelChecked = level.mix();
	
	Pan2UGenInternal*  internal = new Pan2UGenInternal(input, positionChecked, levelChecked);
	internalUGens[0] = internal;
	internalUGens[1] = internal->getProxy(1);
	
	float posValue = positionChecked.getValue(0) * 0.5f + 0.5f;
	float pos0 = internal->getTable().lookup(0, posValue);
	float pos1 = internal->getTable().lookup(1, posValue);
	
	if(input.getNumChannels() <= 1)
	{
		float inputValue = input.getValue(0) * levelChecked.getValue(0);
		internalUGens[0]->initValue(inputValue * pos0);
		internalUGens[1]->initValue(inputValue * pos1);	
	}
	else
	{
		float inputValue0 = input.getValue(0) * levelChecked.getValue(0);
		float inputValue1 = input.getValue(1) * levelChecked.getValue(0);
		internalUGens[0]->initValue(inputValue0 * pos0);
		internalUGens[1]->initValue(inputValue1 * pos1);	
	}
}



END_UGEN_NAMESPACE