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

#ifndef _UGEN_ugen_Arrays_H_
#define _UGEN_ugen_Arrays_H_

#include "ugen_SmartPointer.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "ugen_Random.h"



template<class ObjectType>
class ObjectArrayInternal : public SmartPointer
{
public:	
	ObjectArrayInternal(const int size, const bool isNullTerminated)
	:	size_(size <= 0 ? 0 : size), 
		array(size_ == 0 ? 0 : new ObjectType[size_]),
		arrayIsNullTerminated(isNullTerminated),
		ownsTheData(true)
	{
	}
	
	ObjectArrayInternal(const int size, ObjectType *dataToUse, const bool isNullTerminated)
	:	size_(size <= 0 ? 0 : size), 
		array(size_ == 0 ? 0 : dataToUse),
		arrayIsNullTerminated(isNullTerminated),
		ownsTheData(size_ == 0 ? true : false)
	{
	}
	
	~ObjectArrayInternal()
	{
		if(ownsTheData) 
			delete [] array;
	}
	
	inline int size() const throw() { return size_; }
	inline int length() const throw() 
	{ 
		if(size_ == 0) 
			return 0;
		else
			return arrayIsNullTerminated ? size_ - 1 : size_; 
	}
	
	inline ObjectType* getArray() throw() { return array; }
	inline const ObjectType* getArray() const throw() { return array; }
	inline bool isNullTerminated() const throw() { return arrayIsNullTerminated; }
	inline void setNullTerminated(const bool state) throw() { arrayIsNullTerminated = state; } 
	
	inline void add(ObjectType const& item) throw()
	{
		const int newSize = size_ + 1;
		ObjectType *newArray = new ObjectType[newSize];
		
		if(arrayIsNullTerminated)
		{
			const int length = size_ - 1;
			
			newArray[size_] = array[length];
			newArray[length] = item;
			
			for(int i = 0; i < length; i++)
			{
				newArray[i] = array[i];
			}
		}
		else
		{
			const int length = size_;
			newArray[size_] = item;
			
			for(int i = 0; i < length; i++)
			{
				newArray[i] = array[i];
			}
		}
		
		if(ownsTheData)
			delete [] array;
		
		size_ = newSize;
		array = newArray;
	}
	
	inline void remove(const int index) throw()
	{		
		if(index < 0 || index >= size_) return;
		
		if(size_ == 1)
		{
			if(ownsTheData)
				delete [] array;
			
			size_ = 0;
		}
		else
		{
			const int newSize = size_ - 1;
			ObjectType *newArray = new ObjectType[newSize];
			
			for(int i = 0; i < index; i++)
			{
				newArray[i] = array[i];
			}
			
			for(int i = index; i < newSize; i++)
			{
				newArray[i] = array[i+1];
			}
			
			if(ownsTheData)
				delete [] array;
			
			size_ = newSize;
			array = newArray;
		}
	}
	
private:
	int size_;
	ObjectType *array;
	bool arrayIsNullTerminated;
	bool ownsTheData;

	
	ObjectArrayInternal();
	ObjectArrayInternal (const ObjectArrayInternal&);
	const ObjectArrayInternal& operator= (const ObjectArrayInternal&);
};


#define ObjectArrayConcatOperatorsDefine(Base, Type)					\
	Base<Type> operator<< (Base<Type> const& other) const throw()		\
	{																	\
		return Base<Type>(*this, other);								\
	}																	\
																		\
	Base<Type> operator<< (Type const& other) const throw()				\
	{																	\
		return Base<Type>(*this, other);								\
	}																	\
																		\
	Base<Type> operator, (Base<Type> const& other) const throw()		\
	{																	\
		return Base<Type>(*this, other);								\
	}																	\
																		\
	Base<Type> operator, (Type const& other) const throw()				\
	{																	\
		return Base<Type>(*this, other);								\
	}																	\
																		\
	Base<Type>& operator<<= (Base<Type> const& other) throw()			\
	{																	\
		return operator= (Base<Type>(*this, other));					\
	}																	\
																		\
	Base<Type>& operator<<= (Type const& other) throw()					\
	{																	\
		return operator= (Base<Type>(*this, other));					\
	}


template<class ObjectType>
class ObjectArray :	public SmartPointerContainer< ObjectArrayInternal<ObjectType> >
{
public:
	class InitialObject
	{
	public:
		InitialObject() throw() : valid(false) { }
		InitialObject(ObjectType initialObject) throw() 
		:	object(initialObject), valid(true) 
		{ 
		}
		InitialObject(const char* initialObject) throw()  // hack for Text
		:	object(initialObject), valid(true) 
		{ 
		}
		
		ObjectType object;
		const bool valid;
	};
	
	
	ObjectArray() throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> > //(0)
		(new ObjectArrayInternal<ObjectType>(0, false))
	{
	}
		
	ObjectArray(const int size, const bool needsNullTermination) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >
			(new ObjectArrayInternal<ObjectType>(size, needsNullTermination))
	{
	}
	
	static ObjectArray<ObjectType> withSize(const int size) throw()
	{
		return ObjectArray<ObjectType>(size, false);
	}	
	
	ObjectArray(const int size, ObjectType *dataToUse, const bool needsNullTermination = false) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >
			(new ObjectArrayInternal<ObjectType>(size, dataToUse, needsNullTermination))
	{
	}
	
	
//	ObjectArray(const int size, ObjectType *dataToUse, const bool needsNullTermination = false) throw()
//	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> > (size, dataToUse, needsNullTermination)
//	{
//	}
	
	ObjectArray(ObjectArray<ObjectType> const& copy) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >(copy)
	{
	}
	
	template<class CopyType>
	ObjectArray(ObjectArray<CopyType> const& copy) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >
		(new ObjectArrayInternal<ObjectType>(copy.size(), copy.isNullTerminated()))
	{
		const int size = this->size();
		for(int i = 0; i < size; i++)
		{
			this->put(i, (ObjectType)copy[i]);
		}
	}
	
	template<class CollectionType>
	static ObjectArray<ObjectType> collect(ObjectArray<CollectionType> const& collection) throw()
	{
		const int length = collection.length();
		
		ObjectArray<ObjectType> result(length, false);
		
		for(int i = 0; i < length; i++)
		{
			result.put(i, ObjectType(collection[i]));
		}
		
		return result;
	}
	
	template<class CollectionType1, class CollectionType2>
	static ObjectArray<ObjectType> collect(ObjectArray<CollectionType1> const& collection1,
										   ObjectArray<CollectionType2> const& collection2) throw()
	{
		const int length = ugen::max(collection1.length(), collection2.length());
		
		ObjectArray<ObjectType> result(length, false);
		
		for(int i = 0; i < length; i++)
		{
			result.put(i, ObjectType(collection1.wrapAt(i), 
									 collection2.wrapAt(i)));
		}
		
		return result;
	}
	
	template<class CollectionType1, 
			 class CollectionType2, 
			 class CollectionType3>
	static ObjectArray<ObjectType> collect(ObjectArray<CollectionType1> const& collection1,
										   ObjectArray<CollectionType2> const& collection2,
										   ObjectArray<CollectionType3> const& collection3) throw()
	{
		const int length = ugen::max(ugen::max(collection1.length(), 
											   collection2.length()), 
											   collection3.length());
		
		ObjectArray<ObjectType> result(length, false);
		
		for(int i = 0; i < length; i++)
		{
			result.put(i, ObjectType(collection1.wrapAt(i), 
									 collection2.wrapAt(i),
									 collection3.wrapAt(i)));
		}
		
		return result;
	}
	
	template<class CollectionType1, 
			 class CollectionType2, 
			 class CollectionType3, 
			 class CollectionType4>
	static ObjectArray<ObjectType> collect(ObjectArray<CollectionType1> const& collection1,
										   ObjectArray<CollectionType2> const& collection2,
										   ObjectArray<CollectionType3> const& collection3,
										   ObjectArray<CollectionType4> const& collection4) throw()
	{
		const int length = ugen::max(ugen::max(ugen::max(collection1.length(), 
														 collection2.length()), 
														 collection3.length()),
														 collection4.length());
		
		ObjectArray<ObjectType> result(length, false);
		
		for(int i = 0; i < length; i++)
		{
			result.put(i, ObjectType(collection1.wrapAt(i), 
									 collection2.wrapAt(i),
									 collection3.wrapAt(i),
									 collection4.wrapAt(i)));
		}
		
		return result;
	}
	
	ObjectArray<ObjectType> copy() const throw()
	{
		const int size = this->size();
		ObjectArray<ObjectType> newArray = ObjectArray<ObjectType>(size, this->isNullTerminated());
		ObjectType *newArrayPtr = newArray.getArray();
		const ObjectType *thisArrayPtr = this->getArray();
		for(int i = 0; i < size; i++)
		{
			newArrayPtr[i] = thisArrayPtr[i];
		}
		
		return newArray;
	}
	
	ObjectArray(ObjectType const& single) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >
		(new ObjectArrayInternal<ObjectType>(1, false))
	{
		this->put(0, single);
	}		
	
	static int countValidInitialisers(InitialObject const& i03) throw()
	{
		int size = 3;
		
		if(i03.valid) size++; else return size;
		
		return size;
	}	
	
	ObjectArray(InitialObject const &i00,
				InitialObject const &i01,
				InitialObject const &i02,
				InitialObject const &i03 = InitialObject()) throw()
	:	SmartPointerContainer< ObjectArrayInternal<ObjectType> >
		(new ObjectArrayInternal<ObjectType>(countValidInitialisers(i03), false))
	{
		ObjectType *thisArray = this->getArray();
		
		if(i00.valid) thisArray[ 0] = i00.object; else return;
		if(i01.valid) thisArray[ 1] = i01.object; else return;
		if(i02.valid) thisArray[ 2] = i02.object; else return;
		if(i03.valid) thisArray[ 3] = i03.object; else return;
	}
		
	ObjectArray(ObjectArray<ObjectType> const& array0, 
				ObjectArray<ObjectType> const& array1) throw()
	{
		const bool bothNullTerminated = array0.isNullTerminated() && array1.isNullTerminated();
		const bool eitherNullTerminated = array0.isNullTerminated() || array1.isNullTerminated();
		const int summedSizes = array0.size() + array1.size();
		const int newSize = bothNullTerminated ? summedSizes - 1 : summedSizes;
		
		if(newSize > 0)
		{
			this->setInternal(new ObjectArrayInternal<ObjectType>(newSize, eitherNullTerminated));
			
			int newIndex = 0;
			
			ObjectType *array = this->getArray();
			
			if(array != 0)
			{
				const ObjectType *arrayPtr0 = array0.getArray();
				const ObjectType *arrayPtr1 = array1.getArray();
				
				for(int i = 0; i < array0.length(); i++) {
					array[newIndex++] = arrayPtr0[i];
				}
								
				for(int i = 0; i < array1.length(); i++) {
					array[newIndex++] = arrayPtr1[i];
				}
				
				if(eitherNullTerminated)
					array[newIndex] = getNull();
			}
		}
	}
	
