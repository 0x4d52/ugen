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

#ifndef _UGEN_ugen_AudioQueueIOHost_H_
#define _UGEN_ugen_AudioQueueIOHost_H_

#include "../core/ugen_UGen.h"
#include "ugen_NSDeleter.h"


END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

struct AQCallbackStruct;

/** AudioQueue / iPhone audio support.
 You should use this as your base class rather than UIViewController.
 You then need to implement a constructGraph method and call the 
 initAudioQueue method at the end of your loadView method. 
 @see JuceIOHost 
 @ingroup Hosts */
@interface AudioQueueIOHostController : UIViewController
{
	/** Private internal structure */
	AQCallbackStruct *aqc;
	NSDeleter* deleter;
	
}
/** Initialises the AudioQueue framework and structures.
 You should call this at the end of your loadView method
 just before returning. */
- (int) initAudioQueue:(BOOL)isOutputOnly;

/** @internal */
- (void) audioQueueInputCallback: (AudioQueueRef) inQ bufferRef: (AudioQueueBufferRef) inQB;

/** @internal */
- (void) audioQueueOutputCallback: (AudioQueueRef) inQ bufferRef: (AudioQueueBufferRef) outQB;

/** Construct a UGen graph.
 You must implement this in your subclass. You should return a UGen which will be the UGen graph which is 
 performed and rendered to the host. The input parameter may be ignored if only signal generation is required 
 or may be used if a processing algorithm is being implemented (e.g., filtering incoming audio data).
  
 @param input	The input UGen which will contain audio data from the host.
 @return		the UGen graph which will be performed */
- (UGen) constructGraph: (UGen) input;

- (void) cleanUp;
@end

BEGIN_UGEN_NAMESPACE


// C++ headers here ?


#endif // _UGEN_ugen_AudioQueueIOHost_H_
