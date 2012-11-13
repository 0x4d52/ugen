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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_ASR.h"


// now do a find/replace on "" here too to remove it 

// it's probably best to copy/paste the ASR_InputsWithTypesOnly and ASR_InputsNoTypes definitions here
// to help the readability in this cpp file

ASRUGenInternal::ASRUGenInternal(const float attackTime, const float sustainLevel, const float releaseTime, const UGen::DoneAction doneAction) throw()
:	ReleasableUGenInternal(NoInputs),
	attackTime_(attackTime),
	sustainLevel_(sustainLevel),
	releaseTime_(releaseTime),
	doneAction_(doneAction),
	currentValue(0.0),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{	
	setAttackSegment();
}

void ASRUGenInternal::prepareForBlock(const int /*actualBlockSize*/, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	senderUserData = userData;
	if(isDone()) sendDoneInternal();
}

void ASRUGenInternal::processBlock(bool& shouldDelete, const unsigned int /*blockID*/, const int /*channel*/) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	// AttackSegment
	if(currentSegment == AttackSegment)
	{
		if(attackTime_ <= 0.f) setSustainSegment();
		
		while(numSamplesToProcess && currentSegment == AttackSegment)
		{
			*outputSamples++ = (float)currentValue;
			currentValue += increment;
			
			if(currentValue >= sustainLevel_) setSustainSegment();
			
			--numSamplesToProcess;
		}
	}
	
	// SustainSegment
	while(numSamplesToProcess && currentSegment == SustainSegment)
	{
		*outputSamples++ = (float)currentValue;
		--numSamplesToProcess;
	}
	
	// ReleaseSegment
	while(numSamplesToProcess && currentSegment == ReleaseSegment)
	{
		*outputSamples++ = (float)currentValue;
		currentValue -= increment;
		
		if(currentValue <= 0.f) setEnvDone();
		
		--numSamplesToProcess;
	}
	
	// StealSegment
	if(currentSegment == StealSegment)
	{
		increment = currentValue / (float)numSamplesToProcess;
	
		while(numSamplesToProcess && currentSegment == StealSegment)
		{
			*outputSamples++ = (float)currentValue;
			currentValue -= increment;
			--numSamplesToProcess;
		}
		
		setEnvDone();
	}
	
	// clean up
	if(numSamplesToProcess > 0)
	{
		while(numSamplesToProcess) 
		{
			*outputSamples++ = 0.f;
			shouldDelete = shouldDelete ? true : shouldDeleteValue;
			--numSamplesToProcess;
		}
	}
	
}

void ASRUGenInternal::release() throw()
{
	setIsReleasing();
	setReleaseSegment();
}

void ASRUGenInternal::steal() throw()
{
	setIsStealing();
	setStealSegment();
}

ASR::ASR(const float attackTime, const float sustainLevel, const float releaseTime, const UGen::DoneAction doneAction) throw()
{
	ugen_assert(attackTime >= 0.f);
	ugen_assert(sustainLevel > 0.f);
	ugen_assert(releaseTime >= 0.f);
	
	initInternal(1);
	internalUGens[0] = new ASRUGenInternal(attackTime, sustainLevel, releaseTime, doneAction);
}

END_UGEN_NAMESPACE
