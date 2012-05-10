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

#ifndef _UGEN_Minima_H_
#define _UGEN_Minima_H_

#include "../core/ugen_UGen.h"

// A new UGen also needs a UGenInternal which does the actual processing. 
// UGen and its subclasses are primarily for constructing the UGen graphs.
// Simple internals should inherit from UGenInternal.
/** @ingroup UGenInternals */
class MinimaUGenInternal : public UGenInternal
{
public:
	// Constructor.
	MinimaUGenInternal(UGen const& input) throw();
	
	UGenInternal* getChannel(const int channel) throw();
	
	// This is called when the internal is needed to process a new block of samples.
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs }; // used mainly by the 'inputs' array for the UGenInternal's UGen inputs
	
protected:
	bool didDecreaseLastTime;
	float lastValue;
};

// A macro for declaring a UGen subclass.
// The parentheses around the 2nd and 3rd arguments are required.
// Items within these parentheses should be comma-separated just as for argument declarations and calls.
UGenSublcassDeclarationNoDefault
(
	Minima,					// The UGen name, this will inherit from UGen
	(input),				// argument list for the Constructor, AR and KR methods as they would be CALLED
	(UGen const& input),	// argument list for the Constructor, AR and KR methods as they are to be DECLARED
	COMMON_UGEN_DOCS		// Documentation (for Doxygen)
);



#endif // _UGEN_Minima_H_
