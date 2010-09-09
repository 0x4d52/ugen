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

#ifndef UGEN_JUCETIMERDELETER_H
#define UGEN_JUCETIMERDELETER_H

#include "../core/ugen_Deleter.h"

/** Stores and deletes SmartPointer objects using a timer.
 
 The default Deleter deletes objects immediately using the delete operator.
 This stores SmartPointers (e.g., UGenInternal) in a juce::Array which is checked
 periodically, any items found in the Array are deleted. This defers the deletion
 of these objects to a different Thread, rather than performing the delete operator
 on an audio procesing Thread for example. 
 
 @see UGen::setDeleter() */
class JuceTimerDeleter :	public Deleter,
							public Timer
{
public:
	JuceTimerDeleter() throw();
	~JuceTimerDeleter() throw();
	
	/** Adds an item to the Array.
	 @param internalToDelete The items to be deleted. */
	void deleteInternal(SmartPointer* internalToDelete) throw();
	
	/** Clears the Array and deletes all of the items now.
	 This is useful during shutdown for example. */
	void flush() throw();

private:
	void timerCallback();
	Array<SmartPointer*, CriticalSection> toDelete;
};


#endif // UGEN_JUCETIMERDELETER_H