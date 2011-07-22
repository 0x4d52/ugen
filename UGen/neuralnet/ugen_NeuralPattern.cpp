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

#include "ugen_NeuralPattern.h"


NeuralPatternSimpleInternal::NeuralPatternSimpleInternal(NumericalArray<float> const& _inputVector,
														 NumericalArray<float> const& _outputVector) throw()
:	inputVector(_inputVector.copy()), 
	outputVector(_outputVector.copy())
{
}

void NeuralPatternSimpleInternal::write(TextFileWriter const& _file) const throw()
{
	const int size = 128;
	char buf[size];
	
	TextFileWriter file = _file;
	
	file.write("\nNeuralPatternSimple:v1\n");
	
	const int numInputs = inputVector.size();
	snprintf(buf, size, "Inputs:%3d\n", numInputs);
	file.write(buf);
	
	const float *inputVectorArray = inputVector.getArray();
	for(int i = 0; i < numInputs; i++)
	{
		snprintf(buf, size, "%3d %.16f\n", i, inputVectorArray[i]);
		file.write(buf);
	}
	
	const int numOutputs = outputVector.size();
	snprintf(buf, size, "Outputs:%3d", numOutputs);
	file.write(buf);
	
	const float *outputVectorArray = outputVector.getArray();
	for(int i = 0; i < numOutputs; i++)
	{
		file.write("\n");
		snprintf(buf, size, "%3d %.16f", i, outputVectorArray[i]);
		file.write(buf);
	}
}

void NeuralPattern::read(TextFileReader const& _file) throw()
{
	TextFileReader file = _file;
	
	Text text;
	int line = 0;
	int numInputs = -1, numOutputs = -1;
		
	NumericalArray<float> inputVector;
	NumericalArray<float> outputVector;
	
	while((text = file.readLine()).size() != 0)
	{
		if(line == 0)
		{
			// should be the format/version string
			if(text.contains("NeuralPatternSimple:v1") == false)
			{
				printf("NeuralPattern::read wrong version type\n");
				return;
			}
		}
		else if(numInputs < 0)
		{
			// should be the number of inputs			
			sscanf(text.getArray(), "Inputs:%d", &numInputs);
			
			if(numInputs < 1) 
			{
				printf("NeuralPattern::read invalid number of inputs\n");
				return;
			}
			
			//printf("ins: %d\n", numInputs);
			
			inputVector = NumericalArray<float>::withSize(numInputs);
		}
		else if(numInputs > 0)
		{
			int index;
			float value;
			sscanf(text.getArray(), "%d %f", &index, &value);
			numInputs--;
			
			//printf("in[%3d] = %f\n", index, value);
			
			inputVector[index] = value;
		}
		else if(numOutputs < 0)
		{
			// should be the number of outputs			
			sscanf(text.getArray(), "Outputs:%d", &numOutputs);
			
			if(numOutputs < 1) 
			{
				printf("NeuralPattern::read invalid number of numOutputs\n");
				return;
			}
			
			//printf("outs: %d\n", numOutputs);
			
			outputVector = NumericalArray<float>::withSize(numOutputs);
		}
		else if(numOutputs > 0)
		{
			int index;
			float value;
			sscanf(text.getArray(), "%d %f", &index, &value);
			numOutputs--;
			
			//printf("out[%3d] = %f\n", index, value);
			
			outputVector[index] = value;
		}
		
		if((numInputs == 0) && (numOutputs == 0))
		{
			//printf("CREATING PATTERN\n");
			setInternal(new NeuralPatternSimpleInternal(inputVector, outputVector));
			return;
		}
		
		line++;
	}		
}

void NeuralPatternArray::write(TextFileWriter const& _file) const throw()
{	
	TextFileWriter file = _file;
	
	const NeuralPattern* array = getArray();
	
	if(array != 0)
	{
		file.write("NeuralPatternArray:v1");
		
		const int numPatterns = size();
		for(int i = 0; i < numPatterns; i++)
		{
			array[i].write(file);
		}
	}
}
void NeuralPatternArray::read(TextFileReader const& _file) throw()
{
	TextFileReader file = _file;
	
	Text text = file.readLine();
	
	if(text.contains("NeuralPatternArray:v1"))
	{
		//printf("is valid NeuralPatternArray file\n");
		
		while(file.isEof() == false)
		{
			NeuralPattern pat;
			pat.read(file);
			
			if((pat.getNumInputs() > 0) && (pat.getNumOutputs() > 0))
			{
				//printf("..adding a pattern..\n");
				
				this->add(pat);
			}
		}
	}

}


END_UGEN_NAMESPACE
