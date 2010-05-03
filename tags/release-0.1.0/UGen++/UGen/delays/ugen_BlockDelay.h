// $Id: ugen_BlockDelay.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/delays/ugen_BlockDelay.h $

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

#ifndef _UGEN_ugen_BlockDelay_H_
#define _UGEN_ugen_BlockDelay_H_

// don't forget to add this header file to UGen.h

#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class BlockDelayUGenInternal : public UGenInternal
{
public:
	BlockDelayUGenInternal(UGen const& input) throw();
	UGenInternal* getChannel(const int channel) throw();									// necessary if there are input ugens which may have more than one channel
	//UGenInternal* getKr() throw();														// necessary if there is an actual control rate version (see below)
	//void prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw();	// necessary if there are input ugens, these need preparing too
	void prepareForBlockInternal(const int actualBlockSize, const unsigned int blockID) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:
	Buffer delayBuffer;
	float *delayBufferSamples;
};

/**
	A single processing block delay.
 
	You need to insert one of these wherever there is a feedback loop in a UGen graph (usually via a Plug).
 
 
 @ingroup AllUGens DelayUGens
 */
UGenSublcassDeclarationNoDefault(BlockDelay, (input), (UGen const& input), COMMON_UGEN_DOCS);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define BlockDelay BlockDelay()
#endif


#endif // _UGEN_ugen_BlockDelay_H_
