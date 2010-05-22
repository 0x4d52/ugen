// $Id: ugen_UGenInternal.h 1011 2010-05-12 16:24:29Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/core/ugen_UGenInternal.h $

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

#ifndef UGEN_UGENINTERNAL_H
#define UGEN_UGENINTERNAL_H

//#define DEBUGREFCOUNT

#include "ugen_SmartPointer.h"

class UGen;

/** @internal */
class UGenOutput
{
public:
	UGenOutput() throw();
	~UGenOutput();
	
	inline void prepareForBlock(const int actualBlockSize)
	{
		ugen_assert(actualBlockSize > 0);
		
		if(externalOutput != 0)
		{
			ugen_assert(dynamic_cast<UGenOutput*> (externalOutput) != 0);
			
			externalOutput->prepareForBlock(actualBlockSize);
			blockSize = externalOutput->getBlockSize();
			block = externalOutput->getSampleData();
		}
		else if(!usingExternalOutput)
		{
			blockSize = actualBlockSize;
			
			if(actualBlockSize > allocatedBlockSize)
			{		
				delete [] block;
				allocatedBlockSize = blockSize;
				block = new float[allocatedBlockSize];
			}
		}
	}
	//void optimiseBlockSize(); // reduce block sizes to minimum
	
	//inline float getOutputValue() const					{ if(usingExternalOutput) return externalOutput->getOutputValue(); else return output;		}
	//inline void setOutputValue(const float newValue)	{ if(usingExternalOutput) externalOutput->setOutputValue(newValue); else output = newValue;	}
	inline int getBlockSize() const						{ return blockSize;			}
	inline float* getSampleData() const					{ return block;				}
	inline void zeroAllData()							{ memset(block, 0, blockSize * sizeof(float)); }
	void initValue(const float value) throw();
	
	void useExternalOutput(UGenOutput* externalOutputToUse);
	void useExternalOutput(float* externalOutputToUse, const int externalBlockSize);
	
private:
	//float output;
	int blockSize;
	int allocatedBlockSize;
	float *block;
	bool usingExternalOutput;
	UGenOutput* externalOutput;
};


/** Subclasses of this do almost all of the processing during audio rendering.
 
 An array of these is held by each UGen to represent its output channels of any processing.
 Each UGenInternal may have zero or more inputs as other UGen instances (e.g., in the case of oscillators 
 this might be for continuous control of frequency, for filters it might be the audio input and
 frequency). A UGenInternal may have only ONE output, for multiple outputs which need to be kept
 in communication (e.g., panning) you may need to use the ProxyOwnerUGenInternal as your base class
 instead (this uses zero or more ProxyUGenInternal objects for the additional channels).
 
 More tba..
 
 @see ProxyOwnerUGenInternal, ProxyUGenInternal
 @ingroup UGenInternals
 */
class UGenInternal	: public SmartPointer
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	UGenInternal(const int numInputs) throw();
	
	/**
	 A special constructor for the Mix UGen.
	 */	
	UGenInternal(UGen *mixInputToUse) throw();
	~UGenInternal();
	
	UGenInternal* getChannelInternal(const int channel) throw();
	virtual UGenInternal* getKr() throw();
	
	virtual void releaseInternal() throw();
	virtual void stealInternal() throw();
	// NB. should use 64 bit ints for blockID to avoid the complexity of spotting the overflow
	inline bool shouldBeDeletedNow(unsigned int blockID) const throw()	{ return isScheduledForDeletion && blockID > blockIDtoBeDeletedAfter; }
	
	/// @} <!-- end Construction and destruction -->
	
	/// @name Tests
	/// @{
	
	virtual inline bool isProxy() const throw()			{ return false;							}
	virtual inline bool isScalar() const throw()		{ return false;							}
	virtual inline bool isConst() const throw()			{ return false;							}
	virtual inline bool isNull() const throw()			{ return false;							}
	
	/// @} <!-- end Tests -->
	
	/// @name Rate
	/// @{
	
	enum Rates
	{
		AudioRate = 1,
		ControlRate = 2,
		AudioAndControlRate = AudioRate | ControlRate
	};
	
	inline bool isAudioRate() const throw()				{ return rate & AudioRate;					}
	inline bool isAudioRateOnly() const throw()			{ return rate == AudioRate;					}
	inline bool isControlRate()	const throw()			{ return (rate & ControlRate) != 0;			}
	inline bool isControlRateOnly()	const throw()		{ return rate == ControlRate;				}
	inline bool isAudioAndControlRate() const throw()	{ return (rate & AudioAndControlRate) != 0;	}
	
	/// @} <!-- end Rate -->
	
	
	/// @name Rendering
	/// @{
	
	virtual void prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID) throw();
	virtual void prepareForBlock(const int actualBlockSize, const unsigned int blockID)	 throw() { }
	
	virtual float* processBlockInternal(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	/** Process a block of samples for this blockID and channel.
	 
	 This is the main rendering function and needs to be implement by all subclasses.
	 
	 @param shouldDelete	A UGen graph can be scheduled for deletion by setting this to true
							on return. If it is already true it should NOT be set to false.
	 @param blockID			This is a sample counter, you can assume that calls to this function
							will have blockIDs which are higher that the last call (until the maximum
							integer is reached).
	 @param channel			The channel index to process.
	 */
	virtual void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) = 0; 
	
	/// @{ <!-- end Rendering -->
	
	/// @name Current data and value
	/// @{
	
	inline int getBlockSize() const throw()				{ return uGenOutput.getBlockSize();		}
	inline UGenOutput* getOutputPtr() throw()			{ return &uGenOutput;					}
	inline UGenOutput& getOutputRef() throw()			{ return uGenOutput;					}
	inline float* getSampleData() throw()				{ return uGenOutput.getSampleData();	}
	inline const float* getSampleData() const throw()	{ return uGenOutput.getSampleData();	}
	virtual int getNumChannels() const throw()			{ return 1;								}
	virtual float getValue(const int channel) const throw();
	float poll(const int channel) throw();
	const UGen& getInput(const int index) throw();	
	
	/** Initialise the UGen output value.
	 
	 This helps getValue() return the correct (or at lease approximately correct) 
	 value even before processing begins. Oscillators can generally set their own value
	 here in their UGenInternal-inherited constructor. Processing-type UGen classes will 
	 probably need to set the values in the UGen class as the UGenInternal classes are
	 created (since they may contain multichannel inputs). */
	virtual void initValue(const float value) throw();
	
	
	/// @} <!-- end Current data and value -->
	
	
