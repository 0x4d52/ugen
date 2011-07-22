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

#ifndef _UGEN_ugen_FSinOsc_H_
#define _UGEN_ugen_FSinOsc_H_

#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class FSinOscUGenInternal : public UGenInternal
{
public:
	FSinOscUGenInternal(UGen const& freq, const float initialPhase, const int channel) throw();
	UGenInternal* getChannel(const int channel) throw();									
	//UGenInternal* getKr() throw();															
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Freq, NumInputs };
	
protected:
	float currentFreq;
	double b1, y1, y2;
};


//class FSinOscUGenInternalK : public FSinOscUGenInternal
//{
//public:
//	FSinOscUGenInternalK(FSinOsc_InputsWithTypesOnly) throw();
//	UGenInternal* getKr() throw() { incrementRefCount(); return this; }
//	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
//	
//private:
//	float value;
//};

#define FSinOsc_Docs	@param freq	The frequency of the sine wave.

/** Fast sine oscillator.
 Implemented using a ringing filter.
 This generally works best with a fixed frequency although the frequency may
 be modulated. Unlike the SuperCollider FSinOsc the amplitude does not vary
 when the frequency is modulated.
 
 @ingroup AllUGens OscUGens FilterUGens
 @see SinOsc*/
DirectMulAddUGenDeclaration(FSinOsc,	(freq), 
										(freq, MulAdd_ArgsCall), 
										(UGen const& freq = 440.f), 
										(UGen const& freq = 440.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS FSinOsc_Docs MulAddArgs_Docs);



#endif // _UGEN_ugen_FSinOsc_H_
