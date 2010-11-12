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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "../core/ugen_SmartPointer.h"
#include "ugen_NSDeleter.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

@interface NSDeleterPeer : NSObject
{
@public
	SmartPointer* deletee_;
}

- (void) setDeletee: (SmartPointer*) deletee;
@end


@implementation NSDeleterPeer
- (void) setDeletee: (SmartPointer*) deletee
{
	deletee_ = deletee;
}

-(void) dealloc
{
	//printf("NSDeleterPeer::dealloc called %p\n", deletee_);
	
	delete deletee_;
	[super dealloc];
}

@end

BEGIN_UGEN_NAMESPACE


void NSDeleter::deleteInternal(SmartPointer* internalToDelete) throw()
{
	NSDeleterPeer* peer = [NSDeleterPeer alloc];
	[peer setDeletee:internalToDelete];
	//[peer release];
	
	// sched release on another thread:
	[peer performSelectorOnMainThread:@selector(release) withObject:nil waitUntilDone:NO];
	
	//printf("NSDeleter::deleteInternal returning %p\n", internalToDelete);
}


END_UGEN_NAMESPACE

#endif