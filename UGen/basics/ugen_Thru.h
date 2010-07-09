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

#ifndef _UGEN_ugen_Thru_H_
#define _UGEN_ugen_Thru_H_

#include "../core/ugen_UGen.h"

#define Thru_InputsWithTypesAndDefaults	UGen const& input
#define Thru_InputsWithTypesOnly		UGen const& input
#define Thru_InputsNoTypes				input

/** @ingroup UGenInternals */
class ThruUGenInternal : public UGenInternal
{
public:
	ThruUGenInternal(Thru_InputsWithTypesAndDefaults) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:	
};

#define Thru_Docs	@param input	Copies the input and passes it to the output. \
									(Multichannel in and multichannel out.)

/** Does nothing, passes input to output.
 This does provide an individual output object for each input which may be useful 
 (for example) at the outputs to a host where the finla output UGen may contain
 copies of identical internals. In this case there would be only one output sample
 buffer so using something like UGen::setOutputs() might try to instruct the 
 UGenInternal to write its data to two places at the same time (which is not
 possible). Wrapping the final output in the Thru UGen means there are multiple
 DIFFERENT internals as the final output so UGen::setOutputs() would work
 successfully.
 @ingroup AllUGens ControlUGens */
UGenSublcassDeclaration(Thru, (Thru_InputsNoTypes), (Thru_InputsWithTypesAndDefaults), COMMON_UGEN_DOCS Thru_Docs);



#endif // _UGEN_ugen_Thru_H_