	/** Size is the actual number of elements of storage needed for the array.
	 If this is a null terminated array the null value is inlcuded in the size. */
	inline int size() const throw() 
	{ return this->getInternal() == 0 ? 0 : this->getInternal()->size(); }
	
	/** Length is the number of real elements of storage available.
	 For arrays that are NOT null terminated this is synonymous with size().
	 If this is a null terminated array the length is one less than the size. */
	inline int length() const throw() 
	{ return this->getInternal() == 0 ? 0 : this->getInternal()->length(); }
	
	/** Actual memory requirements of the array elements and the null terminator (if needed).
	 Equivalent to size() * sizeof(ObjectType). */
	inline int memorySize() const throw() 
	{ return this->getInternal() == 0 ? 0 : this->getInternal()->size() * sizeof(ObjectType); }
	
	inline ObjectType* getArray() throw() 
	{ ugen_assert(this->getInternal()); return this->getInternal() == 0 ? 0 : this->getInternal()->getArray(); }
	
	inline const ObjectType* getArray() const throw() 
	{ ugen_assert(this->getInternal()); return this->getInternal() == 0 ? 0 : this->getInternal()->getArray(); }
	
	inline operator const ObjectType*() const throw() { return this->getArray(); }
	inline operator ObjectType*() throw() { return this->getArray(); }
	
//	template<class ReturnType>
//	inline operator const ReturnType*() const throw() 
//	{ 
//		printf("const cast\n");
//		return (const ReturnType*)this->getArray(); 
//	}
//	
//	template<class ReturnType>
//	inline operator ReturnType*() throw() 
//	{ 
//		printf("non const cast\n");
//		return (ReturnType*)this->getArray(); 
//	}
	
	inline bool isNullTerminated() const throw()
	{ ugen_assert(this->getInternal()); return this->getInternal() == 0 ? false : this->getInternal()->isNullTerminated(); }
	
	inline void setNullTerminated(const bool state) throw()
	{ ugen_assert(this->getInternal()); if(this->getInternal() != 0) this->getInternal()->setNullTerminated(state); }
	
	ObjectArrayConcatOperatorsDefine(ObjectArray, ObjectType);
	
	/** Adds an item in place. */
	void add(ObjectType const& item) throw()
	{ 
		if(this->getInternal() != 0)
		{
			this->getInternal()->add(item); 
		}
		else
		{
			this->setInternal(new ObjectArrayInternal<ObjectType>(1, false));
			this->put(0, item);
		}
	}
	
	/** Adds several items in place. */
	void add(ObjectArray<ObjectType> const& other) throw()
	{
		const int length = other.length();
		for(int i = 0; i < length; i++)
		{
			add(other[i]);
		}
	}
	
	void remove(const int index) throw()
	{ 
		if(this->getInternal() != 0)
		{
			this->getInternal()->remove(index); 
		}
	}
	
	void put(const int index, ObjectType const& item) throw()
	{
		if(this->getInternal() == 0 || index < 0 || index >= this->size()) 
		{
			return;
		}
		
		this->getArray()[index] = item;
	}
	
	ObjectType& operator[] (const int index) throw()
	{
		if(this->getInternal() == 0 || index < 0 || index >= this->size()) 
		{
			return getNull();
		}
		else
			return this->getArray()[index];
	}
	
	const ObjectType& operator[] (const int index) const throw()
	{
		if(this->getInternal() == 0 || index < 0 || index >= this->size()) 
		{
			return getNull();
		}
		else
			return this->getArray()[index];
	}
	
	ObjectType& at (const int index) throw()
	{
		if(this->getInternal() == 0 || index < 0 || index >= this->size()) 
		{
			return getNull();
		}
		else
			return this->getArray()[index];
	}
	
	const ObjectType& at (const int index) const throw()
	{
		if(this->getInternal() == 0 || index < 0 || index >= this->size()) 
		{
			return getNull();
		}
		else
			return this->getArray()[index];
	}
	
	ObjectArray<ObjectType> at (ObjectArray<int> const& indices) const throw()
	{
		if(this->getInternal() == 0) { return *this; }
			
		const bool needsNull = this->isNullTerminated();
		const int length = indices.length();
		const int size = needsNull ? length + 1 : length;
		
		ObjectArray<ObjectType> result(size, needsNull);
		ObjectType* resultArray = result.getArray();
		const ObjectType* thisArray = this->getArray();
		
		const int thisSize = this->size();
		for(int i = 0; i < length; i++)
		{
			const int index = indices[i];
			resultArray[i] = index < 0 || index >= thisSize ? getNull() : thisArray[index];
		}
		
		if(needsNull)
			resultArray[length] = getNull();
		
		return result;
	}

	ObjectArray<ObjectType> atUnchecked (ObjectArray<int> const& indices) const throw()
	{		
		ugen_assert(this->getInternal() != 0);
		
		const bool needsNull = this->isNullTerminated();
		const int length = indices.length();
		const int size = needsNull ? length + 1 : length;
		
		ObjectArray<ObjectType> result(size, needsNull);
		ObjectType* resultArray = result.getArray();
		const ObjectType* thisArray = this->getArray();
		
		for(int i = 0; i < length; i++)
		{
			const int index = indices[i];
			resultArray[i] = thisArray[index];
		}
		
		if(needsNull)
			resultArray[length] = getNull();
		
		return result;
	}
	
	
	ObjectType& atUnchecked (const int index) throw()
	{
		ugen_assert(this->getInternal() != 0);
		return this->getInternal()->getArray()[index];
	}

	const ObjectType& atUnchecked (const int index) const throw()
	{
		ugen_assert(this->getInternal() != 0);
		return this->getInternal()->getArray()[index];
	}	
	
