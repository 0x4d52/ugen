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

#ifndef UGEN_UGENARRAY_H
#define UGEN_UGENARRAY_H

#include "ugen_UGen.h"
#include "ugen_SmartPointer.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "ugen_Arrays.h"

class UGenInternal;
template<class ObjectType> class ObjectArray;


/** Store and manipulate arrays of UGen instances.
 
 A UGen is effectively an array of channels, a UGenArray is an array of UGen instances. Thus
 A UGenArray is potentially a 2D array.
 
 @see Mix, MixFill, Bank, Spawn */
class UGenArray
{
public:
	/// @name Construction and destruction
	/// @{
		
	/** Construct a UGenArray with a certain number of slots. 
	 @param size The number of UGen slots (may be 0 - the default). */
	UGenArray(const int size = 0) throw();
	
	/** Construct a UGenArray with a certain number of slots. @param size The number of UGen slots. */
	static UGenArray withSize(const int size) { return UGenArray(size); }
	
	/** Construct a UGenArray with a single element
		This allows the use of a UGen where a UGenArray is required and it will be converted implicitly.
		@param ugen The UGen to use as the single element. */
	UGenArray(UGen const& ugen) throw();
	
	/** Construct a UGenArray from an ObjectArray<UGen>.
		This allows you to use the special features of the UGenArray class that is UGen-aware. */
	UGenArray(ObjectArray<UGen> const& array) throw();
	
	/** Copy constructor. */
	UGenArray(UGenArray const& copy) throw();
	
	/** Concatenate two UGenArray instances.
		@param	array0		The first UGenArray
		@param	array1		The second UGenArray
		@param	removeNulls	If true, null UGen instances will be removed from the resulting UGenArray. 
							If false, all UGen instances will be included. */
	UGenArray(UGenArray const& array0, UGenArray const& array1, const bool removeNulls) throw();
	
	/** Assignment operator. */
	UGenArray& operator= (UGenArray const& other) throw();
	
	/** Destructor. */
	~UGenArray() throw();
	
	/// @} <!-- end Construction and destruction ------------------------------------------ -->
	
	/** @internal */
	class Internal : public SmartPointer
	{
	public:
		Internal(const int size) throw();
		~Internal() throw();
		
		inline int size() const throw() { return size_; }
		inline const UGen* getArray() const throw() { return array; }
		inline UGen* getArray() throw() { return array; }
		
		void add(UGen const& item) throw();
		void add(const int numItems, const UGen* items) throw();
		void remove(const int index, const bool reallocate) throw();
		void removeNulls(const bool reallocate = false) throw();
		void reallocate() throw();
		void clear() throw();
		void clearQuick() throw();
				
	private:
		int size_;
		int allocatedSize;
		UGen* array;
		
		Internal (const Internal&);
		const Internal& operator= (const Internal&);
	};
	
	/// @name Array manipulation
	/// @{
	
	/** Get the array. 
	 @return The array or 0 if this is a null UGenArray */
	UGen* getArray() throw()						{ return internal->getArray(); }
	
	/** Get the array for read-only operations.
	 @return The array or 0 if this is a null UGenArray */
	const UGen* getArray() const throw()			{ return internal->getArray(); }
	
	/** Convert the UGenArray to an ObjectArray<UGen>. */
	operator const ObjectArray<UGen>() const throw();
	
	/** Get the available size of the UGenArray.
	 @return The number of available slots in the array */
	inline int size() const throw()					{ return internal->size(); }
	
	/** Find the number of non-null UGen instances in the arrary.
	 @return The number of UGen instances in the array that are not null UGen instances.  */
	int sizeNotNull() const throw()
	{				
		int numNotNull = 0;
		int actualSize = internal->size();
		for(int i = 0; i < actualSize; i++)
		{
			if(internal->getArray()[i].isNotNull()) numNotNull++;
		}
		return numNotNull;
	}
	
	/** The largest number of channels the UGen instances in the UGenArray contains
	 @return The maximum number of channels. */
	int findMaxNumChannels() const throw();
	
	/** Adds an item in-place. */
	void add(UGen const& other) throw();
	
	/** Add one or more items in-place. */
	void add(UGenArray const& other) throw();
	
	/** Remove an item at the index (ignored if the index is out of range) in-place. 
	 This doesn't reallocate memory, items at the end of the array are set to null. */
	UGen remove(const int index, const bool reallocate = false) throw();
	
	/** Removes a particular UGen from the UGenArray - in-place. */
	void removeItem(UGen const& item, const bool reallocate = false) throw();
	
