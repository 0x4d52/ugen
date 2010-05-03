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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceMultiSlider.h"



MultiSliderUGenInternal::MultiSliderUGenInternal(UGen const& input, MultiSlider *_sliders) throw()
:	ProxyOwnerUGenInternal(NumInputs, input.getNumChannels()-1),
	sliders(_sliders->attachToMultiSliderUGenInternal() ? _sliders : 0),
	values(sliders && input.getNumChannels() ? new double[input.getNumChannels()] : 0)
{
	inputs[Input] = input;
		
	if(values)
		memset(values, 0, input.getNumChannels() * sizeof(double));
	
	if(sliders && sliders->isValidComponent())
	{
		sliders->getInterceptsMouseClicks(oldClick, oldChildClick);
		sliders->setInterceptsMouseClicks(false, false);
		
		sliders->setNumSliders(input.getNumChannels());
		for(int i = 0; i < input.getNumChannels(); i++)
		{
			values[i] = sliders->getSlider(i)->getValue();
		}
		
		startTimer(40);
	}
}

MultiSliderUGenInternal::~MultiSliderUGenInternal()
{
	stopTimer();
	
	if(sliders && sliders->isValidComponent())
	{
		sliders->detachFromMultiSliderUGenInternal();
		sliders->setInterceptsMouseClicks(oldClick, oldChildClick);
	}
	
	delete [] values;
}

void MultiSliderUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	const int blockSize = uGenOutput.getBlockSize();
			
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		float *outputSamples = proxies[channel]->getSampleData();
		const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
		
		if(values) values[channel] = *inputSamples;
		
		memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
	}
}

void MultiSliderUGenInternal::timerCallback()
{
	for(int i = 0; i < getNumChannels(); i++)
	{
		sliders->getSlider(i)->setValue(values[i], true, false);
	}
}

MultiSliderUGen::MultiSliderUGen(UGen const& input, MultiSlider *sliders) throw()
{
	MultiSliderUGenInternal *internal = new MultiSliderUGenInternal(input, sliders);
	initInternal(input.getNumChannels());
	generateFromProxyOwner(internal);
}

END_UGEN_NAMESPACE

#endif // JUCE_VERSION

