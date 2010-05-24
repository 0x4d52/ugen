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

#ifndef _UGEN_ugen_Env_H_
#define _UGEN_ugen_Env_H_

#include "../core/ugen_SmartPointer.h"
#include "../buffers/ugen_Buffer.h"
#include "ugen_EnvCurve.h"





/**
 A specification for a segmented envelope.
 
 An Env can have any number of segments which can stop at a particular value or 
 loop several segments when sustaining. An Env can have several shapes for its segments.
 
 An Env is usually passed as an argument to the EnvGen UGen which performs the Env.
 
 */
class Env
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	/**
	 Creates an Env with supplied specification.
	 */
	Env(Buffer const& levels = Buffer(0.0, 1.0, 0.0),		/**<	A Buffer of levels. e.g. B(0.0, 1.0, 1.0, 0.0) you can also use the macro LL - List Levels. */
		Buffer const& times = Buffer(1.0, 1.0),				/**<	A Buffer of times. e.g., B(0.1, 0.8, 0.1) would have a total duration of 1sec. Macro LT - List Times can be used. 
																	There should be one fewer time than level. */
		EnvCurveList const& curves = EnvCurve(EnvCurve::Linear),	/**<	The shape of each segment. An EnvCurveList, with upto 
																			the same number of elements as the times Buffer. 
																			E.g., C(-0.5, 0.5, 0.0)*/
		const int releaseNode = -1,							/**<	The index of the level to sustain at until released. 
																	If this is -1 the envelope will not sustain and will have a fixed duration. */
		const int loopNode = -1								/**<	The index of the level to loop back to from the releaseNode.
																	If this is -1 the envelope will sustain at the relaseNode. */
	) throw();
	
