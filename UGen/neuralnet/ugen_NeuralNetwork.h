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

#ifndef _UGEN_ugen_NeuralNetwork_H_
#define _UGEN_ugen_NeuralNetwork_H_

#include "../core/ugen_SmartPointer.h"
#include "../core/ugen_Arrays.h"
#include "ugen_NeuralNetworkConfig.h"
#include "ugen_NeuralLayer.h"
#include "ugen_NeuralNode.h"
#include "ugen_NeuralPattern.h"

class NeuralNetworkBaseInternal : public SmartPointer
{
public:
	virtual int getNumLayersIncludingInput() const = 0;
	virtual int getNumLayersExcludingInput() const = 0;
	virtual int getNumInputs() const = 0;
	virtual int getNumOutputs() const = 0;
	virtual IntArray getStructure() const = 0;
	virtual bool hasInputsAndOutputs(const int numInputs, const int numOutputs) const = 0;
	virtual int getNumNodesOnLayer(const int layer) const = 0;
	virtual float getLearnRate() const = 0;
	virtual float getActFuncOffset() const = 0;
	virtual void setLearnRate(const float value) = 0;
	virtual void setActFuncOffset(const float value) = 0;
	
	virtual const NumericalArray<float> getErrorVector() const = 0;

	virtual void init(const float weightMaximum) = 0;
	virtual void randomise(const float amount) = 0;
	virtual void set(const int layer, const int node, NumericalArray<float> const& weightVector, const float threshold) = 0;
	virtual void setThreshold(const int layer, const int node, const float threshold) = 0;
	virtual void setWeight(const int layer, const int node, const int weightIndex, const float weight) = 0;
	virtual void get(const int layer, const int node, NumericalArray<float> *weightVector, float& threshold) const = 0;
	
	virtual void write(TextFileWriter const& file) const = 0;
	
	virtual NumericalArray<float> propogate(NumericalArray<float> const& inputVector) = 0;
	virtual void backProp(NumericalArray<float> const& inputVector, NumericalArray<float> const& targetVector) = 0;

	virtual void train(NeuralPatternArray const& patterns, const int count) = 0;
};

class NeuralNetworkSimpleInternal : public NeuralNetworkBaseInternal
{
public:
	static const float defaultLearnRate;
	static const float defaultActFuncOffset;

	NeuralNetworkSimpleInternal(IntArray const& numNodes, 
								const float learnRate, 
								const float actFuncOffset) throw();
	
	inline int getNumLayersIncludingInput() const throw() { return layers.size()+1; }
	inline int getNumLayersExcludingInput() const throw() { return layers.size(); }
	inline int getNumInputs() const throw() { return numInputs; }
	inline int getNumOutputs() const throw() { return numOutputs; }
	IntArray getStructure() const throw();
	inline bool hasInputsAndOutputs(const int numInputsCheck, const int numOutputsCheck) const throw() 
	{ 
		return (numInputs == numInputsCheck) && (numOutputs == numOutputsCheck);  
	}
	inline int getNumNodesOnLayer(const int layer) const throw() { return layers[layer].getNumNodes(); }
	inline float getLearnRate() const throw() { return learnRate; }
	inline float getActFuncOffset() const throw() { return actFuncOffset; }
	inline void setLearnRate(const float value) { learnRate = value; }
	inline void setActFuncOffset(const float value) { actFuncOffset = value; }
	
	inline const NumericalArray<float> getErrorVector() const throw() { return errorVector; }
	
	void init(const float weightMaximum) throw();
	void randomise(const float amount) throw();
	void set(const int layer, const int node, NumericalArray<float> const& weightVector, const float threshold) throw();
	void setThreshold(const int layer, const int node, const float threshold) throw();
	void setWeight(const int layer, const int node, const int weightIndex, const float weight) throw();
	void get(const int layer, const int node, NumericalArray<float> *weightVector, float& threshold) const throw();
	
	void write(TextFileWriter const& file) const throw();
	
	NumericalArray<float> propogate(NumericalArray<float> const& inputVector) throw();
	void backProp(NumericalArray<float> const& inputVector, NumericalArray<float> const& targetVector) throw();
	
	void train(NeuralPatternArray const& patterns, const int count) throw();
	
private:
	float learnRate, actFuncOffset;
	NeuralLayerArray layers;
	int numInputs;
	int numOutputs;
	NumericalArray<float> errorVector;
};

