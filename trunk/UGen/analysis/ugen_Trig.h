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

#ifndef _UGEN_Trig_H_
#define _UGEN_Trig_H_

#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class TrigUGenInternal : public UGenInternal
{
public:
	TrigUGenInternal(UGen const& input) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:
	float lastTrig;
};

/** Outputs 1 when input goes from zero or less to positive, otherwise outputs 0. */
UGenSublcassDeclarationNoDefault
(
 Trig,
 (input),
 (UGen const& input),
 COMMON_UGEN_DOCS
);


#endif // _UGEN_Trig_H_
