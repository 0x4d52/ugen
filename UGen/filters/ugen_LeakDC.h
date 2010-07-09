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

#ifndef _UGEN_ugen_LeakDC_H_
#define _UGEN_ugen_LeakDC_H_


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class LeakDCUGenInternal : public UGenInternal
{
public:
	LeakDCUGenInternal(UGen const& input, UGen const& coeff) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void initValue(const float value) throw();	
	
	enum Inputs { Input, Coeff, NumInputs };
	
protected:	
	float b1, y1, x1;
};


UGenInternalControlRateDeclaration(LeakDCUGenInternal,	(input, coeff),
														(UGen const& input, UGen const& coeff));

/** DC offset removal filter.
 @ingroup AllUGens FilterUGens */
DirectMulAddUGenDeclaration(LeakDC,	(input, coeff),
									(input, coeff, MulAdd_ArgsCall),
									(UGen const& input, UGen const& coeff = 0.99), 
									(UGen const& input, UGen const& coeff = 0.99, MulAdd_ArgsDeclare), COMMON_UGEN_DOCS);




#endif // _UGEN_ugen_LeakDC_H_