	ObjectType& wrapAt (const int index) throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) { return getNull(); }
		
		int indexToUse = index;
		while(indexToUse < 0)
			indexToUse += this->size();
		
		return this->getArray()[(unsigned int)indexToUse % (unsigned int)this->size()];
	}
	
	const ObjectType& wrapAt (const int index) const throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) { return getNull(); }
		
		int indexToUse = index;
		while(indexToUse < 0)
			indexToUse += this->size();
		
		return this->getArray()[(unsigned int)indexToUse % (unsigned int)this->size()];
	}
		
	ObjectType& first () throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else
			return this->getArray()[0];
	}
	
	const ObjectType& first () const throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else
			return this->getArray()[0];
	}
	
	ObjectType& last () throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else
			return this->getArray()[this->size() - 1];
	}
	
	const ObjectType& last () const throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
			return getNull();
		else
			return this->getArray()[this->size() - 1];
	}
	
	ObjectType& choose () throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else
			return this->getArray()[rand(this->length())];
	}
	
	const ObjectType& choose () const throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else
			return this->getArray()[rand(this->length())];
	}
	
	template<class WeightType>
	ObjectType& wchoose (ObjectArray<WeightType> const& weights) throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else 
		{
			const int length = this->length();
			ObjectArray<WeightType> thresholds = ObjectArray<WeightType>(length, false);
			
			WeightType sum = 0;
			for(int i = 0; i < length; i++)
			{
				sum += weights.wrapAt(i);
				thresholds[i] = sum;
			}
			
			WeightType random = rand(sum);
			
			for(int i = 0; i < length; i++)
			{
				if(random < thresholds[i])
					return this->getArray()[i];
			}
			
			return this->getArray()[length-1];
		}
	}
	
	template<class WeightType>
	const ObjectType& wchoose (ObjectArray<WeightType> const& weights) const throw()
	{
		if(this->getInternal() == 0 || this->size() == 0) 
		{
			return getNull();
		}
		else 
		{
			const int length = this->length();
			ObjectArray<WeightType> thresholds = ObjectArray<WeightType>(length, false);
			
			WeightType sum = 0;
			for(int i = 0; i < length; i++)
			{
				sum += weights.wrapAt(i);
				thresholds[i] = sum;
			}
			
			WeightType random = rand(sum);
			
			for(int i = 0; i < length; i++)
			{
				if(random < thresholds[i])
					return this->getArray()[i];
			}
			
			return this->getArray()[length-1];
		}
	}
	
	static ObjectType& getNull() throw()
	{
		static ObjectType null;
		null = ObjectType();
		return null;
	}	
	
	bool operator== (ObjectArray<ObjectType> const& other) const throw()
	{
		const int size = this->size();
		if(size != other.size()) return false;
		if(size == 0) return true;
		
		const ObjectType *thisArray = this->getArray();
		const ObjectType *otherArray = other.getArray();
		
		
		for(int i = 0; i < size; i++)
		{
			if(thisArray[i] != otherArray[i])
				return false;
		}
		
		return true;
	}
	
	bool operator== (const ObjectType* otherArray) const throw()
	{		
		ugen_assert(otherArray != 0);
		
		const ObjectType *thisArray = this->getArray();
		
		const int size = this->size();
		for(int i = 0; i < size; i++)
		{
			if(thisArray[i] != otherArray[i])
				return false;
		}
		
		return true;
	}
	
	bool operator!= (ObjectArray<ObjectType> const& other) const throw()
	{
		return !operator== (other);
	}
	
	bool operator!= (const ObjectType* otherArray) const throw()
	{
		return !operator== (otherArray);
	}
	
	bool contains (ObjectType const& itemToSearchFor) const throw()
	{
		const ObjectType *array = this->getArray();
		
		if(array != 0)
		{
			const int size = this->size();
			for(int i = 0; i < size; i++)
			{
				if(array[i] == itemToSearchFor)
					return true;
			}
		}		
		
		return false;
	}
		
	bool contains (ObjectArray<ObjectType> const& itemsToSearchFor) const throw()
	{
		const ObjectType *array = this->getArray();
		const ObjectType *items = itemsToSearchFor.getArray();
		
		if(array != 0 && items != 0)
		{
			const int length = this->length();
			int itemsToFind = itemsToSearchFor.length();
			
			if(length == 0 || itemsToFind == 0)
				return false;
			
			int nextIndexToFind = 0;
			
			for(int i = 0; i < length; i++)
			{
				if(array[i] == items[nextIndexToFind])
				{
					itemsToFind--;
					
					while(itemsToFind > 0)
					{
						i++;
						nextIndexToFind++;
						
						if((i > length) || (array[i] != items[nextIndexToFind]))
							return false;
						
						itemsToFind--;
					}
					
					return true;
				}
			}
		}		
		
		return false;
	}
		
	int indexOf (ObjectType const& itemToSearchFor, const int startIndex = 0) const throw()
	{
		const ObjectType *array = this->getArray();
		
		if(array != 0)
		{
			const int size = this->size();
			for(int i = startIndex < 0 ? 0 : startIndex; i < size; i++)
			{
				if(array[i] == itemToSearchFor)
					return i;
			}
		}		
		
		return -1;
	}
	
	/** Search for a particular sub sequence withing the array. */
	int indexOf (ObjectArray<ObjectType> const& itemsToSearchFor, const int startIndex = 0) const throw()
	{
		const ObjectType *array = this->getArray();
		const ObjectType *items = itemsToSearchFor.getArray();

		if(array != 0 && items != 0)
		{
			const int length = this->length();
			int itemsToFind = itemsToSearchFor.length();
			
			if(length == 0 || itemsToFind == 0)
				return -1;
			
			int nextIndexToFind = 0;
			
			for(int i = startIndex < 0 ? 0 : startIndex; i < length; i++)
			{
				if(array[i] == items[nextIndexToFind])
				{
					int foundAtIndex = i;
					
					itemsToFind--;
					
					while(itemsToFind > 0)
					{
						i++;
						nextIndexToFind++;
						
						if((i > length) || (array[i] != items[nextIndexToFind]))
							return -1;
						
						itemsToFind--;
					}
					
					return foundAtIndex;
				}
			}
		}		
		
		return -1;
	}
	
	/** Search for any items in a given array. */
	int indexOfAny (ObjectArray<ObjectType> const& itemsToSearchFor, const int startIndex = 0) const throw()
	{
		const ObjectType *array = this->getArray();
		
		if(array != 0)
		{
			const int length = this->length();
			const int itemsToFind = itemsToSearchFor.length();
			
			if(length == 0 || itemsToFind == 0)
				return -1;
			
			for(int i = startIndex < 0 ? 0 : startIndex; i < length; i++)
			{
				if(itemsToSearchFor.contains(array[i]))
					return i;
			}
		}
		
		return -1;
	}
	
	ObjectArray<ObjectType> from(const int startIndex) const throw()
	{
		return range(startIndex);
	}
	
	ObjectArray<ObjectType> range(const int startIndex) const throw()
	{
		return this->range(startIndex, 0x7fffffff);
	}
	
	ObjectArray<ObjectType> to(const int endIndex) const throw()
	{
		return range(0, endIndex);
	}
	
	ObjectArray<ObjectType> range(const int startIndex, const int endIndex) const throw()
	{
		const int size = this->size();
		if(size == 0) return ObjectArray<ObjectType>();
		
		const int startIndexChecked = startIndex < 0 ? 0 : startIndex;
		const int endIndexChecked = endIndex > size ? size : endIndex;

		if(startIndexChecked >= endIndexChecked) return ObjectArray<ObjectType>();
		
		const ObjectType *sourceArray = this->getArray();
		if(sourceArray == 0) return ObjectArray<ObjectType>();
		
		const bool needsNull = this->isNullTerminated();
		
		const int diff = endIndexChecked - startIndexChecked;
		const int newSize = needsNull && (endIndexChecked < size) ? diff + 1 : diff;
				
		ObjectArray<ObjectType> result(newSize, needsNull);
		ObjectType *resultArray = result.getArray();
		
		int resultIndex = 0;
		for(int sourceIndex = startIndexChecked; sourceIndex < endIndexChecked; sourceIndex++)
		{
			resultArray[resultIndex++] = sourceArray[sourceIndex];
		}		
		
		if(needsNull)
			resultArray[resultIndex] = this->getNull();
		
		return result;
	}
	
	ObjectArray<ObjectType> replace(ObjectArray<ObjectType> const& find, ObjectArray<ObjectType> const& substitute) const throw()
	{
		const int size = this->size();
		const int findLength = find.length();
		int startIndex = 0;
		int findIndex;
		
		ObjectArray<ObjectType> result;
		
		while((findIndex = this->indexOf(find, startIndex)) >= 0)
		{
			result <<= this->range(startIndex, findIndex);
			result <<= substitute;
			startIndex = findIndex + findLength;
		}
		
		result <<= this->range(startIndex, size);
		
		return result;
	}
	
	ObjectArray<ObjectType> remove(const ObjectType item) const throw()
	{
		const ObjectType *array = this->getArray();
		
		ObjectArray<ObjectType> result;
		
		if(array != 0)
		{
			const int length = this->length();
			for(int i = 0; i < length; i++)
			{
				if(array[i] != item)
					result <<= array[i];
			}
		}
		
		return result;
	}
	
	ObjectArray<ObjectType> remove(ObjectArray<ObjectType> const& items) const throw()
	{
		const ObjectType *array = this->getArray();
		
		ObjectArray<ObjectType> result;
		
		if(array != 0)
		{
			const int length = this->length();
			for(int i = 0; i < length; i++)
			{
				if(items.contains(array[i]) == false)
					result <<= array[i];
			}
		}
		
		return result;
	}
	
	
	ObjectArray<ObjectArray<ObjectType> > group(const int groupSize) const throw()
	{
		const int length = this->length();
		if(length == 0) return ObjectArray<ObjectType>();
		
		const int groupSizeChecked = groupSize < 1 ? 1 : groupSize;
		const int groupSizeMinus1 = groupSizeChecked-1;
		const bool needsNull = this->isNullTerminated();
		
		ObjectArray< ObjectArray<ObjectType> > result;
		ObjectArray<ObjectType> element;
		
		const ObjectType *thisArray = this->getArray();
		
		for(int i = 0; i < length; i++)
		{
			element <<= thisArray[i];
			if((i % groupSizeChecked) == groupSizeMinus1)
			{
				if(needsNull) 
				{
					element <<= this->getNull();
					element.setNullTerminated(true);
				}
				
				result <<= element;
				element = ObjectArray<ObjectType>();
			}
		}
		
		if(element.length() != 0)
		{
			if(needsNull) 
			{
				element <<= this->getNull();
				element.setNullTerminated(true);
			}
			
			result <<= element;
		}
		
		return result;
	}
	
	ObjectArray<ObjectArray<ObjectType> > split(ObjectArray<ObjectType> delimiters) const throw()
	{
		const int length = this->length();
		if(length == 0) return ObjectArray<ObjectType>();
		
		const bool needsNull = this->isNullTerminated();
		
		ObjectArray< ObjectArray<ObjectType> > result;
		ObjectArray<ObjectType> element;
		
		const ObjectType *thisArray = this->getArray();
		if(thisArray != 0)
		{
			for(int i = 0; i < length; i++)
			{
				const ObjectType item = thisArray[i];
				
				if(delimiters.contains(item))
				{
					if(element.length() > 0)
					{
						if(needsNull) 
						{
							element <<= this->getNull();
							element.setNullTerminated(true);
						}
						
						result <<= element;
						element = ObjectArray<ObjectType>();	
					}
				}
				else
				{
					element <<= item;
				}
			}
			
			if(element.length() != 0)
			{
				if(needsNull) 
				{
					element <<= this->getNull();
					element.setNullTerminated(true);
				}
				
				result <<= element;
			}
		}
		
		return result;		
	}
	
	ObjectArray<ObjectArray<ObjectType> > splitSequence(ObjectArray<ObjectType> delimitingSequence) const throw()
	{
		const int size = this->size();
		ObjectArray< ObjectArray<ObjectType> > result;
		
		if(size != 0)
		{
			const int findLength = delimitingSequence.length();
			int startIndex = 0;
			int findIndex;
			
			while((findIndex = this->indexOf(delimitingSequence, startIndex)) >= 0)
			{
				result <<= this->range(startIndex, findIndex);
				startIndex = findIndex + findLength;
			}
			
			result <<= this->range(startIndex, size);
		}
		
		return result;		
	}
	
};

#define ObjectArrayAssignmentDefinition(DerivedArrayType, ElementType)										\
			DerivedArrayType& operator= (ObjectArray<ElementType> const& other) throw()						\
			{																								\
				return operator= (static_cast<DerivedArrayType const&> (other) );							\
			}


// these following macors need to take into account that arrays might be null terminated!
#define NumericalArrayBinaryOperatorBodyCommon																\
			const unsigned int leftSize = this->size();														\
			if(leftSize == 0) return rightOperand;															\
			const unsigned int rightSize = rightOperand.size();												\
			if(rightSize == 0) return *this;																\
			const bool eitherNullTerminated = this->isNullTerminated() || rightOperand.isNullTerminated();	\
			const unsigned int leftLength = this->length();													\
			const unsigned int rightLength = rightOperand.length();											\
			const int newLength = leftLength > rightLength ? leftLength : rightLength;						\
			const int newSize = leftSize > rightSize ? leftSize : rightSize;								\
			const NumericalType *leftArray = this->getArray();										\
			const NumericalType *rightArray = rightOperand.getArray()


#define NumericalArrayBinaryOperatorBody(OPERATOR)															\
			NumericalArrayBinaryOperatorBodyCommon;															\
			NumericalArray<NumericalType> result(NumericalArraySpec(newSize, false), eitherNullTerminated);	\
			NumericalType *resultArray = result.getArray();													\
			if(eitherNullTerminated) resultArray[newLength] = 0;											\
																											\
			for(unsigned int i = 0; i < newLength; i++)	{													\
				resultArray[i] = leftArray[i % leftLength] OPERATOR rightArray[i % rightLength];			\
			}																								\
			return result

#define NumericalArrayBinaryComparisonOperatorBody(OPERATOR)												\
			NumericalArrayBinaryOperatorBodyCommon;															\
			NumericalArray<bool> result(NumericalArraySpec(newSize, false), false);							\
			bool *resultArray = result.getArray();															\
																											\
			for(unsigned int i = 0; i < newLength; i++)	{													\
				resultArray[i] = leftArray[i % leftLength] OPERATOR rightArray[i % rightLength];			\
			}																								\
			return result

#define NumericalArrayBinaryFunctionBody(FUNCTION)															\
			NumericalArrayBinaryOperatorBodyCommon;															\
			NumericalArray<NumericalType> result(NumericalArraySpec(newSize, false), eitherNullTerminated);	\
			NumericalType *resultArray = result.getArray();													\
			if(eitherNullTerminated) resultArray[newLength] = 0;											\
																											\
			for(unsigned int i = 0; i < newLength; i++) {													\
				resultArray[i] = FUNCTION((double)(leftArray[i % leftLength]),								\
										  (double)(rightArray[i % rightLength]));							\
			}																								\
			return result

