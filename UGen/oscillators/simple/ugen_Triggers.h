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


#ifndef _UGEN_ugen_Triggers_H_
#define _UGEN_ugen_Triggers_H_

#include "../../core/ugen_UGen.h"

#ifdef Trig
#undef Trig
#endif

class SAHUGenInternal : public UGenInternal
{
public:
	SAHUGenInternal(UGen const& input, UGen const& trig) throw();
	UGenInternal* getChannel(const int channel) throw();					
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Trig, NumInputs };
	
protected:
	float currentValue;
	float lastTrig;
};

/** Sample and hold.
 Example:
 @code
	UGen trig = Dust::AR(5);
	UGen random = LFNoise1::AR(20).linexp(-1, 1, 100, 1000);
	UGen output = SinOsc::AR(SAH::AR(random, trig), 0, 0.3);
 @endcode
 @ingroup AllUGens */
UGenSublcassDeclarationNoDefault(SAH, (input, trig), (UGen const& input, UGen const& trig), COMMON_UGEN_DOCS);


class ToggleFFUGenInternal : public UGenInternal
{
public:
	ToggleFFUGenInternal(UGen const& trig) throw();
	UGenInternal* getChannel(const int channel) throw();					
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Trig, NumInputs };
	
protected:
	float currentValue;
	float lastTrig;
};

/** Toggling flip flop.
 Toggles between zero and one upon receiving a trigger.
 @code
	 UGen trig = Dust::AR(5);
	 UGen ff = ToggleFF::AR(trig) * 400 + 400;
	 UGen output = SinOsc::AR(ff.lag(0.01), 0, 0.3); 
 @endcode
 @ingroup AllUGens */
UGenSublcassDeclarationNoDefault(ToggleFF, (trig), (UGen const& trig), COMMON_UGEN_DOCS);




#endif // _UGEN_ugen_Triggers_H_
