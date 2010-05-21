// $Id: ugen_EnvGen.cpp 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/envelopes/ugen_EnvGen.cpp $

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

#include "ugen_EnvGen.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../core/ugen_Constants.h"



EnvGenUGenInternal::EnvGenUGenInternal(Env const& env, const UGen::DoneAction doneAction) throw()
:	ReleasableUGenInternal(0),
	env_(env),
	doneAction_(doneAction),
	currentValue(0.0),
	stepsUntilTarget(0),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	currentValue = env_.getLevels().getSampleUnchecked(0);
	setSegment(0, UGen::getSampleRate());
	
	initValue(currentValue);
}

UGenInternal* EnvGenUGenInternal::getKr() throw() 
{ 
	return new EnvGenUGenInternalK(env_, doneAction_); 
}

void EnvGenUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	if(isStealing() == false && shouldSteal() == true)
	{
		setIsStealing();
		const int numLevels = env_.getLevels().size();
		float stealValue = (float)currentValue;
		float targetValue = env_.getLevels().getSampleUnchecked(numLevels-1);
		float inc = (targetValue - stealValue) / (float)numSamplesToProcess;
		while(numSamplesToProcess)
		{
			*outputSamples++ = stealValue;
			stealValue += inc;
			--numSamplesToProcess;
		}
		shouldDelete = shouldDelete ? true : shouldDeleteValue;
		currentValue = targetValue;
		currentCurve = EnvCurve(EnvCurve::Empty);
		setIsDone();
	}
	else
	{
		if(isReleasing() == false && shouldRelease() == true)
		{
			int releaseNode = env_.getReleaseNode();
//			if(releaseNode < 0)
//			{
//				releaseNode = env_.getTimes().size() - 1;
//			}
//			
//			setSegment(releaseNode, UGen::getSampleRate());
			
			if(releaseNode >= 0) // try this, - don't release if releaseNode is -1
			{
				setSegment(releaseNode, UGen::getSampleRate());
			}
		}		
		
		while(numSamplesToProcess && isDone() == false)
		{
			int samplesThisTime = min(stepsUntilTarget, numSamplesToProcess);
			numSamplesToProcess -= samplesThisTime;
			stepsUntilTarget -= samplesThisTime;
			
			// select from different curves here..., use a different loop
			
			switch(currentCurve.getType())
			{
				case EnvCurve::Numerical:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						b1 *= grow;
						currentValue = a2 - b1;
						--samplesThisTime;
					}
					break;
				case EnvCurve::Linear:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						currentValue += grow;
						--samplesThisTime;
					}
					break;
				case EnvCurve::Exponential:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						currentValue *= grow;
						--samplesThisTime;
					}
					break;
				case EnvCurve::Sine:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						double y0 = b1 * y1 - y2; 
						currentValue = a2 - y0;
						y2 = y1; 
						y1 = y0;
						--samplesThisTime;
					}
					break;
				case EnvCurve::Welch:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						double y0 = b1 * y1 - y2; 
						currentValue = a2 + y0;
						y2 = y1; 
						y1 = y0;
						--samplesThisTime;
					}
					break;
				case EnvCurve::Empty:
				case EnvCurve::Step:
				default:
					while(samplesThisTime)
					{
						*outputSamples++ = (float)currentValue;
						--samplesThisTime;
					}
			}
			
			if(stepsUntilTarget <= 0)
			{
				if(setSegment(currentSegment + 1, UGen::getSampleRate()) == true)
				{
					shouldDelete = shouldDelete ? true : shouldDeleteValue;
					const int numLevels = env_.getLevels().size();
					currentValue = env_.getLevels().getSampleUnchecked(numLevels-1);
					currentCurve = EnvCurve(EnvCurve::Empty);
					setIsDone();
					goto exit;
				}
			}
		}
	}
		
exit:
	while(numSamplesToProcess--)
	{
		*outputSamples++ = (float)currentValue;
	}
}

void EnvGenUGenInternal::release() throw()
{
	
}

void EnvGenUGenInternal::steal() throw()
{
	
}

bool EnvGenUGenInternal::setSegment(const int segment, const double stepsPerSecond) throw()
{	
	ugen_assert(stepsPerSecond > 0.0);
	
	if(segment == env_.getReleaseNode())
	{	
		if(shouldRelease() == true)
		{
			currentSegment = segment;
			setIsReleasing();
		} 
		else
		{
			const int loopNode = env_.getLoopNode();
			if(loopNode < 0)
			{
				currentCurve = EnvCurve(EnvCurve::Empty);
				stepsUntilTarget = 0x7FFFFFFF; // very large !
				return false;
			}
			else
			{
				currentSegment = loopNode;
				currentValue = env_.getLevels().getSampleUnchecked(currentSegment);
			}
		}
	}
	else
	{
		currentSegment = segment;
	}
	
	const int numSegments = env_.getTimes().size();
	if(currentSegment >= numSegments) return true; // env done
	
	double targetTime = env_.getTimes().getSampleUnchecked(currentSegment);
	double targetValue = env_.getLevels().getSampleUnchecked(currentSegment + 1);
	
	stepsUntilTarget = (int)(stepsPerSecond * targetTime);
	if(stepsUntilTarget < 1) 
	{
		stepsUntilTarget = 1;
		currentCurve = EnvCurve(EnvCurve::Linear);
	}
	else
		currentCurve = env_.getCurves()[currentSegment];
	
	double w, curveValue;
	switch(currentCurve.getType())
	{
		case EnvCurve::Numerical:
			curveValue = currentCurve.getCurve();
			if(std::fabs(curveValue) > 0.001)
			{
				double a1 = (targetValue - currentValue) / (1.0 - std::exp(curveValue));	
				a2 = currentValue + a1;
				b1 = a1; 
				grow = std::exp(curveValue / (double)stepsUntilTarget);
				break;
			}
			else
			{	
				currentCurve = EnvCurve(EnvCurve::Linear);
				// fall through to the next case..
			}
		case EnvCurve::Linear:
			grow = (targetValue - currentValue) / (double)stepsUntilTarget;
			break;
		case EnvCurve::Exponential:
			grow = std::pow(targetValue / currentValue, 1.0 / (double)stepsUntilTarget);
			break;
		case EnvCurve::Sine:
			w = pi / (double)stepsUntilTarget;
			
			a2 = (targetValue + currentValue) * 0.5;
			b1 = 2.0 * std::cos(w);
			y1 = (targetValue - currentValue) * 0.5;
			y2 = y1 * std::sin(pi2 - w);
			currentValue = a2 - y1;
			
			break;
		case EnvCurve::Welch:
			w = pi2 / (double)stepsUntilTarget;
			
			b1 = 2.0 * std::cos(w);
			
			if (targetValue >= currentValue) 
			{
				a2 = currentValue;
				y1 = 0.;
				y2 = -std::sin(w) * (targetValue - currentValue);
			} 
			else 
			{
				a2 = targetValue;
				y1 = currentValue - targetValue;
				y2 = std::cos(w) * (currentValue - targetValue);
			}
			
			currentValue = a2 + y1;
			
			break;
		case EnvCurve::Empty:
		case EnvCurve::Step:
		default:
			currentValue = targetValue;
	}
	
	return false; // ready for next segment
}

