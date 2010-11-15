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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Lines.h"
#include "ugen_EnvGen.h"
#include "../basics/ugen_Thru.h"

//=============================== LLine ======================================


LLineUGenInternal::LLineUGenInternal(const float start, const float end, const float duration, const UGen::DoneAction doneAction) throw()
:	UGenInternal(0),
	start_(start), 
	end_(end), 
	duration_(duration),
	currentValue(start_), 
	increment((end - start) * UGen::getReciprocalSampleRate() / duration),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	// assert if duration is negative?
	
	initValue(currentValue);
}

UGenInternal* LLineUGenInternal::getKr() throw()
{ 
	return new LLineUGenInternalK(start_, end_, duration_, doneAction_); 
}


void LLineUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	if(increment > 0.f) {
		while(numSamplesToProcess--)
		{
			// perhaps the increment should happen after the first sample output?
			// this is they way I'm going to do the Linen class below
			//currentValue += increment;
			
			if(currentValue >= end_) {
				*outputSamples++ = end_;
				while(numSamplesToProcess--)  {
					*outputSamples++ = end_;
				}
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				break;
			}
			else
			{
				*outputSamples++ = currentValue;
				currentValue += increment;
			}
		}
	} else {
		while(numSamplesToProcess--)
		{
			//currentValue += increment;
			
			if(currentValue <= end_) {
				*outputSamples++ = end_;
				while(numSamplesToProcess--)  {
					*outputSamples++ = end_;
				}
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				break;
			}
			else
			{
				*outputSamples++ = currentValue;
				currentValue += increment;
			}
		}
	}
}

LLineUGenInternalK::LLineUGenInternalK(const float start, const float end, const float duration, const UGen::DoneAction doneAction) throw()
:	LLineUGenInternal(start, end, duration, doneAction)
{
	rate = ControlRate;
}

void LLineUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	const double krIncrement = increment * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = currentValue;
		
		if(numKrSamples == 0 && nextValue != end_)
		{
			nextValue += krIncrement;
			
			if((increment > 0.0 && nextValue >= end_) || 
			   (increment < 0.0 && nextValue <= end_))
			{
				nextValue = end_;
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
			}
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition += numKrSamples;
		
		
		if(nextValue == currentValue)
		{
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = nextValue;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		else
		{
			float valueSlope = (nextValue - currentValue) / (float)UGen::getControlRateBlockSize();
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = currentValue;
				currentValue += valueSlope;
				--numSamplesToProcess;
				--numKrSamples;
			}
			
			currentValue = nextValue;
		}
	}		
}


LLine::LLine(Buffer const& start,	
			 Buffer const& end,
			 Buffer const& duration,
			 const UGen::DoneAction doneAction) throw()
{
	Buffer inputs[] = { start, end, duration };
	const int numInputChannels = findMaxInputSizes(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		ugen_assert(duration.wrapAt(i) > 0.f);
		
		internalUGens[i] = new LLineUGenInternal(start.wrapAt(i), 
												 end.wrapAt(i), 
												 duration.wrapAt(i), 
												 doneAction);
	}
}

#ifndef UGEN_NOEXTGPL

XLine::XLine(Buffer const& startBuffer,	
			 Buffer const& endBuffer,
			 Buffer const& durationBuffer,
			 const UGen::DoneAction doneAction) throw()
{	
	Buffer inputs[] = { startBuffer, endBuffer, durationBuffer };
	const int numInputChannels = findMaxInputSizes(numElementsInArray(inputs), inputs);
	initInternal(numInputChannels);
	
	for(int i = 0; i < numInternalUGens; i++)
	{		
		const float start = startBuffer.wrapAt(i);
		const float end = endBuffer.wrapAt(i);
		const float duration = durationBuffer.wrapAt(i);
		
		ugen_assert(duration > 0.f);		
		
		UGen xline;
		
		if((start > 0.f && end > 0.f) || (start < 0.f && end < 0.f))
		{
			// real exponential
			Env env = Env(Buffer((double)start, (double)end), duration, EnvCurve::Exponential);	
			xline = EnvGen::AR(env, doneAction);
		}
		else
		{		
			// pseudo exponential where start or end are zero or the line crosses zero
			
			const float epsilon = 0.0001f;
			
			if(start < 0.f && end > 0.f) {
				Env env = Env(Buffer((double)epsilon, (double)(end - start + epsilon)), duration, EnvCurve::Exponential);	
				xline = EnvGen::AR(env, doneAction) - (epsilon - start);
			}
			else if(start > 0.f && end < 0.f) {
				Env env = Env(Buffer((double)(start - end + epsilon), (double)epsilon), duration, EnvCurve::Exponential);	
				xline = EnvGen::AR(env, doneAction) - (epsilon - end);
			}
			else if(start == 0.f) {
				if(end > 0.f) {
					Env env = Env(Buffer((double)(start + epsilon), (double)(end + epsilon)), duration, EnvCurve::Exponential);	
					xline = EnvGen::AR(env, doneAction) - epsilon;
				} else {
					Env env = Env(Buffer((double)(start - epsilon), (double)(end - epsilon)), duration, EnvCurve::Exponential);	
					xline = EnvGen::AR(env, doneAction) + epsilon;
				}
			}
			else if(end == 0.f) {
				if(start > 0.f) {
					Env env = Env(Buffer((double)(start + epsilon), (double)(end + epsilon)), duration, EnvCurve::Exponential);	
					xline = EnvGen::AR(env, doneAction) - epsilon;
				} else {
					Env env = Env(Buffer((double)(start - epsilon), (double)(end - epsilon)), duration, EnvCurve::Exponential);	
					xline = EnvGen::AR(env, doneAction) + epsilon;
				}
			}
		}
		
		internalUGens[i] = new ThruUGenInternal(xline);
	}
}

