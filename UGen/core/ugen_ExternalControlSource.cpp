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

#ifndef UGEN_NOEXTGPL

#include "ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_ExternalControlSource.h"
#include "../filters/control/ugen_Lag.h"

ExternalControlSource::ExternalControlSource()  throw()
:	internal(0)	
{ 
}

ExternalControlSource::ExternalControlSource(ExternalControlSource const& copy) throw()
:	internal(0)	
{
	internal = copy.internal;
	if(internal != 0) 
		internal->incrementRefCount();
}

ExternalControlSource& ExternalControlSource::operator= (ExternalControlSource const& other) throw()
{
	if(this != &other) 
	{
		if(internal != 0) 
			internal->decrementRefCount();
		
		internal = other.internal;
		if(internal != 0) 
			internal->incrementRefCount();
	}
	
	return *this;
}

ExternalControlSource::~ExternalControlSource()	 throw()	
{ 
	if(internal != 0) 
		internal->decrementRefCount();	  
}

float ExternalControlSource::getValue() const throw()				
{ 
	ugen_assert(internal != 0);
	return internal == 0 ? 0.f : internal->getValue();  
}

const float* ExternalControlSource::getValuePtr() const throw()
{
	ugen_assert(internal != 0);
	return internal == 0 ? 0 : internal->getValuePtr(); 
}

UGen ExternalControlSource::kr(const double lagTime) throw()
{
	ugen_assert(lagTime >= 0.0);
	
	if(lagTime <= 0.0)
		return UGen(*this);
	else
		return UGen(Lag(*this, lagTime)).kr();
}

UGen ExternalControlSource::krInternal(const double lagTime) throw()
{
	ugen_assert(lagTime >= 0.0);
	
	if(lagTime <= 0.0)
		return UGen(*this);
	else
		return UGen(Lag(*this, lagTime)).kr();
}

ExternalControlSourceUGenInternal::ExternalControlSourceUGenInternal
(ExternalControlSource const& externalControlSource) throw()
:	FloatPtrUGenInternal(externalControlSource.getValuePtr()),
	externalControlSource_(externalControlSource)
{ 
}


END_UGEN_NAMESPACE

#endif // gpl
