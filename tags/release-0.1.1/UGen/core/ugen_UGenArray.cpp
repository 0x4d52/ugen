// $Id: ugen_UGenArray.cpp 1011 2010-05-12 16:24:29Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/core/ugen_UGenArray.cpp $

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

#include "ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_UGenArray.h"
#include "ugen_UGenInternal.h"
#include "../basics/ugen_MixUGen.h"


UGenArray::UGenArrayInternal::UGenArrayInternal(const int size) throw()
:	size_(size),
	array(new UGen[size_])
{
	ugen_assert(size > 0);
}

UGenArray::UGenArrayInternal::~UGenArrayInternal() throw()
{
	delete [] array;
	array = 0;
	size_ = 0;
}

UGenArray::UGenArray() throw()
:	internal(0)
{
}

UGenArray::UGenArray(const int size) throw()
:	internal(new UGenArrayInternal(size))
{
	ugen_assert(size > 0);
}

UGenArray::UGenArray(UGen const& ugen) throw()
:	internal(new UGenArrayInternal(1))
{
	internal->array[0] = ugen;
}

UGenArray::UGenArray(UGenArray const& copy) throw()
:	internal(copy.internal)
{
	if(internal != 0)
		internal->incrementRefCount();
}

UGenArray::UGenArray(UGenArray const& array0, UGenArray const& array1, const bool removeNulls) throw()
:	internal(0)
{
	if(removeNulls == false)
	{
		int newSize = array0.size() + array1.size();
		
		if(newSize != 0) {
			internal = new UGenArrayInternal(newSize);
			
			int newIndex = 0;
			
			for(int i = 0; i < array0.size(); i++) {
				internal->array[newIndex++] = array0.internal->array[i];
			}
			
			for(int i = 0; i < array1.size(); i++) {
				internal->array[newIndex++] = array1.internal->array[i];
			}
		}
	}
	else
	{	
		int newSize = array0.sizeNotNull() + array1.sizeNotNull();
		
		if(newSize != 0) {
			internal = new UGenArrayInternal(newSize);
			
			int newIndex = 0;
			
			for(int i = 0; i < array0.size(); i++) {
				if(array0.internal->array[i].isNotNull())
					internal->array[newIndex++] = array0.internal->array[i];
			}
			
			for(int i = 0; i < array1.size(); i++) {
				if(array1.internal->array[i].isNotNull())
					internal->array[newIndex++] = array1.internal->array[i];
			}
		}
	}
}

UGenArray::~UGenArray() throw()
{
	if(internal != 0)
		internal->decrementRefCount();
}

UGenArray UGenArray::operator<< (UGenArray const& other) throw()
{
	return UGenArray(*this, other, true);
}

UGenArray UGenArray::operator<< (UGen const& other) throw()
{
	return UGenArray(*this, other, true);
}

UGenArray UGenArray::operator, (UGenArray const& other) throw()
{
	return UGenArray(*this, other, false);
}

UGenArray UGenArray::operator, (UGen const& other) throw()
{
	return UGenArray(*this, other, false);
}

UGenArray& UGenArray::operator= (UGenArray const& other) throw()
{
	if (this != &other)
    {		
		if(internal != 0)
			internal->decrementRefCount();
		
		if(other.internal != 0)
			other.internal->incrementRefCount();
		
		internal = other.internal;
    }
	
    return *this;
}

UGenArray& UGenArray::operator<<= (UGenArray const& other) throw()
{
	return operator= (UGenArray(*this, other, true)); // removes null ugens in the process
}

UGenArray& UGenArray::operator+= (UGenArray const& other) throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0)
		operator= (other);
	else
		operator= (*this + other);
	
	return *this;
}

UGenArray& UGenArray::operator-= (UGenArray const& other) throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0)
		operator= (other.neg());
	else
		operator= (*this - other);
	
	return *this;
}

UGenArray& UGenArray::operator*= (UGenArray const& other) throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0)
		operator= (0.f);
	else
		operator= (*this * other);
	
	return *this;
}

UGenArray& UGenArray::operator/= (UGenArray const& other) throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0)
		operator= (other);
	else
		operator= (*this / other);
	
	return *this;
}


int UGenArray::findMaxNumChannels() const throw()
{
	//ugen_assert(internal != 0); OK not an assertion! just answer 0 is fine
	
	if(internal == 0) return 0;
		
	int maxNumChannels = 0;
	
	for(int i = 0; i < internal->size_; i++)
	{
		int numChannels = internal->array[i].getNumChannels();
		maxNumChannels = numChannels > maxNumChannels ? numChannels : maxNumChannels;
	}
	
	return maxNumChannels;
}

void UGenArray::put(const int index, UGen const& item) throw()
{
	if(internal == 0 || index < 0 || index >= internal->size_) 
	{ 
		ugen_assertfalse;
		return;
	}
	
	internal->array[index] = item;
}

UGen& UGenArray::operator[] (const int index) throw()
{	
	if(internal == 0 || index < 0 || index >= internal->size_) 
	{
		ugen_assertfalse;
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->array[index];
}

UGen& UGenArray::at(const int index) throw()
{	
	if(internal == 0 || index < 0 || index >= internal->size_) 
	{
		ugen_assertfalse;
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->array[index];
}

const UGen& UGenArray::operator[] (const int index) const throw()
{	
	if(internal == 0 || index < 0 || index >= internal->size_) 
	{
		ugen_assertfalse;
		return UGen::getNull();
	}
	
	return internal->array[index];
}

const UGen& UGenArray::at(const int index) const throw()
{	
	if(internal == 0 || index < 0 || index >= internal->size_) 
	{
		ugen_assertfalse;
		return UGen::getNull();
	}
	
	return internal->array[index];
}

UGen& UGenArray::wrapAt(const int index) throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}

	int indexToUse = index;
	while(indexToUse < 0)
		indexToUse += internal->size_;
	
	return internal->array[indexToUse % internal->size_];
}

