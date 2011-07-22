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

#include "ugen_NeuralLayer.h"


NeuralLayerSimpleInternal::NeuralLayerSimpleInternal(const int numNodes, const int numNodesOnPreviousLayer) throw()
:	nodes(numNodes < 1 ? 1 : numNodes),
	outputVector(NumericalArray<float>::newClear(nodes.size())),
	inputVector(NumericalArray<float>::newClear(numNodesOnPreviousLayer < 1 ? 1 : numNodesOnPreviousLayer)),
	adjustVector(NumericalArray<float>::newClear(inputVector.size()))
{	
	const int numWeights = inputVector.size();
	const int size = nodes.size();
	for(int i = 0; i < size; i++)
	{
		nodes[i] = NeuralNode(numWeights);
	}
}

void NeuralLayerSimpleInternal::init(const float weightMaximum) throw()
{
	const int size = nodes.size();
	for(int i = 0; i < size; i++)
	{
		nodes[i].init(weightMaximum);
	}
}

void NeuralLayerSimpleInternal::randomise(const float amount) throw()
{
	const int size = nodes.size();
	for(int i = 0; i < size; i++)
	{
		nodes[i].init(amount);
	}
}

void NeuralLayerSimpleInternal::set(const int node, NumericalArray<float> const& weightVector, const float threshold) throw()
{
	nodes[node].set(weightVector, threshold);
}

void NeuralLayerSimpleInternal::setThreshold(const int node, const float threshold) throw()
{
	nodes[node].setThreshold(threshold);
}

void NeuralLayerSimpleInternal::setWeight(const int node, const int weightIndex, const float weight) throw()
{
	nodes[node].setWeight(weightIndex, weight);
}

void NeuralLayerSimpleInternal::get(const int node, NumericalArray<float> *weightVector, float& threshold) const throw()
{
	nodes[node].get(weightVector, threshold); ////xxx
}

NumericalArray<float>& NeuralLayerSimpleInternal::propogate(NumericalArray<float> const& _inputVector) throw()
{
	memcpy(inputVector.getArray(), _inputVector.getArray(), inputVector.size() * sizeof(float));
	
	float* outputVectorPtr = outputVector.getArray();
	
	NeuralNode *nodesPtr = nodes.getArray();
	const int size = nodes.size();
	for(int i = 0; i < size; i++)
	{
		outputVectorPtr[i] = nodesPtr[i].propogate(inputVector);
	}	
	
	return outputVector;
}

NumericalArray<float>& NeuralLayerSimpleInternal::backProp(NumericalArray<float>& errorVector, const float actFuncOffset, const float learnRate) throw()
{
	memset(adjustVector.getArray(), 0, adjustVector.size() * sizeof(float));
	
	float* errorVectorPtr = errorVector.getArray();
	
	NeuralNode *nodesPtr = nodes.getArray();
	const int size = nodes.size();
	for(int i = 0; i < size; i++)
	{
		nodesPtr[i].backProp(inputVector, errorVectorPtr[i], actFuncOffset, learnRate, adjustVector);
	}
	
	return adjustVector;
}


END_UGEN_NAMESPACE
