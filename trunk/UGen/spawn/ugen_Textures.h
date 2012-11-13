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

#ifndef _UGEN_ugen_Textures_H_
#define _UGEN_ugen_Textures_H_

#ifndef UGEN_NOEXTGPL

#include "../core/ugen_UGen.h"
#include "../basics/ugen_BinaryOpUGens.h"
#include "../basics/ugen_UnaryOpUGens.h"
#include "../basics/ugen_MixUGen.h"
#include "../envelopes/ugen_EnvGen.h"
#include "ugen_Spawn.h"
#include "ugen_TSpawn.h"
#include "ugen_VoicerBase.h"

/** @ingroup UGenInternals */
class XFadeTextureUGenInternal : public SpawnUGenInternal
{
public:
	XFadeTextureUGenInternal(const float sustainTime_,
							 const float transitionTime_,
							 const int numChannels, 
							 const int maxRepeats) throw()
	:	SpawnUGenInternal(numChannels, transitionTime_ + sustainTime_, maxRepeats),
		sustainTime(sustainTime_),
		transitionTime(transitionTime_)
	{
		ugen_assert(sustainTime >= 0.f);
		ugen_assert(transitionTime >= 0.f);
		ugen_assert((sustainTime+transitionTime) > 0.f);
	}
	
	float sustainTime;
	float transitionTime;
	
protected:
};

EVENT_DOCS_OWNED(XFadeTexture, XFadeTextureEventBase)
template<class OwnerType = void>
class XFadeTextureEventBase : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(XFadeTextureEventBase)
	XFadeTextureEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(XFadeTextureUGenInternal& spawn, const int eventCount) = 0;
};

EVENT_DOCS_VOID(XFadeTexture, XFadeTextureEventBase)
template<>
class XFadeTextureEventBase<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(XFadeTextureUGenInternal& spawn, const int eventCount) = 0;
};

typedef XFadeTextureEventBase<> XFadeTextureEventDefault;

/** @ingroup UGenInternals */
template <class XFadeTextureEventType, class OwnerType>
class XFadeTextureEventUGenInternal : public XFadeTextureUGenInternal
{
public:
	XFadeTextureEventUGenInternal(const float sustainTime,
								  const float transitionTime,
								  const int numChannels, 
								  const int maxRepeats,
								  OwnerType* o) throw()
	:	XFadeTextureUGenInternal(sustainTime, transitionTime, numChannels, maxRepeats), 
		event_(o)
	{ 
	}
	
	XFadeTextureEventUGenInternal(const float sustainTime,
								  const float transitionTime,
								  const int numChannels, 
								  const int maxRepeats,
								  XFadeTextureEventType const& e) throw()
	:	XFadeTextureUGenInternal(sustainTime, transitionTime, numChannels, maxRepeats), 
		event_(e)
	{ 
	}

	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) throw()
	{
		UGen event = event_.spawnEvent(*this, eventCount);
		spawn.nextTime = transitionTime + sustainTime;
		Env env(Buffer(0.0, 1.0, 1.0, 0.0), 
				Buffer(transitionTime, sustainTime, transitionTime), 
				EnvCurve::Welch);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone);
	}

protected:
	XFadeTextureEventType event_;
};

/** @ingroup UGenInternals */
template <class XFadeTextureEventType>
class XFadeTextureEventUGenInternal<XFadeTextureEventType, void> : public XFadeTextureUGenInternal
{
public:
	XFadeTextureEventUGenInternal(const float sustainTime,
								  const float transitionTime,
								  const int numChannels, 
								  const int maxRepeats,
								  void* o) throw()
	:	XFadeTextureUGenInternal(sustainTime, transitionTime, numChannels, maxRepeats)
	{ 
	}
	
	XFadeTextureEventUGenInternal(const float sustainTime,
								  const float transitionTime,
								  const int numChannels, 
								  const int maxRepeats,
								  XFadeTextureEventType const& e) throw()
	:	XFadeTextureUGenInternal(sustainTime, transitionTime, numChannels, maxRepeats), 
		event_(e)
	{ 
	}

	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount)
	{
		UGen event = event_.spawnEvent(*this, eventCount);
		spawn.nextTime = transitionTime + sustainTime;
		Env env(Buffer(0.0, 1.0, 1.0, 0.0), 
				Buffer(transitionTime, sustainTime, transitionTime), 
				EnvCurve::Welch);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone);
	}

