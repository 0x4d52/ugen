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

#ifndef _UGEN_ugen_Chain_H_
#define _UGEN_ugen_Chain_H_

#include "../core/ugen_UGen.h"
#include "../core/ugen_UGenArray.h"
#include "../basics/ugen_MixUGen.h"

/** @ingroup UGenInternals */
class ChainBaseUGenInternal : public ProxyOwnerUGenInternal
{
public:
	ChainBaseUGenInternal(UGen const& input, const int size, const int numChannels) throw();
	~ChainBaseUGenInternal();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:
	const int size_;
	float** const bufferData;
	UGenArray chain;
};

/** @ingroup UGenInternals */
class BankBaseUGenInternal : public ProxyOwnerUGenInternal
{
public:
	BankBaseUGenInternal(UGen const& input, const int size, const int numChannels) throw();
	~BankBaseUGenInternal();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:
	const int size_;
	float** const bufferData;
	UGenArray bank;
	UGen mixer;
};

#define EVENT_CHAINBANKFUNCTION_DOCS					You must implement this virtual function in your subclass.						\
														The parent Bank or Chain UGen calls this function multiple times to				\
														create an array of UGen instances to use in a chain or bank structure.			\
														@param input		If this is used in a Bank UGen this will be the				\
																			input to the Bank and all events get the same input.		\
																			If this is used in a Chain UGen	the first link (when		\
																			eventCount is 0) will get the input to the chain but		\
																			subsequent links will get the output of the previous		\
																			link as their input.										\
														@param eventCount	The index of the event instance starting at 0 for			\
																			the first event.											\
														@return				The UGen graph for this event instance.


EVENT_DOCS_OWNED(Chain or Bank, ChainBankEvent)
template<class OwnerType = void> POSTDOC(EVENT_OWNER_DOC)
class ChainBankEvent : public EventBase<OwnerType>
{
public:
	EVENT_OWNEDCONSTRUCTOR(ChainBankEvent)
	ChainBankEvent(OwnerType* o) : EventBase<OwnerType> (o) { }
	PREDOC(EVENT_CHAINBANKFUNCTION_DOCS)
	virtual UGen createEvent(UGen const& input, const int eventCount) = 0;
};

EVENT_DOCS_VOID(Chain or Bank, ChainBankEvent)
template<>
class ChainBankEvent<void> : public EventBase<void>
{
public:
	PREDOC(EVENT_CHAINBANKFUNCTION_DOCS)
	virtual UGen createEvent(UGen const& input, const int eventCount) = 0;
};

typedef ChainBankEvent<> ChainBankEventDefault;

/** @ingroup UGenInternals */
template <class ChainLinkType, class OwnerType>
class ChainUGenInternal : public ChainBaseUGenInternal
{
public:
	ChainUGenInternal(UGen const& input, const int size, const int numChannels, OwnerType* o) throw()
	:	ChainBaseUGenInternal(input, size, numChannels), chainLinkCreator(o)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
	ChainUGenInternal(UGen const& input, const int size, const int numChannels, ChainLinkType const& e) throw()
	:	ChainBaseUGenInternal(input, size, numChannels), chainLinkCreator(e)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}

private:
	void init(UGen const& input, const int size)
	{
		UGen nextInput = input;
		for(int i = 0; i < size; i++)
		{
			nextInput = chainLinkCreator.createEvent(nextInput, i);
			chain.put(i, nextInput);
		}
	}
	
protected:
	ChainLinkType chainLinkCreator;
};

/** @ingroup UGenInternals */
template <class ChainLinkType>
class ChainUGenInternal<ChainLinkType, void> : public ChainBaseUGenInternal
{
public:
	ChainUGenInternal(UGen const& input, const int size, const int numChannels, void* o) throw()
	:	ChainBaseUGenInternal(input, size, numChannels)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
	ChainUGenInternal(UGen const& input, const int size, const int numChannels, ChainLinkType const& e) throw()
	:	ChainBaseUGenInternal(input, size, numChannels), chainLinkCreator(e)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}

private:
	void init(UGen const& input, const int size)
	{
		UGen nextInput = input;
		for(int i = 0; i < size; i++)
		{
			nextInput = chainLinkCreator.createEvent(nextInput, i);
			chain.put(i, nextInput);
		}
	}
	
protected:
	ChainLinkType chainLinkCreator;
};

/** @ingroup UGenInternals */
template <class BankEventType, class OwnerType>
class BankUGenInternal : public BankBaseUGenInternal
{
public:
	BankUGenInternal(UGen const& input, const int size, const int numChannels, OwnerType* o) throw()
	:	BankBaseUGenInternal(input, size, numChannels), bankEventCreator(o)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
	BankUGenInternal(UGen const& input, const int size, const int numChannels, BankEventType const& e) throw()
	:	BankBaseUGenInternal(input, size, numChannels), bankEventCreator(e)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
private:
	void init(UGen const& input, const int size)
	{
		for(int i = 0; i < size; i++)
		{
			bank.put(i, bankEventCreator.createEvent(input, i));
		}
		
		mixer = Mix(&bank);		
	}
	
protected:
	BankEventType bankEventCreator;
};

/** @ingroup UGenInternals */
template <class BankEventType>
class BankUGenInternal<BankEventType, void> : public BankBaseUGenInternal
{
public:
	BankUGenInternal(UGen const& input, const int size, const int numChannels, void* o) throw()
	:	BankBaseUGenInternal(input, size, numChannels)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
	BankUGenInternal(UGen const& input, const int size, const int numChannels, BankEventType const& e) throw()
	:	BankBaseUGenInternal(input, size, numChannels), bankEventCreator(e)
	{ 
		ugen_assert(size > 0);
		init(input, size);
	}
	
private:
	void init(UGen const& input, const int size)
	{
		for(int i = 0; i < size; i++)
		{
			bank.put(i, bankEventCreator.createEvent(input, i));
		}
		
		mixer = Mix(&bank);		
	}
	
protected:
	BankEventType bankEventCreator;
};