//	Env(Buffer const& levels = 0.f,		/**<	A Buffer of levels. e.g. B(0.0, 1.0, 1.0, 0.0) you can also use the macro LL - List Levels. */
//		Buffer const& times = 1.f,		/**<	A Buffer of times. e.g., B(0.1, 0.8, 0.1) would have a total duration of 1sec. Macro LT - List Times can be used. 
//												There should be one fewer time than level. */
//		EnvCurve const& curve = EnvCurve::Linear,	/**<	 The shape of each segment. This is an EnvCurve (which can simply be a float) 
//															 setting each segment to the same shape. */
//		const int releaseNode = -1, 
//		const int loopNode = -1) throw();
	
	
	Env(Env const& copy) throw();
	Env& operator= (Env const& other) throw();
	
	~Env() throw();
	
	/** Creates a new envelope specification which has a trapezoidal shape.
	 @param attackTime		The duration of the attack portion.
	 @param sustainTime		the duration of the sustain portion.	 
	 @param releaseTime		The duration of the release portion.
	 @param sustainLevel	The level of the sustain portion.
	 @param curve			The curvature of the envelope.
	 @return				The Env envelope specification. */
	static Env linen(const double attackTime = 1.0, 
					 const double sustainTime = 2.0, 
					 const double releaseTime = 1.0, 
					 const double sustainLevel = 1.0,
					 EnvCurve const& curve = EnvCurve::Linear) throw();
	
	/**  Creates a new envelope specification which has a triangle shape.
	 @param duration	The duration of the envelope.
	 @param level		The peak level of the envelope.
	 @return			The Env envelope specification. */
	static Env triangle(const double duration = 1.0, 
						const double level = 1.0) throw();
	
	/**  Creates a new envelope specification which has a hanning window shape.
	 @param duration	The duration of the envelope.
	 @param level		The peak level of the envelope.
	 @return			The Env envelope specification. */	
	static Env sine(const double duration = 1.0, 
					const double level = 1.0) throw();
	
	/**  Creates a new envelope specification which (usually) has a percussive shape.
	 
	 @param attackTime	The duration of the attack portion.
	 @param releaseTime The duration of the release portion.
	 @param level		The peak level of the envelope.
	 @param curve		The curvature of the envelope.
	 @return			The Env envelope specification. */	
	static Env perc(const double attackTime = 0.01, 
					const double releaseTime = 1.0, 
					const double level = 1.0, 
					EnvCurve const& curve = -4.0) throw();
	
	/**  Creates a new envelope specification which is shaped like traditional analog attack-decay-sustain-release (adsr) envelopes.
	 
	 @param attackTime		The duration of the attack portion.
	 @param decayTime		The duration of the decay portion.
	 @param sustainLevel	The level of the sustain portion as a ratio of the peak level.
	 @param releaseTime		The duration of the release portion.
	 @param level			The peak level of the envelope.
	 @param curve			The curvature of the envelope.
	 @return				The Env envelope specification. */	
	static Env adsr(const double attackTime = 0.01, 
					const double decayTime = 0.3, 
					const double sustainLevel = 0.5, 
					const double releaseTime = 1.0, 
					const double level = 1.0, 
					EnvCurve const& curve = -4.0) throw();
	
	/**  Creates a new envelope specification which is shaped like traditional analog attack-sustain-release (asr) envelopes.
	 @param attackTime		The duration of the attack portion.
	 @param sustainLevel	The level of the sustain portion as a ratio of the peak level.
	 @param releaseTime		The duration of the release portion.
	 @param level			The peak level of the envelope.
	 @param curve			The curvature of the envelope.
	 @return				The Env envelope specification. */		
	static Env asr(const double attackTime = 0.01, 
				   const double sustainLevel = 1.0, 
				   const double releaseTime = 1.0, 
				   const double level = 1.0, 
				   EnvCurve const& curve = -4.0) throw();
	
	/// @} <!-- end Construction and destruction ------------------------------------------- -->

	/// @internal
	class EnvInternal : public SmartPointer
	{
	public:	
		EnvInternal(Buffer const& levels,
					Buffer const& times, 
					EnvCurveList const& curves, 
					const int releaseNode, 
					const int loopNode) throw();
		
		
		friend class Env;
		
	private:
		Buffer levels_;
		Buffer times_;
		EnvCurveList curves_;
		int releaseNode_;
		int loopNode_;
	};
	
	/// @name Envelope access and manipulation
	/// @{
	
	inline Buffer&			getTimes() const throw()	{ return internal->times_;	}
	inline Buffer&			getLevels() const throw()	{ return internal->levels_; }
	inline EnvCurveList&	getCurves() const throw()	{ return internal->curves_; }
	
	inline int getReleaseNode() const throw()	{ return internal->releaseNode_;	}
	inline int getLoopNode() const throw()		{ return internal->loopNode_;		}
	
	/** Returns the sum the time values in the envelope. */
	double duration() const throw();
	
	/** Returns a new envelope with the levels scaled by a constant. */
	Env levelScale(const double scale) const throw();
	
	/** Returns a new envelope with the levels offset by a constant. */
	Env levelBias(const double bias) const throw();
	
	/** Returns a new envelope with the time values scaled by a constant. */
	Env timeScale(const double scale) const throw();
	
	Env blend(Env const& other, const double fraction) const throw();
	Env simplifyCurves() const throw();
	
	/** Get the level of the Env ata given time.
	 This ignores loopNode and releaseNode if the are set. */
	float lookup(float time) const throw();
	
	/** Turn the Env into a table in a Buffer.
	 The new Buffer has a duration which is the sum of this Env's times.
	 This ignores loopNode and releaseNode if the are set. */
	operator Buffer () const throw();
	
	/** Write the Env into an exisiting Buffer.
	 This fits the Env into whatever size the Buffer is.
	 This ignores loopNode and releaseNode if the are set. */
	void writeToBuffer(Buffer& buffer, const int channel = 0) const throw();
	
	/// @} <!-- end Envelope access and manipulation ----------------------------- -->
	
private:
	EnvInternal* internal;
};


#endif // _UGEN_ugen_Env_H_