protected:
	XFadeTextureEventType event_;
};

#define TEXTURE_TRANSITIONTIME_DOC	@param transitionTime	The transition time (in seconds)  of the envelope.			\
															The envelope transition is a Welch envelope segment			\
															giving it a -3dB midpoint.									\


#define XFadeTexture_OwnerInputsWithTypesAndDefaults	const float sustainTime = 4.f,			\
														const float transitionTime = 4.f,		\
														const int numChannels = 1,				\
														const int maxRepeats = 0,				\
														OwnerType* o = 0
#define XFadeTexture_OwnerInputsNoTypes					sustainTime, transitionTime, numChannels, maxRepeats, o
#define XFadeTexture_EventInputsWithTypesAndDefaults	XFadeTextureEventType const& e,			\
														const float sustainTime = 4.f,			\
														const float transitionTime = 4.f,		\
														const int numChannels = 1,				\
														const int maxRepeats = 0
#define XFadeTexture_EventInputsNoTypes					e, sustainTime, transitionTime, numChannels, maxRepeats

#define XFadeTexture_Docs	EVENT_TEMPLATE_DOC(XFadeTexture)													\
							EVENT_UGEN_OWNEREVENT_DOC															\
							@param sustainTime		The sustain time (in seconds) of the envelope.				\
							TEXTURE_TRANSITIONTIME_DOC															\
							EVENT_UGEN_NUMCHANNELS_DOC(XFadeTexture)											\
							EVENT_UGEN_MAXREPEATS_DOC(XFadeTexture)												\

/** Crossfade events at timed intevals.
 
 Creates a series of cross faded sounds from a user event class. The user event class
 should return a graph of unit generators that produce a continuous sound. XFadeTexture 
 will apply an envelope to the sound to cross fade different invocations of the event.
 
 This is different from Spawn which requires the envelope to be specified by the user.
 
 @ingroup AllUGens EventUGens
 @see XFadeTextureEventBase<OwnerType>, Spawn, OverlapTexture, TrigXFade */
template <class XFadeTextureEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(XFadeTexture))
class XFadeTexture : public UGen																													
{																																				
public:																																										EVENT_CONSTRUCTOR_DOC(XFadeTexture, XFadeTexture_Docs)		
	XFadeTexture (XFadeTexture_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);
		generateFromProxyOwner(new XFadeTextureEventUGenInternal<XFadeTextureEventType, OwnerType> 
									(sustainTime, transitionTime, numChannels, maxRepeats, o));										
	}
	
	EVENT_CONSTRUCTOR_DOC(XFadeTexture, XFadeTexture_Docs)
	XFadeTexture (XFadeTexture_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);
		generateFromProxyOwner(new XFadeTextureEventUGenInternal<XFadeTextureEventType, OwnerType> 
									(sustainTime, transitionTime, numChannels, maxRepeats, e));										
	}

	EventUGenMethodsDeclare(XFadeTexture, 
							XFadeTextureEventType, 
							OwnerType, 
							(XFadeTexture_OwnerInputsNoTypes), 
							(XFadeTexture_OwnerInputsWithTypesAndDefaults), 
							(XFadeTexture_EventInputsNoTypes), 
							(XFadeTexture_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS XFadeTexture_Docs);
};


/** @ingroup UGenInternals */
class OverlapTextureUGenInternal : public SpawnUGenInternal
{
public:
	OverlapTextureUGenInternal(const float sustainTime_,
							   const float transitionTime_,
							   const int density_,
							   const int numChannels, 
							   const int maxRepeats)  throw()
	:	SpawnUGenInternal(numChannels, (2.f * transitionTime_ + sustainTime_) / density_, maxRepeats),
		sustainTime(sustainTime_),
		transitionTime(transitionTime_),
		density(density_)
	{
		ugen_assert(sustainTime >= 0.f);
		ugen_assert(transitionTime >= 0.f);
		ugen_assert((sustainTime+transitionTime) > 0.f);
		ugen_assert(density > 0);
	}
	
	float sustainTime;
	float transitionTime;
	int density;
	
protected:
};


