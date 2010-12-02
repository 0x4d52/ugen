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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && defined(UGEN_IOS_COREMIDI)

BEGIN_UGEN_NAMESPACE

#include "ugen_iOSMidiInput.h"

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
		ByteArray messages(packet->length, (unsigned char*)packet->data, false);
		
		NSEnumerator *enumerator = [delegates objectEnumerator];
		id<MidiInputDelegate> delegate;
		while ((delegate = [enumerator nextObject])) 
		{
			[delegate handleIncomingMidiMessage: messages midiSource: source];
		}			
		
		MIDIInputSender::getInstance().sendIncomingMidiMessage(source, messages);
		
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

MIDIInputSender::MIDIInputSender() throw()
{
	peer = [MidiInput sharedInstance];
}

MIDIInputSender& MIDIInputSender::getInstance()
{
	static MIDIInputSender singleton;
	return singleton;
}

void MIDIInputSender::addMidiReceiver (MIDIInputReceiver* const receiver) throw()
{
	receivers.add(receiver);
}

void MIDIInputSender::removeMidiReceiver (MIDIInputReceiver* const receiver) throw()
{
	receivers.removeItem(receiver);
}

void MIDIInputSender::sendIncomingMidiMessage(void* source, ByteArray const& message) throw()
{
	for(int i = 0; i < receivers.length(); i++)
	{
		receivers[i]->handleIncomingMidiMessage(source, message);
	}
}


END_UGEN_NAMESPACE

#endif

