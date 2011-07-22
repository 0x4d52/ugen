// $Id$
// $HeadURL$

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
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_Spawn_H_
#define _UGEN_ugen_Spawn_H_

#include "../core/ugen_UGen.h"
#include "../core/ugen_UGenArray.h"

#define _FILEID_ _UGEN_ugen_Spawn_H_

/** @ingroup UGenInternals */
class SpawnBaseUGenInternal : public ProxyOwnerUGenInternal
{
public:
	SpawnBaseUGenInternal(const int numInputs, const int numChannels, const int maxRepeats) throw();
	~SpawnBaseUGenInternal();// throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void releaseInternal() throw();
	void stealInternal() throw();
	void initEvents() throw();
	bool stopAllEvents() throw();
	bool shouldStopAllEvents() { return stopEvents; }
	
	inline UGenArray& getEvents() { return events; }
		
protected:	
	const int numChannels;
	UGenArray events;
	UGen mixer;
	int currentEventIndex;
	const int maxRepeats_;
	float** const bufferData;
	
	inline void accumulateSamples(float *outputSamples, const float *inputSamples, int numSamplesToProcess) throw()
	{
		while(numSamplesToProcess--)
		{
			*outputSamples++ += *inputSamples++;
		}
	}
	
	inline bool reachedMaxRepeats() throw()
	{
		//return maxRepeats_ > 0 && currentEventIndex >= (maxRepeats_ - 1);
		return maxRepeats_ > 0 && currentEventIndex >= maxRepeats_; // testing this....?
	}
	
private:
	bool stopEvents;
};

/** @ingroup UGenInternals */
class SpawnUGenInternal : public SpawnBaseUGenInternal
{
public:
	SpawnUGenInternal(const int numChannels, const double nextTime, const int maxRepeats) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	virtual UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) = 0;
	
	double nextTime;
	
protected:	
	unsigned int nextTimeSamples;	
};





EVENT_DOCS_OWNED(Spawn, SpawnEventBase)
template<class OwnerType = void> POSTDOC(EVENT_OWNER_DOC)
class SpawnEventBase : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(SpawnEventBase)
	SpawnEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) = 0;
};

EVENT_DOCS_VOID(Spawn, SpawnEventBase)
template<>
class SpawnEventBase<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) = 0;
};

typedef SpawnEventBase<> SpawnEventDefault;


/** @ingroup UGenInternals */
template <class SpawnEventType, class OwnerType>
class SpawnEventUGenInternal : public SpawnUGenInternal
{
public:
	SpawnEventUGenInternal(const int numChannels, const double nextTime, const int maxRepeats, OwnerType* o) throw()
	:	SpawnUGenInternal(numChannels, nextTime, maxRepeats), event_(o) { }
	SpawnEventUGenInternal(const int numChannels, const double nextTime, const int maxRepeats, SpawnEventType const& e) throw()
	:	SpawnUGenInternal(numChannels, nextTime, maxRepeats), event_(e) { }

	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) throw()
	{
		return event_.spawnEvent(spawn, eventCount);
	}

protected:
	SpawnEventType event_;
};

/** @ingroup UGenInternals */
template <class SpawnEventType>
class SpawnEventUGenInternal<SpawnEventType, void> : public SpawnUGenInternal
{
public:
	SpawnEventUGenInternal(const int numChannels, const double nextTime, const int maxRepeats, void* o) throw()
	:	SpawnUGenInternal(numChannels, nextTime, maxRepeats) { }
	SpawnEventUGenInternal(const int numChannels, const double nextTime, const int maxRepeats, SpawnEventType const& e) throw()
	:	SpawnUGenInternal(numChannels, nextTime, maxRepeats), event_(e) { }
	
	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) throw()
	{
		return event_.spawnEvent(spawn, eventCount);
	}
	
protected:
	SpawnEventType event_;
};




#define Spawn_OwnerInputsWithTypesAndDefaults	const int numChannels = 1, const double nextTime = 1.0, const int maxRepeats = 0, OwnerType* o = 0
#define Spawn_OwnerInputsNoTypes				numChannels, nextTime, maxRepeats, o
#define Spawn_EventInputsWithTypesAndDefaults	SpawnEventType const& e, const int numChannels = 1, const double nextTime = 1.0, const int maxRepeats = 0
#define Spawn_EventInputsNoTypes				e, numChannels, nextTime, maxRepeats