const UGen& UGenArray::wrapAt(const int index) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGen::getNull();
	
	int indexToUse = index;
	while(indexToUse < 0)
		indexToUse += internal->size_;
	
	return internal->array[indexToUse % internal->size_];
}

UGen& UGenArray::first() throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->array[0];
}

UGen& UGenArray::last() throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->array[internal->size_ - 1];
}

const UGen& UGenArray::first() const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGen::getNull();
	
	return internal->array[0];
}

const UGen& UGenArray::last() const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGen::getNull();
	
	return internal->array[internal->size_ - 1];
}

UGenArray UGenArray::at(IntArray const& indices) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
		return UGenArray();
	else
	{
		UGenArray newArray(size);
		
		for(int i = 0; i < size; i++)
		{
			const int index = indices.at(i);
			newArray.put(i, at(index));
		}
		
		return newArray;
	}
}

UGenArray UGenArray::operator[] (IntArray const& indices) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
		return UGenArray();
	else
	{
		UGenArray newArray(size);
		
		for(int i = 0; i < size; i++)
		{
			const int index = indices.at(i);
			newArray.put(i, at(index));
		}
		
		return newArray;
	}
}

UGenArray UGenArray::wrapAt(IntArray const& indices) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
		return UGenArray();
	else
	{
		UGenArray newArray(size);
		
		for(int i = 0; i < size; i++)
		{
			const int index = indices.at(i);
			newArray.put(i, wrapAt(index));
		}
		
		return newArray;
	}
}

UGenArray UGenArray::range(const int startIndex, const int endIndex) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	const int startIndexChecked = clip(startIndex, 0, internal->size_);
	const int endIndexChecked = clip(endIndex, 0, internal->size_);
	
	const int size = endIndexChecked - startIndexChecked;
	
	if(size <= 0)
		return UGenArray();
	else
	{
		UGenArray newArray(size);
		
		for(int i = 0; i < size; i++)
		{
			newArray.put(i, at(i + startIndexChecked));
		}
		
		return newArray;
	}
}

UGenArray UGenArray::from(const int startIndex) const throw()
{
	return range(startIndex);
}

UGenArray UGenArray::to(const int endIndex) const throw()
{
	return range(0, endIndex);
}

UGenArray UGenArray::range(const int startIndex) const throw()
{
	return range(startIndex, 0x7fffffff);
}


int UGenArray::indexOf(UGen const& itemsToSearchFor, const int startIndex) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return -1;
	
	for(int i = startIndex < 0 ? 0 : startIndex; i < internal->size_; i++)
	{
		if(internal->array[i].containsIdenticalInternalsAs(itemsToSearchFor))
			return i;
	}
	
	return -1;
}

bool UGenArray::contains(UGen const& itemsToSearchFor) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return false;
	
	for(int i = 0; i < internal->size_; i++)
	{
		if(internal->array[i].containsIdenticalInternalsAs(itemsToSearchFor))
			return true;
	}
	
	return false;
}

const UGen& UGenArray::getUGenWithUserData(const int userDataToSearchFor) const throw()
{
	ugen_assert(internal != 0);
	
	if(internal != 0) 
	{
		for(int i = 0; i < internal->size_; i++)
		{
			if(internal->array[i].userData == userDataToSearchFor)
				return internal->array[i];
		}
	}
	
	return UGen::getNull();
}

void UGenArray::release() throw()
{
	ugen_assert(internal != 0);
	
	if(internal != 0) 
	{
		for(int i = 0; i < internal->size_; i++)
		{
			internal->array[i].release();
		}
	}
}

UGenArray UGenArray::interleave() const throw()
{
	// need to check this
	
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	int maxNumChannels = findMaxNumChannels();
	UGenArray interleavedArray(maxNumChannels);
	
	for(int channel = 0; channel < maxNumChannels; channel++)
	{
		UGen item;
		
		for(int arrayIndex = 0; arrayIndex < internal->size_; arrayIndex++)
		{
			item <<= wrapAt(arrayIndex).wrapAt(channel);
		}
		
		interleavedArray.put(channel, item);
	}
	
	return interleavedArray;
}

UGen UGenArray::mixEach() const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGen::getNull();
	
	UGen mixedArray;
	
	for(int i = 0; i < internal->size_; i++)
	{
		mixedArray <<= internal->array[i].mix();
	}
	
	return mixedArray;
}

UGen UGenArray::mix() const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGen::getNull();
	
//	UGen mixedArray;
//	
//	int maxNumChannels = findMaxNumChannels();
//	
//	for(int outputIndex = 0; outputIndex < maxNumChannels; outputIndex++)
//	{
//		UGen item;
//		
//		for(int arrayIndex = 0 ; arrayIndex < internal->size_; arrayIndex++)
//		{
//			item <<= internal->array[arrayIndex].wrapAt(outputIndex);
//		}
//		
//		mixedArray <<= item.mix();
//	}
//		
//	return mixedArray;
	
	return Mix(*this);
}

UGenArray UGenArray::operator- () const throw()
{
	ugen_assert(internal != 0);
	
	if(internal == 0) return UGenArray();
	
	UGenArray newArray(internal->size_);
	
	for(int i = 0; i < internal->size_; i++)
	{
		newArray.internal->array[i] = internal->array[i].neg();
	}
	
	return newArray;
}


END_UGEN_NAMESPACE