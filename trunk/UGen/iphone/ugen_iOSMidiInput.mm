/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && defined(UGEN_IOS_COREMIDI)

BEGIN_UGEN_NAMESPACE

#include "ugen_iOSMidiInput.h"
#include "../basics/ugen_MixUGen.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif


/// A helper that NSLogs an error message if "c" is an error code
#define NSLogError(c,str) do{if (c) NSLog(@"Error (%@): %u:%@", str, c,[NSError errorWithDomain:NSMachErrorDomain code:c userInfo:nil]);}while(false)

//==============================================================================

void MIDIInputNotifyProc(const MIDINotification *message, void *refCon);
void MIDIInputReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon);

@interface MidiInput ()
- (void) scanExistingDevices;
@end

//==============================================================================

static MidiInput *sharedSingleton = nil;


@implementation MidiInput

//@synthesize delegate;
@synthesize numberOfConnectedDevices;

+ (MidiInput*)sharedInstance
{
    if (sharedSingleton == nil) {
        sharedSingleton = [[super allocWithZone:NULL] init];
    }
    return sharedSingleton;
}

+ (id)allocWithZone:(NSZone *)zone
{
    return [self sharedInstance];
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (id)retain
{
    return self;
}

- (NSUInteger)retainCount
{
    return NSUIntegerMax;  //denotes an object that cannot be released
}

- (void)release
{
    //do nothing
}

- (id)autorelease
{
    return self;
}

- (id) init
{
    if ((self = [super init]))
    {
		MIDINetworkSession* session = [MIDINetworkSession defaultSession];
		session.enabled = YES;
		session.connectionPolicy = MIDINetworkConnectionPolicy_Anyone;		
				
        OSStatus s = MIDIClientCreate((CFStringRef)@"MidiMonitor MIDI Client", MIDIInputNotifyProc, self, &client);
        NSLogError(s, @"Create MIDI client");

        s = MIDIOutputPortCreate(client, (CFStringRef)@"MidiMonitor Output Port", &outputPort);
        NSLogError(s, @"Create output MIDI port");

        s = MIDIInputPortCreate(client, (CFStringRef)@"MidiMonitor Input Port", MIDIInputReadProc, self, &inputPort);
        NSLogError(s, @"Create input MIDI port");

		delegates = [[NSMutableSet alloc] initWithCapacity:1];
		
        [self scanExistingDevices];
    }

    return self;
}

- (void) dealloc
{
    if (outputPort)
    {
        OSStatus s = MIDIPortDispose(outputPort);
        NSLogError(s, @"Dispose MIDI port");
    }

    if (inputPort)
    {
        OSStatus s = MIDIPortDispose(inputPort);
        NSLogError(s, @"Dispose MIDI port");
    }

    if (client)
    {
        OSStatus s = MIDIClientDispose(client);
        NSLogError(s, @"Dispose MIDI client");
    }
	
	[delegates removeAllObjects];
	[delegates release];

    [super dealloc];
}

- (void) addDelegate:(id<MidiInputDelegate>)object
{
	[delegates addObject:object];
}

- (void) removeDelegate:(id<MidiInputDelegate>)object
{
	[delegates removeObject:object];
}


//==============================================================================
#pragma mark Connect/disconnect

NSString *DescrptionOfEndpoint(MIDIEndpointRef ref)
{
    NSString *string = nil;

    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity(ref, &entity);

    CFPropertyListRef properties = nil;
    OSStatus s = MIDIObjectGetProperties(entity, &properties, true);
    if (s)
        string = [NSString stringWithFormat:@"Error getting properties: %@",
                  [NSError errorWithDomain:NSMachErrorDomain code:s userInfo:nil] ];
    else
        string = [NSString stringWithFormat:@"%@", properties];
    CFRelease(properties);

    return string;
}

- (void) connectSource:(MIDIEndpointRef)source
{
    ++numberOfConnectedDevices;
	
	NSEnumerator *enumerator = [delegates objectEnumerator];
	id<MidiInputDelegate> delegate;
	while ((delegate = [enumerator nextObject])) 
	{
		[delegate midiInput:self
					  event:[NSString stringWithFormat:@"Added a source: %@", DescrptionOfEndpoint(source)]];
	}
	
    OSStatus s = MIDIPortConnectSource(inputPort, source, self);
    NSLogError(s, @"Connecting to MIDI source");
}

- (void) disconnectSource:(MIDIEndpointRef)source
{
    --numberOfConnectedDevices;

	NSEnumerator *enumerator = [delegates objectEnumerator];
	id<MidiInputDelegate> delegate;
	while ((delegate = [enumerator nextObject])) 
	{
		[delegate midiInput:self event:@"Removed a source"];
	}
		
    OSStatus s = MIDIPortDisconnectSource(inputPort, source);
    NSLogError(s, @"Disconnecting from MIDI source");
}

- (void) connectDestination:(MIDIEndpointRef)destination
{
	NSEnumerator *enumerator = [delegates objectEnumerator];
	id<MidiInputDelegate> delegate;
	while ((delegate = [enumerator nextObject])) 
	{
		[delegate midiInput:self event:@"Added a destination"];
	}
}

- (void) disconnectDestination:(MIDIEndpointRef)destination
{
	NSEnumerator *enumerator = [delegates objectEnumerator];
	id<MidiInputDelegate> delegate;
	while ((delegate = [enumerator nextObject])) 
	{
		[delegate midiInput:self event:@"Removed a device"];
	}	
}

- (void) scanExistingDevices
{
    const ItemCount numberOfDestinations = MIDIGetNumberOfDestinations();
    const ItemCount numberOfSources      = MIDIGetNumberOfSources();

    for (ItemCount index = 0; index < numberOfDestinations; ++index)
        [self connectDestination:MIDIGetDestination(index)];
    for (ItemCount index = 0; index < numberOfSources; ++index)
        [self connectSource:MIDIGetSource(index)];

    numberOfConnectedDevices = numberOfSources;
}

//==============================================================================
#pragma mark Notifications

- (void) midiNotifyAdd:(const MIDIObjectAddRemoveNotification *)notification
{
    if (notification->childType == kMIDIObjectType_Destination)
        [self connectDestination:(MIDIEndpointRef)notification->child];
    else if (notification->childType == kMIDIObjectType_Source)
        [self connectSource:(MIDIEndpointRef)notification->child];
}

- (void) midiNotifyRemove:(const MIDIObjectAddRemoveNotification *)notification
{
    if (notification->childType == kMIDIObjectType_Destination)
        [self disconnectDestination:(MIDIEndpointRef)notification->child];
    else if (notification->childType == kMIDIObjectType_Source)
        [self disconnectSource:(MIDIEndpointRef)notification->child];
}

- (void) midiNotify:(const MIDINotification*)notification
{
    switch (notification->messageID)
    {
        case kMIDIMsgObjectAdded:
            [self midiNotifyAdd:(const MIDIObjectAddRemoveNotification *)notification];
            break;
        case kMIDIMsgObjectRemoved:
            [self midiNotifyRemove:(const MIDIObjectAddRemoveNotification *)notification];
            break;
        case kMIDIMsgSetupChanged:
        case kMIDIMsgPropertyChanged:
        case kMIDIMsgThruConnectionsChanged:
        case kMIDIMsgSerialPortOwnerChanged:
        case kMIDIMsgIOError:
            break;
    }
}

void MIDIInputNotifyProc(const MIDINotification *message, void *refCon)
{
    MidiInput *self = (MidiInput*)refCon;
    [self midiNotify:message];
}

//==============================================================================
#pragma mark MIDI Input

// NOTE: Called on a separate high-priority thread, not the main runloop
- (void) midiRead:(const MIDIPacketList *)pktlist midiSource:(void*)source
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		
	const MIDIPacket *packet = &pktlist->packet[0];
    for (int i = 0; i < pktlist->numPackets; ++i)
    {
//		UInt64 time = packet->timeStamp;
//		NSLog(@"timeStamp: %lld", time);
		
		ByteArray messages(packet->length, (unsigned char*)packet->data, false);
		
		NSEnumerator *enumerator = [delegates objectEnumerator];
		id<MidiInputDelegate> delegate;
		while ((delegate = [enumerator nextObject])) 
		{
			[delegate handleIncomingMidiMessage: messages midiSource: source];
		}			
		
		MidiInputSender::getInstance().sendIncomingMidiMessage(source, messages);
		
        packet = MIDIPacketNext(packet);
    }	
	
	[pool release];
}

void MIDIInputReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
{
    MidiInput *self = (MidiInput*)readProcRefCon;
    [self midiRead:pktlist midiSource:srcConnRefCon];
}

//==============================================================================
#pragma mark MIDI Output

- (void) sendMidi:(const UInt8*)data size:(UInt32)size
{
    NSLog(@"%s(%u bytes to core MIDI)", __func__, unsigned(size));
    assert(size < 65536);
    Byte packetBuffer[size+100];
    MIDIPacketList *packetList = (MIDIPacketList*)packetBuffer;
    MIDIPacket     *packet     = MIDIPacketListInit(packetList);

    packet = MIDIPacketListAdd(packetList, sizeof(packetBuffer), packet, 0, size, data);

    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); ++index)
    {
        MIDIEndpointRef outputEndpoint = MIDIGetDestination(index);
        if (outputEndpoint)
        {
            // Send it
            OSStatus s = MIDISend(outputPort, outputEndpoint, packetList);
            NSLogError(s, @"Sending MIDI");
        }
    }
}

@end

//==============================================================================

NSUInteger ListInterfaces(id<MidiInputDelegate> delegate)
{
//#define PGLog NSLog
#define PGLog(...) [delegate midiInput:nil event:[NSString stringWithFormat:__VA_ARGS__]]
    PGLog(@"%s: # external devices=%u", __func__, MIDIGetNumberOfExternalDevices());
    PGLog(@"%s: # devices=%u",          __func__, MIDIGetNumberOfDevices());
    PGLog(@"%s: # sources=%u",          __func__, MIDIGetNumberOfSources());
    PGLog(@"%s: # destinations=%u",     __func__, MIDIGetNumberOfDestinations());

    MIDIClientRef  client = 0;
    CFStringRef    clientName = (CFStringRef)@"MIDI Updater";
    MIDINotifyProc notifyProc = nil;
    OSStatus s = MIDIClientCreate(clientName, notifyProc, nil, &client);
    NSLogError(s, @"Creating MIDI client");

    for (ItemCount index = 0; index < MIDIGetNumberOfExternalDevices(); ++index)
    {
        PGLog(@"%s: External device %u", __func__, index);
        MIDIDeviceRef device = MIDIGetDevice(index);
        if (device)
        {
            //CFRelease(device);
        }
    }

    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); ++index)
    {
        MIDIEndpointRef endpoint = MIDIGetDestination(index);
        if (endpoint)
        {
            PGLog(@"%s: Destination index %u", __func__, index);
            CFStringRef name = nil;
            OSStatus s = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &name);
            if (s)
                NSLogError(s, @"Getting dest name");
            else
                PGLog(@"Name=%@", name);
            CFRelease(name);

            CFPropertyListRef properties = nil;
            s = MIDIObjectGetProperties(endpoint, &properties, true);
            if (s)
                NSLogError(s, @"Getting properties");
            else
                PGLog(@"Properties=%@", properties);
            CFRelease(properties); properties = nil;

            MIDIEntityRef entity = 0;
            s = MIDIEndpointGetEntity(endpoint, &entity);

            s = MIDIObjectGetProperties(entity, &properties, true);
            if (s)
                NSLogError(s, @"Getting entity properties");
            else
                PGLog(@"Entity properties=%@", properties);
            CFRelease(properties); properties = nil;

            SInt32 offline = 0;
            s = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyOffline, &offline);
            if (s)
                NSLogError(s, @"Getting offline properties");
            else
                PGLog(@"Entity offline=%d", offline);

            //CFRelease(entity); entity = nil;

            //CFRelease(endpoint);
            //PGLog(@"Done");
        }
    }

    if (client)
        MIDIClientDispose(client);

    PGLog(@"Found all interfaces");

    return MIDIGetNumberOfDestinations();
}

