// $Id: ugen_Env.cpp 1014 2010-05-20 18:33:07Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/envelopes/ugen_Env.cpp $

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

#include "ugen_Env.h"
#include "../core/ugen_UGen.h"
#include "../basics/ugen_InlineBinaryOps.h"



Env::EnvInternal::EnvInternal(Buffer const& levels, 
							  Buffer const& times, 
							  EnvCurveList const& curves, 
							  const int releaseNode, 
							  const int loopNode) throw()
:	levels_(levels),
	times_(times),
	curves_(curves),
	releaseNode_(releaseNode),
	loopNode_(loopNode)
{
}


Env::Env(Buffer const& levels, 
		 Buffer const& times, 
		 EnvCurveList const& curves, 
		 const int releaseNode, 
		 const int loopNode) throw()
{
	internal = new EnvInternal(levels, times, curves, releaseNode, loopNode);
}

//Env::Env(Buffer const& times, 
//		 Buffer const& levels, 
//		 EnvCurve const& curve, 
//		 const int releaseNode, 
//		 const int loopNode) throw()
//{
//	internal = new EnvInternal(levels, times, curve, releaseNode, loopNode);
//}

Env::Env(Env const& copy) throw()
:	internal(copy.internal)
{
	internal->incrementRefCount();
}

Env& Env::operator= (Env const& other) throw()
{
	if (this != &other)
    {	
		other.internal->incrementRefCount();
		internal->decrementRefCount();
		internal = other.internal;
    }
	
    return *this;
}

Env::~Env() throw()
{
	internal->decrementRefCount();
}

double Env::duration() const throw()
{
	return internal->times_.sum();
}

Env Env::levelScale(const double scale) const throw()
{
	return Env(internal->levels_ * scale,
			   internal->times_, 
			   internal->curves_,
			   internal->releaseNode_,
			   internal->loopNode_);
}

Env Env::levelBias(const double bias) const throw()
{
	return Env(internal->levels_ + bias,
			   internal->times_, 
			   internal->curves_,
			   internal->releaseNode_,
			   internal->loopNode_);
}

Env Env::timeScale(const double scale) const throw()
{
	ugen_assert(scale > 0.0);
	
	return Env(internal->levels_,
			   internal->times_ * scale, 
			   internal->curves_,
			   internal->releaseNode_,
			   internal->loopNode_);
}

Env Env::blend(Env const& other, const double fraction) const throw()
{
	ugen_assert(fraction >= 0.0 && fraction <= 1.0);

	return Env(internal->levels_.blend(other.internal->levels_, fraction),
			   internal->times_.blend(other.internal->times_, fraction),
			   internal->curves_,
			   internal->releaseNode_,
			   internal->loopNode_);
}

Env Env::simplifyCurves() const throw()
{
	// todo:
	// this will remove any unnecessary curves if they are all linear or with an abs value < 0.001
	return *this;
}

float Env::lookup(float time) const throw()
{
	const int numTimes = getTimes().size();
	const int numLevels = getLevels().size();
	const int lastLevel = numLevels-1;
	
	ugen_assert(numTimes == lastLevel);
	
	if(numLevels < 1) return 0.f;
	if(time <= 0.f || numTimes == 0) return getLevels().getSampleUnchecked(0);
	
	float lastTime = 0.f;
	float stageTime = 0.f;
	int stageIndex = 0;
	
	while(stageTime < time && stageIndex < numTimes)
	{
		lastTime = stageTime;
		stageTime += getTimes().getSampleUnchecked(stageIndex);
		stageIndex++;
	}
	
	if(stageIndex > numTimes) return getLevels().getSampleUnchecked(lastLevel);
		
	float level0 = getLevels().getSampleUnchecked(stageIndex-1);
	float level1 = getLevels().getSampleUnchecked(stageIndex);
	
	EnvCurve curve = getCurves()[stageIndex-1];
	EnvCurve::CurveType type = curve.getType();
	float curveValue = curve.getCurve();
	
	if((lastTime - stageTime)==0.f)
	{
	   return level1;
	}
	else if(type == EnvCurve::Linear)
	{
		return linlin(time, lastTime, stageTime, level0, level1);
	}
	else if(type == EnvCurve::Numerical)
	{
		if(abs(curveValue) <= 0.001)
		{
			return linlin(time, lastTime, stageTime, level0, level1);
		}
		else
		{			
			float pos = (time-lastTime) / (stageTime-lastTime);
			float denom = 1.f - std::exp(curveValue);
			float numer = 1.f - std::exp(pos * curveValue);
			return level0 + (level1 - level0) * (numer/denom);
		}
	}
	else if(type == EnvCurve::Sine)
	{
		return linsin(time, lastTime, stageTime, level0, level1);
	}
	else if(type == EnvCurve::Exponential)
	{
		return linexp(time, lastTime, stageTime, level0, level1);
	}
	else if(type == EnvCurve::Welch)
	{
		return linwelch(time, lastTime, stageTime, level0, level1);
	}
	else
	{
		// Empty or Step
		return level1;
	}
}

