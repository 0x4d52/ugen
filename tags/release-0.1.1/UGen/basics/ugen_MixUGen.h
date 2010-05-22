// $Id: ugen_MixUGen.h 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_MixUGen.h $

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


#ifndef UGEN_MIXUGEN_H
#define UGEN_MIXUGEN_H

#include "../core/ugen_UGen.h"
#include "../core/ugen_UGenArray.h"


/** A UGenInternal which mixes input UGen channels down to a single channel. 
 @see Mix, MixArrayUGenInternal, MixFill
 @ingroup UGenInternals */
class MixUGenInternal : public UGenInternal
{
public:
	/** Constructor using a reference to a UGen.
	 @param array					A reference to the UGen to be mixed.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	MixUGenInternal(UGen const& array, bool shouldAllowAutoDelete = true) throw();
	
	/** Constructor using a pointer to a UGen.
	 @param array					A pointer to the UGen to be mixed.
									Make sure the object this points to is not deleted before this object.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	MixUGenInternal(UGen* array, bool shouldAllowAutoDelete = true) throw();
	
	/** Render a block of audio. */
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	
private:
	bool shouldAllowAutoDelete_;
};

/** A UGenInternal which mixes a UGenArray down to a multichannel UGen. 
 This is a ProxyOwnerUGenInternal and will create a number of proxy outputs
 depending on the maximum number of channels found by examining the number 
 of channels in each UGen in the UGenArray.
 
 If the UGenArray contained:
 @code { {A, B}, {C, D}, {E, F} } @endcode
 ..where AB, CD and EF are stereo UGens, Mix would mix this to:
 
 @code { (A+C+E), (B+D+F) }@endcode
 i.e., each left channel combined with the others and each right channel
 combined with the others.
 
 @todo (Perhaps there could be an argument to force a particular number of channels?)
 
 @see Mix, MixUGenInternal, MixFill
 @ingroup UGenInternals */
class MixArrayUGenInternal : public ProxyOwnerUGenInternal
{
public:
	/** Constructor using a reference to a UGenArray.
	 @param array					A reference to the UGenArray to be mixed.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	MixArrayUGenInternal(UGenArray const& array, bool shouldAllowAutoDelete = true) throw();
	
	/** Constructor using a pointer to a UGenArray.
	 @param array					A pointer to the UGenArray to be mixed.
									Make sure the object this points to is not deleted before this object.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	MixArrayUGenInternal(UGenArray* array, bool shouldAllowAutoDelete = true) throw();
	
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void releaseInternal() throw(); // has non-standard inputs 
	void stealInternal() throw(); // has non-standard inputs 
	float getValue(const int channel) const throw();
	
private:
	UGenArray array_;
	UGenArray* arrayPtr;
	UGenArray& arrayRef;
	bool shouldAllowAutoDelete_;
};


/** Mix channels.
 This can mix a single UGen with multiple channels down to a single channel, or
 a UGenArray (containing multiple UGen instances) down to a multichannel UGen. 
 
 For example:
 @code
	 UGen source = SinOsc::AR(U(100, 200, 300, 400), 0, 0.1); // four channels..
	 UGen mix = Mix::AR(source); // ..mixed to one channel
 @endcode
 
 or using UGenArray instances:
 @code
	 UGenArray source1 = SinOsc::AR(U(100, 200), 0, 0.1);   // two channels
	 UGenArray source2 = SinOsc::AR(U(400, 600), 0, 0.1);   // two channels
	 UGenArray source3 = SinOsc::AR(U(1000, 1200), 0, 0.1); // two channels
	 UGenArray array = (source1, source2, source3); // Array of three two channel UGen instances	
	 UGen mix = Mix::AR(array); // ..Mixed to two channels 
 @endcode
 
 @ingroup AllUGens MathsUGens
 @see MixUGenInternal, MixArrayUGenInternal, MixFill*/
class Mix : public UGen 
{ 
public: 
	/// @internal
	Mix () throw() : UGen() { }
	
