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

#ifndef _UGEN_ugen_NeuralNode_H_
#define _UGEN_ugen_NeuralNode_H_

#include "../core/ugen_SmartPointer.h"
#include "../core/ugen_Arrays.h"
#include "ugen_NeuralNetworkConfig.h"

class NeuralNodeBaseInternal : public SmartPointer
{
public:
	virtual void init(const float weightMaximum) = 0;
	virtual void randomise(const float amount) = 0;
	
	virtual void set(NumericalArray<float> const& weightVector, const float threshold) = 0;
	virtual void setThreshold(const float threshold) = 0;
	virtual void setWeight(const int index, const float weight) = 0;
	virtual void get(NumericalArray<float> *weightVector, float& threshold) const = 0;
//	virtual void write(TextFileWriter const& file) const = 0;
	
	virtual int getNumWeights() = 0;
	virtual float propogate(NumericalArray<float> const& inputVector) = 0;
	virtual void backProp(NumericalArray<float> const& inputVector, 
						  const float error, const float actFuncOffset, const float learnRate, 
						  NumericalArray<float>& adjustVector) = 0;	
};

class NeuralNodeSimpleInternal : public NeuralNodeBaseInternal
{
public:
	static const float defaultWeight;
	
	NeuralNodeSimpleInternal(const int numWeights) throw();
	void init(const float weightMaximum) throw();
	void randomise(const float amount) throw();
	
	void set(NumericalArray<float> const& weightVector, const float threshold) throw();
	void setThreshold(const float threshold) throw();
	void setWeight(const int index, const float weight) throw();
	void get(NumericalArray<float> *weightVector, float& threshold) const throw();
	
//	void write(TextFileWriter const& file) const throw();
	
	inline int getNumWeights() throw() { return weightVector.size(); }
	float propogate(NumericalArray<float> const& inputVector) throw();
	void backProp(NumericalArray<float> const& inputVector, 
				  const float error, const float actFuncOffset, const float learnRate, 
				  NumericalArray<float>& adjustVector) throw();
	
private:	
	float threshold, output;
	NumericalArray<float> weightVector;
	
	static const float ne1; // cached version of e1 for the float/double depending on type of network
	static const float one; // cached versions of 0.0 and 1.0 for the float/double depending on type of network
	static const float zero;
};

class NeuralNode : public SmartPointerContainer<NeuralNodeBaseInternal>
{
public:	
	NeuralNode(const int numWeights = 0) throw()
	:	SmartPointerContainer<NeuralNodeBaseInternal>
		(numWeights == 0 ? 0 : new NeuralNodeSimpleInternal(numWeights))
	{
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
	
	inline void set(NumericalArray<float> const& weightVector, const float threshold) throw()
	{
		if(getInternal() != 0) 
		{
			getInternal()->set(weightVector, threshold); 
		}		
	}
	
	inline void setThreshold(const float threshold) throw()
	{
		if(getInternal() != 0) 
		{
			getInternal()->setThreshold(threshold); 
		}
	}
	
	inline void setWeight(const int index, const float weight) throw()
	{
		if(getInternal() != 0) 
		{
			getInternal()->setWeight(index, weight); 
		}
	}
	
	inline void get(NumericalArray<float> *weightVector, float& threshold) const throw()
	{
		if(getInternal() != 0) 
		{
			getInternal()->get(weightVector, threshold); 
		}				
	}
	
//	inline void write(TextFileWriter const& file) const throw()
//	{
//		if(getInternal() != 0)
//		{
//			getInternal()->write(file); 
//		} 		
//	}	

	inline int getNumWeights() throw() 
	{ 
		if(getInternal() != 0) 
		{
			return getInternal()->getNumWeights(); 
		}
		else return 0;
	}
	
	inline float propogate(NumericalArray<float> const& inputVector) throw()
	{
		if(getInternal() != 0) 
		{
			return getInternal()->propogate(inputVector); 
		}
		else return 0.0;
	}
	
	inline void backProp(NumericalArray<float> const& inputVector, 
						 const float error, const float actFuncOffset, const float learnRate, 
						 NumericalArray<float>& adjustVector) throw()
	{
		if(getInternal() != 0) 
		{
			getInternal()->backProp(inputVector, error, actFuncOffset, learnRate, adjustVector); 
		}
	}

};

class NeuralNodeArray : public ObjectArray<NeuralNode>
{
public:
	NeuralNodeArray(const int size)
	:	ObjectArray<NeuralNode> (size, false) 
	{
	}
		
	ObjectArrayAssignmentDefinition(NeuralNodeArray, NeuralNode)

};


#endif // _UGEN_ugen_NeuralNode_H_
