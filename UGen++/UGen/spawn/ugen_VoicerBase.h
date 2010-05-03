// $Id: ugen_VoicerBase.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/spawn/ugen_VoicerBase.h $

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

#ifndef _UGEN_ugen_VoiceBase_H_
#define _UGEN_ugen_VoiceBase_H_

#include "../core/ugen_UGen.h"
#include "ugen_Spawn.h"

#define _FILEID_ _UGEN_ugen_VoiceBase_H_



typedef struct VoicerUserDataStruct
{
	char midiChannel;
	char midiNote;
	char unused1;
	char unused2;
} VoicerUserDataStruct;

typedef union VoicerUserDataUnion
{
	VoicerUserDataStruct s;
	int i;
} VoicerUserDataUnion;

/** @ingroup UGenInternals */
class VoicerBaseUGenInternal : public SpawnBaseUGenInternal
{
public:
	VoicerBaseUGenInternal(const int numChannels, const int numVoices, const bool forcedSteal) throw();

	/** Send a MIDI note message to the voicer.
	 This should spawn a new voice if it is a note on (velocity is not zero), or release a voice if
	 it is a note off.
	 @param midiChannel
	 @param midiNote
	 @param velocity
	 */
	void sendMidiNote(const int midiChannel, 
					  const int midiNote,
					  const int velocity) throw();

	virtual UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
							const int eventCount, 
							const int midiChannel, 
							const int midiNote,
							const int velocity) = 0;
	
	/** Steal one or more notes with a particular MIDI note and/or MIDI channel.
	 
	 @param midiChannel		The MIDI channel on which the MIDI note should be to be stolen.
	 @param midiNote		The MIDI note number that should be stolen.
	 @param forcedSteal		If @c true this will stop the note immediately, if @c false 
							the voice will be allowed to fade out quickly.
	 @param stealAll		If @c true all notes of this note number and channel will
							be stolen, if @c false only the first to be found will be
							stolen.
	 @return				@c True if a voice was actually stolen or @c false if no
							matching voice was found (nothing was stolen).
	 @see UGen::steal() */
	bool stealNote(const int midiChannel, 
				   const int midiNote, 
				   const bool forcedSteal = false, 
				   const bool stealAll = true) throw();
	
	enum StealMode
	{
		StealOldest,
		StealNewest,
		StealHighest,
		StealLowest,
		NumStealModes
	};
	
	static int createUserData(const int midiChannel, const int midiNote) throw();
	static void parseUserData(const int userData, int& midiChannel, int& midiNote) throw();
	static VoicerUserDataStruct getUserData(const int userData) throw();	
	
protected:	
	int numVoices_;
	int stealMode_;
	const bool forcedSteal_;
	int ageCounter;
	
	static const int stealingUserData;
	
	int countNonstealingVoices() const throw();
	UGen& chooseStealee() throw();
	UGen& chooseReleasee(const int midiChannel, const int midiNote) throw();
};


#define VoicerEventBase_spawnEvent_Docs		EVENT_SPAWNFUNCTION_DOCS															\
											@param midiChannel	The MIDI channel of the calling MIDI note on event.				\
											@param midiNote		The MIDI note number of the calling MIDI note on event.			\
											@param velocity		The velocity of the calling MIDI note event. This should		\
																in the range 1-127 since velocity values of 0 should have		\
																been filtered out to turn already sounding notes off (if		\
																the note number matched the note number of an already			\
																sounding note).

EVENT_DOCS_OWNED(VoicerBase, VoicerEventBase)
template<class OwnerType = void>
class VoicerEventBase : public EventBase<OwnerType>
{	
public:	
	EVENT_OWNEDCONSTRUCTOR(VoicerEventBase)
	VoicerEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	
	PREDOC(VoicerEventBase_spawnEvent_Docs)
	virtual UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
							const int eventCount,
							const int midiChannel,
							const int midiNote,
							const int velocity)	= 0;
};

EVENT_DOCS_VOID(VoicerBase, VoicerEventBase)
template<>
class VoicerEventBase<void> : public EventBase<void>
{	
public:		
	PREDOC(VoicerEventBase_spawnEvent_Docs)
	virtual UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
							const int eventCount,
							const int midiChannel,
							const int midiNote,
							const int velocity)	= 0;
};

