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

#ifndef _UGEN_ugen_SOS_H_
#define _UGEN_ugen_SOS_H_


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"


#define SOS_InputsWithTypesAndDefaults	UGen const& input, UGen const& a0, UGen const& a1, UGen const& a2, UGen const& b1, UGen const& b2
#define SOS_InputsWithTypesOnly			UGen const& input, UGen const& a0, UGen const& a1, UGen const& a2, UGen const& b1, UGen const& b2
#define SOS_InputsNoTypes				input, a0, a1, a2, b1, b2
#define SOS_InputsEnum					Input, A0, A1, A2, B1, B2

/** @ingroup UGenInternals */
class SOSUGenInternal : public UGenInternal
{
public:
	SOSUGenInternal(SOS_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void initValue(const float value) throw();
	
	enum Inputs { SOS_InputsEnum, NumInputs };
	
protected:
	float y1, y2;
};

#define SOS_Docs	@param input	The input source to filter.		\
					@param a0		a0 coefficient.					\
					@param a1		a1 coefficient.					\
					@param a2		a2 coefficient.					\
					@param b1		b1 coefficient.					\
					@param b2		b2 coefficient.	

/** A standard second order filter section. 
 Filter coefficients are given directly rather than calculated for you.
 
 Formula is equivalent to:
 
 out(i) = (a0 * in(i)) + (a1 * in(i-1)) + (a2 * in(i-2)) + (b1 * out(i-1)) + (b2 * out(i-2)) 
 
 @ingroup AllUGens FilterUGens
 @see LPF, HPF, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ, BAllPass
 */
DirectMulAddUGenDeclaration(SOS,	(SOS_InputsNoTypes), 
									(SOS_InputsNoTypes, MulAdd_ArgsCall), 
									(SOS_InputsWithTypesAndDefaults), 
									(SOS_InputsWithTypesAndDefaults, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS SOS_Docs MulAddArgs_Docs);



#endif // _UGEN_ugen_SOS_H_
