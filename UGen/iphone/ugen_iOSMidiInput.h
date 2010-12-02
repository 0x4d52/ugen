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
 
 CoreMIDI iOS code originally by Pete Goodliffe.
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_iOSMidiInput_H_
#define _UGEN_ugen_iOSMidiInput_H_

#include "ugen_iOSVersionDetection.h"
#include "../core/ugen_Arrays.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

@class MidiInput;

/// Delegate protocol for MidiInput class.
/// Adopt this protocol in your object to receive events from MIDI
///
/// IMPORTANT NOTE:
/// MIDI input is received from a high priority background thread
@protocol MidiInputDelegate

// Raised on main run loop
- (void) midiInput:(MidiInput*)input event:(NSString*)event;

/// NOTE: Raised on high-priority background thread.
///
/// To do anything UI-ish, you must forward the event to the main runloop
/// (e.g. use performSelectorOnMainThread:withObject:waitUntilDone:)
//- (void) midiInput:(MidiInput*)input midiReceived:(const MIDIPacketList *)packetList midiSource:(void*)source;

- (void) handleIncomingMidiMessage:(ByteArray const&)message midiSource:(void*)source;

@end

//typedef id<MidiInputDelegate> NSMidiInputDelegate;

/// Class for receiving MIDI input from any MIDI device.
///
/// If you intend your app to support iOS 3.x which does not have CoreMIDI
/// support, weak link to the CoreMIDI framework, and only create a
/// MidiInput object if you are running the right version of iOS.
@interface MidiInput : NSObject
{
    MIDIClientRef           client;
    MIDIPortRef             outputPort;
    MIDIPortRef             inputPort;
    NSUInteger              numberOfConnectedDevices;	
	NSMutableSet*			delegates;
}

//@property (nonatomic,assign)   id<MidiInputDelegate> delegate;
@property (nonatomic,readonly) NSUInteger            numberOfConnectedDevices;

+ (MidiInput*)sharedInstance;

/// Send a MIDI byte stream to every connected MIDI port
- (void) sendMidi:(const UInt8*)bytes size:(UInt32)size;
- (void) addDelegate:(id<MidiInputDelegate>)object;
- (void) removeDelegate:(id<MidiInputDelegate>)object;

@end

/// Dump a list of MIDI interfaces as events on this delegate.
///
/// A helpful diagnostic, and an example of how to enumerate devices
NSUInteger ListInterfaces(id<MidiInputDelegate> delegate);

BEGIN_UGEN_NAMESPACE


// C++ headers here ?

class MidiInputReceiver
{
public:
	MidiInputReceiver() throw();
	virtual ~MidiInputReceiver();
	virtual void handleIncomingMidiMessage (void* source, ByteArray const& message) = 0;
};

class MidiInputSender
{
public:
	static MidiInputSender& getInstance();
	
	void addMidiReceiver (MidiInputReceiver* const receiver) throw();
	void removeMidiReceiver (MidiInputReceiver* const receiver) throw();
	void sendIncomingMidiMessage(void* source, ByteArray const& message) throw();
	
private:
	MidiInputSender() throw();

	ObjectArray<MidiInputReceiver*> receivers;
	void *peer;
};




#endif // _UGEN_ugen_iOSMidiInput_H_
