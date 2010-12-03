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

#ifndef _UGEN_ugen_JuceVoicer_H_
#define _UGEN_ugen_JuceVoicer_H_


#include "../core/ugen_UGen.h"
#include "io/ugen_JuceMIDIInputBroadcaster.h"
#include "../spawn/ugen_VoicerBase.h"
#include "../core/ugen_Arrays.h"


/** @ingroup UGenInternals */
class VoicerUGenInternal :  public VoicerBaseUGenInternal,
							public JuceMIDIInputListener
{
public:
	VoicerUGenInternal(const int numChannels, const int midiChannel, const int numVoices, const bool forcedSteal, const bool direct) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();	
	void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw();
	void sendMidiBuffer(MidiBuffer const& midiMessages) throw();
	
	void setController(const int index, const float value) throw();
	float getController(const int index) const throw();
	const float* getControllerPtr(const int index) const throw();
	inline UGen getControllerUGen(const int index) const throw() { return getControllerPtr(index); }
	
	void setKeyPressure(const int index, const float value) throw();
	float getKeyPressure(const int index) const throw();
	const float* getKeyPressurePtr(const int index) const throw();
	inline UGen getKeyPressureUGen(const int index) const throw() { return getKeyPressurePtr(index); }
	
	void setPitchWheel(const float value) throw();
	float getPitchWheel() const throw();
	const float* getPitchWheelPtr() const throw();
	inline UGen getPitchWheelUGen() const throw() { return getPitchWheelPtr(); }
	
	void setChannelPressure(const float value) throw();
	float getChannelPressure() const throw();
	const float* getChannelPressurePtr() const throw();	
	inline UGen getChannelPressureUGen() const throw() { return getChannelPressurePtr(); }
	
	void setProgram(const int value) throw();
	int getProgram() const throw();
	const int* getProgramPtr() const throw();	
	inline UGen getProgramUGen() const throw() { return getProgramPtr();  }
	
private:
	CriticalSection lock;
	const int midiChannel_;
	MidiBuffer midiMessages;
	
	FloatArray controllers;
	FloatArray keyPressure;
	float pitchWheel, channelPressure;	
	int program;
};


EVENT_DOCS_OWNED(Voicer, VoicerEvent)
template<class OwnerType = void>
class VoicerEvent : public EventBase<OwnerType>
{	
public:	
	EVENT_OWNEDCONSTRUCTOR(VoicerEvent)
	VoicerEvent(OwnerType* o) : EventBase<OwnerType> (o) { }
	
	PREDOC(VoicerEvent_spawnEvent_Docs)
	virtual UGen spawnEvent(VoicerUGenInternal& spawn, 
							const int eventCount,
							const int midiChannel,
							const int midiNote,
							const int velocity)	= 0;
};

EVENT_DOCS_VOID(Voicer, VoicerEvent)
template<>
class VoicerEvent<void> : public EventBase<void>
{	
public:		
	PREDOC(VoicerEvent_spawnEvent_Docs)
	virtual UGen spawnEvent(VoicerUGenInternal& spawn, 
							const int eventCount,
							const int midiChannel,
							const int midiNote,
							const int velocity)	= 0;
};

typedef VoicerEvent<> VoicerEventDefault;

/** @ingroup UGenInternals */
template <class VoicerEventType, class OwnerType>
class VoicerEventUGenInternal : public VoicerUGenInternal
{
public:
	VoicerEventUGenInternal(const int numChannels,
							const int midiChannel,
							OwnerType* owner,
							const int numVoices,
							const bool forcedSteal,
							const bool direct) throw()
	:	VoicerUGenInternal(numChannels, midiChannel, numVoices, forcedSteal, direct), 
	event(owner)
	{ 
	}
	
	VoicerEventUGenInternal(const int numChannels,
							const int midiChannel,
							VoicerEventType const& e,
							const int numVoices,
							const bool forcedSteal,
							const bool direct) throw()
	:	VoicerUGenInternal(numChannels, midiChannel, numVoices, forcedSteal, direct), 
	event(e)
	{ 
	}
	
	UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity)
	{
		return event.spawnEvent(*this, eventCount, midiChannel, midiNote, velocity);
	}
	
protected:
	VoicerEventType event;
};


/** @ingroup UGenInternals */
template <class VoicerEventType>
class VoicerEventUGenInternal<VoicerEventType, void> : public VoicerUGenInternal
{
public:
	VoicerEventUGenInternal(const int numChannels,
							const int midiChannel,
							void* owner,
							const int numVoices,
							const bool forcedSteal,
							const bool direct) throw()
	:	VoicerUGenInternal(numChannels, midiChannel, numVoices, forcedSteal, direct)
	{ 
	}
	
	VoicerEventUGenInternal(const int numChannels,
							const int midiChannel,
							VoicerEventType const& e,
							const int numVoices,
							const bool forcedSteal,
							const bool direct) throw()
	:	VoicerUGenInternal(numChannels, midiChannel, numVoices, forcedSteal, direct), 
	event(e)
	{ 
	}
	
	UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity)
	{
		return event.spawnEvent(*this, eventCount, midiChannel, midiNote, velocity);
	}
	
protected:
	VoicerEventType event;
};


/** Voice events from MIDI events.
 
 This uses Juce or iOS to actually listen to MIDI inputs and generate events based on MIDI note data.
 
 @ingroup AllUGens EventUGens
 @see VoicerEventBase<OwnerType>, VoicerBase */
template <class VoicerEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(Voicer))
class Voicer : public UGen																													
{																																				
public:																																			
	Voicer (Voicer_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);
		generateFromProxyOwner(new VoicerEventUGenInternal<VoicerEventType, OwnerType>
							   (numChannels, midiChannel, o, numVoices, forcedSteal, direct));
	}
	
	Voicer (Voicer_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);
		generateFromProxyOwner(new VoicerEventUGenInternal<VoicerEventType, OwnerType> 
							   (numChannels, midiChannel, e, numVoices, forcedSteal, direct));
	}
	
	EventUGenMethodsDeclare(Voicer, 
							VoicerEventType, 
							OwnerType, 
							(Voicer_OwnerInputsNoTypes), 
							(Voicer_OwnerInputsWithTypesAndDefaults), 
							(Voicer_EventInputsNoTypes), 
							(Voicer_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS Voicer_Docs);																		
};



#endif // _UGEN_ugen_JuceVoicer_H_