Env::operator Buffer () const throw()
{
	float duration = getTimes().sum();
	int size = UGen::getSampleRate() * duration;
	Buffer buffer = BufferSpec(size, 1, false);
	float *bufferSamples = buffer.getData();
	
	double time = 0.0;
	for(int i = 0; i < size; i++, time += UGen::getReciprocalSampleRate())
	{
		*bufferSamples++ = lookup(time);
	}
	
	return buffer;
}

void Env::writeToBuffer(Buffer& buffer, const int channel) const throw()
{
	double duration = getTimes().sum();
	int size = buffer.size();
	
	ugen_assert(size > 0);
	ugen_assert(buffer.getNumChannels() > 0);
	
	float *bufferSamples = buffer.getData(channel % buffer.getNumChannels());
	
	double timeInc = duration / size;
	double time = 0.0;
	for(int i = 0; i < size; i++, time += timeInc)
	{
		*bufferSamples++ = lookup(time);
	}
}

Env Env::linen(const double attackTime, 
			   const double sustainTime, 
			   const double releaseTime, 
			   const double sustainLevel,
			   EnvCurve const& curve) throw()
{
	ugen_assert(attackTime >= 0.0);
	ugen_assert(sustainTime >= 0.0);
	ugen_assert(releaseTime >= 0.0);
	ugen_assert(sustainLevel >= 0.0);
	ugen_assert((attackTime + sustainTime + releaseTime) > 0.0);
	
	return Env(Buffer(0.0, sustainLevel, sustainLevel, 0.0), 
			   Buffer(attackTime, sustainTime, releaseTime),
			   curve);
}

Env Env::triangle(const double duration, 
				  const double level) throw()
{
	ugen_assert(duration > 0.0);
	ugen_assert(level > 0.0);

	const double durationHalved = duration * 0.5f;
	return Env(Buffer(0.0, level, 0.0), 
			   Buffer(durationHalved, durationHalved));
}

Env Env::sine(const double duration, 
			  const double level) throw()
{
	ugen_assert(duration > 0.0);
	ugen_assert(level > 0.0);
	
	const double durationHalved = duration * 0.5f;
	return Env(Buffer(0.0, level, 0.0), 
			   Buffer(durationHalved, durationHalved),
			   EnvCurve::Sine);	
}

Env Env::perc(const double attackTime, 
			  const double releaseTime, 
			  const double level, 
			  EnvCurve const& curve) throw()
{
	ugen_assert(attackTime >= 0.0);
	ugen_assert(releaseTime >= 0.0);
	ugen_assert(level > 0.0);
	ugen_assert((attackTime + releaseTime) > 0.0);
	
	return Env(Buffer(0.0, level, 0.0), 
			   Buffer(attackTime, releaseTime),
			   curve);	
}

Env Env::adsr(const double attackTime, 
			  const double decayTime, 
			  const double sustainLevel, 
			  const double releaseTime,
			  const double level, 
			  EnvCurve const& curve) throw()
{
	ugen_assert(attackTime >= 0.0);
	ugen_assert(decayTime >= 0.0);
	ugen_assert(sustainLevel >= 0.0);
	ugen_assert(releaseTime >= 0.0);
	ugen_assert(level > 0.0);
	ugen_assert((attackTime + decayTime + releaseTime) > 0.0);
	
	return Env(Buffer(0.0, level, (level * sustainLevel), 0.0), 
			   Buffer(attackTime, decayTime, releaseTime),
			   curve, 2);
}

Env Env::asr(const double attackTime, 
			 const double sustainLevel, 
			 const double releaseTime, 
			 const double level, 
			 EnvCurve const& curve) throw()
{
	ugen_assert(attackTime >= 0.0);
	ugen_assert(sustainLevel >= 0.0);
	ugen_assert(releaseTime >= 0.0);
	ugen_assert(level > 0.0);
	ugen_assert((attackTime + releaseTime) > 0.0);
	
	return Env(Buffer(0.0, (level * sustainLevel), 0.0), 
			   Buffer(attackTime, releaseTime),
			   curve, 1);
}


END_UGEN_NAMESPACE
