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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_iPhoneAUIOHost_H_
#define _UGEN_ugen_iPhoneAUIOHost_H_

#include "ugen_NSDeleter.h"
#include "../core/ugen_UGen.h"
#include "../core/ugen_UGenArray.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

/** AudioUnit / iPhone audio support.
 You should subclass this - implementint the constructGraph method - and create ONE of 
 them in your app. The best location is probably the app delegate class but if 
 your app has only one view controller it might be more convenient to put it in there. 
 You then need to init and initAudio. e.g.
 @code host = [[[MyUIKitAUIOHostSubclass alloc] init] initAudio]; @endcode
 
 If the host is located in the app delegate the best place for this is probably in
 applicationDidFinishLaunching:
 
 To stop audio cleanly, call shutdown. Again in the app delegate the best place for 
 this is in applicationWillTerminate: (dealloc may never get called on the iPhone).
 
 This is the recommended method for audio support on the iPhone (it is better than
 the AudioQueueIOHostController class for example).
 
 @warning This class has changed in recent revisions, it is no longer a subclass of
 UIViewController, or NSObject<UIApplicationDelegate>. This is to make it more flexible
 with the various ways InterfaceBuilder allow interface construction with various heirarchies.
 
 @see JuceIOHost AudioQueueIOHostController
 @ingroup Hosts */

@interface UIKitAUIOHost : NSObject 
{
	AudioStreamBasicDescription format;
	AURenderCallbackStruct		inputProc;
	Float64						hwSampleRate;
	AudioUnit					rioUnit;
	UGen						rawInput;
	UGen						postFadeOutput;
	UGen						preFadeOutput;
	int							bufferSize;
	float						*floatBuffer;
	UInt32						audioInputIsAvailable;
	UInt32						numInputChannels;
	UInt32						numOutputChannels;
	bool						isRunning;
	float						fadeInTime;
	UGenArray					others;
	NSLock*						nsLock;
	NSDeleter*					deleter;
	int							preferredBufferSize;
}

/** Initialises the AudioUnit framework and structures.
 You must call this method after the object is initialised. 
 
 @warning This was previously not the case where it was called automatically.  
 Allowing it to be called manually gives greater flexibility. */
- (id)initAudio;
- (void)setPreferredBufferSize:(int)size;
- (void)setFormat;
- (int)setupRemoteIO;
- (OSStatus)renderCallback:(UInt32)inNumberFrames 
		   withActionFlags:(AudioUnitRenderActionFlags*)ioActionFlags
			   atTimeStamp:(const AudioTimeStamp*)inTimeStamp 
			   withBuffers:(AudioBufferList*)ioData;
- (void)propertyCallback:(AudioSessionPropertyID)inID
				withSize:(UInt32)inDataSize	
			withProperty:(const void *)inPropertyValue;
- (void)fixAudioRouteIfSetToReceiver;
- (void)interruptionCallback:(UInt32)inInterruption;

/** Construct a UGen graph.
 You must implement this in your subclass. You should return a UGen which will be the UGen graph which is 
 performed and rendered to the host. The input parameter may be ignored if only signal generation is required 
 or may be used if a processing algorithm is being implemented (e.g., filtering incoming audio data).
 
 @param input	The input UGen which will contain audio data from the host.
 @return		the UGen graph which will be performed */
- (UGen)constructGraph:(UGen)input;

- (void)addOther:(UGen)ugen;

- (void)lock;
- (void)unlock;
- (BOOL)tryLock;

- (void)isAboutToShutdown;
- (void)shutdown;

@end




BEGIN_UGEN_NAMESPACE


// C++ headers here ?


#endif // _UGEN_ugen_iPhoneAUIOHost_H_
