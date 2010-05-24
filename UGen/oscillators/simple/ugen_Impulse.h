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

#ifndef _UGEN_ugen_Impulse_H_
#define _UGEN_ugen_Impulse_H_

#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

#define Impulse_InputsWithTypesAndDefaults	UGen const& freq = 440.f
#define Impulse_InputsWithTypesOnly			UGen const& freq
#define Impulse_InputsNoTypes				freq

/** @ingroup UGenInternals */
class ImpulseUGenInternal : public UGenInternal
{
public:
	ImpulseUGenInternal(Impulse_InputsWithTypesOnly) throw();
	UGenInternal* getChannel(const int channel) throw();									
	UGenInternal* getKr() throw();															
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Freq, NumInputs };
	
protected:
	float currentPhase;	
};

/** @ingroup UGenInternals */
class ImpulseUGenInternalK : public ImpulseUGenInternal
{
public:
	ImpulseUGenInternalK(Impulse_InputsWithTypesOnly) throw();
	UGenInternal* getKr() throw() { incrementRefCount(); return this; }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
private:
	float value;
};

#define Impulse_Docs	@param freq	The frequency of the impulses (in Hertz).

/** Impulse oscillator. 
 Outputs non band limited single sample impulses. */
DirectMulAddUGenDeclaration(Impulse,	(Impulse_InputsNoTypes), 
										(Impulse_InputsNoTypes, MulAdd_ArgsCall), 
										(Impulse_InputsWithTypesAndDefaults), 
										(Impulse_InputsWithTypesAndDefaults, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS Impulse_Docs MulAddArgs_Docs);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Impulse Impulse()
#endif


#endif // _UGEN_ugen_Impulse_H_