#define NumericalArrayBinaryFunctionDefinition(FUNCTION)																	\
			NumericalArray<NumericalType> FUNCTION (NumericalArray<NumericalType> const& rightOperand) const throw()		\
			{																												\
				NumericalArrayBinaryFunctionBody(ugen::FUNCTION);															\
			}

#define NumericalArrayUnaryFunctionBody(FUNCTION)															\
			const int newSize = this->size();																\
			const int newLength = this->length();															\
			if(newSize == 0) return *this;																	\
			const bool needsNull = this->isNullTerminated();												\
			NumericalArray<NumericalType> result(NumericalArraySpec(newSize, false), needsNull);			\
			const NumericalType *thisArray = this->getArray();										\
			NumericalType *resultArray = result.getArray();													\
																											\
			for(int i = 0; i < newSize; i++)																\
				resultArray[i] = FUNCTION((double)thisArray[i]);											\
																											\
			if(needsNull) resultArray[newLength] = 0;														\
			return result

#define NumericalArrayUnaryFunctionDefinition(FUNCTION)														\
			NumericalArray<NumericalType> FUNCTION () const throw()											\
			{																								\
				NumericalArrayUnaryFunctionBody(ugen::FUNCTION);											\
			}

class NumericalArraySpec
{
public:
	NumericalArraySpec(const int size, const bool zeroData) throw() 
	:	size_(size), 
		zeroData_(zeroData) 
	{ 
		ugen_assert(size > 0);
	}
	
	const int size_;
	const bool zeroData_;
};

#define END InitialNumber()


template<class NumericalType>
class NumericalArray : public ObjectArray<NumericalType>
{
public:
	
	class InitialNumber
	{
	public:
		InitialNumber() throw() : value(0), valid(false) { }
		InitialNumber(NumericalType initialValue) throw()
		:	value(initialValue), valid(true) 
		{ 
		}
		
		const NumericalType value;
		const bool valid;
	};
	
	NumericalArray() throw()
	:	ObjectArray<NumericalType>(0, false)
	{
	}	
	
	static void roundCopy(const double inValue, char& outValue) throw()
	{
		outValue = char(inValue+0.5);
	}
	
	static void roundCopy(const double inValue, short& outValue) throw()
	{
		outValue = short(inValue+0.5);
	}
	
	static void roundCopy(const double inValue, int& outValue) throw()
	{
		outValue = int(inValue+0.5);
	}
	
	static void roundCopy(const double inValue, float& outValue) throw()
	{
		outValue = (float)inValue;
	}
	
	static void roundCopy(const double inValue, double& outValue) throw()
	{
		outValue = inValue;
	}	
	
	template<class CopyType>
	NumericalArray(NumericalArray<CopyType> const& copy) throw()
	:	ObjectArray<NumericalType>(copy.size(), copy.isNullTerminated())
	{		
		NumericalType *thisArray = this->getArray();
		const CopyType *copyArray = copy.getArray();
		
		if(thisArray != 0 && copyArray != 0)
		{
			const int size = this->size();
			for(int i = 0; i < size; i++)
			{
				roundCopy(copyArray[i], thisArray[i]);
			}
		}		
	}
	
	template<class CopyType>
	NumericalArray(ObjectArray<CopyType> const& copy) throw()
	:	ObjectArray<NumericalType>(copy.size(), copy.isNullTerminated())
	{		
		NumericalType *thisArray = this->getArray();
		const CopyType *copyArray = copy.getArray();
		
		if(thisArray != 0 && copyArray != 0)
		{
			const int size = this->size();
			for(int i = 0; i < size; i++)
			{
				roundCopy(copyArray[i], thisArray[i]);
			}
		}		
	}
	
		
	NumericalArray(NumericalArray<NumericalType> const& array0, 
				   NumericalArray<NumericalType> const& array1) throw()
	:	ObjectArray<NumericalType>(array0, array1)
	{
	}
	
	ObjectArrayConcatOperatorsDefine(NumericalArray, NumericalType);
		
		
	NumericalArray(NumericalArraySpec const& spec) throw()
	:	ObjectArray<NumericalType>(spec.size_, false)
	{
		if(spec.zeroData_) clear();
	}
	
	NumericalArray(NumericalArraySpec const& spec, const bool needsNullTermination) throw()
	:	ObjectArray<NumericalType>(spec.size_, needsNullTermination)
	{
		if(spec.zeroData_) clear();
	}
	
	NumericalArray(const int size, NumericalType* dataToUse, const bool needsNullTermination) throw()
	:	ObjectArray<NumericalType>(size, dataToUse, needsNullTermination)
	{
		ugen_assert(size > 0);
	}
	
	static NumericalArray<NumericalType> withSize(const int size, const bool zeroData = false) throw()
	{
		return NumericalArray<NumericalType>(NumericalArraySpec(size, zeroData));
	}
	
	static NumericalArray<NumericalType> newClear(const int size) throw()
	{
		return NumericalArray<NumericalType>(NumericalArraySpec(size, true));
	}
		
	static NumericalArray<NumericalType> line(const int size, const NumericalType start, const NumericalType end) throw()
	{
		ugen_assert(size >= 2);
		
		const int numValues = size < 2 ? 2 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		
		double inc = double(end - start) / (numValues - 1);
		double currentValue = start;
		NumericalType *outputValues = newArray.getArray();
		
		for(int i = 0; i < numValues; i++)
		{
			roundCopy(currentValue, outputValues[i]);
			currentValue += inc;
		}
		
		return newArray;
	}
	
	static NumericalArray<NumericalType> series(const int size, 
												const NumericalType start, 
												const NumericalType grow) throw()
	{
		ugen_assert(size >= 2);
		
		const int numValues = size < 2 ? 2 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		
		NumericalType currentValue = start;
		NumericalType *outputValues = newArray.getArray();
		
		for(int i = 0; i < numValues; i++)
		{
			outputValues[i] = currentValue;
			currentValue += grow;
		}
		
		return newArray;
		
	}
	
	static NumericalArray<NumericalType> geom(const int size, 
											  const NumericalType start, 
											  const NumericalType grow) throw()
	{
		ugen_assert(size >= 2);
		
		const int numValues = size < 2 ? 2 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		
		NumericalType currentValue = start;
		NumericalType *outputValues = newArray.getArray();
		
		for(int i = 0; i < numValues; i++)
		{
			outputValues[i] = currentValue;
			currentValue *= grow;
		}
		
		return newArray;		
	}
	
	
	static NumericalArray<NumericalType> rand(const int size, 
											  const NumericalType lower, 
											  const NumericalType upper) throw()
	{
		ugen_assert(size > 0);
		
		const int numValues = size < 1 ? 1 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		NumericalType *outputValues = newArray.getArray();
		NumericalType diff = upper-lower;
		NumericalType randFactor = (NumericalType)1 / (NumericalType)RAND_MAX;
		
		for(int i = 0; i < numValues; i++)
		{
			NumericalType randomValue = rand() * randFactor;
			
			//outputValues[i] = Ran088::defaultGenerator().next(lower, upper);
			outputValues[i] = randomValue * diff + lower;
		}
		
		return newArray;
	}
	
	static NumericalArray<NumericalType> rand2(const int size, 
											   const NumericalType positive) throw()
	{
		return rand(size, -positive, positive);
	}
	
#ifndef UGEN_NOEXTGPL
	static NumericalArray<NumericalType> exprand(const int size, 
												 const NumericalType lower, 
												 const NumericalType upper) throw()
	{
		ugen_assert(size > 0);
		
		const int numValues = size < 1 ? 1 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		NumericalType *outputValues = newArray.getArray();
		
		for(int i = 0; i < numValues; i++)
		{
			outputValues[i] = Ran088::defaultGenerator().nextExp(lower, upper);
		}
		
		return newArray;		
	}
	
	static NumericalArray<NumericalType> linrand(const int size, 
												 const NumericalType lower, 
												 const NumericalType upper) throw()
	{
		ugen_assert(size > 0);
		
		const int numValues = size < 1 ? 1 : size;
		
		NumericalArray<NumericalType> newArray = NumericalArray<NumericalType>::withSize(numValues);
		NumericalType *outputValues = newArray.getArray();
		
		for(int i = 0; i < numValues; i++)
		{
			outputValues[i] = Ran088::defaultGenerator().nextLin(lower, upper);
		}
		
		return newArray;				
	}
#endif // gpl
	
	static NumericalArray<NumericalType> sineTable(const int size, 
												   const float repeats = 1.f, 
												   const NumericalType peak = 1) throw()
	{
		ugen_assert(repeats > 0.f);
		return NumericalArray<double>::line(size, 0.0, twoPi * repeats).sin() * peak;
	}
	
	static NumericalArray<NumericalType> cosineTable(const int size, 
													 const float repeats = 1.f, 
													 const NumericalType peak = 1) throw()
	{
		ugen_assert(repeats > 0.f);
		return NumericalArray<double>::line(size, 0.0, twoPi * repeats).cos() * peak;
	}
	
	static NumericalArray<NumericalType> cosineWindow(const int size, 
													  const float repeats = 1.f, 
													  const NumericalType peak = 1) throw()
	{
		ugen_assert(repeats > 0.f);
		return NumericalArray<double>::line(size, 0.0, pi * repeats).sin() * peak;
	}
	
	static NumericalArray<NumericalType> triangleWindow(const int size, 
														const NumericalType peak = 1) throw()
	{
		NumericalArray<NumericalType> bartlett = NumericalArray<NumericalType>::bartlettWindow(size+2, peak);
		return bartlett.range(1, size+1);
	}
	
	static NumericalArray<NumericalType> bartlettWindow(const int size, 
														const NumericalType peak = 1) throw()
	{
		return -NumericalArray<NumericalType>::line(size, -peak, peak).abs() + NumericalArray<NumericalType>(peak);
	}
	
	static NumericalArray<NumericalType> hannWindow(const int size, 
													const NumericalType peak = 1) throw()
	{
		return (-NumericalArray<double>::cosineTable(size) + 1.f) * 0.5f * peak;
	}
	
	static NumericalArray<NumericalType> hammingWindow(const int size, 
													   const NumericalType peak = 1) throw()
	{
		return (-NumericalArray<double>::cosineTable(size) * 0.46f + 0.54f) * peak;
	}
	
	static NumericalArray<NumericalType> blackmanWindow(const int size, 
														const float alpha = 0.16f, 
														const NumericalType peak = 1) throw()
	{
		float a0 = (1.f-alpha) * 0.5f;
		float a1 = 0.5f;
		float a2 = alpha * 0.5f;
		
		return (-NumericalArray<double>::cosineTable(size) * a1 + NumericalArray<double>::cosineTable(size, 2.f) * a2 + a0) * peak;
	}
	
