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

#include "ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_UGenInternal.h"
#include "ugen_UGen.h"
#include "ugen_UGenArray.h"
#include "../basics/ugen_ScalarUGens.h"


//=========================== UGenOutput ==================================

UGenOutput::UGenOutput() throw()
:	blockSize(UGen::getEstimatedBlockSize()),
	allocatedBlockSize(blockSize),
	block(blockSize <= 0 ? 0 : new float[blockSize]),
	usingExternalOutput(false),
	externalOutput(0)
{
	ugen_assert(blockSize > 0);
	initValue(0.f);
}

UGenOutput::~UGenOutput()
{
	if(usingExternalOutput == false)
		delete [] block;
	
	block = 0;
	blockSize = 0;
	allocatedBlockSize = 0;
	externalOutput = 0;
}

void UGenOutput::initValue(const float value) throw()
{
	if(block)
		block[blockSize-1] = value;
}

void UGenOutput::useExternalOutput(UGenOutput* externalOutputToUse)
{
	if(externalOutputToUse == 0)
	{
		float value = 0.f;
		
		if(block)
			value = block[blockSize-1];
		
		usingExternalOutput = false;
		blockSize = UGen::getEstimatedBlockSize();
		allocatedBlockSize = blockSize;
		block = new float[blockSize];
		externalOutput = 0;
		
		initValue(value);
	}
	else
	{
		float value = 0.f;
		
		if(block)
			value = block[blockSize-1];
		
		if(usingExternalOutput == false)
			delete [] block;
		
		usingExternalOutput = true;
		block = externalOutputToUse->block;
		blockSize = externalOutputToUse->blockSize;
		allocatedBlockSize = 0;
		externalOutput = externalOutputToUse;
		
		initValue(value);
	}
}

void UGenOutput::useExternalOutput(float* externalOutputToUse, const int externalBlockSize)
{
	ugen_assert(externalBlockSize > 0);
	
	if(externalOutputToUse == 0)
	{
		float value = 0.f;
		
		if(block)
			value = block[blockSize-1];
		
		usingExternalOutput = false;
		blockSize = UGen::getEstimatedBlockSize();
		allocatedBlockSize = blockSize;
		block = new float[blockSize];
		externalOutput = 0;
		
		initValue(value);
	}
	else
	{
		float value = 0.f;
		
		if(block)
			value = block[blockSize-1];
		
		if(usingExternalOutput == false)
			delete [] block;
		
		usingExternalOutput = true;
		block = externalOutputToUse;
		blockSize = externalBlockSize;
		allocatedBlockSize = 0;
		externalOutput = 0;
		
		initValue(value);
	}
	
}



//=========================== UGenInternal ==================================

UGenInternal::UGenInternal(const int numInputs) throw()
:	userData(0),
	numInputs_(numInputs),
	rate(AudioAndControlRate),
	ownsInputsPointer(true),
	isScheduledForDeletion(false),
	inputs(numInputs_ > 0 ? new UGen[numInputs_] : 0),
	lastBlockID(-1),
	blockIDtoBeDeletedAfter(0xFFFFFFFF)
{
	ugen_assert(numInputs >= 0);
}

UGenInternal::UGenInternal(UGen *mixInputToUse) throw()
:	userData(0),
	numInputs_(1),
	rate(AudioAndControlRate),
	ownsInputsPointer(false),
	isScheduledForDeletion(false),
	inputs(mixInputToUse),
	lastBlockID(-1),
	blockIDtoBeDeletedAfter(0xFFFFFFFF)
{
}

UGenInternal::~UGenInternal() //throw()
{
	if(ownsInputsPointer) 
		delete [] inputs;
}

UGenInternal* UGenInternal::getChannelInternal(const int channel) throw()
{
	return getChannel(channel);
}

// this assume that output channels is based only on input channel which is not always the case e.g., convolution]
//UGenInternal* UGenInternal::getChannelInternal(const int channel) throw()
//{
//	int numInternalChannels = findMaxInputChannels();
//	
//	if(channel < 0 || numInternalChannels < 2)
//	{
//		incrementRefCount();
//		return this;																		
//	}
//	else
//	{		
//		return getChannel(channel);
//						  
////		if(numInputs_ > 0)
////		{
////			UGenInternal* internals[numInputs_];
////			getInternalChannels(channel, internals);
////			return getChannel(channel, internals);
////		}
////		else
////		{
////			return getChannel(channel, 0);
////		}
//	}
//}

UGenInternal* UGenInternal::getChannel(const int /*channel*/) throw()
{
	// no need to copy as this is a single channel
	incrementRefCount();
	return this;
}

UGenInternal* UGenInternal::getKr() throw()
{
	// default is to return itself, subclasses should return a control rate version if applicable
	incrementRefCount();
	return this;
}


float* UGenInternal::processBlockInternal(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	if(blockID != lastBlockID)
	{
		processBlock(shouldDelete, blockID, channel);
		
		if(isScheduledForDeletion == false && shouldDelete == true)
		{
			isScheduledForDeletion = true;
			blockIDtoBeDeletedAfter = blockID;
		}
		
		lastBlockID = blockID;
	}
	
	return uGenOutput.getSampleData();
}