typedef VoicerEventBase<> VoicerEventBaseDefault;

/** @ingroup UGenInternals */
template <class VoicerEventType, class OwnerType = void>
class VoicerBaseEventUGenInternal : public VoicerBaseUGenInternal
{
public:
	VoicerBaseEventUGenInternal(const int numChannels,
								OwnerType* o,
								const int numVoices,
								const bool forcedSteal) throw()
	:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal), 
		event_(o)
	{ 
	}

	VoicerBaseEventUGenInternal(const int numChannels,
								VoicerEventType const& e,
								const int numVoices,
								const bool forcedSteal) throw()
	:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal), 
		event_(e)
	{ 
	}

	UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity)
	{
		return event_.spawnEvent(*this, eventCount, midiChannel, midiNote, velocity);
	}


protected:
	VoicerEventType event_;
};

/** @ingroup UGenInternals */
template <class VoicerEventType>
class VoicerBaseEventUGenInternal<VoicerEventType, void> : public VoicerBaseUGenInternal
{
public:
	VoicerBaseEventUGenInternal(const int numChannels,
								void* o,
								const int numVoices,
								const bool forcedSteal) throw()
	:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal)
	{ 
	}

	VoicerBaseEventUGenInternal(const int numChannels,
								VoicerEventType const& e,
								const int numVoices,
								const bool forcedSteal) throw()
	:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal), 
		event_(e)
	{ 
	}
	
	UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity)
	{
		return event_.spawnEvent(*this, eventCount, midiChannel, midiNote, velocity);
	}


protected:
	VoicerEventType event_;
};

#define VOICERBASE_VOICE_DOCS	@param	numVoices		The maximum number of voices before voices are stolen.					\
														Set this to 0 or less for no limit.										\
								@param	forcedSteal		If false, voices will be stolen with a very rapid fade. If true			\
														voices are stolen immediately (and thus may introduce a noticeable		\
														click).

#define VoicerBase_OwnerInputsWithTypesAndDefaults	const int numChannels = 1,				\
													OwnerType* o = 0,						\
													const int numVoices = 0,				\
													const bool forcedSteal = false
#define VoicerBase_OwnerInputsNoTypes				numChannels, o, numVoices, forcedSteal
#define VoicerBase_EventInputsWithTypesAndDefaults	VoicerEventType const& e,				\
													const int numChannels = 1,				\
													const int numVoices = 0,				\
													const bool forcedSteal = false			
#define VoicerBase_EventInputsNoTypes				e, numChannels, numVoices, forcedSteal

#define VoicerBase_Docs					EVENT_TEMPLATE_DOC(VoicerBase)																	\
										EVENT_UGEN_OWNEREVENT_DOC																		\
										EVENT_UGEN_NUMCHANNELS_DOC(VoicerBase)															\
										VOICERBASE_VOICE_DOCS


/** Voice events from MIDI events.
 
 @ingroup AllUGens EventUGens
 @see VoicerEventBase<OwnerType>, Voicer */
template <class VoicerEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(VoicerBase))
class VoicerBase : public UGen																													
{																																				
public:																																			
	VoicerBase (VoicerBase_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);
		generateFromProxyOwner(new VoicerBaseEventUGenInternal<VoicerEventType, OwnerType> 
									(numChannels, o, numVoices, forcedSteal));										
	}	

	VoicerBase (VoicerBase_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);		
		generateFromProxyOwner(new VoicerBaseEventUGenInternal<VoicerEventType, OwnerType> 
									(numChannels, e, numVoices, forcedSteal));										
	}	

	EventUGenMethodsDeclare(VoicerBase, 
							VoicerEventType, 
							OwnerType, 
							(VoicerBase_OwnerInputsNoTypes), 
							(VoicerBase_OwnerInputsWithTypesAndDefaults), 
							(VoicerBase_EventInputsNoTypes), 
							(VoicerBase_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS VoicerBase_Docs);
};

#undef _FILEID_
#endif // _UGEN_ugen_VoiceBase_H_