	static int sourceLength(const NumericalType* nullTerminatedSourceArray) throw()
	{
		ugen_assert(nullTerminatedSourceArray != 0);
		
		int size = 0;
		
		while(nullTerminatedSourceArray[size])
			size++;
		
		return size;
	}
	
	static int sourceSize(const NumericalType* nullTerminatedSourceArray) throw()
	{
		return sourceLength(nullTerminatedSourceArray) + 1;
	}
	
	NumericalArray(const NumericalType* nullTerminatedSourceArray) throw()
	:	ObjectArray<NumericalType>(sourceSize(nullTerminatedSourceArray), true)
	{
		ugen_assert(nullTerminatedSourceArray != 0);
		
		NumericalType *thisArray = this->getArray();
		
		if(thisArray != 0)
		{
			const int size = this->size();
			for(int i = 0; i < size; i++)
			{
				thisArray[i] = nullTerminatedSourceArray[i];
			}			
		}
	}
	
	static NumericalArray<NumericalType> withArray(const int size, 
												   const NumericalType* sourceArray, 
												   const bool needsNullTermination = false) throw()
	{
		ugen_assert(sourceArray != 0);
		
		NumericalArray<NumericalType> result = NumericalArray<NumericalType>(NumericalArraySpec(size, false), 
																			 needsNullTermination);
		
		NumericalType *thisArray = result.getArray();
		
		if(thisArray != 0)
		{
			if(needsNullTermination)
			{
				const int length = size-1;
				for(int i = 0; i < length; i++)
				{
					thisArray[i] = sourceArray[i];
				}
				thisArray[length] = 0;
			}
			else
			{
				for(int i = 0; i < size; i++)
				{
					thisArray[i] = sourceArray[i];
				}				
			}
		}
		
		return result;
	}
	
	static NumericalArray<NumericalType> withArrayNoCopy(const int size, 
														 NumericalType* sourceArray, 
														 const bool needsNullTermination = false) throw()
	{
		ugen_assert(sourceArray != 0);
		return NumericalArray<NumericalType>(size, sourceArray, needsNullTermination);
	}
	
	NumericalArray(NumericalType value) throw()
	:	ObjectArray<NumericalType>(value)
	{
	}

	static int countValidInitialisers(InitialNumber const& i03) throw()
	{
		int size = 3;
		
		if(i03.valid) size++; else return size;

		return size;
	}	
	
	NumericalArray(InitialNumber const &i00,
				   InitialNumber const &i01,
				   InitialNumber const &i02,
				   InitialNumber const &i03 = InitialNumber()) throw()
	:	ObjectArray<NumericalType>(countValidInitialisers(i03), false)
	{
		NumericalType *thisArray = this->getArray();
		
		if(i00.valid) thisArray[ 0] = i00.value; else return;
		if(i01.valid) thisArray[ 1] = i01.value; else return;
		if(i02.valid) thisArray[ 2] = i02.value; else return;
		if(i03.valid) thisArray[ 3] = i03.value; else return;
	}
	
	static int countValidInitialisers(InitialNumber const &i05,
									  InitialNumber const &i06,
									  InitialNumber const &i07) throw()
	{
		int size = 5;
		
		if(i05.valid) size++; else return size;
		if(i06.valid) size++; else return size;
		if(i07.valid) size++; else return size;
		
		return size;
	}
	
	NumericalArray(InitialNumber const &i00,
				   InitialNumber const &i01,
				   InitialNumber const &i02,
				   InitialNumber const &i03,
				   InitialNumber const &i04,
				   InitialNumber const &i05 = InitialNumber(),
				   InitialNumber const &i06 = InitialNumber(),
				   InitialNumber const &i07 = InitialNumber()) throw()
	:	ObjectArray<NumericalType>(countValidInitialisers(i05, i06, i07), 
								   false)
	{
		NumericalType *thisArray = this->getArray();
		
		if(i00.valid) thisArray[ 0] = i00.value; else return;
		if(i01.valid) thisArray[ 1] = i01.value; else return;
		if(i02.valid) thisArray[ 2] = i02.value; else return;
		if(i03.valid) thisArray[ 3] = i03.value; else return;
		if(i04.valid) thisArray[ 4] = i04.value; else return;
		if(i05.valid) thisArray[ 5] = i05.value; else return;
		if(i06.valid) thisArray[ 6] = i06.value; else return;
		if(i07.valid) thisArray[ 7] = i07.value; else return;
	}
	
	static int countValidInitialisers(InitialNumber const &i09,
									  InitialNumber const &i10,
									  InitialNumber const &i11,
									  InitialNumber const &i12,
									  InitialNumber const &i13,
									  InitialNumber const &i14,
									  InitialNumber const &i15) throw()
	{
		int size = 9;
		
		if(i09.valid) size++; else return size;
		if(i10.valid) size++; else return size;
		if(i11.valid) size++; else return size;
		if(i12.valid) size++; else return size;
		if(i13.valid) size++; else return size;
		if(i14.valid) size++; else return size;
		if(i15.valid) size++; else return size;
		
		return size;
	}

	NumericalArray(InitialNumber const &i00,
				   InitialNumber const &i01,
				   InitialNumber const &i02,
				   InitialNumber const &i03,
				   InitialNumber const &i04,
				   InitialNumber const &i05,
				   InitialNumber const &i06,
				   InitialNumber const &i07,
				   InitialNumber const &i08,
				   InitialNumber const &i09 = InitialNumber(),
				   InitialNumber const &i10 = InitialNumber(),
				   InitialNumber const &i11 = InitialNumber(),
				   InitialNumber const &i12 = InitialNumber(),
				   InitialNumber const &i13 = InitialNumber(),
				   InitialNumber const &i14 = InitialNumber(),
				   InitialNumber const &i15 = InitialNumber()) throw()
	:	ObjectArray<NumericalType>(countValidInitialisers(i09, i10, i11, i12, i13, i14, i15), 
								   false)
	{
		NumericalType *thisArray = this->getArray();
		
		if(i00.valid) thisArray[ 0] = i00.value; else return;
		if(i01.valid) thisArray[ 1] = i01.value; else return;
		if(i02.valid) thisArray[ 2] = i02.value; else return;
		if(i03.valid) thisArray[ 3] = i03.value; else return;
		if(i04.valid) thisArray[ 4] = i04.value; else return;
		if(i05.valid) thisArray[ 5] = i05.value; else return;
		if(i06.valid) thisArray[ 6] = i06.value; else return;
		if(i07.valid) thisArray[ 7] = i07.value; else return;
		if(i08.valid) thisArray[ 8] = i08.value; else return;
		if(i09.valid) thisArray[ 9] = i09.value; else return;
		if(i10.valid) thisArray[10] = i10.value; else return;
		if(i11.valid) thisArray[11] = i11.value; else return;
		if(i12.valid) thisArray[12] = i12.value; else return;
		if(i13.valid) thisArray[13] = i13.value; else return;
		if(i14.valid) thisArray[14] = i14.value; else return;
		if(i15.valid) thisArray[15] = i15.value; else return;
	}
	
	static int countValidInitialisers(InitialNumber const &i17,
									  InitialNumber const &i18,
									  InitialNumber const &i19,
									  InitialNumber const &i20,
									  InitialNumber const &i21,
									  InitialNumber const &i22,
									  InitialNumber const &i23,
									  InitialNumber const &i24,
									  InitialNumber const &i25,
									  InitialNumber const &i26,
									  InitialNumber const &i27,
									  InitialNumber const &i28,
									  InitialNumber const &i29,
									  InitialNumber const &i30,
									  InitialNumber const &i31) throw()
	{
		int size = 17;
		
		if(i17.valid) size++; else return size;
		if(i18.valid) size++; else return size;
		if(i19.valid) size++; else return size;
		if(i20.valid) size++; else return size;
		if(i21.valid) size++; else return size;
		if(i22.valid) size++; else return size;
		if(i23.valid) size++; else return size;
		if(i24.valid) size++; else return size;
		if(i25.valid) size++; else return size;
		if(i26.valid) size++; else return size;
		if(i27.valid) size++; else return size;
		if(i28.valid) size++; else return size;
		if(i29.valid) size++; else return size;
		if(i30.valid) size++; else return size;
		if(i31.valid) size++; else return size;
		
		return size;
	}

	NumericalArray(InitialNumber const &i00,
				   InitialNumber const &i01,
				   InitialNumber const &i02,
				   InitialNumber const &i03,
				   InitialNumber const &i04,
				   InitialNumber const &i05,
				   InitialNumber const &i06,
				   InitialNumber const &i07,
				   InitialNumber const &i08,
				   InitialNumber const &i09,
				   InitialNumber const &i10,
				   InitialNumber const &i11,
				   InitialNumber const &i12,
				   InitialNumber const &i13,
				   InitialNumber const &i14,
				   InitialNumber const &i15,
				   InitialNumber const &i16,
				   InitialNumber const &i17 = InitialNumber(),
				   InitialNumber const &i18 = InitialNumber(),
				   InitialNumber const &i19 = InitialNumber(),
				   InitialNumber const &i20 = InitialNumber(),
				   InitialNumber const &i21 = InitialNumber(),
				   InitialNumber const &i22 = InitialNumber(),
				   InitialNumber const &i23 = InitialNumber(),
				   InitialNumber const &i24 = InitialNumber(),
				   InitialNumber const &i25 = InitialNumber(),
				   InitialNumber const &i26 = InitialNumber(),
				   InitialNumber const &i27 = InitialNumber(),
				   InitialNumber const &i28 = InitialNumber(),
				   InitialNumber const &i29 = InitialNumber(),
				   InitialNumber const &i30 = InitialNumber(),
				   InitialNumber const &i31 = InitialNumber()) throw()
	:	ObjectArray<NumericalType>(countValidInitialisers(     i17, i18, i19,
														  i20, i21, i22, i23,
														  i24, i25, i26, i27,
														  i28, i29, i30, i31), 
								   false)
	{
		NumericalType *thisArray = this->getArray();
		
		if(i00.valid) thisArray[ 0] = i00.value; else return;
		if(i01.valid) thisArray[ 1] = i01.value; else return;
		if(i02.valid) thisArray[ 2] = i02.value; else return;
		if(i03.valid) thisArray[ 3] = i03.value; else return;
		if(i04.valid) thisArray[ 4] = i04.value; else return;
		if(i05.valid) thisArray[ 5] = i05.value; else return;
		if(i06.valid) thisArray[ 6] = i06.value; else return;
		if(i07.valid) thisArray[ 7] = i07.value; else return;
		if(i08.valid) thisArray[ 8] = i08.value; else return;
		if(i09.valid) thisArray[ 9] = i09.value; else return;
		if(i10.valid) thisArray[10] = i10.value; else return;
		if(i11.valid) thisArray[11] = i11.value; else return;
		if(i12.valid) thisArray[12] = i12.value; else return;
		if(i13.valid) thisArray[13] = i13.value; else return;
		if(i14.valid) thisArray[14] = i14.value; else return;
		if(i15.valid) thisArray[15] = i15.value; else return;
		if(i16.valid) thisArray[16] = i16.value; else return;
		if(i17.valid) thisArray[17] = i17.value; else return;
		if(i18.valid) thisArray[18] = i18.value; else return;
		if(i19.valid) thisArray[19] = i19.value; else return;
		if(i20.valid) thisArray[20] = i20.value; else return;
		if(i21.valid) thisArray[21] = i21.value; else return;
		if(i22.valid) thisArray[22] = i22.value; else return;
		if(i23.valid) thisArray[23] = i23.value; else return;
		if(i24.valid) thisArray[24] = i24.value; else return;
		if(i25.valid) thisArray[25] = i25.value; else return;
		if(i26.valid) thisArray[26] = i26.value; else return;
		if(i27.valid) thisArray[27] = i27.value; else return;
		if(i28.valid) thisArray[28] = i28.value; else return;
		if(i29.valid) thisArray[29] = i29.value; else return;
		if(i30.valid) thisArray[30] = i30.value; else return;
		if(i31.valid) thisArray[31] = i31.value; else return;
	}
	
