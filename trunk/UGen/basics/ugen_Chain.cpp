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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Chain.h"

ChainBaseUGenInternal::ChainBaseUGenInternal(UGen const& input, const int size, const int numChannels) throw()
:	ProxyOwnerUGenInternal(1, numChannels-1),
	size_(size),
	bufferData(new float*[numChannels]),
	chain(UGenArray(size_))
{
	ugen_assert(size > 0);
	ugen_assert(numChannels > 0);
	inputs[Input] = input;
}

ChainBaseUGenInternal::~ChainBaseUGenInternal()
{
	delete [] bufferData;
}

void ChainBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	const int numSamplesToProcess = uGenOutput.getBlockSize();	
	const int numChannels = getNumChannels();
	for(int channel = 0; channel < numChannels; channel++)
	{
		bufferData[channel] = proxies[channel]->getSampleData();
	}
	
	const int sizeMinusOne = size_ - 1;
	for(int link = 0; link < sizeMinusOne; link++)
	{
		chain[link].prepareForBlock(numSamplesToProcess, blockID);
		chain[link].processBlock(shouldDelete, blockID, -1);
	}
	
	UGen& last = chain.last();
	last.prepareForBlock(numSamplesToProcess, blockID);
	last.setOutputs(bufferData, numSamplesToProcess, numChannels);
	last.processBlock(shouldDelete, blockID, -1);
}

BankBaseUGenInternal::BankBaseUGenInternal(UGen const& input, const int size, const int numChannels) throw()
:	ProxyOwnerUGenInternal(1, numChannels-1),
	size_(size),
	bufferData(new float*[numChannels]),
	bank(UGenArray(size_)),
	mixer(Mix(&bank))
{
	ugen_assert(size > 0);
	ugen_assert(numChannels > 0);
	inputs[Input] = input;
}

BankBaseUGenInternal::~BankBaseUGenInternal()
{
	delete [] bufferData;
}

void BankBaseUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	const int numSamplesToProcess = uGenOutput.getBlockSize();	
	const int numChannels = getNumChannels();
	for(int channel = 0; channel < numChannels; channel++)
	{
		bufferData[channel] = proxies[channel]->getSampleData();
	}
	
	mixer.prepareForBlock(numSamplesToProcess, blockID);
	mixer.setOutputs(bufferData, numSamplesToProcess, numChannels);
	mixer.processBlock(shouldDelete, blockID, -1);
}


END_UGEN_NAMESPACE
