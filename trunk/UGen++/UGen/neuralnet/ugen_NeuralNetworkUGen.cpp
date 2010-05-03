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

#include "ugen_NeuralNetworkUGen.h"


NeuralNetworkUGenUGenInternal::NeuralNetworkUGenUGenInternal(UGen const& input, 
															 UGen const& trig, 
															 UGen const& target, 
															 UGen const& patternTrig, 
															 NeuralNetwork const& _network,
															 NeuralPatternArray const& _patterns) throw()
:	ProxyOwnerUGenInternal(NumInputs, _network.getNumOutputs()-1),
	network(_network),
	patterns(_patterns),
	inputVector(NumericalArray<float>::newClear(network.getNumInputs())),
	outputVector(network.propogate(inputVector)),
	targetVector(NumericalArray<float>::newClear(network.getNumOutputs())),
	lastTrig(0.f),
	lastPatternTrig(0.f),
	outputSampleData(new float*[network.getNumOutputs()]),
	inputSampleData(new float*[network.getNumInputs()]),
	targetSampleData(new float*[network.getNumOutputs()])
{
	inputs[Input] = input;
	inputs[Trig] = trig;
	inputs[Target] = target;
	inputs[PatternTrig] = patternTrig;
}

NeuralNetworkUGenUGenInternal::~NeuralNetworkUGenUGenInternal()
{
	delete [] outputSampleData;
	delete [] inputSampleData;
	delete [] targetSampleData;
}

void NeuralNetworkUGenUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	const float* trigSamples = inputs[Trig].processBlock(shouldDelete, blockID, 0);
	const float* patternTrigSamples = inputs[PatternTrig].processBlock(shouldDelete, blockID, 0);
	
	// get input pointers
	for(int input = 0; input < network.getNumInputs(); input++)
	{
		inputSampleData[input] = inputs[Input].processBlock(shouldDelete, blockID, input);
	}
	
	// get output and target pointers
	for(int output = 0; output < network.getNumOutputs(); output++)
	{
		outputSampleData[output] = proxies[output]->getSampleData();
		targetSampleData[output] = inputs[Target].processBlock(shouldDelete, blockID, output);
	}	
		
	for(int sample = 0; sample < blockSize; sample++)
	{
		float thisTrig = trigSamples[sample];
		float thisPatternTrig = patternTrigSamples[sample];
		
		if(thisTrig > 0.f && lastTrig <= 0.f)
		{
			for(int input = 0; input < network.getNumInputs(); input++)
			{
				inputVector[input] = inputSampleData[input][sample];
			}
			
			outputVector = network.propogate(inputVector);
		}
		
		if(thisPatternTrig > 0.f && lastPatternTrig <= 0.f)
		{
			for(int input = 0; input < network.getNumInputs(); input++)
			{
				inputVector[input] = inputSampleData[input][sample];
			}
			
			for(int output = 0; output < network.getNumOutputs(); output++)
			{
				targetVector[output] = targetSampleData[output][sample];
			}
			
			patterns.add(NeuralPattern(inputVector, targetVector));
		}
		
		lastTrig = thisTrig;
		lastPatternTrig = thisPatternTrig;
		
		for(int channel = 0; channel < getNumChannels(); channel++)
		{
			outputSampleData[channel][sample] = outputVector[channel];
		}
	}
}

NeuralNetworkUGen::NeuralNetworkUGen(UGen const& input, 
									 UGen const& trig, 
									 UGen const& target, 
									 UGen const& patternTrig, 
									 NeuralNetwork const& network,
									 NeuralPatternArray const& patterns) throw()
{	
	const int numNetworkInputs = network.getNumInputs();
	const int numNetworkOutputs = network.getNumOutputs();
	
	NeuralNetworkUGenUGenInternal *internal = 
		new NeuralNetworkUGenUGenInternal(input.withNumChannels(numNetworkInputs, false), 
										  trig.mix(),
										  target.withNumChannels(numNetworkOutputs, false), 
										  patternTrig.mix(), 
										  network, 
										  patterns);
	initInternal(numNetworkOutputs);
	generateFromProxyOwner(internal);
}



END_UGEN_NAMESPACE