	// perhaps use a var args thing for more than 32, with an END tag at the end of the list?
	
	NumericalArray<NumericalType>& operator= (ObjectArray<NumericalType> const& other) throw()
	{
		return operator= (static_cast<NumericalArray<NumericalType> const&> (other) );
	}
	
	NumericalArray<NumericalType> operator+ (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryOperatorBody(+);
	}
	
	NumericalArray<NumericalType> operator- (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryOperatorBody(-);
	}
	
	NumericalArray<NumericalType> operator* (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryOperatorBody(*);
	}
	
	NumericalArray<NumericalType> operator/ (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryOperatorBody(/);
	}
	
	NumericalArray<NumericalType>& operator+= (NumericalArray<NumericalType> const& rightOperand) throw()
	{
		return operator= (*this + rightOperand);
	}
	
	NumericalArray<NumericalType>& operator-= (NumericalArray<NumericalType> const& rightOperand) throw()
	{
		return operator= (*this - rightOperand);
	}

	NumericalArray<NumericalType>& operator*= (NumericalArray<NumericalType> const& rightOperand) throw()
	{
		return operator= (*this * rightOperand);
	}

	NumericalArray<NumericalType>& operator/= (NumericalArray<NumericalType> const& rightOperand) throw()
	{
		return operator= (*this / rightOperand);
	}
	
	/** Compare two arrays.
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */
	NumericalArray<bool> operator< (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(<);
	}
	
	/** Compare two arrays.
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */	
	NumericalArray<bool> operator<= (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(<=);
	}
	
	/** Compare two arrays.
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */	
	NumericalArray<bool> operator> (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(>);
	}
	
	/** Compare two arrays.
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */	
	NumericalArray<bool> operator>= (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(>=);
	}
	
	/** Compare the two arrays for equality.
	 We can't use == here since this is needed to check object identity. 
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */
	NumericalArray<bool> compareEqual (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(==);
	}
	
	/** Compare the two arrays for inequality.
	 We can't use != here since this is needed to check object identity.
	 Returns a bool array (will be only 0 or 1) with the result of the comparison. 
	 This always returns a bool array with is NOT null terminated, regardless of whether the source
	 arrays are null terminated. */
	NumericalArray<bool> compareNotEqual (NumericalArray<NumericalType> const& rightOperand) const throw()
	{
		NumericalArrayBinaryComparisonOperatorBody(!=);
	}
	
	// bitwise?
	
	NumericalArrayBinaryFunctionDefinition(pow);
	NumericalArrayBinaryFunctionDefinition(hypot);
	NumericalArrayBinaryFunctionDefinition(atan2);
	NumericalArrayBinaryFunctionDefinition(min);
	NumericalArrayBinaryFunctionDefinition(max);
	NumericalArrayBinaryFunctionDefinition(clip2);
	
	NumericalArrayUnaryFunctionDefinition(neg);
	NumericalArrayUnaryFunctionDefinition(abs);
	NumericalArrayUnaryFunctionDefinition(reciprocal);
	NumericalArrayUnaryFunctionDefinition(sin);
	NumericalArrayUnaryFunctionDefinition(cos);
	NumericalArrayUnaryFunctionDefinition(tan);
	NumericalArrayUnaryFunctionDefinition(asin);
	NumericalArrayUnaryFunctionDefinition(acos);
	NumericalArrayUnaryFunctionDefinition(atan);
	NumericalArrayUnaryFunctionDefinition(sqrt);
	NumericalArrayUnaryFunctionDefinition(log);
	NumericalArrayUnaryFunctionDefinition(log2);
	NumericalArrayUnaryFunctionDefinition(log10);
	NumericalArrayUnaryFunctionDefinition(exp);
	NumericalArrayUnaryFunctionDefinition(squared);
	NumericalArrayUnaryFunctionDefinition(cubed);
#ifndef UGEN_NOEXTGPL
	NumericalArrayUnaryFunctionDefinition(midicps);
	NumericalArrayUnaryFunctionDefinition(cpsmidi);
	NumericalArrayUnaryFunctionDefinition(distort);
#endif
	
	NumericalArray<NumericalType> operator- () throw()
	{
		NumericalArrayUnaryFunctionBody(ugen::neg);
	}
	
	inline void clear() throw()
	{
		NumericalType *array = this->getArray();
		
		if(array != 0)
		{
			const int size = this->size();
			for(int i = 0; i < size; i++)
			{
				array[i] = 0;
			}
		}		
	}
	
	bool contains (const NumericalType* items) const throw()
	{
		ugen_assert(items != 0);
		return ObjectArray<NumericalType>::contains(NumericalArray<NumericalType>(items));
	}
	
	/** Search for a particular sub sequence withing the array. */
	int indexOf (const NumericalType* items, const int startIndex = 0) const throw()
	{
		ugen_assert(items != 0);
		ugen_assert(startIndex >= 0);
		return ObjectArray<NumericalType>::indexOf(NumericalArray<NumericalType>(items), startIndex);
	}
	
	/** Search for any items in a given array. */
	int indexOfAny (const NumericalType *items, const int startIndex = 0) const throw()
	{
		ugen_assert(items != 0);
		ugen_assert(startIndex >= 0);
		return ObjectArray<NumericalType>::indexOfAny(NumericalArray<NumericalType>(items), startIndex);
	}
	
	ObjectArray<NumericalType> replace(NumericalArray<NumericalType> const& find, const NumericalType *substitute) const throw()
	{
		ugen_assert(substitute != 0);
		return ObjectArray<NumericalType>::replace(find, NumericalArray<NumericalType>(substitute));
	}
	
	ObjectArray<NumericalType> replace(const NumericalType *find, NumericalArray<NumericalType> const& substitute) const throw()
	{
		ugen_assert(find != 0);
		return ObjectArray<NumericalType>::replace(NumericalArray<NumericalType>(find), substitute);
	}
	
	ObjectArray<NumericalType> replace(const NumericalType *find, const NumericalType *substitute) const throw()
	{
		ugen_assert(find != 0);
		ugen_assert(substitute != 0);
		return ObjectArray<NumericalType>::replace(NumericalArray<NumericalType>(find), NumericalArray<NumericalType>(substitute));
	}
	
	ObjectArray< ObjectArray<NumericalType> > split(const NumericalType *delimiters) const throw()
	{
		ugen_assert(delimiters != 0);
		return ObjectArray<NumericalType>::split(NumericalArray<NumericalType>(delimiters));
	}

	ObjectArray< ObjectArray<NumericalType> > splitSequence(const NumericalType *delimitingSequence) const throw()
	{
		ugen_assert(delimitingSequence != 0);
		return ObjectArray<NumericalType>::splitSequence(NumericalArray<NumericalType>(delimitingSequence));
	}	
	
	void print(const char *prefix = 0, const bool oneLine = false) const throw()
	{
		if(oneLine)
		{
			if(prefix) std::cout << prefix << ": ";
			
			for(int i = 0; i < this->size(); i++)
			{
				std::cout << this->at(i) << " ";
			}
			
			std::cout << std::endl;
		}
		else
		{
			for(int i = 0; i < this->size(); i++)
			{
				if(prefix) std::cout << prefix;
				
				std::cout << "[" << i << "] = " << this->at(i) << std::endl;
			}
		}
	}
		
};


typedef NumericalArray<bool>			BoolArray;
typedef NumericalArray<char>			CharArray;
typedef NumericalArray<short>			ShortArray;
typedef NumericalArray<int>				IntArray;
typedef NumericalArray<long>			LongArray;
typedef NumericalArray<unsigned char>	UnsignedCharArray;
typedef NumericalArray<unsigned short>	UnsignedShortArray;
typedef NumericalArray<unsigned int>	UnsignedIntArray;
typedef NumericalArray<unsigned long>	UnsignedLongArray;
typedef NumericalArray<float>			FloatArray;
typedef NumericalArray<double>			DoubleArray;


template<class ArrayType, class RowType = ObjectArray<ArrayType> >
class ObjectArray2DBase : public ObjectArray< RowType >
{
public:
	ObjectArray2DBase(const int rows = 0) throw()
	:	ObjectArray< RowType > (rows < 0 ? 0 : rows, false) 
	{
	}

	ObjectArray2DBase(RowType const& single) throw()
	:	ObjectArray< RowType >(single)
	{
	}
	
	static int countValidInitialisers(RowType const& i03) throw()
	{
		int size = 3;

		if(i03.size() > 0) size++; else return size;

		return size;
	}	

	ObjectArray2DBase(RowType const &i00,
					  RowType const &i01,
					  RowType const &i02,
					  RowType const &i03 = RowType()) throw()
	:	ObjectArray<RowType>(countValidInitialisers(i03), false) 
	{
		RowType *thisArray = this->getArray();
		
		if(i00.size() > 0) thisArray[ 0] = i00; else return;
		if(i01.size() > 0) thisArray[ 1] = i01; else return;
		if(i02.size() > 0) thisArray[ 2] = i02; else return;
		if(i03.size() > 0) thisArray[ 3] = i03; else return;
	}


