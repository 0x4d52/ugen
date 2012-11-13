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

#ifndef _UGEN_ugen_NeuralLayer_H_
#define _UGEN_ugen_NeuralLayer_H_

#include "../core/ugen_SmartPointer.h"
#include "../core/ugen_Arrays.h"
#include "ugen_NeuralNetworkConfig.h"
#include "ugen_NeuralNode.h"

class NeuralLayerBaseInternal : public SmartPointer
{
public:
	virtual int getNumNodes() const = 0;
	virtual int getNumInputs() const = 0;
	virtual int getNumOutputs() const = 0;
	
	virtual void init(const float weightMaximum) = 0;
	virtual void randomise(const float amount) = 0;
	virtual void set(const int node, NumericalArray<float> const& weightVector, const float threshold) = 0;
	virtual void setThreshold(const int node, const float threshold) = 0;
	virtual void setWeight(const int node, const int weightIndex, const float weight) = 0;
	virtual void get(const int node, NumericalArray<float> *weightVector, float& threshold) const = 0;
	
//	virtual void write(TextFileWriter const& file) const = 0;
	
	virtual NumericalArray<float>& propogate(NumericalArray<float> const& inputVector) = 0;
	virtual NumericalArray<float>& backProp(NumericalArray<float>& errorVector, const float actFuncOffset, const float learnRate) = 0;
};

class NeuralLayerSimpleInternal : public NeuralLayerBaseInternal
{
public:
	NeuralLayerSimpleInternal(const int numNodes, const int numNodesOnPreviousLayer) throw();
	
	inline int getNumNodes() const throw() { return nodes.size(); }
	inline int getNumInputs() const throw() { return inputVector.size(); }
	inline int getNumOutputs() const throw() { return outputVector.size(); }
	
	void init(const float weightMaximum) throw();
	void randomise(const float amount) throw();
	void set(const int node, NumericalArray<float> const& weightVector, const float threshold) throw();
	void setThreshold(const int node, const float threshold) throw();
	void setWeight(const int node, const int weightIndex, const float weight) throw();
	void get(const int node, NumericalArray<float> *weightVector, float& threshold) const throw();
	
//	void write(TextFileWriter const& file) const throw();
	
	NumericalArray<float>& propogate(NumericalArray<float> const& inputVector) throw();
	NumericalArray<float>& backProp(NumericalArray<float>& errorVector, const float actFuncOffset, const float learnRate) throw();
	
private:
	NeuralNodeArray nodes;
	NumericalArray<float> outputVector;
	NumericalArray<float> inputVector;
	NumericalArray<float> adjustVector;
};

class NeuralLayer : public SmartPointerContainer<NeuralLayerBaseInternal>
{
public:
	NeuralLayer(const int null = 0) throw() 
	:	SmartPointerContainer<NeuralLayerBaseInternal>(0)
	{ (void)null;}
	
	NeuralLayer(const int numNodes, const int numNodesOnPreviousLayer) throw()
	:	SmartPointerContainer<NeuralLayerBaseInternal>
		(new NeuralLayerSimpleInternal(numNodes, numNodesOnPreviousLayer))
	{
	}
	
	inline int getNumNodes() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getNumNodes();
		}
		else return 0;
	}
	
	inline int getNumInputs() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getNumInputs();
		}
		else return 0;
	}
	
	inline int getNumOutputs() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getNumOutputs();
		}
		else return 0;
	}
	
	inline void init(const float weightMaximum) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->init(weightMaximum);
		}
	}
	
	inline void randomise(const float amount) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->randomise(amount);
		}
	}
	
	inline void set(const int node, NumericalArray<float> const& weightVector, const float threshold) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->set(node, weightVector, threshold);
		}		
	}
	
	void setThreshold(const int node, const float threshold) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->setThreshold(node, threshold);
		}
	}
	
	void setWeight(const int node, const int weightIndex, const float weight) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->setWeight(node, weightIndex, weight);
		}		
	}
	
	inline void get(const int node, NumericalArray<float> *weightVector, float& threshold) const throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->get(node, weightVector, threshold);
		}		
	}
	
//	inline void write(TextFileWriter const& file) const throw()
//	{
//		if(getInternal() != 0)
//		{
//			getInternal()->write(file); 
//		} 		
//	}	
	
	inline NumericalArray<float> propogate(NumericalArray<float> const& inputVector) throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->propogate(inputVector);
		}
		else return NumericalArraySpec(0, false);
	}
	
	inline NumericalArray<float> backProp(NumericalArray<float>& errorVector, const float actFuncOffset, const float learnRate) throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->backProp(errorVector, actFuncOffset, learnRate);
		}
		else return NumericalArraySpec(0, false);
	}
	
};

class NeuralLayerArray : public ObjectArray<NeuralLayer>
{
public:
	NeuralLayerArray(const int size)
	:	ObjectArray<NeuralLayer> (size, false) 
	{
	}
		
	ObjectArrayAssignmentDefinition(NeuralLayerArray, NeuralLayer)
};



#endif // _UGEN_ugen_NeuralLayer_H_
