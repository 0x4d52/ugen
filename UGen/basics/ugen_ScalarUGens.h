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

#ifndef UGEN_SCALARUGENS_H
#define UGEN_SCALARUGENS_H


#include "../core/ugen_UGen.h"


#define PtrUGenProcessBlock()														\
	int numSamplesToProcess = uGenOutput.getBlockSize();							\
	float* outputSamples = uGenOutput.getSampleData();								\
	float nextValue = (float)*ptr;													\
	value_ = nextValue;																\
	for(int i = 0; i < numSamplesToProcess; ++i)									\
	{																				\
		outputSamples[i] = nextValue;												\
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
	UGenInternal* getKr() throw();
	inline bool isNull() const throw()			{ return true;  }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	inline float getValue(const int /*channel*/) const throw()					{ return 0.f;	}
	inline bool isConst() const throw()											{ return true;	}
	
	static NullUGenInternal* getInstance() throw();
	
private:
	NullUGenInternal() throw(); // private so you need to call getInstance()
//	~NullUGenInternal(); 
};

UGenSublcassDeclarationNoDefault(NullUGen, (), (), COMMON_UGEN_DOCS Plug_Docs);


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

/** @ingroup UGenInternals */
class CharPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	CharPtrUGenInternal(char const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID,const  int channel) throw();
	
private:
	char const *ptr;
};

/** @ingroup UGenInternals */
class UnsignedCharPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	UnsignedCharPtrUGenInternal(unsigned char const *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID,const  int channel) throw();
	
private:
	unsigned char const *ptr;
};

#if defined(UGEN_IPHONE)
/** @ingroup UGenInternals */
class BOOLPtrUGenInternal : public ScalarBaseUGenInternal
{
public:
	BOOLPtrUGenInternal(signed char *valuePtr) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID,const  int channel) throw();
	
private:
	signed char *ptr;
};
#endif



#endif // SCALARUGENS_H