	static int countValidInitialisers(RowType const &i05,
									  RowType const &i06,
									  RowType const &i07) throw()
	{
		int size = 5;
		
		if(i05.size() > 0) size++; else return size;
		if(i06.size() > 0) size++; else return size;
		if(i07.size() > 0) size++; else return size;
		
		return size;
	}

	ObjectArray2DBase(RowType const &i00,
					  RowType const &i01,
					  RowType const &i02,
					  RowType const &i03,
					  RowType const &i04,
					  RowType const &i05 = RowType(),
					  RowType const &i06 = RowType(),
					  RowType const &i07 = RowType()) throw()
	:	ObjectArray<RowType>(countValidInitialisers(i05, i06, i07), false)
	{
		RowType *thisArray = this->getArray();
		
		if(i00.size() > 0) thisArray[ 0] = i00; else return;
		if(i01.size() > 0) thisArray[ 1] = i01; else return;
		if(i02.size() > 0) thisArray[ 2] = i02; else return;
		if(i03.size() > 0) thisArray[ 3] = i03; else return;
		if(i04.size() > 0) thisArray[ 4] = i04; else return;
		if(i05.size() > 0) thisArray[ 5] = i05; else return;
		if(i06.size() > 0) thisArray[ 6] = i06; else return;
		if(i07.size() > 0) thisArray[ 7] = i07; else return;
	}

	static int countValidInitialisers(RowType const &i09,
									  RowType const &i10,
									  RowType const &i11,
									  RowType const &i12,
									  RowType const &i13,
									  RowType const &i14,
									  RowType const &i15) throw()
	{
		int size = 9;
		
		if(i09.size() > 0) size++; else return size;
		if(i10.size() > 0) size++; else return size;
		if(i11.size() > 0) size++; else return size;
		if(i12.size() > 0) size++; else return size;
		if(i13.size() > 0) size++; else return size;
		if(i14.size() > 0) size++; else return size;
		if(i15.size() > 0) size++; else return size;
		
		return size;
	}

	ObjectArray2DBase(RowType const &i00,
					  RowType const &i01,
					  RowType const &i02,
					  RowType const &i03,
					  RowType const &i04,
					  RowType const &i05,
					  RowType const &i06,
					  RowType const &i07,
					  RowType const &i08,
					  RowType const &i09 = RowType(),
					  RowType const &i10 = RowType(),
					  RowType const &i11 = RowType(),
					  RowType const &i12 = RowType(),
					  RowType const &i13 = RowType(),
					  RowType const &i14 = RowType(),
					  RowType const &i15 = RowType()) throw()
	:	ObjectArray<RowType>(countValidInitialisers(     i09, i10, i11, 
													i12, i13, i14, i15), 
							 false)
	{
		RowType *thisArray = this->getArray();
		
		if(i00.size() > 0) thisArray[ 0] = i00; else return;
		if(i01.size() > 0) thisArray[ 1] = i01; else return;
		if(i02.size() > 0) thisArray[ 2] = i02; else return;
		if(i03.size() > 0) thisArray[ 3] = i03; else return;
		if(i04.size() > 0) thisArray[ 4] = i04; else return;
		if(i05.size() > 0) thisArray[ 5] = i05; else return;
		if(i06.size() > 0) thisArray[ 6] = i06; else return;
		if(i07.size() > 0) thisArray[ 7] = i07; else return;
		if(i08.size() > 0) thisArray[ 8] = i08; else return;
		if(i09.size() > 0) thisArray[ 9] = i09; else return;
		if(i10.size() > 0) thisArray[10] = i10; else return;
		if(i11.size() > 0) thisArray[11] = i11; else return;
		if(i12.size() > 0) thisArray[12] = i12; else return;
		if(i13.size() > 0) thisArray[13] = i13; else return;
		if(i14.size() > 0) thisArray[14] = i14; else return;
		if(i15.size() > 0) thisArray[15] = i15; else return;
	}

	static int countValidInitialisers(RowType const &i17,
									  RowType const &i18,
									  RowType const &i19,
									  RowType const &i20,
									  RowType const &i21,
									  RowType const &i22,
									  RowType const &i23,
									  RowType const &i24,
									  RowType const &i25,
									  RowType const &i26,
									  RowType const &i27,
									  RowType const &i28,
									  RowType const &i29,
									  RowType const &i30,
									  RowType const &i31) throw()
	{
		int size = 17;
		
		if(i17.size() > 0) size++; else return size;
		if(i18.size() > 0) size++; else return size;
		if(i19.size() > 0) size++; else return size;
		if(i20.size() > 0) size++; else return size;
		if(i21.size() > 0) size++; else return size;
		if(i22.size() > 0) size++; else return size;
		if(i23.size() > 0) size++; else return size;
		if(i24.size() > 0) size++; else return size;
		if(i25.size() > 0) size++; else return size;
		if(i26.size() > 0) size++; else return size;
		if(i27.size() > 0) size++; else return size;
		if(i28.size() > 0) size++; else return size;
		if(i29.size() > 0) size++; else return size;
		if(i30.size() > 0) size++; else return size;
		if(i31.size() > 0) size++; else return size;
		
		return size;
	}

	ObjectArray2DBase(RowType const &i00,
					  RowType const &i01,
					  RowType const &i02,
					  RowType const &i03,
					  RowType const &i04,
					  RowType const &i05,
					  RowType const &i06,
					  RowType const &i07,
					  RowType const &i08,
					  RowType const &i09,
					  RowType const &i10,
					  RowType const &i11,
					  RowType const &i12,
					  RowType const &i13,
					  RowType const &i14,
					  RowType const &i15,
					  RowType const &i16,
					  RowType const &i17 = RowType(),
					  RowType const &i18 = RowType(),
					  RowType const &i19 = RowType(),
					  RowType const &i20 = RowType(),
					  RowType const &i21 = RowType(),
					  RowType const &i22 = RowType(),
					  RowType const &i23 = RowType(),
					  RowType const &i24 = RowType(),
					  RowType const &i25 = RowType(),
					  RowType const &i26 = RowType(),
					  RowType const &i27 = RowType(),
					  RowType const &i28 = RowType(),
					  RowType const &i29 = RowType(),
					  RowType const &i30 = RowType(),
					  RowType const &i31 = RowType()) throw()
	:	ObjectArray<RowType>(countValidInitialisers(     i17, i18, i19,
													i20, i21, i22, i23,
													i24, i25, i26, i27,
													i28, i29, i30, i31), 
							 false)
	{
		RowType *thisArray = this->getArray();
		
		if(i00.size() > 0) thisArray[ 0] = i00; else return;
		if(i01.size() > 0) thisArray[ 1] = i01; else return;
		if(i02.size() > 0) thisArray[ 2] = i02; else return;
		if(i03.size() > 0) thisArray[ 3] = i03; else return;
		if(i04.size() > 0) thisArray[ 4] = i04; else return;
		if(i05.size() > 0) thisArray[ 5] = i05; else return;
		if(i06.size() > 0) thisArray[ 6] = i06; else return;
		if(i07.size() > 0) thisArray[ 7] = i07; else return;
		if(i08.size() > 0) thisArray[ 8] = i08; else return;
		if(i09.size() > 0) thisArray[ 9] = i09; else return;
		if(i10.size() > 0) thisArray[10] = i10; else return;
		if(i11.size() > 0) thisArray[11] = i11; else return;
		if(i12.size() > 0) thisArray[12] = i12; else return;
		if(i13.size() > 0) thisArray[13] = i13; else return;
		if(i14.size() > 0) thisArray[14] = i14; else return;
		if(i15.size() > 0) thisArray[15] = i15; else return;
		if(i16.size() > 0) thisArray[16] = i16; else return;
		if(i17.size() > 0) thisArray[17] = i17; else return;
		if(i18.size() > 0) thisArray[18] = i18; else return;
		if(i19.size() > 0) thisArray[19] = i19; else return;
		if(i20.size() > 0) thisArray[20] = i20; else return;
		if(i21.size() > 0) thisArray[21] = i21; else return;
		if(i22.size() > 0) thisArray[22] = i22; else return;
		if(i23.size() > 0) thisArray[23] = i23; else return;
		if(i24.size() > 0) thisArray[24] = i24; else return;
		if(i25.size() > 0) thisArray[25] = i25; else return;
		if(i26.size() > 0) thisArray[26] = i26; else return;
		if(i27.size() > 0) thisArray[27] = i27; else return;
		if(i28.size() > 0) thisArray[28] = i28; else return;
		if(i29.size() > 0) thisArray[29] = i29; else return;
		if(i30.size() > 0) thisArray[30] = i30; else return;
		if(i31.size() > 0) thisArray[31] = i31; else return;
	}

	ObjectArrayAssignmentDefinition(ObjectArray2DBase, RowType);
	
	ObjectArray2DBase(ObjectArray2DBase<ArrayType,RowType> const& array0, 
					  ObjectArray2DBase<ArrayType,RowType> const& array1) throw()
	:	ObjectArray< RowType >(array0, array1)
	{
	}
		
	bool contains(ArrayType const& item) const throw()
	{
		const int numRows = this->length();
		for(int i = 0; i < numRows; i++)
		{
			if(this->atUnchecked(i).ObjectArray<ArrayType>::contains(item))
				return true;
		}
		
		return false;
	}
	
	/** Search for the row and column index of an item.
	 On input row and column are the start indices to search so should be
	 set to zero before calling this function if you want to search the whole
	 2D array. On exit they will contain -1 if the item is not found. */
	void indexOf(ArrayType const& item, int& row, int& column) const throw()
	{
		const int numRows = this->length();
		for(int i = row < 0 ? 0 : row; i < numRows; i++)
		{
			int c = this->atUnchecked(i).ObjectArray<ArrayType>::indexOf(item, column);
			if(c >= 0)
			{
				row = i;
				column = c;
				return;
			}
		}
		
		row = -1;
		column = -1;
	}
	
	bool isMatrix() const throw()
	{
		const int numRows = this->length();
		if(numRows < 2) return false;
		
		int numColumns = this->atUnchecked(0).length();
		if(numColumns < 2) return false;
		
		int prevNumColumns = numColumns;
		for(int row = 1; row < numRows; row++)
		{
			numColumns = this->atUnchecked(row).length();
			if(numColumns != prevNumColumns)
				return false;
			
			prevNumColumns = numColumns;
		}
		
		return true;
	}
	
	int numRows() const throw()
	{
		return this->size();
	}
	
	int numColumns() const throw()
	{
		const int numRows = this->length();
		if(numRows < 1) return 0;
		
		int numColumns = this->atUnchecked(0).length();
		
		int maxNumColumns = numColumns;
		for(int row = 1; row < numRows; row++)
		{
			numColumns = this->atUnchecked(row).length();
			if(numColumns > maxNumColumns)
				maxNumColumns = numColumns;
		}
		
		return maxNumColumns;
	}

