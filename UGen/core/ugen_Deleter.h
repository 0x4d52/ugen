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

#ifndef UGEN_DELETER_H
#define UGEN_DELETER_H

class SmartPointer;

/** A SmartPointer deleter. 
 
 A Deleter is used to delete SmartPointer objects (e.g., UGenInternal) rather than 
 calling delete directly. This default Deleter does simply call delete but other hosts
 could use another thread to delete objects so this is not done in an audio callback
 thread (for example).
 
 So instead of this:
 @code
 delete mySmartPointerObj;
 @endcode
 
 ..do this:
 @code
 UGen::getDeleter()->deleteInternal(mySmartPointerObj);
 @endcode
  
 @see getDeleter(), setDeleter(), JuceTimerDeleter */
class Deleter
{
public:
	Deleter() throw();
	virtual ~Deleter() throw();
	virtual void deleteInternal(SmartPointer* internalToDelete) throw();
	virtual void flush() throw() { };
};

/** A Deleter which doesn't actually delete anything.
 
 This Deleter just leaks memory by ignoring requests to delete anything. This is
 probably pretty useless but could be used to test the effect of a real memory leak
 over a long period. Initially it was used to isolate a problem where objects being deleted
 were causing a crash. This class was used so that the objects weren't actually deleted so
 the problem (which is now fixed) could be put to one side.
 
 Obviously use this with caution! */
class LeakingDeleter : public Deleter
{
public:
	void deleteInternal(SmartPointer* internalToDelete) throw() { } // just leak!
};


#endif // UGEN_DELETER_H