class NeuralNetwork :	public SmartPointerContainer<NeuralNetworkBaseInternal>
{
public:
	NeuralNetwork(const int null = 0)
	:	SmartPointerContainer<NeuralNetworkBaseInternal>(0)
	{
		(void)null;
	}
	
	NeuralNetwork(const int numLayers, 
				  const int *numNodes, 
				  const float learnRate, 
				  const float actFuncOffset) throw()
	:	SmartPointerContainer<NeuralNetworkBaseInternal>
		(new NeuralNetworkSimpleInternal(IntArray(numLayers, numNodes), learnRate, actFuncOffset))
	{
	}
	
	NeuralNetwork(IntArray const& numNodes, 
				  const float learnRate, 
				  const float actFuncOffset) throw()
	:	SmartPointerContainer<NeuralNetworkBaseInternal>
		(new NeuralNetworkSimpleInternal(numNodes, learnRate, actFuncOffset))
	{
	}
	
	NeuralNetwork(const int numInputs, 
				  const int numHidden, 
				  const int numOutputs, 
				  const float learnRate, 
				  const float actFuncOffset) throw()
	:	SmartPointerContainer<NeuralNetworkBaseInternal>
		(new NeuralNetworkSimpleInternal(IntArray(numInputs, numHidden, numOutputs), learnRate, actFuncOffset))
	{
	}
	
	inline int getNumLayersIncludingInput() const throw() 
	{ 
		if(getInternal() != 0)
		{
			return getInternal()->getNumLayersIncludingInput(); 
		} 
		else return 0;
	}
	
	inline int getNumLayersExcludingInput() const throw() 
	{ 
		if(getInternal() != 0)
		{
			return getInternal()->getNumLayersExcludingInput(); 
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
	
	inline IntArray getStructure() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getStructure(); 
		} 
		else return NumericalArraySpec(0, true);
	}
	
	inline bool hasInputsAndOutputs(const int numInputs, const int numOutputs) const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->hasInputsAndOutputs(numInputs, numOutputs); 
		} 
		else return false;
	}
	
	inline int getNumNodesOnLayer(const int layer) const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getNumNodesOnLayer(layer); 
		} 
		else return 0;		
	}
	
	inline float getLearnRate() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getLearnRate(); 
		} 
		else return 0.0;				
	}
	
	inline float getActFuncOffset() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->getActFuncOffset(); 
		} 
		else return 0.0;				
	}
	
	inline void setLearnRate(const float learnRate) 
	{ 
		if(getInternal() != 0)
		{
			return getInternal()->setLearnRate(learnRate); 
		}
	}
	
	inline void setActFuncOffset(const float actFuncOffset)
	{
		if(getInternal() != 0)
		{
			return getInternal()->setActFuncOffset(actFuncOffset); 
		}
	}
	
	inline const NumericalArray<float> getErrorVector() const throw() 
	{ 
		if(getInternal() != 0)
		{
			return getInternal()->getErrorVector(); 
		} 
		else return NumericalArraySpec(0, false);
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
	
	inline void set(const int layer, const int node, NumericalArray<float> const& weightVector, const float threshold) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->set(layer, node, weightVector, threshold); 
		} 		
	}
	
	void setThreshold(const int layer, const int node, const float threshold) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->setThreshold(layer, node, threshold); 
		}		
	}
	
	void setWeight(const int layer, const int node, const int weightIndex, const float weight) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->setWeight(layer, node, weightIndex, weight); 
		}		
	}
	
	inline void get(const int layer, const int node, NumericalArray<float> *weightVector, float& threshold) const throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->get(layer, node, weightVector, threshold); 
		} 		
	}
	
	inline void write(TextFileWriter const& file) const throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->write(file); 
		} 		
	}
	
	void read(TextFileReader const& file) throw();
	
	inline NumericalArray<float> propogate(NumericalArray<float> const& inputVector) throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->propogate(inputVector); 
		} 
		else return NumericalArraySpec(0, false);		
	}
		
	inline void backProp(NumericalArray<float> const& inputVector, NumericalArray<float> const& targetVector) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->backProp(inputVector, targetVector); 
		} 
	}
	
	inline void train(NeuralPatternArray const& patterns, const int count = 1) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->train(patterns, count); 
		} 
	}
	
};


#endif // _UGEN_ugen_NeuralNetwork_H_
