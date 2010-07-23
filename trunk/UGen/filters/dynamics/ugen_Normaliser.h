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
 
 ==============================================================================
 */

#ifndef _UGEN_Normaliser_H_
#define _UGEN_Normaliser_H_

#include "../../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class NormaliserUGenInternal : public UGenInternal
{
public:
	NormaliserUGenInternal(UGen const& input, UGen const& level, const float duration) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	enum Inputs { Input, Level, NumInputs };
	
protected:
	int numBuffersProcessed, bufferPosition, bufferSize;
	float slope, level, currentMaximum, prevMaximum, slopeFactor;
	
	Buffer inputBuffer, midBuffer, outputBuffer;
};

/** Normalise a signal to a specified level.
 Not that there is a delay of double the duration argument (which defaults to 0.01s so the
 default delay is 0.02s).
 @ingroup AllUGens FilterUGens */
UGenSublcassDeclarationNoDefault
(
	Normaliser,
	(input, level, duration),
	(UGen const& input, UGen const& level = 1.f, const float duration = 0.01f),	
	COMMON_UGEN_DOCS
);




#endif // _UGEN_Normaliser_H_
