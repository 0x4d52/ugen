// $Id: ugen_Delay.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/delays/ugen_Delay.h $

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

#ifndef _UGEN_ugen_Delay_H_
#define _UGEN_ugen_Delay_H_

#include "../core/ugen_UGen.h"
#include "../core/ugen_Constants.h"
#include "../core/ugen_Value.h"
#include "../basics/ugen_Chain.h"

/** @ingroup UGenInternals */
class DelayBaseUGenInternal : public ProxyOwnerUGenInternal
{
public:
	DelayBaseUGenInternal(const int numInputs, 
						  UGen const& input, 
						  UGen const& delayTime, 
						  Buffer const& delayBuffer, 
						  const bool isMultiTap) throw();
	UGenInternal* getChannel(const int channel) throw() = 0;		// final subclasses now MUST implment this..						
	
	enum Inputs { Input, DelayTime, NumInputs };
	
protected:

	// perhaps move these lookups to Buffer?
	inline float lookupIndexN(const int index)
	{
		return bufferSamples[index];
	}
	
	// must make this more efficient
	inline float lookupIndexL(const float fIndex)
	{
		const int iIndex0 = (int)fIndex;
		const int iIndex1 = iIndex0+1;
		const float frac = fIndex - (float)iIndex0;
		const float value0 = bufferSamples[iIndex0];
		float value1;
		
		if(iIndex1 == delayBuffer_.size()) {
			value1 = bufferSamples[0];
		} else {	
			value1 = bufferSamples[iIndex1];
		}
		
		return value0 + frac * (value1 - value0);
	}
	
	// 0 is one ahead, 1 is this, 2 is on behind, 3 is two behind
	//static float cubicinterp(float x, float y0, float y1, float y2, float y3)
	inline float lookupIndexC(const float fIndex)
	{
		const int iIndex1 = (int)fIndex;
		const int iIndex0 = iIndex1+1;
		const int iIndex2 = iIndex1-1;
		const int iIndex3 = iIndex1-2;
		const float y0 = bufferSamples[iIndex0];
		const float y1 = bufferSamples[iIndex1];
		const float y2 = bufferSamples[iIndex2];
		const float y3 = bufferSamples[iIndex3];
		
		const float frac = fIndex - iIndex1;
		
		// 4-point, 3rd-order Hermite (x-form)
		float c0 = y1;
		float c1 = 0.5f * (y2 - y0);
		float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
		float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
		
		return ((c3 * frac + c2) * frac + c1) * frac + c0;
	}
	
	Buffer delayBuffer_;
	const double delayBufferSize;
	float *bufferSamples;
	int bufferWritePos;
};

