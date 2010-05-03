// $Id: ugen_TSpawn.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/spawn/ugen_TSpawn.h $

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

#ifndef _UGEN_ugen_TSpawn_H_
#define _UGEN_ugen_TSpawn_H_

#include "../core/ugen_UGen.h"
#include "ugen_Spawn.h"

#define _FILEID_ _UGEN_ugen_TSpawn_H_


/** @ingroup UGenInternals */
class TSpawnUGenInternal : public SpawnBaseUGenInternal
{
public:
	TSpawnUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	virtual UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs = 0) = 0;
	void trigger(void* extraArgs = 0) throw();
	
	inline UGen& getTrigger() throw() { return inputs[Trig]; }
	
	enum Inputs { Trig, NumInputs };
	
protected:
	float lastTrig;
	
};

#define ExtraArgs_Doc	@param extraArgs User defined args can be passed via the trigger() message.

EVENT_DOCS_OWNED(TSpawn, TSpawnEventBase)
template<class OwnerType = void>
class TSpawnEventBase : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(TSpawnEventBase)
	TSpawnEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_SPAWNFUNCTION_DOCS ExtraArgs_Doc)
	virtual UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs) = 0;
};

EVENT_DOCS_VOID(TSpawn, TSpawnEventBase)
template<>
class TSpawnEventBase<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_SPAWNFUNCTION_DOCS ExtraArgs_Doc)
	virtual UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs) = 0;
};

typedef TSpawnEventBase<> TSpawnEventDefault;

/** @ingroup UGenInternals */
template <class TSpawnEventType, class OwnerType>
class TSpawnEventUGenInternal : public TSpawnUGenInternal
{
public:
	TSpawnEventUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats, OwnerType* o) throw()
	:	TSpawnUGenInternal(numChannels, trig, maxRepeats), event_(o) { }
	TSpawnEventUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats, TSpawnEventType const& e) throw()
	:	TSpawnUGenInternal(numChannels, trig, maxRepeats), event_(e) { }

	UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs = 0) throw()
	{
		return event_.spawnEvent(spawn, eventCount, extraArgs);
	}

protected:
	TSpawnEventType event_;
};

/** @ingroup UGenInternals */
template <class TSpawnEventType>
class TSpawnEventUGenInternal<TSpawnEventType, void> : public TSpawnUGenInternal
{
public:
	TSpawnEventUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats, void* o) throw()
	:	TSpawnUGenInternal(numChannels, trig, maxRepeats) { }
	TSpawnEventUGenInternal(const int numChannels, UGen const& trig, const int maxRepeats, TSpawnEventType const& e) throw()
	:	TSpawnUGenInternal(numChannels, trig, maxRepeats), event_(e) { }
	
	UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs = 0) throw()
	{
		return event_.spawnEvent(spawn, eventCount, extraArgs);
	}
	
protected:
	TSpawnEventType event_;
};

#define TRIG_TRIG_DOC @param trig			The trigger used to generate events, a trigger occurs when the trig				\
											contains a transition from 0 or less, to greater than 0. This should			\
											be a single channel UGen, if it is multichannel it will be mixed				\
											before being used.																\

#define TSpawn_OwnerInputsWithTypesAndDefaults	const int numChannels = 1, UGen const& trig = 0.f, const int maxRepeats = 0, OwnerType* o = 0
#define TSpawn_OwnerInputsNoTypes				numChannels, trig, maxRepeats, o
#define TSpawn_EventInputsWithTypesAndDefaults	TSpawnEventType const& e, const int numChannels = 1, UGen const& trig = 0.f, const int maxRepeats = 0
#define TSpawn_EventInputsNoTypes				e, numChannels, trig, maxRepeats

#define TSpawn_Docs		EVENT_TEMPLATE_DOC(TSpawn)														\
						EVENT_UGEN_OWNEREVENT_DOC														\
						EVENT_UGEN_NUMCHANNELS_DOC(TSpawn)												\
						TRIG_TRIG_DOC																	\
						EVENT_UGEN_MAXREPEATS_DOC(TSpawn)

/** Create events using a trigger.
 
 @ingroup AllUGens EventUGens
 @see TSpawnEventBase<OwnerType>, Spawn, TrigXFade */
template <class TSpawnEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(TSpawn))
class TSpawn : public UGen																													
{																																				
public:			
	EVENT_CONSTRUCTOR_DOC(TSpawn, TSpawn_Docs)
	TSpawn (TSpawn_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);		
		generateFromProxyOwner(new TSpawnEventUGenInternal<TSpawnEventType, OwnerType> 
									(numChannels, trig.mix(), maxRepeats, o));										
	}
	
	EVENT_CONSTRUCTOR_DOC(TSpawn, TSpawn_Docs)
	TSpawn (TSpawn_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);	
		generateFromProxyOwner(new TSpawnEventUGenInternal<TSpawnEventType, OwnerType> 
									(numChannels, trig.mix(), maxRepeats, e));										
	}

	EventUGenMethodsDeclare(TSpawn, 
							TSpawnEventType, 
							OwnerType, 
							(TSpawn_OwnerInputsNoTypes), 
							(TSpawn_OwnerInputsWithTypesAndDefaults), 
							(TSpawn_EventInputsNoTypes), 
							(TSpawn_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS TSpawn_Docs);
};


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define TSpawn TSpawn()
#endif

#undef _FILEID_
#endif // _UGEN_ugen_TSpawn_H_
