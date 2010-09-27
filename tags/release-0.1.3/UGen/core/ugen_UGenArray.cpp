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
#include "ugen_Arrays.h"


UGenArray::Internal::Internal(const int size) throw()
:	size_(size <= 0 ? 0 : size),
	allocatedSize(size_),
	array(size_ ? new UGen[size_] : 0)
{
}

UGenArray::Internal::~Internal() throw()
{
	delete [] array;
	array = 0;
	size_ = 0;
	allocatedSize = 0;
}

void UGenArray::Internal::add(UGen const& item) throw()
{
	if(allocatedSize > size_)
	{
		array[size_] = item;
		size_++;
	}
	else
	{
		UGen *newArray = new UGen[size_ +  1];
		
		for(int i = 0; i < size_; i++)
		{
			newArray[i] = array[i];
		}
		
		newArray[size_] = item;
		
		delete [] array;
		size_++;
		array = newArray;
		allocatedSize = size_;
	}
}

void UGenArray::Internal::add(const int numItems, const UGen* items) throw()
{	
	ugen_assert(numItems > 0);
	ugen_assert(items != 0);
	
	const int newSize = size_ +  numItems;
	
	if(allocatedSize >= newSize)
	{
		for(int i = size_; i < newSize; i++)
		{
			array[i] = *items++;
		}		
	}
	else 
	{
		UGen *newArray = new UGen[size_ +  numItems];
		
		for(int i = 0; i < size_; i++)
		{
			newArray[i] = array[i];
		}
		
		for(int i = size_; i < newSize; i++)
		{
			newArray[i] = *items++;
		}
		
		delete [] array;
		size_ = newSize;
		array = newArray;	
		allocatedSize = size_;
	}
}

void UGenArray::Internal::remove(const int index, const bool reallocate) throw()
{	
	if(index < 0 || index >= size_) return;
	
	size_--;
	
	for(int i = index; i < size_; i++)
	{
		array[i] = array[i+1];
	}
	
	if(reallocate)
	{
		this->reallocate();
	}
	else
	{
		array[size_] = UGen::getNull();
	}
}

void UGenArray::Internal::removeNulls(const bool reallocate) throw()
{
	int numNull = 0;
	
	for(int i = 0; i < size_; i++)
	{
		if(array[i].isNull())
			numNull++;
	}
	
	if(numNull == 0) return;
	
	const int newSize = size_ - numNull;
	UGen *newArray;
	
	if(reallocate)
	{
		newArray = new UGen[newSize];
		int newIndex = 0;
		
		for(int oldIndex = 0; oldIndex < size_; oldIndex++)
		{
			UGen& item = array[oldIndex];
			
			if(item.isNotNull())
			{
				newArray[newIndex] = item;
				newIndex++;
			}
		}
		
		delete [] array;
		size_ = newSize;
		array = newArray;
		allocatedSize = size_;
	}
	else
	{
		newArray = array;
		
		for(int i = 0; i < size_; i++)
		{
			UGen& item = array[i];
			
			if(item.isNotNull())
			{
				*newArray++ = item;
			}
		}
		
		for(int i = newSize; i < size_; i++)
		{
			array[i] = UGen::getNull();
		}
		
		size_ = newSize;
	}
}

void UGenArray::Internal::reallocate() throw()
{
	if(size_ > 0)
	{
		UGen *newArray = new UGen[size_];
		
		for(int i = 0; i < size_; i++)
		{
			newArray[i] = array[i];
		}
		
		delete [] array;
		array = newArray;
		allocatedSize = size_;
	}
}

void UGenArray::Internal::clear() throw()
{
	delete [] array;
	array = 0;
	size_ = 0;
	allocatedSize = 0;
}

void UGenArray::Internal::clearQuick() throw()
{
	for(int i = 0; i < size_; i++)
	{
		array[i] = UGen::getNull();
	}
	
	size_ = 0;
}

UGenArray::UGenArray(const int size) throw()
:	internal(new Internal(size))
{
}