	/** Removes all UGen instances which are null - in-place. */
	void removeNulls() throw();
	
	/** Removes all items and sets the size to zero - in-place. 
	 @param quick  If true then the array is actually deallocated but all the items
				   are set to null. */
	void clear(bool quick = true) throw();
	
	/** Append two UGenArray objects.
	 All null UGen instances are removed from the result.
	 @param other	The other UGenArray.
	 @return		A UGenArray with all the UGen instances from this UGenArray with the
					UGens in the other UGenArray appended to the end. 
	 @see	UGenArray::operator<<(UGen const& other),
			UGenArray::operator,(UGenArray const& other),
			UGenArray::operator,(UGen const& other),
			UGen::operator<<(UGen const& other),
			UGen::operator,(UGen const& other) */
	UGenArray operator<< (UGenArray const& other) throw();
	
	/** Append a UGen to the end of this UGenArray.
	 All null UGen instances are removed from the result.
	 @param other	A UGen.
	 @return		A UGenArray with all the UGen instances from this UGenArray with the
					other UGen appended to the end. 
	 @see	UGenArray::operator<<(UGenArray const& other),
			UGenArray::operator,(UGenArray const& other),
			UGenArray::operator,(UGen const& other),
			UGen::operator<<(UGen const& other),
			UGen::operator,(UGen const& other) */
	UGenArray operator<< (UGen const& other) throw();
	
	/** Append two UGenArray objects.
	 Null UGen instances are NOT removed from the result.
	 @param other	The other UGenArray.
	 @return		A UGenArray with all the UGen instances from this UGenArray with the
					UGens in the other UGenArray appended to the end. 
	 @see	UGenArray::operator,(UGen const& other),
			UGenArray::operator<<(UGenArray const& other),
			UGenArray::operator<<(UGen const& other),
			UGen::operator<<(UGen const& other),
			UGen::operator,(UGen const& other) */
	UGenArray operator, (UGenArray const& other) throw();
	
	/** Append a UGen to the end of this UGenArray.
	 Null UGen instances are NOT removed from the result.
	 @param other	A UGen.
	 @return		A UGenArray with all the UGen instances from this UGenArray with the
					other UGen appended to the end. 
	 @see	UGenArray::operator,(UGenArray const& other),
			UGenArray::operator<<(UGenArray const& other),
			UGenArray::operator<<(UGen const& other),
			UGen::operator<<(UGen const& other),
			UGen::operator,(UGen const& other) */
	UGenArray operator, (UGen const& other) throw();
	
	/** Append and assignment. */
	UGenArray& operator<<= (UGenArray const& other) throw();
	
	/** Copy a UGen to one of the indexed slots in the UGen array.
	 This does nothing if the index is out of range.
	 @param index	The index of the slot.
	 @param item	The UGen to copy to the slot. */
	void put(const int index, UGen const& item) throw();
	
	/** @return		A reference to one of the UGen instances in the UGenArray,
					or a null UGen if the index is out of range.
	 @param index 	The index of the slot. */
	UGen& operator[] (const int index) throw();
	
	/** @return		A reference to one of the UGen instances in the UGenArray,
					or a null UGen if the index is out of range.
	 @param index 	The index of the slot. */
	UGen& at(const int index) throw();
	
	/** @return		A reference to one of the UGen instances in the UGenArray,
	 or a null UGen if the index is out of range.
	 @param index 	The index of the slot. */
	const UGen& operator[] (const int index) const throw();
	
	/** @return		A reference to one of the UGen instances in the UGenArray,
	 or a null UGen if the index is out of range.
	 @param index 	The index of the slot. */
	const UGen& at(const int index) const throw();	
	
	/** @return A reference to one of the UGen instances in the UGenArray.
	 @param	index	The index of the slot, the index is wrapped to ensure it is in range. */
	UGen& wrapAt(const int index) throw();
	
	/** @return A reference to one of the UGen instances in the UGenArray.
	 @param index	The index of the slot, the index is wrapped to ensure it is in range. */
	const UGen& wrapAt(const int index) const throw();
	
	/** @return A refernce to the first UGen in the array. This 
				returns a reference to a null UGen if the array is null. */	
	UGen& first() throw();
	
	/** @return A refernce to the last UGen in the array (at index size-1). 
				This returns a reference to a null UGen if the array is null. */
	UGen& last() throw();
	
	/** @return A refernce to the first UGen in the array. This 
	 returns a reference to a null UGen if the array is null. */	
	const UGen& first() const throw();
	
