// $Id: ugen_LPF.h 1006 2010-03-18 12:26:09Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/filters/simple/ugen_LPF.h $

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

#ifndef _UGEN_ugen_LPF_H_
#define _UGEN_ugen_LPF_H_


#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class LPFUGenInternal : public UGenInternal
{
public:
	LPFUGenInternal(UGen const& input, UGen const& freq) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Freq, NumInputs };
	
protected:
	float y1, y2, a0, b1, b2, currentFreq;
};

#define LPF_Docs	@param input	The input source to filter.								\
					@param freq		The cut-off frequency.

/** 1st order low pass filter.
 @ingroup AllUGens FilterUGens
 @see BLowPass, RLPF */
DirectMulAddUGenDeclaration(LPF, (input, freq), 
								 (input, freq, MulAdd_ArgsCall),
								 (UGen const& input, UGen const& freq = 1000.f), 
								 (UGen const& input, UGen const& freq = 1000.f, MulAdd_ArgsDeclare),
							COMMON_UGEN_DOCS LPF_Docs MulAddArgs_Docs);


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define LPF LPF()
#endif


#endif // _UGEN_ugen_LPF_H_