UGenArray::UGenArray(UGen const& ugen) throw()
:	internal(new Internal(1))
{
	internal->getArray()[0] = ugen;
}

UGenArray::UGenArray(ObjectArray<UGen> const& array) throw()
:	internal((array.length() <= 0) ? 0 : new Internal(array.length()))
{
	if(internal != 0)
	{
		for(int i = 0; i < internal->size(); i++)
		{
			internal->getArray()[i] = array[i];
		}
	}
}

UGenArray::operator const ObjectArray<UGen>() const throw()
{
	if(internal == 0) return ObjectArray<UGen>();
	
	ObjectArray<UGen> newArray = ObjectArray<UGen>::withSize(internal->size());
	
	for(int i = 0; i < internal->size(); i++)
	{
		newArray[i] = internal->getArray()[i];
	}
	
	return newArray;
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
			internal = new Internal(newSize);
			
			int newIndex = 0;
			
			for(int i = 0; i < array0.size(); i++) {
				internal->getArray()[newIndex++] = array0.internal->getArray()[i];
			}
			
			for(int i = 0; i < array1.size(); i++) {
				internal->getArray()[newIndex++] = array1.internal->getArray()[i];
			}
		}
	}
	else
	{	
		int newSize = array0.sizeNotNull() + array1.sizeNotNull();
		
		if(newSize != 0) {
			internal = new Internal(newSize);
			
			int newIndex = 0;
			
			for(int i = 0; i < array0.size(); i++) {
				if(array0.internal->getArray()[i].isNotNull())
					internal->getArray()[newIndex++] = array0.internal->getArray()[i];
			}
			
			for(int i = 0; i < array1.size(); i++) {
				if(array1.internal->getArray()[i].isNotNull())
					internal->getArray()[newIndex++] = array1.internal->getArray()[i];
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
	if(internal->size() == 0) return 0;
		
	int maxNumChannels = 0;
	
	for(int i = 0; i < internal->size(); i++)
	{
		int numChannels = internal->getArray()[i].getNumChannels();
		maxNumChannels = numChannels > maxNumChannels ? numChannels : maxNumChannels;
	}
	
	return maxNumChannels;
}

void UGenArray::add(UGen const& other) throw()
{
	internal->add(other);
}

void UGenArray::add(UGenArray const& other) throw()
{
	internal->add(other.size(), other.getArray());
}

UGen UGenArray::remove(const int index, const bool reallocate) throw()
{
	UGen item = this->at(index);
	internal->remove(index, reallocate);
	return item;
}

void UGenArray::removeItem(UGen const& item, const bool reallocate) throw()
{
	int index = indexOf(item);
	
	if(index >= 0)
	{
		internal->remove(index, reallocate);
	}
}

void UGenArray::removeNulls() throw()
{
	internal->removeNulls();
}

void UGenArray::clear(bool quick) throw()
{
	if(quick)
	{
		internal->clear();
	}
	else
	{
		internal->clearQuick();
	}
}

void UGenArray::put(const int index, UGen const& item) throw()
{
	if(index < 0 || index >= internal->size()) 
	{ 
		ugen_assertfalse;
		return;
	}
	
	internal->getArray()[index] = item;
}

UGen& UGenArray::operator[] (const int index) throw()
{	
	if(index < 0 || index >= internal->size()) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[index];
}

UGen& UGenArray::at(const int index) throw()
{	
	if(index < 0 || index >= internal->size()) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[index];
}

const UGen& UGenArray::operator[] (const int index) const throw()
{	
	if(index < 0 || index >= internal->size()) 
	{
		return UGen::getNull();
	}
	
	return internal->getArray()[index];
}

const UGen& UGenArray::at(const int index) const throw()
{	
	if(index < 0 || index >= internal->size()) 
	{
		return UGen::getNull();
	}
	
	return internal->getArray()[index];
}

UGen& UGenArray::wrapAt(const int index) throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}

	int indexToUse = index;
	while(indexToUse < 0)
		indexToUse += internal->size();
	
	return internal->getArray()[indexToUse % internal->size()];
}