EVENT_DOCS_OWNED(OverlapTexture, OverlapTextureEventBase)
template<class OwnerType = void>
class OverlapTextureEventBase : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(OverlapTextureEventBase)
	OverlapTextureEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(OverlapTextureUGenInternal& spawn, const int eventCount) = 0;
};

EVENT_DOCS_VOID(OverlapTexture, OverlapTextureEventBase)
template<>
class OverlapTextureEventBase<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_SPAWNFUNCTION_DOCS)
	virtual UGen spawnEvent(OverlapTextureUGenInternal& spawn, const int eventCount) = 0;
};

typedef OverlapTextureEventBase<> OverlapTextureEventDefault;

/** @ingroup UGenInternals */
template <class OverlapTextureEventType, class OwnerType>
class OverlapTextureEventUGenInternal : public OverlapTextureUGenInternal
{
public:
	OverlapTextureEventUGenInternal(const float sustainTime,
									const float transitionTime,
									const int density,
									const int numChannels, 
									const int maxRepeats,
									OwnerType* o) throw()
	:	OverlapTextureUGenInternal(sustainTime, transitionTime, density, numChannels, maxRepeats), 
		event_(o)
	{ 
	}
	
	OverlapTextureEventUGenInternal(const float sustainTime,
									const float transitionTime,
									const int density,
									const int numChannels, 
									const int maxRepeats,
									OverlapTextureEventType const& e) throw()
	:	OverlapTextureUGenInternal(sustainTime, transitionTime, density, numChannels, maxRepeats), 
		event_(e)
	{ 
	}

	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount)
	{
		UGen event = event_.spawnEvent(*this, eventCount);
		spawn.nextTime = (2.f * transitionTime + sustainTime) / density;
		Env env(Buffer(0.0, 1.0, 1.0, 0.0), 
				Buffer(transitionTime, sustainTime, transitionTime), 
				EnvCurve::Welch);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone); 
	}

protected:
	OverlapTextureEventType event_;
};

/** @ingroup UGenInternals */
template <class OverlapTextureEventType>
class OverlapTextureEventUGenInternal<OverlapTextureEventType, void> : public OverlapTextureUGenInternal
{
public:
	OverlapTextureEventUGenInternal(const float sustainTime,
									const float transitionTime,
									const int density,
									const int numChannels, 
									const int maxRepeats,
									void* o) throw()
	:	OverlapTextureUGenInternal(sustainTime, transitionTime, density, numChannels, maxRepeats)
	{ 
	}
	
	OverlapTextureEventUGenInternal(const float sustainTime,
									const float transitionTime,
									const int density,
									const int numChannels, 
									const int maxRepeats,
									OverlapTextureEventType const& e) throw()
	:	OverlapTextureUGenInternal(sustainTime, transitionTime, density, numChannels, maxRepeats), 
		event_(e)
	{ 
	}
	
	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) throw()
	{
		UGen event = event_.spawnEvent(*this, eventCount);
		spawn.nextTime = (2.f * transitionTime + sustainTime) / density;
		Env env(Buffer(0.0, 1.0, 1.0, 0.0), 
				Buffer(eventCount == 0 ? 0.f : transitionTime, sustainTime, transitionTime), 
				EnvCurve::Welch);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone); 
	}
	
protected:
	OverlapTextureEventType event_;
};

#define OverlapTexture_OwnerInputsWithTypesAndDefaults	const float sustainTime = 4.f,			\
														const float transitionTime = 4.f,		\
														const int density = 2,					\
														const int numChannels = 1,				\
														const int maxRepeats = 0,				\
														OwnerType* o = 0
#define OverlapTexture_OwnerInputsNoTypes				sustainTime, transitionTime, density, numChannels, maxRepeats, o
#define OverlapTexture_EventInputsWithTypesAndDefaults	OverlapTextureEventType const& e,			\
														const float sustainTime = 4.f,			\
														const float transitionTime = 4.f,		\
														const int density = 2,					\
														const int numChannels = 1,				\
														const int maxRepeats = 0
#define OverlapTexture_EventInputsNoTypes				e, sustainTime, transitionTime, density, numChannels, maxRepeats

