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
class Dictionary : public SmartPointerContainer< DictionaryInternal<ValueType,KeyType> >
{
public:
	Dictionary() throw()
	:	SmartPointerContainer< DictionaryInternal<ValueType,KeyType> >
		(new DictionaryInternal<ValueType,KeyType>())
	{
	}
	
//private:
	ObjectArray<ValueType>& getValues() throw()
	{ 
		return this->getInternal()->getValues(); 
	}
	
	ObjectArray<KeyType>& getKeys() throw()
	{
		return this->getInternal()->getKeys(); 
	}	
	
public:
	const ObjectArray<ValueType>& getValues() const throw()
	{ 
		return this->getInternal()->getValues(); 
	}
	
	const ObjectArray<KeyType>& getKeys() const throw()
	{
		return this->getInternal()->getKeys(); 
	}
	
	ValueType put(KeyType const& key, ValueType const& value) throw()
	{
		ObjectArray<ValueType>& values = getValues();
		ObjectArray<KeyType>& keys = getKeys();

		int index = values.indexOf(value);
		
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
	
	ValueType& at(KeyType const& key) throw()
	{
		ObjectArray<ValueType>& values = getValues();
		ObjectArray<KeyType>& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	const ValueType& at(KeyType const& key) const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	ValueType& operator[](KeyType const& key) throw()
	{
		ObjectArray<ValueType>& values = getValues();
		ObjectArray<KeyType>& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
	const ValueType& operator[](KeyType const& key) const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = keys.indexOf(key);
		return values[index];
	}
	
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

	const KeyType& keyForValue(ValueType const& value) const
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		int index = values.indexOf(value);
		return keys[index];
	}
	
	KeyType& key(const int index) throw()
	{
		return getKeys()[index];
	}
	
	ValueType& value(const int index) throw()
	{
		return getValues()[index];
	}
	
	const KeyType& key(const int index) const throw()
	{
		return getKeys()[index];
	}
	
	const ValueType& value(const int index) const throw()
	{
		return getValues()[index];
	}
	
	
	int length() const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		ugen_assert(values.length() == keys.length());
		
		return values.length();
	}
	
	int size() const throw()
	{
		ObjectArray<ValueType> const& values = getValues();
		ObjectArray<KeyType> const& keys = getKeys();
		
		ugen_assert(values.size() == keys.size());
		
		return values.size();
	}
	
	
	
};


#endif // _UGEN_ugen_Collections_H_
