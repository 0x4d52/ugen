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

#include "../core/ugen_SmartPointer.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "ugen_EnvCurve.h"


EnvCurveList::EnvCurveListInternal::EnvCurveListInternal(const int initSize) throw() 
:	size(initSize), 
	data(new EnvCurve[size]) 
{
}

EnvCurveList::EnvCurveListInternal::~EnvCurveListInternal()
{ 
	delete [] data;	
}

EnvCurveList::EnvCurveList(EnvCurve const& i00) throw()
{
	internal = new EnvCurveListInternal(1);
	internal->data[0] = i00;
}

EnvCurveList::EnvCurveList(EnvCurve const& i00,
						   EnvCurve const& i01, 
						   EnvCurve const& i02, 
						   EnvCurve const& i03, 
						   EnvCurve const& i04, 
						   EnvCurve const& i05, 
						   EnvCurve const& i06, 
						   EnvCurve const& i07, 
						   EnvCurve const& i08, 
						   EnvCurve const& i09, 
						   EnvCurve const& i10, 
						   EnvCurve const& i11, 
						   EnvCurve const& i12, 
						   EnvCurve const& i13, 
						   EnvCurve const& i14, 
						   EnvCurve const& i15, 
						   EnvCurve const& i16, 
						   EnvCurve const& i17, 
						   EnvCurve const& i18, 
						   EnvCurve const& i19, 
						   EnvCurve const& i20, 
						   EnvCurve const& i21, 
						   EnvCurve const& i22, 
						   EnvCurve const& i23, 
						   EnvCurve const& i24, 
						   EnvCurve const& i25, 
						   EnvCurve const& i26, 
						   EnvCurve const& i27, 
						   EnvCurve const& i28, 
						   EnvCurve const& i29, 
						   EnvCurve const& i30, 
						   EnvCurve const& i31) throw()
{
	int count = 2;
	
	if(i02.equalsInfinity()) goto init; else count++;
	if(i03.equalsInfinity()) goto init; else count++;
	if(i04.equalsInfinity()) goto init; else count++;
	if(i05.equalsInfinity()) goto init; else count++;
	if(i06.equalsInfinity()) goto init; else count++;
	if(i07.equalsInfinity()) goto init; else count++;
	if(i08.equalsInfinity()) goto init; else count++;
	if(i09.equalsInfinity()) goto init; else count++;
	if(i10.equalsInfinity()) goto init; else count++;
	if(i11.equalsInfinity()) goto init; else count++;
	if(i12.equalsInfinity()) goto init; else count++;
	if(i13.equalsInfinity()) goto init; else count++;
	if(i14.equalsInfinity()) goto init; else count++;
	if(i15.equalsInfinity()) goto init; else count++;
	if(i16.equalsInfinity()) goto init; else count++;
	if(i17.equalsInfinity()) goto init; else count++;
	if(i18.equalsInfinity()) goto init; else count++;
	if(i19.equalsInfinity()) goto init; else count++;
	if(i20.equalsInfinity()) goto init; else count++;
	if(i21.equalsInfinity()) goto init; else count++;
	if(i22.equalsInfinity()) goto init; else count++;
	if(i23.equalsInfinity()) goto init; else count++;
	if(i24.equalsInfinity()) goto init; else count++;
	if(i25.equalsInfinity()) goto init; else count++;
	if(i26.equalsInfinity()) goto init; else count++;
	if(i27.equalsInfinity()) goto init; else count++;
	if(i28.equalsInfinity()) goto init; else count++;
	if(i29.equalsInfinity()) goto init; else count++;
	if(i30.equalsInfinity()) goto init; else count++;
	if(i31.equalsInfinity()) goto init; else count++;
	
	
init:
	internal = new EnvCurveListInternal(count);
	internal->data[0] = i00;
	internal->data[1] = i01;
	
	if(i02.equalsInfinity()) goto exit; else internal->data[ 2] = i02;
	if(i03.equalsInfinity()) goto exit; else internal->data[ 3] = i03;
	if(i04.equalsInfinity()) goto exit; else internal->data[ 4] = i04;
	if(i05.equalsInfinity()) goto exit; else internal->data[ 5] = i05;
	if(i06.equalsInfinity()) goto exit; else internal->data[ 6] = i06;
	if(i07.equalsInfinity()) goto exit; else internal->data[ 7] = i07;
	if(i08.equalsInfinity()) goto exit; else internal->data[ 8] = i08;
	if(i09.equalsInfinity()) goto exit; else internal->data[ 9] = i09;
	if(i10.equalsInfinity()) goto exit; else internal->data[10] = i10;
	if(i11.equalsInfinity()) goto exit; else internal->data[11] = i11;
	if(i12.equalsInfinity()) goto exit; else internal->data[12] = i12;
	if(i13.equalsInfinity()) goto exit; else internal->data[13] = i13;
	if(i14.equalsInfinity()) goto exit; else internal->data[14] = i14;
	if(i15.equalsInfinity()) goto exit; else internal->data[15] = i15;
	if(i16.equalsInfinity()) goto exit; else internal->data[16] = i16;
	if(i17.equalsInfinity()) goto exit; else internal->data[17] = i17;
	if(i18.equalsInfinity()) goto exit; else internal->data[18] = i18;
	if(i19.equalsInfinity()) goto exit; else internal->data[19] = i19;
	if(i20.equalsInfinity()) goto exit; else internal->data[20] = i20;
	if(i21.equalsInfinity()) goto exit; else internal->data[21] = i21;
	if(i22.equalsInfinity()) goto exit; else internal->data[22] = i22;
	if(i23.equalsInfinity()) goto exit; else internal->data[23] = i23;
	if(i24.equalsInfinity()) goto exit; else internal->data[24] = i24;
	if(i25.equalsInfinity()) goto exit; else internal->data[25] = i25;
	if(i26.equalsInfinity()) goto exit; else internal->data[26] = i26;
	if(i27.equalsInfinity()) goto exit; else internal->data[27] = i27;
	if(i28.equalsInfinity()) goto exit; else internal->data[28] = i28;
	if(i29.equalsInfinity()) goto exit; else internal->data[29] = i29;
	if(i30.equalsInfinity()) goto exit; else internal->data[30] = i30;
	if(i31.equalsInfinity()) goto exit; else internal->data[31] = i31;
	
exit:
	return;
}

