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
#include "../filters/control/ugen_Lag.h"
#include "../core/ugen_ExternalControlSource.h"


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


#define DeclareIOSMIDIDataSourceCommonFunctions(CLASSNAME,														\
												CONSTRUCTOR_ARGS_CALL,											\
												CONSTRUCTOR_ARGS_DECLARE,										\
												LAGTIME_ARG_CALL,												\
												CONSTRUCTOR_ARGS_DECLARE_UGEN)									\
	CLASSNAME CONSTRUCTOR_ARGS_DECLARE throw();																	\
	CLASSNAME init CONSTRUCTOR_ARGS_DECLARE throw()			{ return CLASSNAME CONSTRUCTOR_ARGS_CALL; }			\
																												\
	static UGen KR CONSTRUCTOR_ARGS_DECLARE_UGEN throw()														\
	{																											\
		return CLASSNAME CONSTRUCTOR_ARGS_CALL.krInternal(LAGTIME_ARG_CALL);									\
	}																											\
																												\
	static UGen AR CONSTRUCTOR_ARGS_DECLARE_UGEN throw()														\
	{																											\
		return Lag(CLASSNAME CONSTRUCTOR_ARGS_CALL, LAGTIME_ARG_CALL);											\
	}																											\
																												\
	void handleIncomingMidiMessage (void* source, ByteArray const& message) throw()


#define DeclareIOSMIDIDataSourceNoDefault(CLASSNAME,								\
										  CONSTRUCTOR_ARGS_CALL,					\
										  CONSTRUCTOR_ARGS_DECLARE,					\
										  LAGTIME_ARG_CALL,							\
										  CONSTRUCTOR_ARGS_DECLARE_UGEN)			\
	class CLASSNAME : public ExternalControlSource																	\
	{																												\
	public:																											\
		DeclareIOSMIDIDataSourceCommonFunctions(CLASSNAME,															\
												CONSTRUCTOR_ARGS_CALL,												\
												CONSTRUCTOR_ARGS_DECLARE,											\
												LAGTIME_ARG_CALL,													\
												CONSTRUCTOR_ARGS_DECLARE_UGEN);										\
	}


#define DeclareIOSMIDIDataSource(CLASSNAME,											\
								 CONSTRUCTOR_ARGS_CALL,								\
								 CONSTRUCTOR_ARGS_DECLARE,							\
								 LAGTIME_ARG_CALL,									\
								 CONSTRUCTOR_ARGS_DECLARE_UGEN)						\
	class CLASSNAME : public ExternalControlSource																	\
	{																												\
	public:																											\
		CLASSNAME () throw() : ExternalControlSource() { }															\
		DeclareIOSMIDIDataSourceCommonFunctions(CLASSNAME,															\
												CONSTRUCTOR_ARGS_CALL,												\
												CONSTRUCTOR_ARGS_DECLARE,											\
												LAGTIME_ARG_CALL,													\
												CONSTRUCTOR_ARGS_DECLARE_UGEN);										\
	}



class MIDIControllerInternal :  public MidiInputReceiver,
								public ExternalControlSourceInternal
{
public:
	MIDIControllerInternal(const int midiChannel, const int controllerNumber,
						   const float minVal, const float maxVal, 
						   const ExternalControlSource::Warp warp, 
						   void* port) throw();
	
	void handleIncomingMidiMessage (void* source, ByteArray const& message) throw();
	
private:
	int midiChannel_;
	int controllerNumber_;
	void* port_;
};

DeclareIOSMIDIDataSourceNoDefault(MIDIController, 
								   (midiChannel, controllerNumber, minVal, maxVal, warp, port), 
								   (const int midiChannel = 1, const int controllerNumber = 1,
									const float minVal = 0.f, const float maxVal = 127.f, 
									const ExternalControlSource::Warp warp = ExternalControlSource::Linear, 
									void* port = 0),
								   lagTime,
								   (const int midiChannel = 1, const int controllerNumber = 1,
									const float minVal = 0.f, const float maxVal = 127.f, 
									const ExternalControlSource::Warp warp = ExternalControlSource::Linear, 
									const double lagTime = 0.1,
									void* port = 0));


class MIDIMostRecentNoteInternal :  public MidiInputReceiver,
									public ExternalControlSourceInternal
{
public:
	MIDIMostRecentNoteInternal(const int midiChannel,
							   const float minVal, const float maxVal, 
							   const ExternalControlSource::Warp warp, 
							   void* port) throw();
	
	void handleIncomingMidiMessage (void* source, ByteArray const& message) throw();
	
private:
	int midiChannel_;
	void* port_;
};

DeclareIOSMIDIDataSourceNoDefault(MIDIMostRecentNote, 
								   (midiChannel, minVal, maxVal, warp, port), 
								   (const int midiChannel = 1,
									const float minVal = 0.f, const float maxVal = 127.f, 
									const ExternalControlSource::Warp warp = ExternalControlSource::Linear, 
									void* port = 0),
								   lagTime,
								   (const int midiChannel = 1,
									const float minVal = 0.f, const float maxVal = 127.f, 
									const ExternalControlSource::Warp warp = ExternalControlSource::Linear, 
									const double lagTime = 0.1,
									void* port = 0));



#endif // _UGEN_ugen_iOSMidiInput_H_