const UGen& UGenInternal::getInput(const int index) throw()
{
	if(index < 0 || index >= numInputs_) 
	{
		ugen_assertfalse;
		return UGen::getNull();
	}
	
	return inputs[index];
}

void UGenInternal::prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	ugen_assert(actualBlockSize > 0);
	
	if(blockID != lastBlockID)
	{
		uGenOutput.prepareForBlock(actualBlockSize);
		
		for(unsigned int i = 0; i < numInputs_; i++)
		{
			inputs[i].prepareForBlock(actualBlockSize, blockID, channel);
		}
		
		prepareForBlock(actualBlockSize, blockID, channel);
	}
}

void UGenInternal::releaseInternal() throw()
{
	for(int i = 0; i < numInputs_; i++)
	{
		inputs[i].release();
	}
}

void UGenInternal::stealInternal() throw()
{
	for(int i = 0; i < numInputs_; i++)
	{
		inputs[i].steal(false);
	}
}

int UGenInternal::findMaxInputChannels() const throw()
{
	int numChannels = 0;
	
	for(int i = 0; i < numInputs_; i++)
	{
		int inputChannels = inputs[i].getNumChannels();
		if(inputChannels > numChannels)
			numChannels = inputChannels;
	}
	
	return numChannels;
}

void UGenInternal::getInternalChannels(const int channel, UGenInternal** internals) throw()
{
	ugen_assert(internals != 0);
	
	for(int i = 0; i < numInputs_; i++)
	{
		internals[i] = getInput(i, channel);
	}
}

void UGenInternal::initValue(const float value) throw()
{
	uGenOutput.initValue(value);
}

float UGenInternal::getValue(const int channel) const throw()			
{ 
	const int blockSize = uGenOutput.getBlockSize();
	
	if(blockSize > 0)
	{
		const float *samples = uGenOutput.getSampleData();
		return samples[blockSize-1];
	}
	else
		return 0.f;
}

float UGenInternal::poll(const int channel) throw()
{
	// the difference will be that poll is going to check that the UGen has rendered!
	return getValue(channel);
}

UGenInternal* UGenInternal::getInput(const int inputIndex, const int channel) throw()
{
	ugen_assert(channel >= 0);
	
	if(inputIndex < 0 || inputIndex >= numInputs_) 
	{
		ugen_assertfalse;
		return 0;
	}
	
	UGen& input = inputs[inputIndex];
	return input.getInternalUGen(channel % input.getNumChannels());
}



//=========================== ProxyOwnerUGenInternal ==================================

ProxyOwnerUGenInternal::ProxyOwnerUGenInternal(const int numInputs, const int numProxies) throw()
:	UGenInternal(numInputs),
	numProxies_(numProxies < 0 ? 0 : numProxies), // numProxies_ shouldn't be less than 0 but MixArray was sending -1 when its input was empty
	proxies(new UGenInternal*[numProxies_+1])
{	
	proxies[0] = this;
	for(int i = 1; i <= numProxies_; i++)
	{
		proxies[i] = new ProxyUGenInternal(this, i);
	}
}

ProxyOwnerUGenInternal::~ProxyOwnerUGenInternal()// throw()
{
	delete [] proxies;
	//proxies = 0; //can't do this if proxies is **const
}

//#if defined(JUCE_VERSION) || defined(DOXYGEN)
//void ProxyOwnerUGenInternal::decrementRefCount() throw()
//{
//	atomicDecrement(refCount);
//	deleteIfOnlyMutualReferencesRemain();
//}
//#else
void ProxyOwnerUGenInternal::decrementRefCount()  throw()
{
	refCount--;
	deleteIfOnlyMutualReferencesRemain();
}
//#endif

void ProxyOwnerUGenInternal::deleteIfOnlyMutualReferencesRemain() throw()
{
	if(getRefCount() > numProxies_) return; 
	
	for(int i = 1 ; i <= numProxies_; i++)
	{
		if(proxies[i]->getRefCount() > 1) return;
	}
	
	for(int i = 1 ; i <= numProxies_; i++)
	{
		UGen::getDeleter()->deleteInternal(proxies[i]);
	}
	
	active = false;
	UGen::getDeleter()->deleteInternal(this);
}

UGenInternal* ProxyOwnerUGenInternal::getProxy(const int index) throw()
{
	// indices are between 0 and numProxies_ (0 is the owner)
	if(index < 0 || index > numProxies_) 
	{
		ugen_assertfalse;
		return 0;
	}
	
	proxies[index]->incrementRefCount();
	return proxies[index];
}

void ProxyOwnerUGenInternal::prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	ugen_assert(actualBlockSize > 0);
	
	if(blockID != lastBlockID)
	{
		UGenInternal::prepareForBlockInternal(actualBlockSize, blockID, channel);
		
		for(int i = 1; i <= numProxies_; i++)
		{
			// if the refCount is only 1 its block won't get 
			// prepared by the normal dsp call chain, check for <=1 just in case
			if(proxies[i]->getRefCount() <= 1)
				proxies[i]->prepareForBlockInternal(actualBlockSize, blockID, channel);
		}	
	}
}