#define OverlapTexture_Docs	EVENT_TEMPLATE_DOC(OverlapTexture)													\
							EVENT_UGEN_OWNEREVENT_DOC															\
							@param sustainTime		The sustain time (in seconds) of the envelope.				\
							TEXTURE_TRANSITIONTIME_DOC															\
							@param density			The number of overlapping events.							\
							EVENT_UGEN_NUMCHANNELS_DOC(OverlapTexture)											\
							EVENT_UGEN_MAXREPEATS_DOC(OverlapTexture)										


/** Overlap and fade in/out multiple events.
 
 Creates a series of overlapped sounds from a user event class. The user event class
 should return a graph of unit generators from its spawnEvent() method that produce a 
 continuous sound.  
 
 OverlapTexture will apply an envelope to the sound to cross fade different invocations
 of the event instances. This is different from Spawn which requires the envelope to be 
 specified by the user.
 
 @ingroup AllUGens EventUGens
 @see OverlapTextureEventBase<OwnerType>, Spawn, XFadeTexture */
template <class OverlapTextureEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(OverlapTexture))
class OverlapTexture : public UGen																													
{																																				
public:				
	EVENT_CONSTRUCTOR_DOC(OverlapTexture, OverlapTexture_Docs)
	OverlapTexture (OverlapTexture_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);	
		generateFromProxyOwner(new OverlapTextureEventUGenInternal<OverlapTextureEventType, OwnerType> 
									(sustainTime, transitionTime, density, numChannels, maxRepeats, o));										
	}	
	
	EVENT_CONSTRUCTOR_DOC(OverlapTexture, OverlapTexture_Docs)
	OverlapTexture (OverlapTexture_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);	
		generateFromProxyOwner(new OverlapTextureEventUGenInternal<OverlapTextureEventType, OwnerType> 
									(sustainTime, transitionTime, density, numChannels, maxRepeats, e));										
	}	

	EventUGenMethodsDeclare(OverlapTexture, 
							OverlapTextureEventType, 
							OwnerType, 
							(OverlapTexture_OwnerInputsNoTypes), 
							(OverlapTexture_OwnerInputsWithTypesAndDefaults), 
							(OverlapTexture_EventInputsNoTypes), 
							(OverlapTexture_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS OverlapTexture_Docs);
};


/** @ingroup UGenInternals */
class TrigXFadeUGenInternal : public TSpawnUGenInternal
{
public:
	TrigXFadeUGenInternal(const int numChannels,
								 UGen const& trig,
								 const float transitionTime_,
								 const int maxRepeats) throw()
	:	TSpawnUGenInternal(numChannels, trig, maxRepeats),
		transitionTime(transitionTime_)
	{
		ugen_assert(transitionTime >= 0.f);
	}
	
	float transitionTime;
	
protected:
};

EVENT_DOCS_OWNED(TrigXFade, TrigXFadeEventBase)
template<class OwnerType = void>
class TrigXFadeEventBase : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(TrigXFadeEventBase)
	TrigXFadeEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_SPAWNFUNCTION_DOCS ExtraArgs_Doc)
	virtual UGen spawnEvent(TrigXFadeUGenInternal& spawn, const int eventCount, void* extraArgs) = 0;
};

EVENT_DOCS_VOID(TrigXFade, TrigXFadeEventBase)
template<>
class TrigXFadeEventBase<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_SPAWNFUNCTION_DOCS ExtraArgs_Doc)
	virtual UGen spawnEvent(TrigXFadeUGenInternal& spawn, const int eventCount, void* extraArgs) = 0;
};

typedef TrigXFadeEventBase<> TrigXFadeEventDefault;

/** @ingroup UGenInternals */
template <class TrigXFadeEventType, class OwnerType>
class TrigXFadeEventUGenInternal : public TrigXFadeUGenInternal
{
public:
	TrigXFadeEventUGenInternal(const int numChannels,
									  UGen const& trig,
									  const float transitionTime,
									  const int maxRepeats,
									  OwnerType* o) throw()
	:	TrigXFadeUGenInternal(numChannels, trig, transitionTime, maxRepeats), 
		event_(o)
	{ 
	}
	
	TrigXFadeEventUGenInternal(const int numChannels,
									  UGen const& trig,
									  const float transitionTime,
									  const int maxRepeats,
									  TrigXFadeEventType const& e) throw()
	:	TrigXFadeUGenInternal(numChannels, trig, transitionTime, maxRepeats), 
		event_(e)
	{ 
	}
	