protected:		
	virtual UGenInternal* getChannel(const int channel) throw();
	int findMaxInputChannels() const throw();
	void getInternalChannels(const int channel, UGenInternal** internals) throw();
	UGenInternal* getInput(const int input, const int channel) throw();
	
	
	const short numInputs_;
	char rate;
	const bool ownsInputsPointer:1;
	bool isScheduledForDeletion:1;
	
	UGen * const inputs;		
	
	unsigned int lastBlockID;		// blockIDs count samples  and should be enough for about 27hrs @ 44.1kHz
									// a unsigned 64-bit int would be enough for nearly 5 billion days (or over 13 centuries)
	unsigned int blockIDtoBeDeletedAfter;
	UGenOutput uGenOutput;
	
private:
	UGenInternal (const UGenInternal&);
    const UGenInternal& operator= (const UGenInternal&);
		
#ifdef DEBUGREFCOUNT
	static int allocationCount;
#endif
	
};


class ProxyUGenInternal;

/** A UGenInternal base which owns other "proxy" channels (i.e., ProxyUGenInternal objects)
 
 @see ProxyUGenInternal
 @ingroup UGenInternals */
class ProxyOwnerUGenInternal : public UGenInternal
{
public:	
	
	/// @name Construction and destruction
	/// @{
	
	ProxyOwnerUGenInternal(const int numInputs, const int numProxies) throw();
	~ProxyOwnerUGenInternal(); // throw();
	void decrementRefCount() throw();
	void deleteIfOnlyMutualReferencesRemain() throw();
	
	/// @} <!-- end Construction and destruction -->
	
	
	int getNumProxies() const throw()		{	return numProxies_;		}
	int getNumChannels() const throw()		{	return numProxies_+1;	}
	UGenInternal* getProxy(const int index) throw();
	void prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID) throw();
	float* processBlockInternal(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
protected:
	const int numProxies_;
	UGenInternal** const proxies; // UGenInternal** proxies;  ??
};

/** A UGenInternal base is owned by a "proxy owner" (i.e., a ProxyOwnerUGenInternal object)
 
 @see ProxyOwnerUGenInternal
 @ingroup UGenInternals */
class ProxyUGenInternal : public UGenInternal
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ProxyUGenInternal(ProxyOwnerUGenInternal* owner, const int proxyChannel) throw();
	~ProxyUGenInternal() throw();
	void decrementRefCount() throw();
	
	/// @} <!-- end Construction and destruction -->
	
	bool isProxy() const throw()			{	return true;						}
	UGenInternal* getOwner() throw()		{	return owner_;						}
	int getProxyChannel() throw();
	int getNumChannels() const throw()		{	return owner_->getNumChannels();	}
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
private:
	ProxyOwnerUGenInternal* const owner_;
	int proxyChannel_;
};

/** A UGenInternal base that is "releasable" in the sense of a envelope which may be released.
 This means it should also be "stealable" i.e., told to stop immediately, or at least very quickly.
 This is normally for envelopes which may have a sustain section (e.g., ASR or EnvGen using a 
 sustaining Env).
 
 @ingroup UGenInternals */
class ReleasableUGenInternal : public UGenInternal
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ReleasableUGenInternal(const int numInputs) throw() 
	:	UGenInternal(numInputs), 
		shouldRelease_(false), 
		shouldSteal_(false),
		isReleasing_(false),
		isStealing_(false),
		isDone_(false) { }
	
	virtual void release() = 0;
	virtual void steal() = 0;
	
	/// @} <!-- end Construction and destruction -->
	
	/// @name Tests
	/// @{
	
	inline bool shouldRelease() const throw()	{ return shouldRelease_;	}
	inline bool shouldSteal() const throw()		{ return shouldSteal_;		}
	inline bool isReleasing() const throw()		{ return isReleasing_;		}
	inline bool isStealing() const throw()		{ return isStealing_;		}
	inline bool isDone() const throw()			{ return isDone_;			}
		
	/// @} <!-- end Tests -->
	
protected:
	inline void releaseInternal() throw()	
	{ 
		if(shouldRelease_ == false && shouldSteal_ != true)
		{
			UGenInternal::releaseInternal(); 
			shouldRelease_ = true; 
			release();
		}
	}
	inline void stealInternal() throw()	
	{
		if(shouldSteal_ == false)
		{
			shouldRelease_ = true;
			shouldSteal_ = true; 
			UGenInternal::stealInternal(); 
			steal();
		}
	}
	inline void setIsReleasing() throw()		{ if(shouldRelease_) isReleasing_ = true;			}
	inline void setIsStealing() throw()			{ if(shouldSteal_) isStealing_ = true;				}
	void setIsDone() throw()					{ isDone_ =	true;									}
	
	bool shouldRelease_;
	bool shouldSteal_;
	bool isReleasing_;
	bool isStealing_;
	bool isDone_;
};


#endif // UGEN_UGENINTERNAL_H
