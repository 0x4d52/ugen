// $Id: ugen_JuceTimerDeleter.cpp 1003 2010-03-12 16:29:12Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/ugen_JuceTimerDeleter.cpp $

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

#include "../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "../core/ugen_UGenInternal.h" // smartpointer instead?
#include "ugen_JuceTimerDeleter.h"

JuceTimerDeleter::JuceTimerDeleter() throw()
{
	startTimer(1);
}

JuceTimerDeleter::~JuceTimerDeleter() throw()
{
	stopTimer();
	flush();
}

void JuceTimerDeleter::deleteInternal(SmartPointer* internalToDelete) throw()
{
	toDelete.add(internalToDelete);
}

void JuceTimerDeleter::timerCallback()
{
	toDelete.getLock().enter();
	
	for(int i = 0; i < toDelete.size(); i++)
	{
		SmartPointer* internalToDelete = toDelete.getUnchecked(i);		
		//if(internalToDelete->getRefCount() >= 0)  // hack for some other bug? how could a refCount get to -1?
			delete internalToDelete;
	}
	
	toDelete.clearQuick();
	toDelete.getLock().exit();
}

void JuceTimerDeleter::flush() throw()
{
	timerCallback();
}


END_UGEN_NAMESPACE

#endif