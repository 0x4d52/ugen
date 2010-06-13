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

#include "ugen_UGen.h"
#include "ugen_SmartPointer.h"


//=========================== SmartPointer ==================================

//static int allocationCount = 0;

SmartPointer::SmartPointer() throw()
:	refCount(1),
	active(true)
{		
	//printf("+++++++, %p, %d\n", this, ++allocationCount);
}

SmartPointer::~SmartPointer()
{
	//printf("-------, %p, %d\n", this, --allocationCount);
	ugen_assert(refCount >= 0);
}

//#if defined(JUCE_VERSION) || defined(DOXYGEN)
//void SmartPointer::incrementRefCount()  throw()
//{	
//	if(active) atomicIncrement (refCount);
//}
//
//void SmartPointer::decrementRefCount()  throw()
//{ 
//	if(active)
//	{
//		if ((atomicDecrementAndReturn (refCount) == 0))
//		{
//			active = false;
//			UGen::getDeleter()->deleteInternal(this);
//		}
//	}
//}
//#else
void SmartPointer::incrementRefCount()  throw()
{	
	if(active) ++refCount; 
}

void SmartPointer::decrementRefCount()  throw()
{ 
	if(active)
	{
		--refCount; 
		if(refCount == 0) 
		{
			active = false;
			UGen::getDeleter()->deleteInternal(this);
		}
	}
}
//#endif


END_UGEN_NAMESPACE