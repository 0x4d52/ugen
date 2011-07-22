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

#ifdef UGEN_VDSP

BEGIN_UGEN_NAMESPACE

#include "../basics/ugen_UnaryOpUGens.h"


void UnaryNegUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel);
	vDSP_vneg(inputSamples, 1, outputSamples, 1, numSamplesToProcess);
}

void UnaryAbsUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
	vDSP_vabs(inputSamples, 1, outputSamples, 1, numSamplesToProcess);
}

void UnaryReciprocalUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
	
//#ifdef UGEN_IPHONE
//	for(int i = 0; i < numSamplesToProcess; i++)
//	{
//		outputSamples[i] = 1.f / inputSamples[i];
//	}
//#else
//	vvrecf(outputSamples, inputSamples, &numSamplesToProcess); // consider vDSP_svdiv...
//#endif
	
	float one = 1.f;
	vDSP_svdiv(&one, inputSamples, 1, outputSamples, 1, numSamplesToProcess);
}

void UnarySquaredUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	const float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
	vDSP_vsq(inputSamples, 1, outputSamples, 1, numSamplesToProcess);
}

void UnaryCubedUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	const float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
	// need to check vDSP_vsq() can work in-place
	vDSP_vsq(inputSamples, 1, outputSamples, 1, numSamplesToProcess);
	vDSP_vsq(outputSamples, 1, outputSamples, 1, numSamplesToProcess);
}

void UnarySqrtUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
{ 
	const int numSamplesToProcess = uGenOutput.getBlockSize(); 
	float* const outputSamples = uGenOutput.getSampleData(); 
	const float* const inputSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
	
#ifdef UGEN_IPHONE
	for(int i = 0; i < numSamplesToProcess; i++)
	{
		outputSamples[i] = ugen::sqrt(inputSamples[i]);
	}
#else	
	vvsqrtf(outputSamples, inputSamples, &numSamplesToProcess);
#endif
}




END_UGEN_NAMESPACE

#endif