EnvGenUGenInternalK::EnvGenUGenInternalK (Env const& env, const UGen::DoneAction doneAction) throw() 
:	EnvGenUGenInternal(env, doneAction)
{ 
	rate = ControlRate; 
	setSegment(0, UGen::getSampleRate() / UGen::getControlRateBlockSize());
} 

void EnvGenUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	int numKrSamples = blockPosition % krBlockSize;
	
	double prevValue = currentValue;
	
	if(isDone()) goto exit;

	while(numSamplesToProcess > 0)
	{
		prevValue = currentValue;
		if(numKrSamples == 0)
		{
			if(isStealing() == false && shouldSteal() == true)
			{
				setIsStealing();
				const int numLevels = env_.getLevels().size();
				float stealValue = (float)currentValue;
				float targetValue = env_.getLevels().getSampleUnchecked(numLevels-1);
				float inc = (targetValue - stealValue) / (float)numSamplesToProcess;
				while(numSamplesToProcess)
				{
					*outputSamples++ = stealValue;
					stealValue += inc;
					--numSamplesToProcess;
				}
				shouldDelete = shouldDelete ? true : shouldDeleteValue;
				currentValue = targetValue;
				currentCurve = EnvCurve(EnvCurve::Empty);
				setIsDone();
				return;
			}
			else
			{
				if(isReleasing() == false && shouldRelease() == true)
				{
					int releaseNode = env_.getReleaseNode();
//					if(releaseNode < 0)
//					{
//						releaseNode = env_.getTimes().size() - 1;
//					}
//					
//					setSegment(releaseNode, UGen::getSampleRate() / krBlockSize);
					
					if(releaseNode >= 0) // try this..
					{
						setSegment(releaseNode, UGen::getSampleRate() / krBlockSize);
					}
				}		
				
				--stepsUntilTarget;
									
				double y0;
				switch(currentCurve.getType())
				{
					case EnvCurve::Numerical:
						currentValue = a2 - b1;
						b1 *= grow;
						break;
					case EnvCurve::Linear:
						currentValue += grow;
						break;
					case EnvCurve::Exponential:
						currentValue *= grow;
						break;
					case EnvCurve::Sine:
						y0 = b1 * y1 - y2; 
						currentValue = a2 - y0;
						y2 = y1; y1 = y0;
						break;
					case EnvCurve::Welch:
						y0 = b1 * y1 - y2; 
						currentValue = a2 + y0;
						y2 = y1; y1 = y0;
						break;
					case EnvCurve::Empty:
					case EnvCurve::Step:
					default:
						;
				}
				
				if(stepsUntilTarget <= 0)
				{
					if(setSegment(currentSegment + 1, UGen::getSampleRate() / krBlockSize) == true)
					{
						shouldDelete = shouldDelete ? true : shouldDeleteValue;
						const int numLevels = env_.getLevels().size();
						currentValue = env_.getLevels().getSampleUnchecked(numLevels-1);
						currentCurve = EnvCurve(EnvCurve::Empty);
						goto exit;
					}
				}
				
			}
		}
		
		numKrSamples = krBlockSize - numKrSamples;
		blockPosition += numKrSamples;
		
		if(prevValue == currentValue)
		{
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = (float)prevValue;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		else
		{
			double valueSlope = (currentValue - prevValue) * UGen::getControlSlopeFactor();
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = (float)prevValue;
				prevValue += valueSlope;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
	}		
	
	return;

exit:
	
	if(prevValue == currentValue)
	{
		while(numSamplesToProcess > 0)
		{
			*outputSamples++ = (float)prevValue;
			--numSamplesToProcess;
		}
	}
	else
	{
		double valueSlope = (currentValue - prevValue) / (double)numSamplesToProcess;
		while(numSamplesToProcess > 0)
		{
			*outputSamples++ = (float)prevValue;
			prevValue += valueSlope;
			--numSamplesToProcess;
		}
	}
	
	setIsDone();
}

EnvGen::EnvGen(Env const& env, const UGen::DoneAction doneAction) throw()
{
	initInternal(1);
	internalUGens[0] = new EnvGenUGenInternal(env, doneAction);
}

END_UGEN_NAMESPACE
