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

#include "ugen_DataRecorder.h"


DataRecorderUGenInternal::DataRecorderUGenInternal(UGen const& input, 
												   UGen const& trig, 
												   Text const& file, 
												   const bool _timeStamp) throw()
:	ProxyOwnerUGenInternal(NumInputs, input.getNumChannels()-1),
	fileWriter(file),
	lastTrig(0.f),
	timeStamp(_timeStamp)
{
	inputs[Input] = input;
	inputs[Trig] = trig;
}

void DataRecorderUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	const float *trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
	
	for(int i = 0; i < numSamplesToProcess; i++)
	{
		float thisTrig = trigSamples[i];
		
		if(thisTrig > 0.f && lastTrig <= 0.f)
		{
			if(timeStamp == true)
			{
				fileWriter.writeValue(blockID+i);
				fileWriter.write(" ");
			}
				
			for(int channel = 0; channel < inputs[Input].getNumChannels(); channel++)
			{
				float *values = inputs[Input].processBlock(shouldDelete, blockID, channel);
				float value = values[i];
				fileWriter.writeValue(value);
				fileWriter.write(" ");
			}
			
			fileWriter.write("\n");
		}
		
		lastTrig = thisTrig;
	}
	
	for(int channel = 0; channel < inputs[Input].getNumChannels(); channel++)
	{
		float *inputValues = inputs[Input].processBlock(shouldDelete, blockID, channel);
		float *outputValues = proxies[channel]->getSampleData();
		memcpy(outputValues, inputValues, numSamplesToProcess * sizeof(float));
	}
}

DataRecorder::DataRecorder(UGen const& input, UGen const& trig, Text const& file, const bool timeStamp) throw()
{
	DataRecorderUGenInternal *internal = new DataRecorderUGenInternal(input, trig.mix(), file, timeStamp);
	initInternal(input.getNumChannels());
	generateFromProxyOwner(internal);	
}

END_UGEN_NAMESPACE
