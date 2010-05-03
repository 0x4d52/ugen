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

#ifndef _UGEN_ugen_NeuralPattern_H_
#define _UGEN_ugen_NeuralPattern_H_

#include "../core/ugen_SmartPointer.h"
#include "../core/ugen_Arrays.h"
#include "ugen_NeuralNetworkConfig.h"

class NeuralPatternBaseInternal : public SmartPointer
{
public:
	virtual int getNumInputs() const = 0;
	virtual int getNumOutputs() const = 0;
	virtual const NumericalArray<float>& getInputVector() const = 0;
	virtual const NumericalArray<float>& getOutputVector() const = 0;
	virtual void write(TextFileWriter const& file) const = 0;
};

class NeuralPatternSimpleInternal : public NeuralPatternBaseInternal
{
public:
	NeuralPatternSimpleInternal(NumericalArray<float> const& inputVector,
								NumericalArray<float> const& outputVector) throw();
	
	int getNumInputs() const throw() { return inputVector.size(); }
	int getNumOutputs() const throw() { return outputVector.size(); }
	
	const NumericalArray<float>& getInputVector() const throw() { return inputVector; }
	const NumericalArray<float>& getOutputVector() const throw() { return outputVector; }
	
	void write(TextFileWriter const& file) const throw();
	
private:
	const NumericalArray<float> inputVector;
	const NumericalArray<float> outputVector;
};

class NeuralPattern : public SmartPointerContainer<NeuralPatternBaseInternal>
{
public:
	NeuralPattern(const int null = 0) throw() 
	:	SmartPointerContainer<NeuralPatternBaseInternal>(0)
	{ }
	
	NeuralPattern(NumericalArray<float> inputVector,
				  NumericalArray<float> outputVector) throw()
	:	SmartPointerContainer<NeuralPatternBaseInternal>
		(new NeuralPatternSimpleInternal(inputVector, outputVector))
	{
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
	
	inline const NumericalArray<float> getInputVector() const throw() 
	{ 
		if(getInternal() != 0) 
		{
			return getInternal()->getInputVector(); 
		}
		else return NumericalArraySpec(0, false);
	}
	
	inline const NumericalArray<float> getOutputVector() const throw() 
	{ 
		if(getInternal() != 0) 
		{
			return getInternal()->getOutputVector(); 
		}
		else return NumericalArraySpec(0, false);
	}
	
	inline void write(TextFileWriter const& file) const throw()
	{
		if(getInternal() != 0) 
		{
			return getInternal()->write(file); 
		}
	}
	
	void read(TextFileReader const& file) throw();

};


class NeuralPatternArray : public ObjectArray<NeuralPattern>
{
public:
	NeuralPatternArray() throw() 
	:	ObjectArray<NeuralPattern>()
	{ 
	}
	
	NeuralPatternArray(NeuralPattern const& pattern) throw()
	:	ObjectArray<NeuralPattern>(pattern)
	{
	}
		
	ObjectArrayAssignmentDefinition(NeuralPatternArray, NeuralPattern);
	
	void write(TextFileWriter const& file) const throw();	
	void read(TextFileReader const& file) throw();
	
};


#endif // _UGEN_ugen_NeuralPattern_H_
