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

#include "ugen_NeuralNode.h"
#include "../core/ugen_Constants.h"
#include "../basics/ugen_InlineBinaryOps.h"

static const double randomFactor = 1.0 / RAND_MAX;

const float NeuralNodeSimpleInternal::defaultWeight = (float)0.1;
const float NeuralNodeSimpleInternal::ne1 = (float)e1;
const float NeuralNodeSimpleInternal::one = (float)1.0;
const float NeuralNodeSimpleInternal::zero = (float)0.0;

NeuralNodeSimpleInternal::NeuralNodeSimpleInternal(const int numWeights) throw()
:	threshold(zero), output(zero), //input(zero), act(zero),
	weightVector(NumericalArray<float>::newClear(numWeights < 1 ? 1 : numWeights))
{		
	init(defaultWeight);
}

void NeuralNodeSimpleInternal::init(const float weightMaximum) throw()
{
	threshold = std::rand() * randomFactor * 2 * weightMaximum - weightMaximum;
	
	for(int i = 0; i < weightVector.size(); i++)
	{
		weightVector[i] = std::rand() * randomFactor * 2 * weightMaximum - weightMaximum;
	}	
}

void NeuralNodeSimpleInternal::randomise(const float amount) throw()
{
	threshold += std::rand() * randomFactor * 2 * amount - amount;
	
	for(int i = 0; i < weightVector.size(); i++)
	{
		weightVector[i] += std::rand() * randomFactor * 2 * amount - amount;
	}	
}

void NeuralNodeSimpleInternal::set(NumericalArray<float> const& newWeightVector, const float newThreshold) throw()
{	
	if(weightVector.size() == newWeightVector.size())
	{
		weightVector = newWeightVector;
		threshold = newThreshold;
	}
}

void NeuralNodeSimpleInternal::setThreshold(const float newThreshold) throw()
{
	threshold = newThreshold;
}

void NeuralNodeSimpleInternal::setWeight(const int index, const float weight) throw()
{
	weightVector[index] = weight;
}

void NeuralNodeSimpleInternal::get(NumericalArray<float> *copiedWeightVector, float& copiedThreshold) const throw()
{
	*copiedWeightVector = weightVector;
	copiedThreshold = threshold;
}

float NeuralNodeSimpleInternal::propogate(NumericalArray<float> const& inputVector) throw()
{
	float input = zero;
	
	const float* inputVectorPtr = inputVector.getArray();
	const float* weightVectorPtr = weightVector.getArray();
	
	const int size = weightVector.size();
	for(int i = 0; i < size; i++)
	{
		input += inputVectorPtr[i] * weightVectorPtr[i];
	}	
	
	float act = input + threshold;
	output = one / (one + (float)ugen::pow(ne1, -act));
	
	return output;
}

void NeuralNodeSimpleInternal::backProp(NumericalArray<float> const& inputVector, 
										const float error, const float actFuncOffset, const float learnRate, 
										NumericalArray<float>& adjustVector) throw()
{
	float output = this->output;
	float adjust = error * (actFuncOffset + (output * (one-output)));
	float learn = adjust * learnRate;
	
	float* weightVectorPtr = weightVector.getArray();
	float* adjustVectorPtr = adjustVector.getArray();
	const float* inputVectorPtr = inputVector.getArray();
	
	const int size = weightVector.size();
	for(int i = 0; i < size; i++)
	{
		weightVectorPtr[i] += inputVectorPtr[i] * learn;
		adjustVectorPtr[i] += weightVectorPtr[i] * adjust;
	}
	
	threshold += learn;
}



END_UGEN_NAMESPACE
