// $Id: ugen_JuceSlider.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/ugen_JuceSlider.h $

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


#ifndef UGEN_JUCESLIDER_H
#define UGEN_JUCESLIDER_H


#include "../basics/ugen_ScalarUGens.h"

/** @ingroup UGenInternals GUITools */
class SliderUGenInternal :	public FloatPtrUGenInternal,
							public SliderListener
{
public:
	SliderUGenInternal(Slider* slider) throw();
	~SliderUGenInternal() throw();
	void sliderValueChanged(Slider* slider) throw();
	
private:
	float value;
	Slider* slider_;
};

/** A UGen which takes its value source from a Juce Slider.
 @ingroup GUITools ControlUGens */
class SliderUGen : public UGen 
{
public:
	SliderUGen(Slider* slider) throw();
};

/** @ingroup UGenInternals GUITools */
class ButtonUGenInternal :	public FloatPtrUGenInternal,
							public ButtonListener
{
public:
	ButtonUGenInternal(Button* button) throw();
	~ButtonUGenInternal() throw();
	void buttonClicked(Button* button) throw();
	
private:
	float value;
	Button* button_;
	bool isToggle;
};

/** @ingroup GUITools ControlUGens */
class ButtonUGen : public UGen 
{
public:
	ButtonUGen(Button* button) throw();
};

/** @ingroup UGenInternals GUITools */
class LabelUGenInternal :	public FloatPtrUGenInternal,
							public LabelListener
{
public:
	LabelUGenInternal(Label* label) throw();
	~LabelUGenInternal() throw();
	void labelTextChanged(Label* label) throw();
	
private:
	float value;
	Label* label_;
};

/** @ingroup GUITools ControlUGens */
class LabelUGen : public UGen 
{
public:
	LabelUGen(Label* label) throw();
};


#endif // UGEN_JUCESLIDER_H