const UGen& UGenArray::wrapAt(const int index) const throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	int indexToUse = index;
	while(indexToUse < 0)
		indexToUse += internal->size();
	
	return internal->getArray()[indexToUse % internal->size()];
}

UGen& UGenArray::first() throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[0];
}

UGen& UGenArray::last() throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[internal->size() - 1];
}

const UGen& UGenArray::first() const throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[0];
}

const UGen& UGenArray::last() const throw()
{	
	if(internal->size() == 0) 
	{
		static UGen null = UGen::getNull();
		return null;
	}
	
	return internal->getArray()[internal->size() - 1];
}

UGenArray UGenArray::at(IntArray const& indices) const throw()
{	
	if(internal->size() == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
	{
		return UGenArray();
	}
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
	if(internal->size() == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
	{
		return UGenArray();
	}
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
	if(internal->size() == 0) return UGenArray();
	
	const int size = indices.length();
	
	if(size == 0)
	{
		return UGenArray();
	}
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
	
	if(internal->size() == 0) return UGenArray();
	
	const int startIndexChecked = clip(startIndex, 0, internal->size());
	const int endIndexChecked = clip(endIndex, 0, internal->size());
	
	const int size = endIndexChecked - startIndexChecked;
	
	if(size <= 0)
	{
		return UGenArray();
	}
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
	if(internal->size() == 0) return -1;
	
	for(int i = startIndex < 0 ? 0 : startIndex; i < internal->size(); i++)
	{
		if(internal->getArray()[i].containsIdenticalInternalsAs(itemsToSearchFor))
			return i;
	}
	
	return -1;
}

bool UGenArray::contains(UGen const& itemsToSearchFor) const throw()
{	
	if(internal->size() == 0) return false;
	
	for(int i = 0; i < internal->size(); i++)
	{
		if(internal->getArray()[i].containsIdenticalInternalsAs(itemsToSearchFor))
			return true;
	}
	
	return false;
}

const UGen& UGenArray::getUGenWithUserData(const int userDataToSearchFor) const throw()
{	
	for(int i = 0; i < internal->size(); i++)
	{
		if(internal->getArray()[i].userData == userDataToSearchFor)
			return internal->getArray()[i];
	}
	
	return UGen::getNull();
}

void UGenArray::release() throw()
{	
	for(int i = 0; i < internal->size(); i++)
	{
		internal->getArray()[i].release();
	}
}

void UGenArray::release(const int userDataToSearchFor) throw()
{	
	for(int i = 0; i < internal->size(); i++)
	{
		const UGen& item = internal->getArray()[i];
		if(item.userData == userDataToSearchFor)
			internal->getArray()[i].release();
	}
}

UGenArray UGenArray::interleave() const throw()
{
	// need to check this
		
	if(internal->size() == 0) return UGenArray();
	
	int maxNumChannels = findMaxNumChannels();
	UGenArray interleavedArray(maxNumChannels);
	
	for(int channel = 0; channel < maxNumChannels; channel++)
	{
		UGen item;
		
		for(int arrayIndex = 0; arrayIndex < internal->size(); arrayIndex++)
		{
			item <<= wrapAt(arrayIndex).wrapAt(channel);
		}
		
		interleavedArray.put(channel, item);
	}
	
	return interleavedArray;
}

UGen UGenArray::mixEach() const throw()
{	
	if(internal->size() == 0) return UGen::getNull();
	
	UGen mixedArray;
	
	for(int i = 0; i < internal->size(); i++)
	{
		mixedArray <<= internal->getArray()[i].mix();
	}
	
	return mixedArray;
}

UGen UGenArray::mix() const throw()
{	
	if(internal->size() == 0) return UGen::getNull();
		
	return Mix(*this);
}

UGenArray UGenArray::operator- () const throw()
{	
	if(internal->size() == 0) return UGenArray();
	
	UGenArray newArray(internal->size());
	
	for(int i = 0; i < internal->size(); i++)
	{
		newArray.internal->getArray()[i] = internal->getArray()[i].neg();
	}
	
	return newArray;
}


END_UGEN_NAMESPACE