BEGIN_UGEN_NAMESPACE

MidiInputReceiver::MidiInputReceiver(const bool activate) throw()
{
	if(activate)
	{
		MidiInputSender& sender = MidiInputSender::getInstance();
		sender.addMidiReceiver(this);
	}
}

MidiInputReceiver::~MidiInputReceiver()
{
	MidiInputSender::getInstance().removeMidiReceiver(this);
}


MidiInputSender::MidiInputSender() throw()
{
	peer = [MidiInput sharedInstance];
}

MidiInputSender& MidiInputSender::getInstance()
{
	static MidiInputSender singleton;
	return singleton;
}

void MidiInputSender::addMidiReceiver (MidiInputReceiver* const receiver) throw()
{
	receivers.add(receiver);
}

void MidiInputSender::removeMidiReceiver (MidiInputReceiver* const receiver) throw()
{
	receivers.removeItem(receiver);
}

void MidiInputSender::sendIncomingMidiMessage(void* source, ByteArray const& message) throw()
{
	for(int i = 0; i < receivers.length(); i++)
	{
		receivers[i]->handleIncomingMidiMessage(source, message);
	}
}

MIDIControllerInternal::MIDIControllerInternal(const int midiChannel, const int controllerNumber,
											   const float minVal, const float maxVal,
											   const ExternalControlSource::Warp warp, void* port) throw()
