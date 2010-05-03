// $Id: ugen_Temporary.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_Temporary.h $

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

#ifndef _UGEN_ugen_Temporary_H_
#define _UGEN_ugen_Temporary_H_


#include "../core/ugen_UGen.h"


// temporary UGen classes written using other UGen classes these could be rewritten in C++ for efficiency

#define Clip_Docs	@param input	The input to limit					\
					@param min		The minimum (i.e., lower) limit		\
					@param max		The maximum (i.e., upper) limit	

/** Clip (i.e., limit) between min and max.
 Implements the expression @code input.min(max).max(min); @endcode
 @ingroup AllUGens MathsUGens
 @see UGen::min(), UGen::max(), BinaryOpMinUGen, BinaryOpMaxUGen */
DefineCustomUGen(Clip,
				(input.min(max).max(min)),
				(UGen const& input, UGen const& min, UGen const& max),
				 COMMON_UGEN_DOCS Clip_Docs);

///**
// Clip between -clip and clip.
// */
//DefineCustomUGen(Clip2,
//				(input.min(clip).max(-clip)),
//				(UGen const& input, UGen const& clip));

#define Invert_Docs	@param input	The value to invert around max.		\
					@param max		The value around which to invert.

/** Invert about a particular value.
 Implements the expression @code max-input; @endcode
 By default this is useful for inverting values around 1.0 e.g., buttons
 which output 0 or 1 and you want to convert this to 1 and 0 respectively.
 @ingroup AllUGens MathsUGens
 @see UGen::operator-, UGen::neg(), UnaryOpNegUGen, LinLin */
DefineCustomUGen(Invert,
				 (max-input),
				 (UGen const& input, UGen const& max = 1.f),
				 COMMON_UGEN_DOCS Invert_Docs);




#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Clip Clip() 
//#define Clip2 Clip2() 
#endif


#endif // _UGEN_ugen_Temporary_H_
