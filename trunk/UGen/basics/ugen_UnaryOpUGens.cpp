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

#include <math.h>
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "../core/ugen_UGenArray.h"
#include "../buffers/ugen_Buffer.h"
#include "ugen_UnaryOpUGens.h"

UnaryOpUGenInternal::UnaryOpUGenInternal(UGen const& operand, const int channel) throw()
:	UGenInternal(NumInputs)
{
	if(channel < 0)
		inputs[Operand] = operand;
	else
	{
		int numChannels = operand.getNumChannels();
		UGenInternal* internal = operand.getInternalUGen(channel % numChannels);
		inputs[Operand] = UGen(internal->getChannelInternal(channel), channel);
	}
}

UnaryOpValueInternal::UnaryOpValueInternal(Value const& operand) throw()
:	operand_(operand)
{
}


// using vfp the internal process block functions are defined in iphone/armasm/ugen_vfp_UnaryOpUGens.cpp
#if defined(UGEN_VFP) || defined(UGEN_NEON) || defined(UGEN_VDSP)
UnaryOpUGenDefinitionNoProcessBlock(Neg,		neg,			neg);
UnaryOpUGenDefinitionNoProcessBlock(Abs,		abs,			abs);
UnaryOpUGenDefinitionNoProcessBlock(Reciprocal,	reciprocal,		reciprocal);
UnaryOpUGenDefinitionNoProcessBlock(Squared,	squared,		squared);
UnaryOpUGenDefinitionNoProcessBlock(Cubed,		cubed,			cubed);
UnaryOpUGenDefinitionNoProcessBlock(Sqrt,		sqrt,			sqrt);
#else
UnaryOpUGenDefinition(Neg,			neg,			neg);
UnaryOpUGenDefinition(Abs,			abs,			abs);
UnaryOpUGenDefinition(Reciprocal,	reciprocal,		reciprocal);
UnaryOpUGenDefinition(Squared,		squared,		squared);
UnaryOpUGenDefinition(Cubed,		cubed,			cubed);
UnaryOpUGenDefinition(Sqrt,			sqrt,			sqrt);
#endif

UnaryOpUGenDefinition(Sin,			sin,			sin);
UnaryOpUGenDefinition(Cos,			cos,			cos);
UnaryOpUGenDefinition(Tan,			tan,			tan);
UnaryOpUGenDefinition(SinH,			sinh,			sinh);
UnaryOpUGenDefinition(CosH,			cosh,			cosh);
UnaryOpUGenDefinition(TanH,			tanh,			tanh);
UnaryOpUGenDefinition(ASin,			asin,			asin);
UnaryOpUGenDefinition(ACos,			acos,			acos);
UnaryOpUGenDefinition(ATan,			atan,			atan);
UnaryOpUGenDefinition(Log,			log,			log);
UnaryOpUGenDefinition(Log2,			log2,			log2);
UnaryOpUGenDefinition(Log10,		log10,			log10);
UnaryOpUGenDefinition(Exp,			exp,			exp);
UnaryOpUGenDefinition(Ceil,			ceil,			ceil);
UnaryOpUGenDefinition(Floor,		floor,			floor);
UnaryOpUGenDefinition(Frac,			frac,			frac);
UnaryOpUGenDefinition(Sign,			sign,			sign);

#ifndef UGEN_NOEXTGPL
UnaryOpUGenDefinition(MIDICPS,		midicps,		midicps);
UnaryOpUGenDefinition(CPSMIDI,		cpsmidi,		cpsmidi);
UnaryOpUGenDefinition(AmpdB,		ampdb,			ampdb);
UnaryOpUGenDefinition(dBAmp,		dbamp,			dbamp);
UnaryOpUGenDefinition(Distort,		distort,		distort);
UnaryOpUGenDefinition(BesselI0,		besselI0,		besselI0);
UnaryOpUGenDefinition(Zap,			zap,			zap);
#endif

END_UGEN_NAMESPACE