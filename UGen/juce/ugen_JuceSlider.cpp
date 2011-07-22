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

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceSlider.h"

SliderValueInternal::SliderValueInternal(Slider *slider) throw()
:	ValueInternal(slider->getValue()),
	slider_(slider)
{
	slider_->addListener(this);
}

SliderValueInternal::~SliderValueInternal() throw()
{
	if(slider_.getComponent())
		slider_->removeListener(this);
}

void SliderValueInternal::sliderValueChanged(Slider* slider) throw()
{
	if(slider != slider_) return;
	
	setValue(slider->getValue()); 
}

SliderValue::SliderValue(Slider* slider) throw()
:	Value(new SliderValueInternal(slider))
{
}


ButtonValueInternal::ButtonValueInternal(Button* button) throw()
:	ValueInternal(0.0),
	button_(button)
{		
	button_->addListener(this);
		
	buttonClicked(button_);	
}

ButtonValueInternal::~ButtonValueInternal() throw()
{
	if(button_.getComponent())
		button_->removeListener(this);
}

void ButtonValueInternal::buttonClicked(Button* button) throw()
{
	if(button != button_) return;
		
	bool isToggle = button_->getClickingTogglesState();
	
	if(isToggle)
	{
		setValue(button_->getToggleState() ? 1.0 : 0.0);
	}
	else
	{
		if(button->isDown()) 
		{
			setValue(1.0);
			button->setTriggeredOnMouseDown(false);
		}
		else
		{
			setValue(0.0);
			button->setTriggeredOnMouseDown(true);
		}
	}
}

ButtonValue::ButtonValue(Button* button) throw()
:	Value(new ButtonValueInternal(button))
{
}


LabelValueInternal::LabelValueInternal(Label* label) throw()
:	ValueInternal(label->getText(false).getDoubleValue()),
	label_(label)
{
	label_->addListener(this);
}

LabelValueInternal::~LabelValueInternal() throw()
{
	if(label_.getComponent())
		label_->removeListener(this);
}

void LabelValueInternal::labelTextChanged(Label* label) throw()
{
	if(label != label_) return;
	
	setValue(label_->getText(false).getDoubleValue());
}

LabelValue::LabelValue(Label* label) throw()
:	Value(new LabelValueInternal(label))
{
}

PluginSlider::PluginSlider(const String& componentName,
						   double minimumToUse, 
						   double maximumToUse, 
						   bool isExponentialToUse,
						   String const& units) throw()
:	Slider(componentName),
	minimum(minimumToUse),
	maximum(maximumToUse),
	isExponential(isExponentialToUse)
{
	// with exponential, the range can't cross zero
//	ugen_assert(!isExponential || ((minimum <= 0.0) && (maximum >= 0.0)));
//	ugen_assert(!isExponential || ((minimum >= 0.0) && (maximum <= 0.0)));
	
	setRange (0.0, 1.0);
	setTextValueSuffix(units);
}

double PluginSlider::getValueFromText (const String& text)
{
	double realValue = Slider::getValueFromText(text);
	
	if(isExponential)
	{
		return explin(realValue, minimum, maximum, 0.0, 1.0);
	}
	else
	{
		return linlin(realValue, minimum, maximum, 0.0, 1.0);
	}
}

const String PluginSlider::getTextFromValue (double normalisedValue)
{
	double realValue = 0.0;
	
	if(isExponential)
	{
		realValue = linexp(normalisedValue, 0.0, 1.0, minimum, maximum);
	}
	else
	{
		realValue = linlin(normalisedValue, 0.0, 1.0, minimum, maximum);
	}
	
	int intValue = (int)realValue;
	int digits = 0;	
	int step = 1;	
	while (step <= intValue) 
	{		
		digits++;		
		step *= 10;	
	}	
	
	String units = getTextValueSuffix();
	return String(realValue, 8 - units.length() - jmax(1, digits)) + units;
}

END_UGEN_NAMESPACE

#endif