/** @ingroup UGenInternals */
class DelayNUGenInternal : public DelayBaseUGenInternal
{
public:
	DelayNUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
class DelayNMultiUGenInternal : public DelayBaseUGenInternal
{
public:
	DelayNMultiUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


/** @ingroup UGenInternals */
class DelayLUGenInternal : public DelayBaseUGenInternal
{
public:
	DelayLUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
class DelayLMultiUGenInternal : public DelayBaseUGenInternal
{
public:
	DelayLMultiUGenInternal(UGen const& input, UGen const& delayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


/** @ingroup UGenInternals */
class RecircBaseUGenInternal : public DelayBaseUGenInternal
{
public:	
	RecircBaseUGenInternal(UGen const& input, 
						   UGen const& delayTime, 
						   UGen const& decayTime, 
						   Buffer const& delayBuffer) throw();
	
	enum Inputs { Input, DelayTime, DecayTime, NumInputs };
	
protected:

	inline float calcFeedback(float delay, float decay)
	{
		if (delay == 0.f) {
			return 0.f;
		} else if (decay > 0.f) {
			return (float)std::exp(log001 * delay / decay);
		} else if (decay < 0.f) {
			return (float)-std::exp(log001 * delay / -decay);
		} else {
			return 0.f;
		}
	}
	
	float currentDelay;
	float currentDecay;
	float feedback;
};

/** @ingroup UGenInternals */
class CombNUGenInternal : public RecircBaseUGenInternal
{
public:
	CombNUGenInternal(UGen const& input, UGen const& delayTime, UGen const& decayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


/** @ingroup UGenInternals */
class CombLUGenInternal : public RecircBaseUGenInternal
{
public:
	CombLUGenInternal(UGen const& input, UGen const& delayTime, UGen const& decayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


/** @ingroup UGenInternals */
class AllpassNUGenInternal : public RecircBaseUGenInternal
{
public:
	AllpassNUGenInternal(UGen const& input, UGen const& delayTime, UGen const& decayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


/** @ingroup UGenInternals */
class AllpassLUGenInternal : public RecircBaseUGenInternal
{
public:
	AllpassLUGenInternal(UGen const& input, UGen const& delayTime, UGen const& decayTime, Buffer const& delayBuffer) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


#define Delay_Docs			@param	input				Input source to delay.											\
							@param	maximumDelayTime	The maximum delay time in seconds this delay UGen could use,	\
														this governs the amount of memory allocated when the UGen		\
														is created.														\
							@param	delayTime			The delay time to use, this must be between 0 and the value		\
														given for the maximumDelayTime on the UGen`s creation	

#define Recirc_Docs			@param	decayTime			The time in seconds for the recirculated signal to decay		\
														by 60 dBs	

/** Simple delay with no interpolation.
 @see DelayL, CombN, CombL, AllpassN, AllpassL
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(DelayN, (input, maximumDelayTime, delayTime),
					   (UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f), 
						COMMON_UGEN_DOCS Delay_Docs);

/** Simple delay with linear interpolation. 
 @see DelayN, CombN, CombL, AllpassN, AllpassL 
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(DelayL, (input, maximumDelayTime, delayTime),
					   (UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f), 
						COMMON_UGEN_DOCS Delay_Docs);

/** Comb delay with no interpolation. 
 @see DelayN, DelayL, CombL, AllpassN, AllpassL 
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(CombN, (input, maximumDelayTime, delayTime, decayTime),
					   (UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f, UGen const& decayTime = 0.2f), 
						COMMON_UGEN_DOCS Delay_Docs Recirc_Docs);

/** Comb delay with linear interpolation. 
 @see DelayN, DelayL, CombN, AllpassN, AllpassL 
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(CombL, (input, maximumDelayTime, delayTime, decayTime),
						(UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f, UGen const& decayTime = 0.2f), 
						COMMON_UGEN_DOCS Delay_Docs Recirc_Docs);

/** Allpass delay with no interpolation. 
 @see DelayN, DelayL, CombN, CombL, AllpassL, BAllPass 
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(AllpassN, (input, maximumDelayTime, delayTime, decayTime),
						(UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f, UGen const& decayTime = 0.2f), 
						COMMON_UGEN_DOCS  Delay_Docs Recirc_Docs);

/** Allpass delay with linear interpolation. 
 @see DelayN, DelayL, CombN, CombL, AllpassN, BAllPass 
 @ingroup AllUGens DelayUGens */
UGenSublcassDeclaration(AllpassL, (input, maximumDelayTime, delayTime, decayTime),
						(UGen const& input, const float maximumDelayTime = 0.2f, UGen const& delayTime = 0.2f, UGen const& decayTime = 0.2f), 
						COMMON_UGEN_DOCS  Delay_Docs Recirc_Docs);

/** Creat a chain of allpass or comb delays.
 @tparam RecircBaseType		This must be one of the following (otherwise you may get very strange errors): 
							CombN, CombL, AllpassN, AllpassL. This will be used as the delay type. 
 @ingroup AllUGens DelayUGens EventUGens */
template<class RecircBaseType>
class RecircBaseChain : public UGen
{
public:
	
	/** A ChainBankEvent which creates a recirculating delay unit. 
		@ingroup Events */
	class RecircBaseChainLink : public ChainBankEvent < >
	{
	public:
		/** Constructor.
			@param	maximumDelayTime	The maximum size of the delay unit in seconds.
			@param	delayTime			The delay time in seconds. This is a Value class so
										will be evaluated for each link created (for most purposes
										some kind of random value would be useful here).
			@param	decayTime			The decay time  to decay by 60dB (in seconds). 
										This is a Value class so will be evaluated for each link 
										created. It may beuseful to use a random value here too. */
		RecircBaseChainLink(const float maximumDelayTime = 0.2f, 
							ValueArray const& delayTime = 0.2f, 
							ValueArray const& decayTime = 0.2f)
		:	maximumDelayTime_(maximumDelayTime),
			delayTime_(delayTime),
			decayTime_(decayTime)
		{
		}
		
		/** Creates the recirculating delay link.
			@param input		The input to the chain (if eventCount is 0) or the output of
								the previous chain link otherwise.
			@param eventCount	The link number. */
		UGen createEvent(UGen const& input, const int eventCount)
		{
			return RecircBaseType::AR(input, maximumDelayTime_, delayTime_, decayTime_);
		}
		
	private:
		float maximumDelayTime_;
		ValueArray const& delayTime_;
		ValueArray const& decayTime_;
	};
	
	/** Construct and audio rate RecircBaseChain chain 
	 @param input				The input to the chain.
	 @param size				The number of links in the chain.
	 @param	maximumDelayTime	The maximum size of the delay unit in seconds.
	 @param	delayTime			The delay time in seconds. This is a Value class so
								will be evaluated for each link created (for most purposes
								some kind of random value would be useful here).
	 @param	decayTime			The decay time  to decay by 60dB (in seconds). 
								This is a Value class so will be evaluated for each link 
								created. It may beuseful to use a random value here too. */
	static UGen AR(UGen const& input, 
				   const int size,
				   const float maximumDelayTime = 0.2f, 
				   ValueArray const& delayTime = 0.2f, 
				   ValueArray const& decayTime = 0.2f)
	{
		int numChannels = input.getNumChannels();
		if(delayTime.size() > numChannels)
			numChannels = delayTime.size();
		if(decayTime.size() > numChannels)
			numChannels = decayTime.size();
			
		return Chain<RecircBaseChainLink>::AR(RecircBaseChainLink(maximumDelayTime, 
																  delayTime, 
																  decayTime),
											  input, size, numChannels);
	}
	
private:
	RecircBaseChain();
};


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define DelayN DelayN()
#define DelayL DelayL()
#define CombN CombN()
#define CombL CombL()
#define AllpassN AllpassN()
#define AllpassL AllpassL()
#endif


#endif // _UGEN_ugen_Delay_H_
