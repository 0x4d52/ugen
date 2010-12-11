// $Id:$
// $HeadURL:$

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
 
 HRTF data from the work at MIT by Bill Gardner and Keith Martin
 http://sound.media.mit.edu/resources/KEMAR.html
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_HRTF_H_
#define _UGEN_ugen_HRTF_H_

#include "../core/ugen_Collections.h"
#include "../buffers/ugen_Buffer.h"

class HRTF
{
public:
	static HRTF& getInstance();
	
	static Buffer getClosestResponse(const float azimuth, const float elevation = 0.f) throw();
	static Buffer getClosestResponseDegrees(const int azimuth, const int elevation = 0) throw();
//	static Buffer getResponse(const float azimuth, const float elevation = 0.f) throw();
//	static Buffer getResponseDegrees(const int azimuth, const int elevation = 0) throw();
	static int closestElevation(const int elevation) throw();
	static int closestAzimuth(const int elevation, const int azimuth) throw();
	
	
private:
	HRTF() throw();
	void addResponse(Text const& key, const short* left, const short* right) throw();

	Dictionary<Buffer> database;
};

#endif // _UGEN_ugen_HRTF_H_