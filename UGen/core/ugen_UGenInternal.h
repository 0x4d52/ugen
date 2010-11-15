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

#ifndef UGEN_UGENINTERNAL_H
#define UGEN_UGENINTERNAL_H


#include "ugen_Arrays.h"

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
	
	inline int getBlockSize() const						{ return blockSize;			}
	inline float* getSampleData() const					{ return block;				}
	inline void zeroAllData()							{ memset(block, 0, blockSize * sizeof(float)); }
	void initValue(const float value) throw();
	
	void useExternalOutput(UGenOutput* externalOutputToUse);
	void useExternalOutput(float* externalOutputToUse, const int externalBlockSize);
	
private:
	int blockSize;
	int allocatedBlockSize;
	float *block;
	bool usingExternalOutput:1;
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
	
	/** User data passed down from the enclosing UGen.
	 Only valid after UGen::prepareForBlock() has been called for a particular block. */
	int userData;
	
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

//	/// @name Memory
//	/// @{
//	
//	virtual int getMemoryUsage() = 0;
//	
//	/// @} <!-- end Memory -->
	
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

class DoneActionReceiver;
typedef ObjectArray<DoneActionReceiver*> DoneActionReceiverArray;

/** This allows some UGens to send messages to indicate they are finsihed. */
class DoneActionSender
{
public:
	DoneActionSender() throw();
	virtual ~DoneActionSender();
	
	void addDoneActionReceiver(DoneActionReceiver* const receiver) throw();
	void removeDoneActionReceiver(DoneActionReceiver* const receiver) throw();
	void sendDoneInternal() throw();
	void sendReleasing(const double time) throw();
	void setIsDone() throw();
	inline bool isDoneSent() const throw() { return doneSent; }
	inline bool isDone() const throw() { return isDone_; }
	
	int senderUserData;
	
private:
	void sendDone() throw();
	
	DoneActionReceiverArray receivers;
	bool isDone_ : 1;
	bool doneSent : 1;
};

/** Subclasses of this receive UGen done action message. */
class DoneActionReceiver
{
public:
	DoneActionReceiver() throw() {}
	virtual ~DoneActionReceiver() {}
	
	/** This must be implmented, sent when a UGen has finished. 
	 It should be sent safely after all processing for a particualr block of
	 audio is complete, in fact it is sent just before the processing of the NEXT block. 
	 Generally this is called from the audio callback thread. */
	virtual void handleDone(const int senderUserData) = 0;
	
	/** This is optional, sent when a UGen is releasing or fading out naturally.
	 NB handleDone() must be implemented too since events which might normally
	 send a "releasing" message will not if they are being stolen instead. You can then pick these
	 up and clean up in handleDone(). This may be called on any thread. 
	 @param time	This will be the estimated time for the release to complete. For example when
					EnvGen sends this message it sends the duration of the last envelope segment.
					If this value is 0 (or less) assume that it was not possible to provide
					an estimated release time. */
	virtual void handleReleasing(const int senderUserData, const double time) { (void)senderUserData; (void)time; };
	
	/** This saves having to get the pointer to a DoneActionReceiver object, it will be casted automatically. */
	operator DoneActionReceiver*() throw() { return this; }
};


/** A UGenInternal base that is "releasable" in the sense of a envelope which may be released.
 This means it should also be "stealable" i.e., told to stop immediately, or at least very quickly.
 This is normally for envelopes which may have a sustain section (e.g., ASR or EnvGen using a 
 sustaining Env).
 
 @ingroup UGenInternals */
class ReleasableUGenInternal :	public UGenInternal,
								public DoneActionSender
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ReleasableUGenInternal(const int numInputs) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw();

	virtual void release() = 0;
	virtual void steal() = 0;
	
	/// @} <!-- end Construction and destruction -->
	
	/// @name Tests
	/// @{
	
	inline bool shouldRelease() const throw()	{ return shouldRelease_;	}
	inline bool shouldSteal() const throw()		{ return shouldSteal_;		}
	inline bool isReleasing() const throw()		{ return isReleasing_;		}
	inline bool isStealing() const throw()		{ return isStealing_;		}
		
	/// @} <!-- end Tests -->
	
protected:
	void releaseInternal() throw();
	void stealInternal() throw();
	void setIsReleasing() throw();
	void setIsStealing() throw();
	
	bool shouldRelease_ : 1;
	bool shouldSteal_ : 1;
	bool isReleasing_ : 1;
	bool isStealing_ : 1;
};

/** Added to UGenInternal classes that can seek a particular point in time.*/
class Seekable
{
public:
	Seekable() throw() {}
	virtual ~Seekable() {}
	/** Get the maximum duration of the seekable.
	 The units will be dependent on the UGenInternal in question. 
	 For longer sounds as sound files it is likely to be in seconds. 
	 For wavetables it will always be 1.0 */
	virtual double getDuration() const = 0;
	/** Get the current position of the seekable.
	 The units will be dependent on the UGenInternal in question. 
	 For longer sounds as sound files it is likely to be in seconds. 
	 For wavetables it will 0...1 */	
	virtual double getPosition() const = 0;
	/** Set the current position of the seekable.
	 The units will be dependent on the UGenInternal in question. 
	 For longer sounds as sound files it is likely to be in seconds. 
	 For wavetables it will 0...1 */		
	virtual void setPosition(const double newPosition) = 0;
};


#endif // UGEN_UGENINTERNAL_H