	/** %Mix UGen channels to a single channel using a reference to a UGen.
	
	 @param array					A reference to the UGen to be mixed.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGen classes
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	Mix (UGen const& array, bool shouldAllowAutoDelete = true) throw(); 
	
	
	/// Audio rate @see Mix (UGen const& array, bool shouldAllowAutoDelete)
	static inline UGen AR (UGen const& array, bool shouldAllowAutoDelete = true) throw()		{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate @see Mix (UGen const& array, bool shouldAllowAutoDelete)
	static inline UGen KR (UGen const& array, bool shouldAllowAutoDelete = true) throw()		{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// Audio rate SC compatability @see Mix (UGen const& array, bool shouldAllowAutoDelete)
	inline UGen ar (UGen const& array, bool shouldAllowAutoDelete = true) throw()				{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate SC compatability @see Mix (UGen const& array, bool shouldAllowAutoDelete)
	inline UGen kr (UGen const& array, bool shouldAllowAutoDelete = true) throw()				{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// @internal
	inline UGen operator() (UGen const& array, bool shouldAllowAutoDelete = true) throw()		{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/** %Mix UGen channels to a single channel using a pointer to a UGen.
	 @param array					A pointer to the UGen to be mixed.
									Make sure the object this points to is not deleted before this object.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGen classes
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	Mix (UGen* array, bool shouldAllowAutoDelete = true) throw(); 
	
	/// Audio rate @see Mix (UGen* array, bool shouldAllowAutoDelete)
	static inline UGen AR (UGen* array, bool shouldAllowAutoDelete = true) throw()				{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate @see Mix (UGen* array, bool shouldAllowAutoDelete)
	static inline UGen KR (UGen* array, bool shouldAllowAutoDelete = true) throw()				{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// Audio rate SC compatability @see Mix (UGen* array, bool shouldAllowAutoDelete)
	inline UGen ar (UGen* array, bool shouldAllowAutoDelete = true) throw()						{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate SC compatability @see Mix (UGen* array, bool shouldAllowAutoDelete)
	inline UGen kr (UGen* array, bool shouldAllowAutoDelete = true) throw()						{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// @internal
	inline UGen operator() (UGen* array, bool shouldAllowAutoDelete = true) throw()				{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/** %Mix a UGenArray to a multichannel UGen using a reference to a UGenArray.
	 @param array					A reference to the UGenArray to be mixed.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	Mix (UGenArray const& array, bool shouldAllowAutoDelete = true) throw(); 
	
	/// Audio rate @see Mix (UGenArray const& array, bool shouldAllowAutoDelete)
	static inline UGen AR (UGenArray const& array, bool shouldAllowAutoDelete = true) throw()	{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate @see Mix (UGenArray const& array, bool shouldAllowAutoDelete)
	static inline UGen KR (UGenArray const& array, bool shouldAllowAutoDelete = true) throw()	{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// Audio rate SC compatability @see Mix (UGenArray const& array, bool shouldAllowAutoDelete)
	inline UGen ar (UGenArray const& array, bool shouldAllowAutoDelete = true) throw()			{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate SC compatability @see Mix (UGenArray const& array, bool shouldAllowAutoDelete)
	inline UGen kr (UGenArray const& array, bool shouldAllowAutoDelete = true) throw()			{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// @internal
	inline UGen operator() (UGenArray const& array, bool shouldAllowAutoDelete = true) throw()	{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/** %Mix a UGenArray to a multichannel UGen using a pointer to a UGenArray.
	 @param array					A pointer to the UGenArray to be mixed.
									Make sure the object this points to is not deleted before this object.
	 @param shouldAllowAutoDelete	If true this behaves like most other UGenInternal objects
									i.e., it may be deleted by a DoneAction (e.g., an envelope
									ending). If false this protects UGen instances further down the chain
									(and itself) from being deleted by DoneActions. */
	Mix (UGenArray* array, bool shouldAllowAutoDelete = true) throw(); 
	
