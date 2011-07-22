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

#ifndef _UGEN_ugen_ExternalControlSource_H_
#define _UGEN_ugen_ExternalControlSource_H_

#include "ugen_UGen.h"
#include "../basics/ugen_ScalarUGens.h"
#include "ugen_SmartPointer.h"


class ExternalControlSourceInternal;

/** Used for mapping data from another source for control purposes.
 
 This is used for MIDI controllers for example. */
class ExternalControlSource
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ExternalControlSource() throw();
	ExternalControlSource(ExternalControlSource const& copy) throw();
	ExternalControlSource& operator= (ExternalControlSource const& other) throw();
	virtual ~ExternalControlSource() throw();
	
	/// @} <!-- end Construction and destruction ---------------------------------------- -->
	
	enum Warp
	{
		Linear,
		Exponential
	};
	
	/// @name Miscellaneous
	/// @{
	
	UGen kr(const double lagTime = 0.1) throw();
	UGen krInternal(const double lagTime = 0.1) throw();
	
	float getValue() const throw();
	const float* getValuePtr() const throw();
	
	/// @} <!-- Miscellaneous -->
		
protected:
	ExternalControlSourceInternal* internal;
};


class ExternalControlSourceInternal : public SmartPointer
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ExternalControlSourceInternal(const float minVal = 0.f, const float maxVal = 127.f, 
								  const ExternalControlSource::Warp warp = ExternalControlSource::Linear) throw()
	: value(minVal), minVal_(minVal), maxVal_(maxVal), warp_(warp) { }
	
	/// @} <!-- end Construction and destruction ----------------------------------------- --> 
	
	/// @name Getting and setting value
	/// @{
	
	inline float getValue() const throw()				{ return value;		}
	inline const float* getValuePtr() const throw()		{ return &value;	}
	inline void setValue(const float newValue) throw()	{ value = newValue; }
	inline void setNormalisedValue(float value0_1) throw()
	{
		switch(warp_)
		{
			case ExternalControlSource::Linear:
				value = value0_1 * (maxVal_ - minVal_) + minVal_;
				break;
			case ExternalControlSource::Exponential:
				value = minVal_ * pow(maxVal_ / minVal_, value0_1);
				break;
		}
	}
	
	/// @} <!-- end Getting and setting value -->
	
	friend class ExternalControlSourceUGenInternal;
	
protected:
	float value;
	float minVal_;
	float maxVal_;
	ExternalControlSource::Warp warp_;
};

/**
 @ingroup UGenInternals
 */
class ExternalControlSourceUGenInternal : public FloatPtrUGenInternal
{
public:
	ExternalControlSourceUGenInternal(ExternalControlSource const& externalControlSource) throw();

private:
	ExternalControlSource externalControlSource_;
};



#endif // _UGEN_ugen_ExternalControlSource_H_
