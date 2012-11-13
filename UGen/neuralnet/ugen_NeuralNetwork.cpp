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

#include "ugen_NeuralNetwork.h"

const float NeuralNetworkSimpleInternal::defaultLearnRate = 0.25f;
const float NeuralNetworkSimpleInternal::defaultActFuncOffset = 0.01f;

NeuralNetworkSimpleInternal::NeuralNetworkSimpleInternal(IntArray const& numNodes, 
														 const float _learnRate, 
														 const float _actFuncOffset) throw()
:	learnRate(_learnRate),
	actFuncOffset(_actFuncOffset),
	layers((numNodes.size() < 2 ? 2 : numNodes.size())-1)
{
	const int numLayers = numNodes.size();
	
	if(getNumLayersIncludingInput() == numLayers)
	{
		const int size = getNumLayersExcludingInput();
		for(int i = 0; i < size; i++)
		{
			int numLayerInputs = numNodes.atUnchecked(i);
			int numLayerOutputs = numNodes.atUnchecked(i+1);
			
			if(numLayerInputs < 1) numLayerInputs = 1;
			if(numLayerOutputs < 1) numLayerOutputs = 1;
			
			layers[i] = NeuralLayer(numLayerOutputs, numLayerInputs);
		}
		
		numInputs = numNodes.atUnchecked(0);
		numOutputs = numNodes.atUnchecked(numLayers-1);
	}
	else if(numLayers == 1)
	{
		int num = numNodes.atUnchecked(0);
		if(num < 1) num = 1;
		
		layers[0] = NeuralLayer(num, num);
		layers[1] = NeuralLayer(num, num);

		numInputs = num;
		numOutputs = num;
	}
	else
	{
		layers[0] = NeuralLayer(1, 1);
		layers[1] = NeuralLayer(1, 1);
		
		numInputs = 1;
		numOutputs = 1;
	}
	
	errorVector = NumericalArray<float>::withSize(numOutputs);
}

IntArray NeuralNetworkSimpleInternal::getStructure() const throw()
{
	IntArray structure = IntArray::withSize(getNumLayersIncludingInput(), false);
	
	structure[0] = getNumInputs();
	for(int i = 1; i < structure.size(); i++)
	{
		structure[i] = getNumNodesOnLayer(i-1);
	}
	
	return structure;
}

void NeuralNetworkSimpleInternal::init(const float weightMaximum) throw()
{
	const int size = getNumLayersExcludingInput();
	for(int i = 0; i < size; i++)
	{
		layers[i].init(weightMaximum);
	}	
}

void NeuralNetworkSimpleInternal::randomise(const float amount) throw()
{
	const int size = getNumLayersExcludingInput();
	for(int i = 0; i < size; i++)
	{
		layers[i].randomise(amount);
	}		
}

void NeuralNetworkSimpleInternal::set(const int layer, 
									  const int node, 
									  NumericalArray<float> const& weightVector, 
									  const float threshold) throw()
{
	layers[layer].set(node, weightVector, threshold);
}

void NeuralNetworkSimpleInternal::setThreshold(const int layer, 
											   const int node, 
											   const float threshold) throw()
{
	layers[layer].setThreshold(node, threshold);
}

void NeuralNetworkSimpleInternal::setWeight(const int layer, 
											const int node, 
											const int weightIndex, 
											const float weight) throw()
{
	layers[layer].setWeight(node, weightIndex, weight);
}

void NeuralNetworkSimpleInternal::get(const int layer, 
									  const int node, 
									  NumericalArray<float> *weightVector, 
									  float& threshold) const throw()
{
	layers[layer].get(node, weightVector, threshold); ///xxx
}

void NeuralNetworkSimpleInternal::write(TextFileWriter const& _file) const throw()
{
	const int intFieldWidth = 4;
	const int size = 128;
	char buf[size];
	
	TextFileWriter file = _file;
	
	file.write("NeuralNetworkSimple:v1\n");
	
	snprintf(buf, size, "learnRate: %f actFuncOffset: %f\n", getLearnRate(), getActFuncOffset());
	file.write(buf);
	
	snprintf(buf, size, "Layers:%*d\n", intFieldWidth, getNumLayersIncludingInput());
	file.write(buf);
	
	snprintf(buf, size, "Layer %*d:%*d\n", intFieldWidth, 0, 
										   intFieldWidth, numInputs);
	file.write(buf);
	
	for(int layer = 0; layer < getNumLayersExcludingInput(); layer++)
	{
		snprintf(buf, size, "Layer %*d:%*d\n", intFieldWidth, layer+1, 
											   intFieldWidth, getNumNodesOnLayer(layer));
		file.write(buf);
	}
		
	for(int layer = 0; layer < getNumLayersExcludingInput(); layer++)
	{
		const int numNodes = getNumNodesOnLayer(layer);
		for(int node = 0; node < numNodes; node++)
		{
			NumericalArray<float> weights;
			float threshold;
			get(layer, node, &weights, threshold);
				
			snprintf(buf, size, "%*d %*d %*d   %.16f\n", intFieldWidth, layer, 
														 intFieldWidth, node, 
														 intFieldWidth, -1, 
														 threshold);
			file.write(buf);
			
			const int numWeights = weights.size();
			for(int weight = 0; weight < numWeights; weight++)
			{
				snprintf(buf, size, "%*d %*d %*d   %.16f\n", intFieldWidth, layer, 
															 intFieldWidth, node, 
															 intFieldWidth, weight, 
															 weights[weight]);
				file.write(buf);
			}
		}
	}
	
}