:	ExternalControlSourceInternal(minVal, maxVal, warp),
	midiChannel_(midiChannel),
	controllerNumber_(controllerNumber),
	port_(port)
{
}

void MIDIControllerInternal::handleIncomingMidiMessage (void* source, ByteArray const& message) throw()
{	
	if(port_ != 0 && port_ != source) return;
	
	int head = 0;
	
	// no running status in iOS core midi?
	
	while(head < message.length())
	{
		unsigned char status = message[head++];
		
		bool isStatus = status & 0x80;
		
		int type = isStatus ? (status & 0xF0) : 0xF0; // default to system since this is partial sysex?		
		
		int value1, value2;
		
		switch(type)
		{
			// system
			case 0xF0: {
				
				while(head < message.length())
				{
					if(message[head] & 0xF0)
					{
						break;
					}
					else
					{
						head++;
					}
				}
				
				continue; // next message...
				
			} break;
				
			// program and channel pressure
			case 0xC0: case 0xD0: {
				value1 = message[head++];
				value2 = 0;
			} break;
				
			// note off, note on, aftertouch, controller, pitch wheel
			case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: {
				value1 = message[head++];
				value2 = message[head++];
			} break;
		}
		
		int channel = (status & 0x0F) + 1;
		
		if(channel != midiChannel_)		continue; // channel
		if(type != 0xB0)				continue; // control change
		if(value1 != controllerNumber_) continue; // controller number
					
		setNormalisedValue(value2 / 127.f);
	}
}

MIDIController::MIDIController(const int midiChannel, const int controllerNumber,
							   const float minVal, const float maxVal, 
							   const ExternalControlSource::Warp warp, 
							   void* port) throw()
{
	internal = new MIDIControllerInternal(midiChannel, controllerNumber, minVal, maxVal, warp, port);
}


MIDIMostRecentNoteInternal::MIDIMostRecentNoteInternal(const int midiChannel,
													   const float minVal, const float maxVal,
													   const ExternalControlSource::Warp warp, void* port) throw()
:	ExternalControlSourceInternal(minVal, maxVal, warp),
	midiChannel_(midiChannel),
	port_(port)
{
}

void MIDIMostRecentNoteInternal::handleIncomingMidiMessage (void* source, ByteArray const& message) throw()
{	
	if(port_ != 0 && port_ != source) return;
	
	int head = 0;
	
	// no running status in iOS core midi?
	
	while(head < message.length())
	{
		unsigned char status = message[head++];
		
		bool isStatus = status & 0x80;
		
		int type = isStatus ? (status & 0xF0) : 0xF0; // default to system since this is partial sysex?		
		
		int value1, value2;
		
		switch(type)
		{
				// system
			case 0xF0: {
				
				while(head < message.length())
				{
					if(message[head] & 0xF0)
					{
						break;
					}
					else
					{
						head++;
					}
				}
				
				continue; // next message...
				
			} break;
				
				// program and channel pressure
			case 0xC0: case 0xD0: {
				value1 = message[head++];
				value2 = 0;
			} break;
				
				// note off, note on, aftertouch, controller, pitch wheel
			case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: {
				value1 = message[head++];
				value2 = message[head++];
			} break;
		}
		
		int channel = (status & 0x0F) + 1;
		
		if(channel != midiChannel_)		continue; // channel
		if(type != 0x90)				continue; // note on
		if(value2 == 0)					continue; // not note off
		
		setNormalisedValue(value1 / 127.f);
	}	
}

MIDIMostRecentNote::MIDIMostRecentNote(const int midiChannel,
									   const float minVal, const float maxVal, 
									   const ExternalControlSource::Warp warp, 
									   void* port) throw()
{
	internal = new MIDIMostRecentNoteInternal(midiChannel, minVal, maxVal, warp, port);
}



VoicerUGenInternal::VoicerUGenInternal(const int numChannels, 
									   const int midiChannel, 
									   const int numVoices, 
									   const bool forcedSteal,
									   const bool direct) throw()
:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal),
	MidiInputReceiver(direct),
	midiChannel_(midiChannel),
	controllers(FloatArray::newClear(128)),
	keyPressure(FloatArray::newClear(128)),
	pitchWheel(0.f),
	channelPressure(0.f),
	program(0)
{
	controllers[7] = 1.f;
	controllers[10] = 0.5f;
	controllers[11] = 1.f;
	
	NSLock* nsLock = [[NSLock alloc] init];
	lockPeer = (void*)nsLock;
}

VoicerUGenInternal::~VoicerUGenInternal()
{
	NSLock* nsLock = (NSLock*)lockPeer;
	[nsLock release];
}

void VoicerUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	if(shouldStopAllEvents() == true) initEvents();
	
	if(midiMessages.length() > 0)
	{
		handleIncomingMidiMessage(0, midiMessages);
		
		lock();
		{
			midiMessages = ByteArray();
		}
		unlock();
	}
	
	lock();
	{
		SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
	}
	unlock();
}

void VoicerUGenInternal::handleIncomingMidiMessage (void* source, ByteArray const& message) throw()
{	
	static const float normalise127 = 1.f / 127.f;
	static const float normalise8191 = 1.f / 8191.f;
	
	int head = 0;
	
	// no running status in iOS core midi?
	
	while(head < message.length())
	{
		unsigned char status = message[head++];
		
		bool isStatus = status & 0x80;
		int type = isStatus ? (status & 0xF0) : 0xF0; // default to system since this is partial sysex?		
		int value1, value2;
		
		switch(type)
		{
			case 0xF0: { // system
				while(head < message.length())
				{
					if(message[head] & 0xF0)
					{
						break;
					}
					else
					{
						head++;
					}
				}
				
				continue; // next message...
			} break;
				// program and channel pressure
			case 0xC0: case 0xD0: {
				value1 = message[head++];
				value2 = 0;
			} break;
				// note off, note on, aftertouch, controller, pitch wheel
			case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: {
				value1 = message[head++];
				value2 = message[head++];
			} break;
		}
		
		int channel = (status & 0x0F) + 1;
		
		if(channel != midiChannel_)	continue; // channel		
		
		if((type == 0x80) || (type == 0x90))
		{
			int velocity = (type == 0x90) ? value2 : 0;
			 
			lock();
			{
				sendMidiNote(channel, value1, velocity);
			}
			unlock();
		}
		else if(type == 0xB0)
		{
			getController(value1) = ((float)value2 * normalise127);
		}
		else if(type == 0xC0)
		{
			getProgram() = (value1); // locks internally
		}
		else if(type == 0xD0)
		{
			getKeyPressure(value1) = ((float)value2 * normalise127);
		}
		else if(type == 0xE0)
		{
			int wheel = (value2 << 7) | value1;
			wheel -= 8192;
			getPitchWheel() = ((float)wheel * normalise8191);
		}		
	}	
}

void VoicerUGenInternal::sendMidiBuffer(ByteArray const& midiMessagesToAdd) throw()
{	
	if(midiMessagesToAdd.length() > 0)
	{			
		lock();
		{
			midiMessages.add(midiMessagesToAdd);
		}
		unlock();		
	}
}

const float& VoicerUGenInternal::getController(const int index) const throw()
{
	return controllers[index];
}

const float& VoicerUGenInternal::getKeyPressure(const int index) const throw()
{
	return keyPressure[index];
}

const float& VoicerUGenInternal::getPitchWheel() const throw()
{
	return pitchWheel;
}

const float& VoicerUGenInternal::getChannelPressure() const throw()
{
	return channelPressure;
}

const int& VoicerUGenInternal::getProgram() const throw()
{
	return program;
}

float& VoicerUGenInternal::getController(const int index) throw()
{
	return controllers[index];
}

float& VoicerUGenInternal::getKeyPressure(const int index) throw()
{
	return keyPressure[index];
}

float& VoicerUGenInternal::getPitchWheel() throw()
{
	return pitchWheel;
}

float& VoicerUGenInternal::getChannelPressure() throw()
{
	return channelPressure;
}

int& VoicerUGenInternal::getProgram() throw()
{
	return program;
}

void VoicerUGenInternal::lock() const throw()
{
	NSLock* nsLock = (NSLock*)lockPeer;
	[nsLock lock];
}

void VoicerUGenInternal::unlock() const throw()
{
	NSLock* nsLock = (NSLock*)lockPeer;
	[nsLock unlock];
}

bool VoicerUGenInternal::tryLock() const throw()
{
	NSLock* nsLock = (NSLock*)lockPeer;
	return [nsLock tryLock];
}


END_UGEN_NAMESPACE

#endif