	RowType flatten() const throw()
	{
		RowType result;
		const int numRows = this->length();
		for(int i = 0; i < numRows; i++)
		{
			const RowType& item = this->atUnchecked(i);
			result <<= item;
		}			
		
		return result;
	}
	
	
	//ObjectArrayConcatOperatorsDefine(ObjectArray2DBase, ArrayType);
	
};

template<class ArrayType, class RowType = ObjectArray<ArrayType> >
class ObjectArray2D : public ObjectArray2DBase< ArrayType, RowType >
{
public:
	ObjectArray2D(const int rows = 0) throw()
	:	ObjectArray2DBase< ArrayType, RowType > (rows < 0 ? 0 : rows)//, false) 
	{
	}

	ObjectArray2D(const int rows, const int columns) throw()
	:	ObjectArray2DBase< ArrayType, RowType > (rows < 0 ? 0 : rows)//, false) 
	{
		if(columns > 0 && rows > 0)
		{
			for(int i = 0; i < rows; i++)
			{
				this->put(i, RowType (columns, false));
			}
		}
	}

	ObjectArray2D(RowType const& single) throw()
	:	ObjectArray2DBase< ArrayType, RowType >(single)
	{
	}

	ObjectArray2D(RowType const &i00,
				  RowType const &i01,
				  RowType const &i02 = RowType(),
				  RowType const &i03 = RowType()) throw()
	:	ObjectArray2DBase<ArrayType, RowType> (i00, i01, i02, i03) 
	{
	}

	ObjectArray2D(RowType const &i00,
				  RowType const &i01,
				  RowType const &i02,
				  RowType const &i03,
				  RowType const &i04,
				  RowType const &i05 = RowType(),
				  RowType const &i06 = RowType(),
				  RowType const &i07 = RowType()) throw()
	:	ObjectArray2DBase<ArrayType, RowType> (i00, i01, i02, i03,
											   i04, i05, i05, i07) 
	{
	}

	ObjectArray2D(RowType const &i00,
				  RowType const &i01,
				  RowType const &i02,
				  RowType const &i03,
				  RowType const &i04,
				  RowType const &i05,
				  RowType const &i06,
				  RowType const &i07,
				  RowType const &i08,
				  RowType const &i09 = RowType(),
				  RowType const &i10 = RowType(),
				  RowType const &i11 = RowType(),
				  RowType const &i12 = RowType(),
				  RowType const &i13 = RowType(),
				  RowType const &i14 = RowType(),
				  RowType const &i15 = RowType()) throw()
	:	ObjectArray2DBase<ArrayType, RowType>(i00, i01, i02, i03,
											  i04, i05, i05, i07,
											  i08, i09, i10, i11, 
											  i12, i13, i14, i15)
	{
	}

	ObjectArray2D(RowType const &i00,
				  RowType const &i01,
				  RowType const &i02,
				  RowType const &i03,
				  RowType const &i04,
				  RowType const &i05,
				  RowType const &i06,
				  RowType const &i07,
				  RowType const &i08,
				  RowType const &i09,
				  RowType const &i10,
				  RowType const &i11,
				  RowType const &i12,
				  RowType const &i13,
				  RowType const &i14,
				  RowType const &i15,
				  RowType const &i16,
				  RowType const &i17 = RowType(),
				  RowType const &i18 = RowType(),
				  RowType const &i19 = RowType(),
				  RowType const &i20 = RowType(),
				  RowType const &i21 = RowType(),
				  RowType const &i22 = RowType(),
				  RowType const &i23 = RowType(),
				  RowType const &i24 = RowType(),
				  RowType const &i25 = RowType(),
				  RowType const &i26 = RowType(),
				  RowType const &i27 = RowType(),
				  RowType const &i28 = RowType(),
				  RowType const &i29 = RowType(),
				  RowType const &i30 = RowType(),
				  RowType const &i31 = RowType()) throw()
	:	ObjectArray2DBase<ArrayType, RowType>(i00, i01, i02, i03,
											  i04, i05, i05, i07,
											  i08, i09, i10, i11, 
											  i12, i13, i14, i15, 
											  i16, i17, i18, i19,
											  i20, i21, i22, i23,
											  i24, i25, i26, i27,
											  i28, i29, i30, i31)
	{
	}

	ObjectArrayAssignmentDefinition(ObjectArray2D, RowType);

	ObjectArray2D(ObjectArray2D<ArrayType,RowType> const& array0, 
				  ObjectArray2D<ArrayType,RowType> const& array1) throw()
	:	ObjectArray2DBase< ArrayType, RowType >(array0, array1)
	{
	}

};


template<class NumericalType, class RowType = NumericalArray<NumericalType> >
class NumericalArray2D : public ObjectArray2DBase< NumericalType, RowType >
{
public:
	NumericalArray2D(const int rows = 0) throw()
	:	ObjectArray2DBase<NumericalType, RowType> (rows < 0 ? 0 : rows, false) 
	{
	}
	
	NumericalArray2D(const int rows, const int columns, const bool zeroData = false) throw()
	:	ObjectArray2DBase<NumericalType, RowType> (rows < 0 ? 0 : rows, false) 
	{
		if(columns > 0 && rows > 0)
		{
			for(int i = 0; i < rows; i++)
			{
				this->put(i, RowType (NumericalArraySpec(columns, zeroData)));
			}
		}
		
	}
	
	NumericalArray2D(ObjectArray< ObjectArray<NumericalType> > const& other) throw()
	:	ObjectArray2DBase<NumericalType, RowType>(other.size())
	{
		const int rows = this->size();
		for(int row = 0; row < rows; row++)
		{
			this->put(row, static_cast<RowType const&> (other[row]));
		}
	}

	NumericalArray2D(RowType const& single) throw()
	:	ObjectArray2DBase<NumericalType, RowType>(single)
	{
	}
	
	NumericalArray2D(RowType const &i00,
					 RowType const &i01,
					 RowType const &i02 = RowType(),
					 RowType const &i03 = RowType()) throw()
	:	ObjectArray2DBase<NumericalType, RowType> (i00, i01, i02, i03) 
	{
	}

	NumericalArray2D(RowType const &i00,
					 RowType const &i01,
					 RowType const &i02,
					 RowType const &i03,
					 RowType const &i04,
					 RowType const &i05 = RowType(),
					 RowType const &i06 = RowType(),
					 RowType const &i07 = RowType()) throw()
	:	ObjectArray2DBase<NumericalType, RowType> (i00, i01, i02, i03,
												   i04, i05, i06, i07) 
	{
	}

	NumericalArray2D(RowType const &i00,
					 RowType const &i01,
					 RowType const &i02,
					 RowType const &i03,
					 RowType const &i04,
					 RowType const &i05,
					 RowType const &i06,
					 RowType const &i07,
					 RowType const &i08,
					 RowType const &i09 = RowType(),
					 RowType const &i10 = RowType(),
					 RowType const &i11 = RowType(),
					 RowType const &i12 = RowType(),
					 RowType const &i13 = RowType(),
					 RowType const &i14 = RowType(),
					 RowType const &i15 = RowType()) throw()
	:	ObjectArray2DBase<NumericalType, RowType>(i00, i01, i02, i03,
												  i04, i05, i05, i07,
												  i08, i09, i10, i11, 
												  i12, i13, i14, i15)
	{
	}

	NumericalArray2D(RowType const &i00,
					 RowType const &i01,
					 RowType const &i02,
					 RowType const &i03,
					 RowType const &i04,
					 RowType const &i05,
					 RowType const &i06,
					 RowType const &i07,
					 RowType const &i08,
					 RowType const &i09,
					 RowType const &i10,
					 RowType const &i11,
					 RowType const &i12,
					 RowType const &i13,
					 RowType const &i14,
					 RowType const &i15,
					 RowType const &i16,
					 RowType const &i17 = RowType(),
					 RowType const &i18 = RowType(),
					 RowType const &i19 = RowType(),
					 RowType const &i20 = RowType(),
					 RowType const &i21 = RowType(),
					 RowType const &i22 = RowType(),
					 RowType const &i23 = RowType(),
					 RowType const &i24 = RowType(),
					 RowType const &i25 = RowType(),
					 RowType const &i26 = RowType(),
					 RowType const &i27 = RowType(),
					 RowType const &i28 = RowType(),
					 RowType const &i29 = RowType(),
					 RowType const &i30 = RowType(),
					 RowType const &i31 = RowType()) throw()
	:	ObjectArray2DBase<NumericalType, RowType>(i00, i01, i02, i03,
												  i04, i05, i05, i07,
												  i08, i09, i10, i11, 
												  i12, i13, i14, i15, 
												  i16, i17, i18, i19,
												  i20, i21, i22, i23,
												  i24, i25, i26, i27,
												  i28, i29, i30, i31)
	{
	}

	ObjectArrayAssignmentDefinition(NumericalArray2D, RowType);

	
	NumericalArray2D(NumericalArray2D<NumericalType> const& array0, 
					 NumericalArray2D<NumericalType> const& array1) throw()
	:	ObjectArray2DBase<NumericalType, RowType>(array0, array1)
	{
	}

	void print(const char *prefix = 0, const bool rowsOnOneLine = false) const throw()
	{
		if(rowsOnOneLine)
		{
			for(int row = 0; row < this->size(); row++)
			{
				if(prefix) std::cout << prefix;
				
				std::cout << "[" << row << "]: ";
								
				this->at(row).print(0, true);
			}
		}
		else
		{
			for(int row = 0; row < this->size(); row++)
			{
				const NumericalArray<NumericalType>& rowArray = this->at(row);
				
				for(int column = 0; column < rowArray.size(); column++)
				{
					if(prefix) std::cout << prefix;
					
					std::cout << "[" << row << "][" << column << "] = " << rowArray[column] << std::endl;
				}		
			}
		}
	}
	
};

typedef NumericalArray2D<char>				CharArray2D;
typedef NumericalArray2D<short>				ShortArray2D;
typedef NumericalArray2D<int>				IntArray2D;
typedef NumericalArray2D<long>				LongArray2D;
typedef NumericalArray2D<unsigned char>		UnsignedCharArray2D;
typedef NumericalArray2D<unsigned short>	UnsignedShortArray2D;
typedef NumericalArray2D<unsigned int>		UnsignedIntArray2D;
typedef NumericalArray2D<unsigned long>		UnsignedLongArray2D;
typedef NumericalArray2D<float>				FloatArray2D;
typedef NumericalArray2D<double>			DoubleArray2D;



#endif // _UGEN_ugen_Arrays_H_
