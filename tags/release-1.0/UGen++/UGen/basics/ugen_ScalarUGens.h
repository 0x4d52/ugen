// $Id: ugen_ScalarUGens.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_ScalarUGens.h $

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

#ifndef UGEN_SCALARUGENS_H
#define UGEN_SCALARUGENS_H


#include "../core/ugen_UGen.h"


#define PtrUGenProcessBlock()														\
	int numSamplesToProcess = uGenOutput.getBlockSize();							\
	float* outputSamples = uGenOutput.getSampleData();								\
	float nextValue = (float)*ptr;													\
	memset(outputSamples, 0, numSamplesToProcess * sizeof(float));					\
	if(nextValue == value_)	{														\
		for(int i = 0; i < numSamplesToProcess; ++i)								\
			outputSamples[i] += nextValue;											\
	} else {																		\
		int numKrSamplesToProcess = UGen::getControlRateBlockSize();				\
		float valueSlope = (nextValue - value_) * UGen::getControlSlopeFactor();	\
		numSamplesToProcess -= numKrSamplesToProcess;								\
		for(int i = 0; i < numKrSamplesToProcess; ++i) {							\
			*outputSamples++ += value_;												\
			value_ += valueSlope;													\
		}																			\
		if(numSamplesToProcess > 0) {												\
			for(int i = 0; i < numSamplesToProcess; ++i)							\
				outputSamples[i] += nextValue;										\
		}																			\
		value_ = nextValue;															\
	}


/** @ingroup UGenInternals */
class ScalarBaseUGenInternal : public UGenInternal
{	
public:
	ScalarBaseUGenInternal(const float value) : UGenInternal(0), value_(value)	{				 }
	inline bool isScalar() const throw()										{ return true;   }
	float getValue(const int /*channel*/) const throw()							{ return value_; }
protected:
	float value_;
};

/** @ingroup UGenInternals */
class NullUGenInternal : public ScalarBaseUGenInternal
{
public:
	NullUGenInternal() throw();
	UGenInternal* getKr() throw();
	inline bool isNull() const throw()			{ return true;  }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	inline float getValue(const int /*channel*/) const throw()					{ return 0.f;	}
	inline bool isConst() const throw()											{ return true;	}
private:
};

/** @ingroup UGenInternals */
class ScalarUGenInternal : public ScalarBaseUGenInternal
{
public:
	ScalarUGenInternal(const float value) throw();
	//UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	inline bool isConst() const throw()	{ return true; }
};

/** @ingroup UGenInternals */
class FloatPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	FloatPtrUGenInternal(float const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
private:
	float const *ptr;
};

/** @ingroup UGenInternals */
class DoublePtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	DoublePtrUGenInternal(double const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
private:
	double const *ptr;
};

/** @ingroup UGenInternals */
class IntPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	IntPtrUGenInternal(int const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID,const  int channel) throw();
	
private:
	int const *ptr;
};

/** @ingroup UGenInternals */
class BoolPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	BoolPtrUGenInternal(bool const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID,const  int channel) throw();
	
private:
	bool const *ptr;
};



#endif // SCALARUGENS_H