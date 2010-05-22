// $Id: ugen_Decay.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/filters/control/ugen_Decay.h $

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

#ifndef _UGEN_ugen_Decay_H_
#define _UGEN_ugen_Decay_H_


#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class DecayUGenInternal : public UGenInternal
{
public:
	DecayUGenInternal(UGen const& input, UGen const& DecayTime) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void initValue(const float value) throw();	

	enum Inputs { Input, DecayTime, NumInputs };
	
protected:	
	float b1, y1, currentDecayTime;
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(DecayUGenInternal, 
								   (input, decayTime),
								   (UGen const& input, UGen const& decayTime));

#define Decay_Docs	@param input		The input signal.					\
					@param decayTime	The 60 dB decay time in seconds.

/** Exponential decay UGen.
 Integrates an input signal with a leak. The coefficient is caculated from a 60 dB 
 decay time. This is the time required for the integrator to lose 99.9 % of its value or -60dB. 
 This is useful for exponential decaying envelopes triggered by impulses. Implements the formula:
 
 @f$ x_{0} = y_{0} + \left( \lambda x_{-1} \right) @f$
 
 @see Lag
 
 @ingroup AllUGens FilterUGens */
DirectMulAddUGenDeclaration(Decay,	(input, decayTime),
									(input, decayTime, MulAdd_ArgsCall),
									(UGen const& input, UGen const& decayTime = 0.2), 
									(UGen const& input, UGen const& decayTime = 0.2, MulAdd_ArgsDeclare), 
							Decay_Docs COMMON_UGEN_DOCS MulAddArgs_Docs);


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Decay Decay()
#endif


#endif // _UGEN_ugen_Decay_H_
