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

#ifndef _UGEN_ugen_RawInputUGens_H_
#define _UGEN_ugen_RawInputUGens_H_

#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class RawInputUGenInternal : public ProxyOwnerUGenInternal
{
public:
	RawInputUGenInternal(const int numChannels) throw();
	~RawInputUGenInternal() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	inline void setInput(const float* block, const int channel) throw() { bufferData[channel] = block; }
	bool isRawInputUGenInternal() const throw() { return true; }
	
protected:
	const float** bufferData;
};

#define AudioIn_Docs	@param numChannels The number of channels the AudioIn UGen can generate.

/** Audio in from an external source. 
	The actual source of the audio must be set on each processing block using
	UGen::setInput() or UGen::setInputs(). The host IO objects (e.g., JuceHostIO
	handle this on your behalf by having an internal AudioIn UGen)
	@see JuceHostIO, UGen::setInput(), UGen::setInputs() */
UGenSublcassDeclaration(AudioIn, (numChannels),
								 (const int numChannels), 
						COMMON_UGEN_DOCS AudioIn_Docs);




#endif // _UGEN_ugen_RawInputUGens_H_
