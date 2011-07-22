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

#ifndef _UGEN_ugen_NeuralNetworkUGen_H_
#define _UGEN_ugen_NeuralNetworkUGen_H_

#include "../core/ugen_UGen.h"
#include "ugen_NeuralNetwork.h"

class NeuralNetworkUGenUGenInternal : public ProxyOwnerUGenInternal
{
public:
	NeuralNetworkUGenUGenInternal(UGen const& input, 
								  UGen const& trig, 
								  UGen const& target, 
								  UGen const& patternTrig, 
								  NeuralNetwork const& network, 
								  NeuralPatternArray const& patterns) throw();
	~NeuralNetworkUGenUGenInternal();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Trig, Target, PatternTrig, NumInputs };
	
protected:
	NeuralNetwork network;
	NeuralPatternArray patterns;
	NumericalArray<float> inputVector;
	NumericalArray<float> outputVector;
	NumericalArray<float> targetVector;
	float lastTrig, lastPatternTrig;
	float** outputSampleData;
	float** inputSampleData;
	float** targetSampleData;
};


UGenSublcassDeclarationNoDefault(NeuralNetworkUGen, 
								 (input, trig, target, patternTrig, network, patterns), 
								 (UGen const& input, 
								  UGen const& trig, 
								  UGen const& target, 
								  UGen const& patternTrig, 
								  NeuralNetwork const& network, 
								  NeuralPatternArray const& patterns),
								 COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_NeuralNetworkUGen_H_
