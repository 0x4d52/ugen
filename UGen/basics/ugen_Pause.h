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

#ifndef _UGEN_ugen_Pause_H_
#define _UGEN_ugen_Pause_H_

// don't forget to add this header file to UGen.h

#include "../core/ugen_UGen.h"

#define Pause_InputsWithTypesAndDefaults	UGen const& input, UGen const& level = 0.f
#define Pause_InputsWithTypesOnly			UGen const& input, UGen const& level
#define Pause_InputsNoTypes					input, level

/** A UGenInternal which saves processing power when an amlitude value is zero.
	@ingroup UGenInternals */
class PauseUGenInternal : public UGenInternal
{
public:
	PauseUGenInternal(Pause_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();									// necessary if there are input ugens which may have more than one channel
	//UGenInternal* getKr() throw();														// necessary if there is an actual control rate version (see below)
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Level, NumInputs };
	
protected:	
	float prevLevel;
};

//// uncomment this block if there is an actual control rate version of this UGen
//UGenInternalControlRateDeclaration(PauseUGenInternal, (Pause_InputsNoTypes), (Pause_InputsWithTypesAndDefaults));

/** A UGen which saves processing power when an amlitude value is zero. 
 @ingroup AllUGens ControlUGens */
UGenSublcassDeclarationNoDefault(Pause, (Pause_InputsNoTypes), (Pause_InputsWithTypesAndDefaults), COMMON_UGEN_DOCS);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Pause Pause()
#endif


#endif // _UGEN_ugen_Pause_H_