#define ChainBank_OwnerInputsWithTypesAndDefaults	UGen const& input = 0.f, const int size = 4, const int numChannels = -1, OwnerType* o = 0
#define ChainBank_OwnerInputsNoTypes				input, size, numChannels, o
#define Chain_EventInputsWithTypesAndDefaults		ChainLinkType const& e, UGen const& input = 0.f, const int size = 4, const int numChannels = -1
#define Bank_EventInputsWithTypesAndDefaults		BankEventType const& e, UGen const& input = 0.f, const int size = 4, const int numChannels = -1
#define ChainBank_EventInputsNoTypes				e, input, size, numChannels

#define Chain_Docs				EVENT_TEMPLATESPECIAL_DOC(Chain, ChainLinkType, ChainBankEvent)									\
								EVENT_UGEN_OWNEREVENT_DOC																		\
								@param input	The input to the Chain.															\
								@param size		The number of events to generate in a chain.									\
								EVENT_UGEN_NUMCHANNELS_DOC(Chain)



/** Construct a chain of UGen instances. 
 This constructs a chain of UGen instances which pass their output to one of more inputs in 
 the next "link" in the chain. This is useful for things like allpass-based 
 reverbs. Chains can be constructed without using this class but the key difference
 is the chain is precalculated in reverse order. This prevents excessively deep
 (and large) call stacks (which in many CPUs slows down almost exponentially
 as the call stack gets deeper). 
 @ingroup AllUGens EventUGens
 @see Bank, ChainBankEvent, ChainBankEvent<void> */
template <class ChainLinkType, class OwnerType = void> POSTDOC(EVENT_TEMPLATESPECIAL_DOC(Chain, ChainLinkType, ChainBankEvent))
class Chain : public UGen																													
{																																				
public:
	EVENT_CONSTRUCTOR_DOC(Chain, Chain_Docs)
	Chain (ChainBank_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		if(numChannels <= 0)
			initInternal(input.getNumChannels());
		else
			initInternal(numChannels);

		generateFromProxyOwner(new ChainUGenInternal<ChainLinkType, OwnerType>
									(input, size, numChannels, o));										
	}

	EVENT_CONSTRUCTOR_DOC(Chain, Chain_Docs)
	Chain (Chain_EventInputsWithTypesAndDefaults) throw()
	{																																			
		if(numChannels <= 0)
			initInternal(input.getNumChannels());
		else
			initInternal(numChannels);
		
		generateFromProxyOwner(new ChainUGenInternal<ChainLinkType, OwnerType>
									(input, size, numChannels, e));										
	}

	EventUGenMethodsDeclare(Chain, 
							ChainLinkType, 
							OwnerType, 
							(ChainBank_OwnerInputsNoTypes), 
							(ChainBank_OwnerInputsWithTypesAndDefaults), 
							(ChainBank_EventInputsNoTypes), 
							(Chain_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS Chain_Docs);	
};


#define Bank_Docs				EVENT_TEMPLATESPECIAL_DOC(Bank, BankEventType, ChainBankEvent)									\
								EVENT_UGEN_OWNEREVENT_DOC																		\
								@param input	The input to the Bank.															\
								@param size		The number of events to generate and mix.										\
								EVENT_UGEN_NUMCHANNELS_DOC(Bank)

/** Construct a bank of UGen instances. 
 This constructs a bank of UGen instances which have a common input and their output is
 mixed. This is useful for things like comb-based reverbs. Banks can be constructed 
 without using this class (e.g. using Mix and a UGenArray or MixFill). However, this
 is symmetrical with the Chain class.
 @ingroup AllUGens EventUGens
 @see MixFill, Chain, ChainBankEvent, ChainBankEvent<void> */
template <class BankEventType, class OwnerType = void> POSTDOC(EVENT_TEMPLATESPECIAL_DOC(Bank, BankEventType, ChainBankEvent))
class Bank : public UGen																													
{																																				
public:
	EVENT_CONSTRUCTOR_DOC(Bank, Bank_Docs)
	Bank (ChainBank_OwnerInputsWithTypesAndDefaults) throw()
	{																																			
		if(numChannels <= 0)
			initInternal(input.getNumChannels());
		else
			initInternal(numChannels);

		generateFromProxyOwner(new BankUGenInternal<BankEventType, OwnerType>
									(input, size, numChannels, o));										
	}

	EVENT_CONSTRUCTOR_DOC(Bank, Bank_Docs)
	Bank (Bank_EventInputsWithTypesAndDefaults) throw()
	{																																			
		if(numChannels <= 0)
			initInternal(input.getNumChannels());
		else
			initInternal(numChannels);
		
		generateFromProxyOwner(new BankUGenInternal<BankEventType, OwnerType>
									(input, size, numChannels, e));										
	}

	EventUGenMethodsDeclare(Bank, 
							BankEventType, 
							OwnerType, 
							(ChainBank_OwnerInputsNoTypes), 
							(ChainBank_OwnerInputsWithTypesAndDefaults), 
							(ChainBank_EventInputsNoTypes), 
							(Bank_EventInputsWithTypesAndDefaults), 
							EVENT_COMMON_UGEN_DOCS Bank_Docs);	
};




#endif // _UGEN_ugen_Chain_H_
