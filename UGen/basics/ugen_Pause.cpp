// $Id: ugen_Pause.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_Pause.cpp $

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

#include "ugen_Pause.h"
#include "ugen_InlineUnaryOps.h"



PauseUGenInternal::PauseUGenInternal(Pause_InputsWithTypesOnly) throw()
:	UGenInternal(NumInputs),
	prevLevel(0.f)
{
	inputs[Input] = input;
	inputs[Level] = level;
}

UGenInternal* PauseUGenInternal::getChannel(const int channel) throw()
{
	return new PauseUGenInternal(inputs[Input].getChannel(channel),
								 inputs[Level].getChannel(channel));
}

void PauseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	float currentLevel = *(inputs[Level].processBlock(shouldDelete, blockID, channel));
	
	if(currentLevel == 0.f && prevLevel == 0.f)
	{
		memset(outputSamples, 0, numSamplesToProcess * sizeof(float));
	}
	else
	{
		float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
		
		if(currentLevel != prevLevel)
		{
			float inc = (currentLevel - prevLevel) / (float)numSamplesToProcess;
			float level = prevLevel;
			while(numSamplesToProcess--)
			{
				*outputSamples++ = *inputSamples++ * level;
				level += inc;
			}
		}
		else
		{
			while(numSamplesToProcess--)
			{
				*outputSamples++ = *inputSamples++ * currentLevel;
			}
		}
	}
	
	prevLevel = currentLevel;
}

Pause::Pause(Pause_InputsWithTypesOnly) throw()
{
	UGen inputs[] = { Pause_InputsNoTypes };
	const int numInputChannels = findMaxInputChannels(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new PauseUGenInternal(Pause_InputsNoTypes);
		internalUGens[i]->initValue(input.getValue(i) * level.getValue(i));
	}
}

END_UGEN_NAMESPACE