#endif // gpl


//=============================== Linen ======================================


LinenUGenInternal::LinenUGenInternal(const float attackTime, 
									 const float sustainTime, 
									 const float releaseTime, 
									 const float sustainLevel, 
									 const UGen::DoneAction doneAction) throw()
:	UGenInternal(0),
	attackTime_(attackTime), 
	sustainTime_(sustainTime), 
	releaseTime_(releaseTime),
	sustainLevel_(sustainLevel),
	currentSegment(AttackSegment),
	numSustainSamplesRemaining(sustainTime_ * UGen::getSampleRate()),
	currentValue(0.0), 
	increment(sustainLevel * UGen::getReciprocalSampleRate() / attackTime_),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	// assert if duration is negative?
}

UGenInternal* LinenUGenInternal::getKr() throw()
{ 
	return new LinenUGenInternalK(attackTime_, sustainTime_, releaseTime_, sustainLevel_, doneAction_); 
}


void LinenUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	// fall through each of these switch..case (purposefully no breaks)
	switch (currentSegment) 
	{
		case AttackSegment:
			if(attackTime_ <= 0.f)
			{
				currentValue = sustainLevel_;
				if(sustainTime_ <= 0.f)
				{
					if(releaseTime_ <= 0.f)
						currentSegment = EnvDone;
					else
						currentSegment = ReleaseSegment;
				}
				else
					currentSegment = SustainSegment;
			}
				
			while(numSamplesToProcess && currentSegment == AttackSegment)
			{
				*outputSamples++ = currentValue;
				currentValue += increment;
				
				if(currentValue >= sustainLevel_) {
					currentValue = sustainLevel_;
					if(sustainTime_ <= 0.f)
						currentSegment = ReleaseSegment;
					else
						currentSegment = SustainSegment;
				}
				
				--numSamplesToProcess;
			}

		case SustainSegment:
			while(numSamplesToProcess && currentSegment == SustainSegment)
			{
				*outputSamples++ = currentValue;

				if(--numSustainSamplesRemaining == 0) {
					if(releaseTime_ < 0.f)
						currentSegment = EnvDone;
					else
					{
						currentSegment = ReleaseSegment;
						increment = sustainLevel_ * UGen::getReciprocalSampleRate() / releaseTime_;
					}
				}
				
				--numSamplesToProcess;
			}
			
		case ReleaseSegment:
			while(numSamplesToProcess && currentSegment == ReleaseSegment)
			{
				*outputSamples++ = currentValue;
				currentValue -= increment;
				
				if(currentValue <= 0.f) {
					currentValue = 0.f;
					currentSegment = EnvDone;
				}
				
				--numSamplesToProcess;
			}
			
		case EnvDone:
		default:
			while(numSamplesToProcess) 
			{
				*outputSamples++ = 0.f;
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				--numSamplesToProcess;
			}
			
	}
	
}

LinenUGenInternalK::LinenUGenInternalK(const float attackTime, 
									   const float sustainTime, 
									   const float releaseTime, 
									   const float sustainLevel, 
									   const UGen::DoneAction doneAction) throw()
:	LinenUGenInternal(attackTime, sustainTime, releaseTime, sustainLevel, doneAction)
{
	rate = ControlRate;
}

void LinenUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	double krIncrement = increment * krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		if(numKrSamples == 0)
		{
			switch (currentSegment) 
			{
				case AttackSegment:
					currentValue += krIncrement;
					if(currentValue >= sustainLevel_)
					{
						currentValue = sustainLevel_;
						currentSegment = SustainSegment;
					}
					break;
				case SustainSegment:
					numSustainSamplesRemaining -= krBlockSize;
					if(numSustainSamplesRemaining <= 0)
					{
						currentSegment = ReleaseSegment;
						increment = sustainLevel_ * UGen::getReciprocalSampleRate() / releaseTime_;
						krIncrement = increment * krBlockSize;
					}
					break;
				case ReleaseSegment:
					currentValue -= krIncrement;
					if(currentValue <= 0.f)
					{
						currentValue = 0.f;
						currentSegment = EnvDone;
						
						shouldDelete = shouldDelete ? true : shouldDeleteValue;
					}
					break;
				case EnvDone:
				default:
					currentValue = 0.f;
					break;
			}
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition += numKrSamples;
		
		while(numSamplesToProcess && numKrSamples)
		{
			*outputSamples++ = currentValue;
			--numSamplesToProcess;
			--numKrSamples;
		}
	}		
}


Linen::Linen(const float attackTime, 
			 const float sustainTime, 
			 const float releaseTime, 
			 const float sustainLevel, 
			 const UGen::DoneAction doneAction) throw()
{
	ugen_assert(attackTime >= 0.f);
	ugen_assert(sustainTime >= 0.f);
	ugen_assert(releaseTime >= 0.f);
	ugen_assert(sustainLevel > 0.f);
	
	initInternal(1);
	internalUGens[0] = new LinenUGenInternal(attackTime, sustainTime, releaseTime, sustainLevel, doneAction);
}




END_UGEN_NAMESPACE