NumericalArray<float> NeuralNetworkSimpleInternal::propogate(NumericalArray<float> const& inputVector) throw()
{
	NumericalArray<float> vector = inputVector;
	
	NeuralLayer *layersPtr = layers.getArray();
	const int size = getNumLayersExcludingInput();
	for(int i = 0; i < size; i++)
	{
		vector = layersPtr[i].propogate(vector);
	}	
	
	return vector;
}

void NeuralNetworkSimpleInternal::backProp(NumericalArray<float> const& inputVector, NumericalArray<float> const& targetVector) throw()
{
	NumericalArray<float> outputVector = propogate(inputVector);	

	float* errorVectorPtr = errorVector.getArray();
	const float* targetVectorPtr = targetVector.getArray();
	const float* outputVectorPtr = outputVector.getArray();
	
	const int size = errorVector.size();
	for(int i = 0; i < size; i++)
	{
		errorVectorPtr[i] = targetVectorPtr[i] - outputVectorPtr[i];
	}
	
	NumericalArray<float> vector = errorVector;
	
	NeuralLayer *layersPtr = layers.getArray();
	for(int i = getNumLayersExcludingInput()-1; i >= 0; i--)
	{
		vector = layersPtr[i].backProp(vector, actFuncOffset, learnRate);
	}
}

void NeuralNetworkSimpleInternal::train(NeuralPatternArray const& patterns, const int count) throw()
{
	const NeuralPattern *patternPtr = patterns.getArray();
	
	if(patternPtr != 0)
	{
		for(int iteration = 0; iteration < count; iteration++)
		{
			const int numPatterns = patterns.size();
			for(int patternIndex = 0; patternIndex < numPatterns; patternIndex++)
			{
				const NeuralPattern& pattern = patternPtr[patternIndex];
				
				if(pattern.getInternal() != 0)
					backProp(pattern.getInputVector(), pattern.getOutputVector());
			}
		}
	}
}

void NeuralNetwork::read(TextFileReader const& _file) throw()
{
	TextFileReader file = _file;
	
	Text text;
	int line = 0;
	int numLayers = -1;
	float learnRate = 0.f, actFuncOffset = 0.f;
	
	IntArray nodes;
	
	while(file.isEof() == false)
	{
		text = file.readLine();
		
		if(line == 0)
		{
			// should be the format/version string
			if(text.contains("NeuralNetworkSimple:v1") == false)
			{
				printf("NeuralNetwork::read wrong version type\n");
				return;
			}
		}
		else if(line == 1)
		{
			// should be learn rate and act func offset			
			sscanf(text.getArray(), "learnRate: %f actFuncOffset: %f", &learnRate, &actFuncOffset);
		}
		else if(line == 2)
		{
			// should be the number of layers			
			sscanf(text.getArray(), "Layers:%d", &numLayers);
			
			if(numLayers < 2) 
			{
				printf("NeuralNetwork::read invalid number of layers\n");
				return;
			}
			
			nodes = IntArray::withSize(numLayers, false);
		}
		else if(numLayers > 0)
		{
			int layer, numNodes;
			sscanf(text.getArray(), "Layer %d:%d", &layer, &numNodes);
			numLayers--;
			
			nodes[layer] = numNodes;
			
			if(numLayers == 0)
			{				
				if(nodes == getStructure())
				{
					setLearnRate(learnRate);
					setActFuncOffset(actFuncOffset);
				}
				else
				{
					setInternal(new NeuralNetworkSimpleInternal(nodes, learnRate, actFuncOffset));
				}
			}
		}
		else
		{
			// it's a weight or a threshold
			int layer, node, index;
			float value;
			sscanf(text.getArray(), "%d %d %d %f", &layer, &node, &index, &value);
			
			if(index < 0)
				setThreshold(layer, node, value);
			else
				setWeight(layer, node, index, value);
		}
		
		line++;
	}		
}



END_UGEN_NAMESPACE
