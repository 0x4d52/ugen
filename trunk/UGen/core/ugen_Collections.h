// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-9 by Martin Robinson www.miajo.co.uk
 
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

#ifndef _UGEN_ugen_Collections_H_
#define _UGEN_ugen_Collections_H_

#include "ugen_SmartPointer.h"
#include "ugen_Arrays.h"
#include "ugen_Text.h"

template<class ValueType, class KeyType = Text>
class DictionaryInternal : public SmartPointer
{
public:
	DictionaryInternal() throw()
	{
	}
	
	~DictionaryInternal() throw()
	{
	}
	
	ObjectArray<ValueType>& getValues() throw()
	{
		return values;
	}
	
	ObjectArray<KeyType>& getKeys() throw()
	{
		return keys;
	}
	
	const ObjectArray<ValueType>& getValues() const throw()
	{
		return values;
	}
	
	const ObjectArray<KeyType>& getKeys() const throw()
	{
		return keys;
	}
	
private:
	ObjectArray<ValueType> values;
	ObjectArray<KeyType> keys;
};


template<class ValueType, class KeyType = Text>
class KeyValuePair
{
public:
	KeyValuePair()
	{
	}
	
	KeyValuePair(KeyType const& key, ValueType const& value)
	:	v(value), k(key)
	{
	}
	
	const KeyType& getKey() { return k; }
	const ValueType& getValue() { return v; }
	
private:
	ValueType v;
	KeyType k;
};


/** A dictionary class for storing key/value pairs.
 Items are stored in an array and accessed via their key. By default the key is a Text string but
 can be any appropriate type. */
template<class ValueType, class KeyType = Text>
class Dictionary : public SmartPointerContainer< DictionaryInternal<ValueType,KeyType> >
{
public:
	typedef KeyValuePair<ValueType,KeyType> KeyValuePairType;
	typedef ObjectArray<KeyValuePairType> KeyValuePairArrayType;
	
	/** Creates an empty dictionary. */
	Dictionary() throw()
	:	SmartPointerContainer< DictionaryInternal<ValueType,KeyType> >
		(new DictionaryInternal<ValueType,KeyType>())
	{
	}
	
	/** Creates a dictionary initialise with an array of key/value pairs. */
	Dictionary(KeyValuePairArrayType pairs) throw()
	:	SmartPointerContainer< DictionaryInternal<ValueType,KeyType> >
		(new DictionaryInternal<ValueType,KeyType>())
	{
		put(pairs);
	}
		
	/** Returns the array of values. */
	const ObjectArray<ValueType>& getValues() const throw()
	{ 
		return this->getInternal()->getValues(); 
	}
	
	/** Returns the array of keys. */
	const ObjectArray<KeyType>& getKeys() const throw()
	{
		return this->getInternal()->getKeys(); 
	}
	
	KeyValuePairArrayType getPairs() const throw()
	{
		KeyValuePairArrayType pairs = KeyValuePairArrayType::withSize(length());
		
		for(int i = 0; i < length(); i++)
		{
			pairs[i] = KeyValuePairType(key(i), value(i));
		}
		
		return pairs;
	}
	
	/** Put an item into the dicionary associated with the specified key. 
	 If an item is already stored agains that key the old value is returned.
	 Otherwise a "null" version of the value is returned. In many case you will
	 not need to use the return value. */
	ValueType put(KeyType const& key, ValueType const& value) throw()
	{
		ObjectArray<ValueType>& values = this->getInternal()->getValues();
		ObjectArray<KeyType>& keys = this->getInternal()->getKeys();

		int index = keys.indexOf(key);
		
		if(index >= 0)
		{
			ValueType& oldValue = values[index];
			values.put(index, value);
			return oldValue;
		}
		else
		{
			keys.add(key);
			values.add(value);
			return ObjectArray<ValueType>::getNull();
		}
	}
	
	ValueType put(KeyValuePairType pair) throw()
	{
		return put(pair.getKey(), pair.getValue());
	}
	
	void put(KeyValuePairArrayType pairs) throw()
	{
		for(int i = 0; i < pairs.length(); i++)
		{
			put(pairs[i]);
		}
	}
	
	
	/** Return a value at the specified key.
	 If the key is not found then a "null" version of the value is returned. */
	ValueType& at(KeyType const& key) throw()
	{
		ObjectArray<ValueType>& values = getValues();
		ObjectArray<KeyType>& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	/** Return a value at the specified key.
	 If the key is not found then a "null" version of the value is returned. */
	const ValueType& at(KeyType const& key) const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	/** Return a value at the specified key.
	 If the key is not found then a "null" version of the value is returned. */
	ValueType& operator[](KeyType const& key) throw()
	{
		ObjectArray<ValueType>& values = this->getInternal()->getValues();
		ObjectArray<KeyType>& keys = this->getInternal()->getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	/** Return a value at the specified key.
	 If the key is not found then a "null" version of the value is returned. */
	const ValueType& operator[](KeyType const& key) const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	/** Remove and return an item at the specified key.
	 If the key is not found then a "null" version of the value is returned. */
	ValueType remove(KeyType const& key) throw()
	{
		ObjectArray<ValueType>& values = getValues();
		ObjectArray<KeyType>& keys = getKeys();
		
		int index = keys.indexOf(key);
		
		if(index >= 0)
		{
			ValueType& removed = values[index];
			keys.remove(index);
			values.remove(index);
			return removed;
		}
		else
		{
			return ObjectArray<ValueType>::getNull();
		}
	}

	/** Return the key for a particular value. */
	const KeyType& keyForValue(ValueType const& value) const
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = values.indexOf(value);
		return keys[index];
	}
	
	/** Get a key at a particular index. */
	KeyType& key(const int index) throw()
	{
		return getKeys()[index];
	}
	
	/** Get a value at a particular index. */
	ValueType& value(const int index) throw()
	{
		return getValues()[index];
	}
	
	/** Get a key at a particular index. */
	const KeyType& key(const int index) const throw()
	{
		return getKeys()[index];
	}
	
	/** Get a value at a particular index. */
	const ValueType& value(const int index) const throw()
	{
		return getValues()[index];
	}
	
	/** Get the number of items stored in the dictionary. */
	int length() const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		ugen_assert(values.length() == keys.length()); // these should be the same length!
		
		return values.length();
	}
	
	/** Get the number of items stored in the dictionary. */
	int size() const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		ugen_assert(values.size() == keys.size()); // these should be the same size!
		
		return values.size();
	}
};


#endif // _UGEN_ugen_Collections_H_
