// $Id: ugen_SmartPointer.h 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/core/ugen_SmartPointer.h $

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

#ifndef _UGEN_ugen_SmartPointer_H_
#define _UGEN_ugen_SmartPointer_H_



class SmartPointer
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	SmartPointer() throw();
	virtual ~SmartPointer();
	void incrementRefCount() throw();
	virtual void decrementRefCount() throw(); 
	
	/// @} <!-- end Construction and destruction -->
	
	/// @name Miscellaneous
	/// @{
	
	int getRefCount() const throw()	{ return refCount; }
	
	/// @} <!-- end Miscellaneous -->
	
protected:
	int refCount;
	bool active;
	
private:
	SmartPointer (const SmartPointer&);
    const SmartPointer& operator= (const SmartPointer&);
};

template<class SmartPointerType>
class SmartPointerContainer
{
public:
	SmartPointerContainer(SmartPointerType* internalToUse = 0) throw() 
	:	internal(internalToUse)
	{
	}
	
	~SmartPointerContainer()
	{
		if(internal != 0) 
			internal->decrementRefCount();
		
		internal = 0;
	}
	
	inline SmartPointerType* getInternal() throw() 
	{
		return internal;
	}
	
	inline const SmartPointerType* getInternal() const throw() 
	{
		return internal;
	}
	
	bool isNull() const throw()
	{
		return internal == 0;
	}
	
	bool isNotNull() const throw()
	{
		return internal != 0;
	}
	
	void setInternal(SmartPointerType* newInternal) throw()
	{
		if(internal != 0)
			internal->decrementRefCount();
		
		internal = newInternal;
	}
	
	SmartPointerContainer(SmartPointerContainer const& copy) throw()
	:	internal(copy.internal)
	{
		if(internal != 0) internal->incrementRefCount();
	}
		
	SmartPointerContainer& operator= (SmartPointerContainer const& other) throw()
	{
		if (this != &other)
		{		
			if(other.internal != 0)
				other.internal->incrementRefCount();
			
			if(internal != 0)
				internal->decrementRefCount();
			
			internal = other.internal;
		}
		
		return *this;		
	}
	
	bool operator== (SmartPointerContainer const& other) const throw()
	{
		return internal == other.internal;
	}
	
	bool operator!= (SmartPointerContainer const& other) const throw()
	{
		return internal != other.internal;
	}
	
private:
	SmartPointerType* internal;
};

template<class SmartPointerType>
class SmartPointerArrayContainer
{
public:
	SmartPointerArrayContainer(const int size) throw() 
	:	size_(size < 0 ? 0 : size),
		internals(size_ < 1 ? 0 : new SmartPointerType*[size_])
	{
		if(internals)
			memset(internals, 0, size_ * sizeof(SmartPointerType*));
	}
	
	~SmartPointerArrayContainer()
	{
		decrementInternals();
		delete [] internals;
		internals = 0;
		size_ = 0;
	}
	
	inline int getNumInternals() const throw()
	{
		return size_;
	}
	
	void initInternals(const int size) throw()
	{
		decrementInternals();
		
		if(size == size_ && internals)
		{
			memset(internals, 0, size_ * sizeof(SmartPointerType*));
			return;
		}
		
		delete [] internals;
		
		internals = 0;
		size_ = size < 0 ? 0 : size;
		
		if(size_ > 0)
		{
			internals = new SmartPointerType*[size_];
			if(internals)
				memset(internals, 0, size_ * sizeof(SmartPointerType*));
		}
	}
	
	inline SmartPointerType* getInternal(const int index = 0) throw() 
	{
		if(index < 0 || index >= size_) 
		{
			ugen_assertfalse;
			return 0;
		}
		
		return internals[index];
	}
	
	inline const SmartPointerType* getInternal(const int index = 0) const throw() 
	{
		if(index < 0 || index >= size_)
		{
			ugen_assertfalse;
			return 0;
		}
		
		return internals[index];
	}
	
	void setInternal(SmartPointerType* newInternal, const int index = 0) throw()
	{
		if(index < 0 || index >= size_) { ugen_assertfalse; return; }
				
		SmartPointerType* oldInternal = internals[index];
		
		if(oldInternal != 0)
			oldInternal->decrementRefCount();
		
		internals[index] = newInternal;
	}
	
	SmartPointerArrayContainer(SmartPointerArrayContainer const& copy) throw()
	:	size_(copy.size_),
		internals(size_ < 1 ? 0 : new SmartPointerType*[size_])
	{
		if(internals)
		{
			copy.incrementInternals();
			memcpy(internals, copy.internals, size_ * sizeof(SmartPointerType*));
		}
	}
	
	SmartPointerArrayContainer& operator= (SmartPointerArrayContainer const& other) throw()
	{
		if (this != &other)
		{		
			other.incrementInternals();
			decrementInternals();
			
			if(size_ < other.size_)
			{
				if(internals != 0) 
					delete [] internals;
				
				internals = new SmartPointerType*[other.size_];
			}
			
			size_ = other.size_;
			
			memcpy(internals, other.internals, size_ * sizeof(SmartPointerType*));
		}
		
		return *this;		
	}
	
	bool operator== (SmartPointerArrayContainer const& other) const throw()
	{
		if(size_ != other.size_) return false;
		if(size_ == 0) return true;
		
		for(int i = 0; i < size_; i++)
		{
			if(internals[i] != other.internals[i])
				return false;
		}
		
		return true;
	}
	
	bool operator!= (SmartPointerArrayContainer const& other) const throw()
	{
		return !operator==(other);
	}
	
private:
	
	void incrementInternals() throw()
	{
		if(internals != 0)
		{
			for(int i = 0; i < size_; i++)
			{
				SmartPointerType* internal = internals[i];
				if(internal)
					internal->incrementRefCount();
			}
		}
	}
	
	
	void decrementInternals() throw()
	{
		if(internals != 0)
		{
			for(int i = 0; i < size_; i++)
			{
				SmartPointerType* internal = internals[i];
				if(internal)
					internal->decrementRefCount();
			}
		}
	}
	
	
	int size_;
	SmartPointerType** internals;
};





#endif // _UGEN_ugen_SmartPointer_H_
