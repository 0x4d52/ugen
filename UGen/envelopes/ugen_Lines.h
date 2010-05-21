// $Id: ugen_Lines.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/envelopes/ugen_Lines.h $

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

#ifndef UGEN_LINES_H
#define UGEN_LINES_H

#include "../core/ugen_UGen.h"

//////// Line //////////

/** @ingroup UGenInternals */
class LLineUGenInternal : public UGenInternal
{
public:
	LLineUGenInternal(const float start,	
					  const float end,
					  const float duration,
					  const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
protected:
	const float start_, end_, duration_;
	float currentValue;
	float increment;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};

/** @ingroup UGenInternals */
class LLineUGenInternalK : public LLineUGenInternal
{
public:
	LLineUGenInternalK(const float start,	
					   const float end,
					   const float duration,
					   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	UGenInternal* getKr() throw() { incrementRefCount(); return this; }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

#define LLine_Docs	@param	start		The starting value, this may be multichannel by				\
										storing the starting values in a single channel Buffer.		\
					@param	end			The ending value, this may be multichannel by				\
										storing the ending values in a single channel Buffer.		\
					@param	duration	The duration the line should take to ramp from the			\
										starting values to the ending value, this may be			\
										multichannel by	storing the durations in a single			\
										channel Buffer.												\
					@param	doneAction	The DoneAction to perform after the line has finished, the	\
										default is to delete the UGen graph.

/** Performs a linear line from start to end over a given duration.
	e.g.,
	@code
	UGen line = LLine::AR(B(0.0, 1.0), B(0.25, 0.7), 5);
	@endcode
	.. a two channel audio rate line over 5 seconds where the "left" channel
	ramps from 0.0 to 0.25 while the "right" channel ramps from 1.0 to 0.7.
	@ingroup AllUGens EnvUGens
	@see XLine, Linen, Env, EnvGen, ASR */
UGenSublcassDeclaration(LLine, (start, end, duration, doneAction), (Buffer const& start,	
																	Buffer const& end,
																	Buffer const& duration,
																	const UGen::DoneAction doneAction = UGen::DoNothing), 
								COMMON_UGEN_DOCS LLine_Docs);

/** Performs an exponential line from start to end over a given duration
 @ingroup AllUGens EnvUGens
 @see LLine, Linen, Env, EnvGen, ASR */
UGenSublcassDeclaration(XLine, (start, end, duration, doneAction), (Buffer const& start,	
																   Buffer const& end,
																   Buffer const& duration,
																   const UGen::DoneAction doneAction = UGen::DoNothing), 
								COMMON_UGEN_DOCS LLine_Docs);


//////// Linen //////////

#define Linen_InputsWithTypes	const float attackTime,								\
								const float sustainTime,							\
								const float releaseTime,							\
								const float sustainLevel,							\
								const UGen::DoneAction doneAction = UGen::DoNothing
#define Linen_InputsNoTypes		attackTime, sustainTime, releaseTime, sustainLevel, doneAction

/** @ingroup UGenInternals */
class LinenUGenInternal : public UGenInternal
{
public:
	LinenUGenInternal(Linen_InputsWithTypes) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum EnvSegment
	{
		AttackSegment,
		SustainSegment,
		ReleaseSegment,
		EnvDone
	};
	
protected:
	const float attackTime_, sustainTime_, releaseTime_, sustainLevel_;
	EnvSegment currentSegment;
	int numSustainSamplesRemaining;
	float currentValue;
	float increment;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};

/** @ingroup UGenInternals */
class LinenUGenInternalK : public LinenUGenInternal
{
public:
	LinenUGenInternalK(Linen_InputsWithTypes) throw();
	UGenInternal* getKr() throw() { incrementRefCount(); return this; }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** Trapezioid envlope. 
 @ingroup AllUGens EnvUGens */
UGenSublcassDeclaration(Linen, (Linen_InputsNoTypes), (Linen_InputsWithTypes), COMMON_UGEN_DOCS);

#undef Linen_InputsWithTypes
// redefine without any default args
#define Linen_InputsWithTypes	const float attackTime,								\
								const float sustainTime,							\
								const float releaseTime,							\
								const float sustainLevel,							\
								const UGen::DoneAction doneAction


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define LLine LLine()
#define Linen Linen()
#endif



#endif // UGEN_LINES_H