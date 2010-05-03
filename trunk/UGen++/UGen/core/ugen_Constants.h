// $Id: ugen_Constants.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/core/ugen_Constants.h $

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

#ifndef _UGEN_ugen_Constants_H_
#define _UGEN_ugen_Constants_H_

/** @file 
	Some useful maths constants.
 */
 
//namespace Constant {

	const double pi     = std::acos(-1.0);
	const double pi2    = pi * .5;
	const double pi4    = pi * .25;
	const double pi32   = pi * 1.5;
	const double twopi  = pi * 2.0;
	const double fourpi = pi * 4.0;
	const double rtwopi = 1.0 / twopi;
	const double log001 = std::log(0.001);
	const double log01  = std::log(0.01);
	const double log1   = std::log(0.1);
	const double rlog2  = 1.0/std::log(2.0);
	const double sqrt2  = std::sqrt(2.0);
	const double rsqrt2 = 1.0 / sqrt2;
	const double sqrt22 = sqrt2 * 0.5;
	const double r440	= 1.0 / 440.0;
	const double r12	= 1.0 / 12.0;
	const double zero	= 0.0;
	const double e1		= std::exp(1.0);


//}

#endif // _UGEN_ugen_Constants_H_
