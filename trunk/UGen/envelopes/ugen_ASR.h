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

#ifndef _UGEN_ugen_ASR_H_
#define _UGEN_ugen_ASR_H_

#include "../core/ugen_UGen.h"

#define ASR_InputsWithTypesAndDefaults	const float attackTime, const float sustainLevel, const float releaseTime, const UGen::DoneAction doneAction = UGen::DoNothing
#define ASR_InputsWithTypesOnly			const float attackTime, const float sustainLevel, const float releaseTime, const UGen::DoneAction doneAction
#define ASR_InputsNoTypes				attackTime, sustainLevel, releaseTime, doneAction

/** @ingroup UGenInternals */
class ASRUGenInternal : public ReleasableUGenInternal
{
public:
	ASRUGenInternal(ASR_InputsWithTypesAndDefaults) throw();
	//UGenInternal* getChannel(const int channel) throw();									// necessary if there are input ugens which may have more than one channel
	//UGenInternal* getKr() throw();														// necessary if there is an actual control rate version (see below)
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();	
	void release() throw();
	void steal() throw();
	
	enum Inputs { NoInputs };
	
	enum EnvSegment
	{
		AttackSegment,
		SustainSegment,
		ReleaseSegment,
		StealSegment,
		EnvDone
	};
	
	inline void setAttackSegment() 
	{
		currentSegment = AttackSegment;
		increment = sustainLevel_ * UGen::getReciprocalSampleRate() / attackTime_;
	}
	
	inline void setSustainSegment() 
	{
		currentSegment = SustainSegment;
		currentValue = sustainLevel_;
	}
	
	inline void setReleaseSegment() 
	{
		currentSegment = ReleaseSegment;
		increment = currentValue * UGen::getReciprocalSampleRate() / releaseTime_;
		sendReleasing(releaseTime_);
	}
	
	inline void setStealSegment() 
	{
		currentSegment = StealSegment;
	}
	
	inline void setEnvDone() 
	{
		currentSegment = EnvDone;
		currentValue = 0.f;
		setIsDone();
	}
	
	
protected:
	const float attackTime_;
	const float sustainLevel_;
	const float releaseTime_;
	const UGen::DoneAction doneAction_; 
	EnvSegment currentSegment;
	double currentValue;
	double increment;
	const bool shouldDeleteValue;
};

//// uncomment this block if there is an actual control rate version of this UGen
//UGenInternalControlRateDeclaration(ASRUGenInternal, (ASR_InputsWithTypes), (ASR_InputsNoTypes));

/** Attack, sustain and release envelope.
 @ingroup AllUGens EnvUGens
 @see EnvGen */
UGenSublcassDeclaration(ASR, (ASR_InputsNoTypes), (ASR_InputsWithTypesAndDefaults), COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_ASR_H_