	/** @return A refernce to the last UGen in the array (at index size-1). 
	 This returns a reference to a null UGen if the array is null. */
	const UGen& last() const throw();
	
	UGenArray at(IntArray const& indices) const throw();
	UGenArray operator[] (IntArray const& indices) const throw();
	UGenArray wrapAt(IntArray const& indices) const throw();
	UGenArray range(const int startIndex, const int endIndex) const throw();
	UGenArray from(const int startIndex) const throw();
	UGenArray to(const int endIndex) const throw();
	UGenArray range(const int startIndex) const throw();
	
	/** Searches the UGenArray for the first UGen it finds with userData which matches the 
	 userDataToSearchFor arguments. This is used (for example) by the
	 Voicer and VoicerBase classes which store MIDI note number information
	 in the userData member. This function is used to find which voice should
	 be released when a note off message is received. To release all voices with
	 matching user data you could use @c release(userData) instead. 
	 @param userDataToSearchFor The userData to search for.
	 @return	The UGen with the userData which matches userDataToSearchFor
				or a null UGen if this is not found in the UGenArray. 
	 @see release() */
	const UGen& getUGenWithUserData(const int userDataToSearchFor) const throw();
	
	/** Find the index of a specific UGen in the UGen array. 
	 @param itemsToSearchFor	The UGen to search for. This uses UGen::containsIdenticalInternalsAs() 
								to compare UGen instances.
	 @param startIndex			The index from which to start searching (deafults to 0).
	 @return The index of the UGen or -1 if it is not found. */
	int indexOf(UGen const& itemsToSearchFor, const int startIndex = 0) const throw();
	
	/** Determine if a UGenArray contains a specific UGen. 
	 @param itemsToSearchFor	The UGen to search for. This uses UGen::containsIdenticalInternalsAs() 
								to compare UGen instances.
	 @return @c true if the UGen instance is found, otherwise false. */
	bool contains(UGen const& itemsToSearchFor) const throw();
	
	UGenArray interleave() const throw();
	
	/** Mixes each UGen in the array down to a single channel UGen.
	 @return A multichannel UGen containin a mix of each UGen in each ouput channel. */
	UGen mixEach() const throw();
	
	/** Mixes this UGenArray down to a multichannel UGen. */
	UGen mix() const throw();
	
	/// @} <!-- end Array manipulation -------------------------------------- -->
	
	
	/** @name Unary Ops. 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except mix() and unary- 
	 
	 For each of these a new UGenArray is returned which is a result of applying the unary operator
	 to each UGen in the UGenArray. The resulting UGenArray should be the same size as the original. */
	/// @{
	
	UGenArray operator- () const throw();
	UnaryOpMethodsDeclare(UGenArray);
	
	/// @} <!-- end Unary Ops ------------------------------------ -->
	
	/** @name Binary Ops. 
	 The definition of these (except the assignment versions) is done by macros in ugen_BinaryOpUGens.cpp
	 
	 For each of these a new UGenArray is returned which is a result of applying the binary operator
	 to each UGen in the two UGenArray. The resulting UGenArray should be the same size as the largest
	 of the two UGenArray instances. If the UGenArray instances are not the same size the index accessing 
	 the smaller UGenArray is wrapped to acces the array cyclically.  */
	/// @{	
	
	BinaryOpMethodsDeclare(UGenArray);
	
	/** Assigment and addition using UGenArray::operator+() and UGenArray::operator=() */
	UGenArray& operator+= (UGenArray const& other) throw();
	
	/** Assigment and subtraction using UGenArray::operator-() and UGenArray::operator=() */
	UGenArray& operator-= (UGenArray const& other) throw();
	
	/** Assigment and multiplication using UGenArray::operator*() and UGenArray::operator=() */
	UGenArray& operator*= (UGenArray const& other) throw();
	
	/** Assigment and division using UGenArray::operator/() and UGenArray::operator=() */
	UGenArray& operator/= (UGenArray const& other) throw();
	
	/// @} <!-- end Binary Ops ----------------------------------- -->
	
	/// @name Miscellaneous
	/// @{
	/** Attempts to release all UGen instances in the UGenArray using UGen::release(). */
	void release() throw();	
	/** Attempts to release all UGen instances with the specified user data in the UGenArray using UGen::release(). */
	void release(const int userDataToSearchFor) throw();
	/// @} <!-- end Miscellaneous -------------------------------- -->
	
private:
	Internal* internal;
};


#endif // UGEN_UGENARRAY_H