	/// Audio rate @see Mix (UGenArray* array, bool shouldAllowAutoDelete)
	static inline UGen AR (UGenArray* array, bool shouldAllowAutoDelete = true) throw()			{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate @see Mix (UGenArray* array, bool shouldAllowAutoDelete)
	static inline UGen KR (UGenArray* array, bool shouldAllowAutoDelete = true) throw()			{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// Audio rate SC compatability @see Mix (UGenArray* array, bool shouldAllowAutoDelete)
	inline UGen ar (UGenArray* array, bool shouldAllowAutoDelete = true) throw()				{ return Mix (array, shouldAllowAutoDelete);			} 
	
	/// Control rate SC compatability @see Mix (UGenArray* array, bool shouldAllowAutoDelete)
	inline UGen kr (UGenArray* array, bool shouldAllowAutoDelete = true) throw()				{ return UGen(Mix (array, shouldAllowAutoDelete)).kr(); } 
	
	/// @internal
	inline UGen operator() (UGenArray* array, bool shouldAllowAutoDelete = true) throw()		{ return Mix (array, shouldAllowAutoDelete);			} 

private:
	void constructMixArrayWithProxies(MixArrayUGenInternal* internal);
};

#define EVENT_MIXFILLFUNCTION_DOCS					You must implement this virtual function in your subclass.						\
													The parent MixFill UGen calls this function multiple times to					\
													create an array of UGen instances to mix.										\
													@param eventCount	The index of the event instance starting at 0 for			\
																		the first event.											\
													@return				The UGen graph for this event instance.

/** This base class provides the mechanism for generating the array of events.
 You should inherit from MixFillEventBase rather than this class and implement
 createEvent().
 @see MixFillEventBase, MixFillEventBase<void>, MixFill, Bank */
class MixFillBaseEventBase
{
public:
	MixFillBaseEventBase() { }
	virtual ~MixFillBaseEventBase() { }
	
	PREDOC(EVENT_MIXFILLFUNCTION_DOCS)
	virtual UGen createEvent(const int eventCount) = 0;
	
	/** This returns a UGen array using the createEvent() function to create each element.
		Do not override this function.
		@param size		The number of items to create in the array.
		@return			The UGen array. */
	UGenArray createArray(const int size)
	{
		ugen_assert(size > 0);
		
		UGenArray array;
		for(int i = 0; i < size; i++)
			array <<= createEvent(i);
		return array;
	}
};

EVENT_DOCS_OWNED(MixFill, MixFillEventBase)
template<class OwnerType = void> POSTDOC(EVENT_OWNER_DOC)
class MixFillEventBase :	public EventBase<OwnerType>, 
							public MixFillBaseEventBase
{
public:
	EVENT_OWNEDCONSTRUCTOR(MixFillEventBase)
	MixFillEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
};

EVENT_DOCS_VOID(MixFill, MixFillEventBase)
template<>
class MixFillEventBase<void> :	public EventBase<void>, 
								public MixFillBaseEventBase
{ 
};

typedef MixFillEventBase<> MixFillEventDefault;

/** @ingroup UGenInternals */
template <class MixFillEventType, class OwnerType>
class MixFillEventUGenInternal : public MixArrayUGenInternal
{
public:
	MixFillEventUGenInternal(const int size, 
							 const int numChannels, 
							 bool shouldAllowAutoDelete, 
							 OwnerType* o) throw()
	:	MixArrayUGenInternal(MixFillEventType(o).createArray(size), 
							 shouldAllowAutoDelete) { }
	
	MixFillEventUGenInternal(MixFillEventType const& e,
							 const int size, 
							 const int numChannels, 
							 bool shouldAllowAutoDelete) throw()
	:	MixArrayUGenInternal((static_cast<MixFillEventType>(e)).createArray(size),  // not sure about this casting away but need to keep const& in the arg!
							 shouldAllowAutoDelete) { }
	
protected:
};

/** @ingroup UGenInternals */
template <class MixFillEventType>
class MixFillEventUGenInternal<MixFillEventType, void> : public MixArrayUGenInternal
{
public:
	MixFillEventUGenInternal(const int size, 
							 const int numChannels, 
							 bool shouldAllowAutoDelete, 
							 void* o) throw()
	:	MixArrayUGenInternal(MixFillEventType().createArray(size), 
							 shouldAllowAutoDelete) { }
	
	MixFillEventUGenInternal(MixFillEventType const& e,
							 const int size, 
							 const int numChannels, 
							 bool shouldAllowAutoDelete) throw()
	:	MixArrayUGenInternal((static_cast<MixFillEventType>(e)).createArray(size),  // not sure about this casting away but need to keep const& in the arg!
							 shouldAllowAutoDelete) { }	
	
protected:
};

#define MixFill_OwnerInputsWithTypesAndDefaults	const int size = 4,					\
												const int numChannels = 1,			\
												bool shouldAllowAutoDelete = true,	\
												OwnerType* o = 0
#define MixFill_OwnerInputsNoTypes				size, numChannels, shouldAllowAutoDelete, o
#define MixFill_EventInputsWithTypesAndDefaults	MixFillEventType const& e,			\
												const int size = 4,					\
												const int numChannels = 1,			\
												bool shouldAllowAutoDelete = true
#define MixFill_EventInputsNoTypes				e, size, numChannels, shouldAllowAutoDelete

#define MixFill_Docs			EVENT_TEMPLATE_DOC(MixFill)																		\
								EVENT_UGEN_OWNEREVENT_DOC																		\
								@param	size			The number of events to generate and mix.								\
								EVENT_UGEN_NUMCHANNELS_DOC(MixFill)																\
								@param	shouldAllowAutoDelete	If true this behaves like most other UGen classes				\
																i.e., it may be deleted by a DoneAction (e.g., an envelope		\
																ending). If false this protects UGen instances further down		\
																the chain (and itself) from being deleted by DoneActions.

/** Fill a UGen array using an event and mix it down.
 
 @ingroup AllUGens EventUGens
 @see Mix, MixFillEventBase, MixFillBaseEventBase, Bank  */
template <class MixFillEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATE_DOC(MixFill))
class MixFill : public UGen																													
{																																				
public:
	EVENT_CONSTRUCTOR_DOC(MixFill, MixFill_Docs)
	MixFill (const int size = 4, 
			 const int numChannels = 1, 
			 bool shouldAllowAutoDelete = true, 
			 OwnerType* o = 0) throw()
	{				
		ugen_assert(size > 0);
		ugen_assert(numChannels > 0);

		initInternal(numChannels);	
		generateFromProxyOwner(new MixFillEventUGenInternal<MixFillEventType, OwnerType> 
									(size, numChannels, shouldAllowAutoDelete, o));										
	}																																			

	EVENT_CONSTRUCTOR_DOC(MixFill, MixFill_Docs)
	MixFill (MixFillEventType const& e,
			 const int size = 4, 
			 const int numChannels = 1, 
			 bool shouldAllowAutoDelete = true) throw()
	{			
		ugen_assert(size > 0);
		ugen_assert(numChannels > 0);
		
		initInternal(numChannels);	
		generateFromProxyOwner(new MixFillEventUGenInternal<MixFillEventType, OwnerType> 
									(e, size, numChannels, shouldAllowAutoDelete));										
	}
		

	EventUGenMethodsDeclare(MixFill, 
							MixFillEventType, 
							OwnerType, 
							(MixFill_OwnerInputsNoTypes), 
							(MixFill_OwnerInputsWithTypesAndDefaults), 
							(MixFill_EventInputsNoTypes), 
							(MixFill_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS MixFill_Docs);																		
};


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Mix Mix()
#define MixFill MixFill()
#endif

#endif // UGEN_MIXUGEN_H