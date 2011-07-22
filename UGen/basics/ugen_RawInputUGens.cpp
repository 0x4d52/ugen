// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_RawInputUGens.h"


RawInputUGenInternal::RawInputUGenInternal(const int numChannels) throw()
:	ProxyOwnerUGenInternal(0, numChannels - 1)
{
	ugen_assert(numChannels > 0);
	
	bufferData = new const float*[numChannels];
	memset(bufferData, 0, numChannels * sizeof(float*));
}

RawInputUGenInternal::~RawInputUGenInternal() throw()
{
	delete [] bufferData;
}

void RawInputUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
	const int numChannels = getNumChannels();
	
	for(int i = 0; i < numChannels; i++)
	{
		const float* inputSamples = bufferData[i];
		if(inputSamples != 0)
			memcpy(proxies[i]->getSampleData(), inputSamples, blockSize * sizeof(float));
	}
	
	// invalidate the buffer pointers..
	memset(bufferData, 0, numChannels * sizeof(float*));
}

bool RawInputUGenInternal::setInput(const float* block, const int channel) throw() 
{ 
	ugen_assert(channel >= 0);
	ugen_assert(channel < getNumChannels());
	ugen_assert(block != 0);
	
	bufferData[channel] = block; 
	return true;
}

AudioIn::AudioIn(const int numChannels) throw()
{
	ugen_assert(numChannels > 0);
	
	initInternal(numChannels);
	generateFromProxyOwner(new RawInputUGenInternal(numChannels));
}

END_UGEN_NAMESPACE