#define Spawn_Docs				EVENT_TEMPLATE_DOC(Spawn)																		\
								EVENT_UGEN_OWNEREVENT_DOC																		\
								EVENT_UGEN_NUMCHANNELS_DOC(Spawn)																\
								@param	nextTime		The time between events in seconds.										\
								EVENT_UGEN_MAXREPEATS_DOC(Spawn)


/** Spawn creates new events at timed intervals in a sample-accurate fashion.				
 You must supply a subclass of SpawnEventBase as the first template parameter			
 you may optionally provide an OwnerType as the second template parameter. If			
 provided this MUST be the same as the OwnerType of the SpawnEventBase subclass.			
 Providing an owner allows you to use another class as an owner or "controller"			
 of the class, allowing access to its members (e.g., for slider or parameter				
 control). The SpawnEventBase will be used to create event instances regularly 
 according to the nextTime argument. Here the pure virtual function								
 SpawnEventBase<OwnerType>::spawnEvent() must be implmenented which returns a			
 UGen with the desired UGen graph for the event instance. 
 
 To declare a SpawnEventBase subclass do something like:
 @code
 class MySpawnEvent : public SpawnEventBase < >
 {
 public:
	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount)	 
	{ 
		return SinOsc::AR(eventCount % 100 * 50, 0, Linen::AR(0.1, 0.5, 0.1, 0.2, UGen::DeleteWhenDone));
	}
 };
 @endcode
 
 implementing a 
 
 @code UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) @endcode 
 function, then use it by doing something like this to create the UGen graph:
 
 @code synth = Spawn<MySpawnEvent>::AR(1, 0.5); @endcode
 
 This would create a single channel Spawn object and generate an event instance from the 
 event class using spawnEvent() every 0.5s.
 
 The UGen graph returned by your spawnEvent() <b>MUST</b> include a UGen that will execute a 
 UGen::DeleteWhenDone DoneAction at some specified time in the future. Examples of UGen classes which
 can execute a DoneAction are Linen, LLine, XLine, EnvGen (using an Env). If this is not done 
 then CPU usage will increase the longer the program runs, and probably crash at some point!
 
 If an owner is provided then the event may access the owner during the creation of new events. e.g.,
 
 @code
 class MySpawnEvent : public SpawnEventBase < MyObject >
 {
 public:
	MySpawnEvent(MyObject* o) : SpawnEventBase < MyObject > (o) { }
 
	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount)	 
	{ 
		float freq = getOwner()->getSomeValueFromTheOwner();
		return SinOsc::AR(freq, Linen::AR(0.1, 0.5, 0.1, 0.2, UGen::DeleteWhenDone));
	}
 };
 @endcode
 
 This assumes getSomeValueFromTheOwner() returns a value in some useful range usable 
 as the frequency of an audio oscillator.
 
 Notice the requirement to provide a constructor with an appropriate argument to pass
 the owner to the base class in this case. Then inside the owner of type MyObject
 @code
 ...
 synth = Spawn<MySpawnEvent,MyObject>::AR(1, 0.5, 0, this);
 @endcode
 
 Within the SpawnEventBase<OwnerType>::spawnEvent() call the event may modify 
 the Spawn nextTime parameter by using e.g.,														
 @code
 ...
 UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount)
 {
	...
	spawn.nextTime = 0.5; // change nextTime to 0.5s
	...
 }
 ...
 @endcode	
 @ingroup AllUGens EventUGens
 @see SpawnEventBase<OwnerType>, TSpawn, XFadeTexture, OverlapTexture, TrigXFade
 */
template <class SpawnEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(Spawn))
class Spawn : public UGen																													
{																																				
public:
	EVENT_CONSTRUCTOR_DOC(Spawn, Spawn_Docs)
	Spawn (Spawn_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);			
		generateFromProxyOwner(new SpawnEventUGenInternal<SpawnEventType, OwnerType> 
								(numChannels, nextTime, maxRepeats, o));										
	}
	
	EVENT_CONSTRUCTOR_DOC(Spawn, Spawn_Docs)
	Spawn (Spawn_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);			
		generateFromProxyOwner(new SpawnEventUGenInternal<SpawnEventType, OwnerType> 
							   (numChannels, nextTime, maxRepeats, e));										
	}
	
	EventUGenMethodsDeclare(Spawn, 
							SpawnEventType, 
							OwnerType, 
							(Spawn_OwnerInputsNoTypes), 
							(Spawn_OwnerInputsWithTypesAndDefaults), 
							(Spawn_EventInputsNoTypes), 
							(Spawn_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS Spawn_Docs);
};



#undef _FILEID_
#endif // _UGEN_ugen_Spawn_H_
