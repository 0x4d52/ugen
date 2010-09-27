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

#ifndef UGEN_OSCILLATORUGENS_H
#define UGEN_OSCILLATORUGENS_H


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"

#define Phasor_InputsWithTypesAndDefaults	UGen const& freq = 440.f, UGen const& phase = 0.f
#define Phasor_InputsWithTypesOnly			UGen const& freq, UGen const& phase
#define Phasor_InputsNoTypes				freq, phase
#define Phasor_InputsEnum					Freq, Phase

/**
	This class is a test class, do not use!
 
	For a sawtooth use LFSaw. This class is used for testing new features etc before rolling
	these out to the rest of the library.
 */
class PhasorUGenInternal : public UGenInternal
{
public:
	PhasorUGenInternal(Phasor_InputsWithTypesOnly, const float initialPhase = 0.f) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
		
	enum Input { Phasor_InputsEnum, NumInputs };

protected:
	double currentPhase;
};

UGenInternalControlRateDeclaration(PhasorUGenInternal, 
								   (Phasor_InputsNoTypes, initialPhase), 
								   (Phasor_InputsWithTypesAndDefaults, const float initialPhase = 0.f));

DirectMulAddUGenDeclaration(Phasor, 
							(Phasor_InputsNoTypes), 
							(Phasor_InputsNoTypes, MulAdd_ArgsCall), 
							(Phasor_InputsWithTypesAndDefaults), 
							(Phasor_InputsWithTypesAndDefaults, MulAdd_ArgsDeclare), COMMON_UGEN_DOCS);



#endif // OSCILLATORUGENS_H
