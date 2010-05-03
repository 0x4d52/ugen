// $Id: ugen_JuceSlider.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/ugen_JuceSlider.cpp $

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

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceSlider.h"

SliderUGenInternal::SliderUGenInternal(Slider* slider) throw()
:	FloatPtrUGenInternal(&value),
	slider_(slider)
{
	slider_->addListener(this);
	value = (float)slider_->getValue();
	value_ = value;
}

SliderUGenInternal::~SliderUGenInternal() throw()
{
	if(slider_->isValidComponent())
		slider_->removeListener(this);
}

void SliderUGenInternal::sliderValueChanged(Slider* slider) throw()
{
	if(slider != slider_) return;
		
	value = (float)slider_->getValue();
	value_ = value;
}

SliderUGen::SliderUGen(Slider* slider) throw()
{
	initInternal(1);	
	internalUGens[0] = new SliderUGenInternal(slider);
}

ButtonUGenInternal::ButtonUGenInternal(Button* button) throw()
:	FloatPtrUGenInternal(&value),
	button_(button)
{		
	button_->addButtonListener(this);
	
	isToggle = dynamic_cast<ToggleButton*> (button) ? true : false;
	
	buttonClicked(button);	
}

ButtonUGenInternal::~ButtonUGenInternal() throw()
{
	if(button_->isValidComponent())
		button_->removeButtonListener(this);
}

void ButtonUGenInternal::buttonClicked(Button* button) throw()
{
	if(button != button_) return;
		
	if(isToggle)
	{
		value = button_->getToggleState() ? 1.f : 0.f;
	}
	else
	{
		if(button->isDown()) 
		{
			value = 1.f;
			button->setTriggeredOnMouseDown(false);
		}
		else
		{
			value = 0.f;
			button->setTriggeredOnMouseDown(true);
		}
	}
	
	value_ = value;
}

ButtonUGen::ButtonUGen(Button* button) throw()
{
	initInternal(1);	
	internalUGens[0] = new ButtonUGenInternal(button);
}

LabelUGenInternal::LabelUGenInternal(Label* label) throw()
:	FloatPtrUGenInternal(&value),
	label_(label)
{
	label->addListener(this);
	value = label_->getText(false).getFloatValue();
	value_ = value;
}

LabelUGenInternal::~LabelUGenInternal() throw()
{
	if(label_->isValidComponent())
		label_->removeListener(this);
}

void LabelUGenInternal::labelTextChanged(Label* label) throw()
{
	if(label != label_) return;
	
	value = label_->getText(false).getFloatValue();
	value_ = value;
}

LabelUGen::LabelUGen(Label* label) throw()
{
	initInternal(1);	
	internalUGens[0] = new LabelUGenInternal(label);
}

END_UGEN_NAMESPACE

#endif