EnvCurveList::EnvCurveList(EnvCurve::CurveType type, const int size) throw()
{
	internal = new EnvCurveListInternal(size);
	for(int i = 0; i < size; i++)
		internal->data[i] = EnvCurve(type);
}

EnvCurveList::EnvCurveList(const double curve, const int size) throw()
{
	internal = new EnvCurveListInternal(size);
	for(int i = 0; i < size; i++)
		internal->data[i] = EnvCurve(curve);
}

EnvCurveList::~EnvCurveList() throw()
{
	internal->decrementRefCount();
}

EnvCurveList::EnvCurveList(EnvCurveList const& copy) throw()
:	internal(copy.internal)
{
	internal->incrementRefCount();
}

EnvCurveList& EnvCurveList::operator= (EnvCurveList const& other) throw()
{
	if (this != &other)
    {		
		other.internal->incrementRefCount();
		internal->decrementRefCount();
		internal = other.internal;
    }
	
    return *this;
}

EnvCurve& EnvCurveList::operator[] (const int index) throw()
{
	if(index < 0) { ugen_assertfalse; return empty; }
	
	return internal->data[index % size()];
}

const EnvCurve& EnvCurveList::operator[] (const int index) const throw()
{
	if(index < 0) { ugen_assertfalse; return empty; }
	
	return internal->data[index % size()];
}

EnvCurveList EnvCurveList::blend(EnvCurveList const& other, float fraction) const throw()
{
	ugen_assert(fraction >= 0.f && fraction <= 1.f);
	
	const int maxSize = ugen::max(size(), other.size());
	const int minSize = ugen::min(size(), other.size());
	fraction = ugen::max(0.f, fraction);
	fraction = ugen::min(1.f, fraction);
	
	EnvCurveList newList(0.0, maxSize);
	
	for(int i = 0; i < minSize; i++)
	{
		EnvCurve& thisCurve = internal->data[i];
		EnvCurve& otherCurve = other.internal->data[i];
		
		if((thisCurve.getType() == EnvCurve::Numerical) && (otherCurve.getType() == EnvCurve::Numerical))
		{
			float thisValue = thisCurve.getCurve();
			float otherValue = otherCurve.getCurve();
			float newValue = thisValue * (1.f-fraction) + otherValue * fraction;
			newList.internal->data[i] = EnvCurve(newValue);
		}
		else
		{
			if(fraction < 0.5f)
				newList.internal->data[i] = thisCurve;
			else
				newList.internal->data[i] = otherCurve;
		}
	}
	
	// should really turn this loop "inside out" save doing the if() for each iteration
	for(int i = minSize; i < maxSize; i++)
	{
		EnvCurve& thisCurve = internal->data[i];
		EnvCurve& otherCurve = other.internal->data[i];
		
		if(size() < other.size())
		{
			if(fraction < 0.5f)
				newList.internal->data[i] = EnvCurve(EnvCurve::Linear);
			else
				newList.internal->data[i] = otherCurve;
		}
		else
		{
			if(fraction < 0.5f)
				newList.internal->data[i] = thisCurve;
			else
				newList.internal->data[i] = EnvCurve(EnvCurve::Linear);
		}
	}
	
	return newList;
}

EnvCurve EnvCurveList::empty;



END_UGEN_NAMESPACE