	UGen spawnEvent(TSpawnUGenInternal& /*spawn*/, const int eventCount, void* extraArgs = 0)
	{
		events.release();
		UGen event = event_.spawnEvent(*this, eventCount, extraArgs);
		Env env(Buffer(0.0, 1.0, 0.0), 
				Buffer(transitionTime, transitionTime), 
				EnvCurve::Welch, 1);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone);
	}
	
protected:
	TrigXFadeEventType event_;
};

/** @ingroup UGenInternals */
template <class TrigXFadeEventType>
class TrigXFadeEventUGenInternal<TrigXFadeEventType, void> : public TrigXFadeUGenInternal
{
public:
	TrigXFadeEventUGenInternal(const int numChannels,
									  UGen const& trig,
									  const float transitionTime,
									  const int maxRepeats,
									  void* o) throw()
	:	TrigXFadeUGenInternal(numChannels, trig, transitionTime, maxRepeats)
	{ 
	}
	
	TrigXFadeEventUGenInternal(const int numChannels,
									  UGen const& trig,
									  const float transitionTime,
									  const int maxRepeats,
									  TrigXFadeEventType const& e) throw()
	:	TrigXFadeUGenInternal(numChannels, trig, transitionTime, maxRepeats), 
		event_(e)
	{ 
	}
	
	UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs = 0)
	{
		events.release();
		UGen event = event_.spawnEvent(*this, eventCount, extraArgs);
		Env env(Buffer(0.0, 1.0, 0.0), 
				Buffer(transitionTime, transitionTime), 
				EnvCurve::Welch, 1);
		return event * EnvGen::KR(env, UGen::DeleteWhenDone);
	}
	
protected:
	TrigXFadeEventType event_;
};

#define TrigXFade_OwnerInputsWithTypesAndDefaults	const int numChannels = 1,				\
													UGen const& trig = 0.f,					\
													const float transitionTime = 0.01f,		\
													const int maxRepeats = 0,				\
													OwnerType* o = 0
#define TrigXFade_OwnerInputsNoTypes				numChannels, trig, transitionTime, maxRepeats, o
#define TrigXFade_EventInputsWithTypesAndDefaults	TrigXFadeEventType const& e,			\
													const int numChannels = 1,				\
													UGen const& trig = 0.f,					\
													const float transitionTime = 0.01f,		\
													const int maxRepeats = 0
#define TrigXFade_EventInputsNoTypes				e, numChannels, trig, transitionTime, maxRepeats

#define TrigXFade_Docs				EVENT_TEMPLATE_DOC(TrigXFade)							\
									EVENT_UGEN_OWNEREVENT_DOC								\
									EVENT_UGEN_NUMCHANNELS_DOC(TrigXFade)					\
									TRIG_TRIG_DOC											\
									TEXTURE_TRANSITIONTIME_DOC								\
									EVENT_UGEN_MAXREPEATS_DOC(TrigXFade)

/** Crossfade events using a trigger.
 
 Creates a series of cross faded sounds from a user event class. The user event class
 should return a graph of unit generators  from its spawnEvent() method that produce a
 continuous sound.  
 
 The trig input will cause a new event to be started and the old one to be faded out.
 TrigXFadeTexture will apply an envelope to the sound to cross fade different invocations
 of the event instance. This is different from Spawn which requires the envelope to be 
 specified by the user.
 
 @ingroup AllUGens EventUGens
 @see TrigXFadeEventBase<OwnerType>, Spawn, TSpawn, XFadeTexture */
template <class TrigXFadeEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(TrigXFade))
class TrigXFade : public UGen																													
{																																				
public:
	TrigXFade (TrigXFade_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);			
		generateFromProxyOwner(new TrigXFadeEventUGenInternal<TrigXFadeEventType, OwnerType> 
									(numChannels, trig.mix(), transitionTime, maxRepeats, o));										
	}	

	TrigXFade (TrigXFade_EventInputsWithTypesAndDefaults) throw()
	{																																			
		initInternal(numChannels);			
		generateFromProxyOwner(new TrigXFadeEventUGenInternal<TrigXFadeEventType, OwnerType> 
							   (numChannels, trig.mix(), transitionTime, maxRepeats, e));										
	}	

	EventUGenMethodsDeclare(TrigXFade, 
							TrigXFadeEventType, 
							OwnerType, 
							(TrigXFade_OwnerInputsNoTypes), 
							(TrigXFade_OwnerInputsWithTypesAndDefaults), 
							(TrigXFade_EventInputsNoTypes), 
							(TrigXFade_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS TrigXFade_Docs);
};

