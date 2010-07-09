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

#ifndef _UGEN_ugen_Lag_H_
#define _UGEN_ugen_Lag_H_


#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class LagUGenInternal : public UGenInternal
{
public:
	LagUGenInternal(UGen const& input, UGen const& lagTime) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void initValue(const float value) throw();	
	
	enum Inputs { Input, LagTime, NumInputs };
	
protected:	
	float b1, y1, currentLagTime;
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LagUGenInternal, (input, lagTime),
													(UGen const& input, UGen const& lagTime));

#define Lag_Docs	@param input		The input signal.					\
					@param lagTime		The 60 dB lag time in seconds.

/** Exponential lag UGen.
 
 A one pole filter where the coefficient is caculated from a 60 dB lag time. 
 This is the time required for the filter to converge to within 0.01 % of a value (60 dB). 
 This is useful for smoothing out control signals. Implements the formula:
 
 @f$ x_{0} = \left( y_{0} + \lambda \left( y_{1} - y_{0} \right) \right) @f$
 
 @see Decay
 
 @ingroup AllUGens FilterUGens */
DirectMulAddUGenDeclaration(Lag,	(input, lagTime),
									(input, lagTime, MulAdd_ArgsCall),
									(UGen const& input, UGen const& lagTime = 0.2), 
									(UGen const& input, UGen const& lagTime = 0.2, MulAdd_ArgsDeclare), 
							Lag_Docs COMMON_UGEN_DOCS MulAddArgs_Docs);


/** @ingroup UGenInternals */
class LagUDUGenInternal : public UGenInternal
{
public:
	LagUDUGenInternal(UGen const& input, UGen const& lagTimeUp, UGen const& lagTimeDown) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void initValue(const float value) throw();	
	
	enum Inputs { Input, LagTimeUp, LagTimeDown, NumInputs };
	
protected:	
	float b1u, b1d, y1, currentLagTimeUp, currentLagTimeDown;
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LagUDUGenInternal, 
								   (input, lagTimeUp, lagTimeDown),
								   (UGen const& input, UGen const& lagTimeUp, UGen const& lagTimeDown));

/** Exponential lag UGen with differnt up/down characteristics. 
 This is the same as the Lag UGen but has separate controls for rising or falling signals.
 This is quite useful for the manipulation of control signal ballistics.
 @ingroup AllUGens FilterUGens */
DirectMulAddUGenDeclaration(LagUD,	(input, lagTimeUp, lagTimeDown),
							(input, lagTimeUp, lagTimeDown, MulAdd_ArgsCall),
							(UGen const& input, UGen const& lagTimeUp = 0.2, UGen const& lagTimeDown = 0.2), 
							(UGen const& input, UGen const& lagTimeUp = 0.2, UGen const& lagTimeDown = 0.2, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);





#endif // _UGEN_ugen_Lag_H_