float* ProxyOwnerUGenInternal::processBlockInternal(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	if(blockID != lastBlockID)
	{
		for(int i = 1; i <= numProxies_; i++)
		{
			// if the refCount is only 1 its block won't get 
			// processed by the normal dsp call chain, check for <=1 just in case
			if(proxies[i]->getRefCount() <= 1)
				proxies[i]->processBlockInternal(shouldDelete, blockID, channel);
		}

		UGenInternal::processBlockInternal(shouldDelete, blockID, channel);
	}
	
	return uGenOutput.getSampleData();
}

//=========================== ProxyUGenInternal ==================================

ProxyUGenInternal::ProxyUGenInternal(ProxyOwnerUGenInternal* owner, const int proxyChannel)	 throw()
:	UGenInternal(0),
	owner_(owner),
	proxyChannel_(proxyChannel)
{		
	ugen_assert(owner);
	
	owner_->incrementRefCount();
}

ProxyUGenInternal::~ProxyUGenInternal() throw()
{
	// not sure if I should be decrementing ref counts here, I thought I'd checked this though...
}

//#if defined(JUCE_VERSION) || defined(DOXYGEN)
//void ProxyUGenInternal::decrementRefCount() throw()
//{
//	ugen_assert(refCount > 0);
//	
//	atomicDecrement(refCount);
//	owner_->deleteIfOnlyMutualReferencesRemain();
//}
//#else
void ProxyUGenInternal::decrementRefCount() throw()
{
	ugen_assert(refCount > 0);
	
	refCount--;
	owner_->deleteIfOnlyMutualReferencesRemain();
}
//#endif

int ProxyUGenInternal::getProxyChannel() throw()					
{ 
	return proxyChannel_;	
}

void ProxyUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	ugen_assert(actualBlockSize > 0);
	
	if(blockID != lastBlockID)
	{
		if(owner_->getRefCount() <= owner_->getNumProxies())
			owner_->prepareForBlockInternal(actualBlockSize, blockID, channel);
	}
}

void ProxyUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	if(blockID != lastBlockID)
	{
		if(owner_->getRefCount() <= owner_->getNumProxies())
		owner_->processBlockInternal(shouldDelete, blockID, channel);
	}
}

DoneActionSender::DoneActionSender() throw()
:	senderUserData(0),
	isDone_(false),
	doneSent(false)
{
}

DoneActionSender::~DoneActionSender()
{
	// this is just in case the done message wasn't sent before this object was disposed
	if(isDone_)
	{
		sendDoneInternal();
	}
}

void DoneActionSender::addDoneActionReceiver(DoneActionReceiver* const receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	if(receivers.contains(receiver)) return;
	
	receivers <<= receiver;	
}

void DoneActionSender::removeDoneActionReceiver(DoneActionReceiver* const receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	
	receivers = receivers.removeItem(receiver);	
}

void DoneActionSender::setIsDone() throw()					
{ 
	isDone_ = true;
}

void DoneActionSender::sendDoneInternal() throw()
{
	if(!doneSent)
	{
		sendDone();
		doneSent = true;
	}
}

void DoneActionSender::sendDone() throw()
{
	const int size = receivers.size();
	for(int i = 0; i < size; i++)
	{
		receivers[i]->handleDone(senderUserData);
	}	
}

void DoneActionSender::sendReleasing(const double time) throw()
{
	const int size = receivers.size();
	for(int i = 0; i < size; i++)
	{
		receivers[i]->handleReleasing(senderUserData, time);
	}		
}

ReleasableUGenInternal::ReleasableUGenInternal(const int numInputs) throw() 
:	UGenInternal(numInputs), 
	shouldRelease_(false), 
	shouldSteal_(false),
	isReleasing_(false),
	isStealing_(false)
{ 
}

void ReleasableUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{
	senderUserData = userData;
	
	if(isDone()) 
	{	
		sendDoneInternal();
	}
}

void ReleasableUGenInternal::releaseInternal() throw()	
{ 
	if(shouldRelease_ == false && shouldSteal_ != true)
	{
		UGenInternal::releaseInternal(); 
		shouldRelease_ = true; 
		release();
	}
}

void ReleasableUGenInternal::stealInternal() throw()	
{
	if(shouldSteal_ == false)
	{
		shouldRelease_ = true;
		shouldSteal_ = true; 
		UGenInternal::stealInternal(); 
		steal();
	}
}

void ReleasableUGenInternal::setIsReleasing() throw()		
{ 
	if(shouldRelease_) 
	{
		isReleasing_ = true;	
	}
}

void ReleasableUGenInternal::setIsStealing() throw()			
{ 
	if(shouldSteal_) 
	{
		isStealing_ = true;	
	}
}


END_UGEN_NAMESPACE