/** A generic event class with an owner.
 This may be used to create a generic event for use with a variety of event-based UGen classes.
 Some flexibility is lost using this rather than a specialised version since this
 generic event cannot acces the UGenInternal to modify its behavious (e.g., changing the
 nextTime parameter in the case of the Spawn UGen).
 
 @ingroup Events
 @see GenericEventBase<void>, MixFill, Spawn, TSpawn, XFadeTexture, OverlapTexture, TrigXFade, VoicerBase, Voicer */
template<class OwnerType = void>
class GenericEventBase :	public MixFillEventBase<OwnerType>,
							public SpawnEventBase<OwnerType>,
							public TSpawnEventBase<OwnerType>,
							public XFadeTextureEventBase<OwnerType>,
							public OverlapTextureEventBase<OwnerType>, 
							public TrigXFadeEventBase<OwnerType>,
							public VoicerEventBase<OwnerType>
{
public:
	GenericEventBase(OwnerType* o) 
	:	MixFillEventBase<OwnerType> (o),
		SpawnEventBase<OwnerType> (o),
		TSpawnEventBase<OwnerType> (o),
		XFadeTextureEventBase<OwnerType> (o),
		OverlapTextureEventBase<OwnerType> (o), 
		TrigXFadeEventBase<OwnerType> (o),
		VoicerEventBase<OwnerType> (o)
	{ }

public:
	UGen spawnEvent(SpawnUGenInternal& spawn, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(TSpawnUGenInternal& spawn, const int eventCount, void* extraArgs) { return this->createEvent(eventCount); }
	UGen spawnEvent(XFadeTextureUGenInternal& spawn, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(TrigXFadeUGenInternal& spawn, const int eventCount, void* extraArgs) { return this->createEvent(eventCount); }
	UGen spawnEvent(OverlapTextureUGenInternal& spawn, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(VoicerBaseUGenInternal& spawn, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity) { return this->createEvent(eventCount); }
};

/** A generic event class with no owner.
 This is a specialised version of GenericEventBase with no owner.
 
 This class may be used to create a generic event for use with a variety of event-based UGen classes.
 Some flexibility is lost using this rather than a specialised version since this
 generic event cannot acces the UGenInternal to modify its behavious (e.g., changing the
 nextTime parameter in the case of the Spawn UGen).
 
 @ingroup Events
 @see GenericEventBase, MixFill, Spawn, TSpawn, XFadeTexture, OverlapTexture, TrigXFade, VoicerBase, Voicer */
template<>
class GenericEventBase<void> : 	public MixFillEventBase<void>,
								public SpawnEventBase<void>,
								public TSpawnEventBase<void>,
								public XFadeTextureEventBase<void>,
								public OverlapTextureEventBase<void>, 
								public TrigXFadeEventBase<void>,
								public VoicerEventBase<void>
{	
public:
	UGen spawnEvent(SpawnUGenInternal& /*spawn*/, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(TSpawnUGenInternal& /*spawn*/, const int eventCount, void* /*extraArgs*/) { return this->createEvent(eventCount); }
	UGen spawnEvent(XFadeTextureUGenInternal& /*spawn*/, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(TrigXFadeUGenInternal& /*spawn*/, const int eventCount, void* /*extraArgs*/) { return this->createEvent(eventCount); }
	UGen spawnEvent(OverlapTextureUGenInternal& /*spawn*/, const int eventCount) { return this->createEvent(eventCount); }
	UGen spawnEvent(VoicerBaseUGenInternal& /*spawn*/, 
					const int eventCount,
					const int /*midiChannel*/,
					const int /*midiNote*/,
					const int /*velocity*/) { return this->createEvent(eventCount); }
};

typedef GenericEventBase<> GenericEventDefault;

#endif // gpl

#endif // _UGEN_ugen_Textures_H_
