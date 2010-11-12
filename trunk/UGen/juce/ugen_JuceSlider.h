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


#ifndef UGEN_JUCESLIDER_H
#define UGEN_JUCESLIDER_H


#include "../core/ugen_Value.h"


class SliderValueInternal :	public ValueInternal,
							public SliderListener
{
public:
	SliderValueInternal(Slider *slider) throw();
	~SliderValueInternal() throw();
	void sliderValueChanged(Slider* slider) throw();
	
protected:
	Slider *slider_;
};

class SliderValue : public Value
{
public:
	SliderValue(Slider* slider) throw();
private:
	SliderValue();
};


class ButtonValueInternal :	public ValueInternal,
							public ButtonListener
{
public:
	ButtonValueInternal(Button* button) throw();
	~ButtonValueInternal() throw();
	void buttonClicked(Button* button) throw();
	
private:
	Button* button_;
};

class ButtonValue : public Value 
{
public:
	ButtonValue(Button* button) throw();
};


class LabelValueInternal :	public ValueInternal,
							public LabelListener
{
public:
	LabelValueInternal(Label* label) throw();
	~LabelValueInternal() throw();
	void labelTextChanged(Label* label) throw();
	
private:
	Label* label_;
};

class LabelValue : public Value 
{
public:
	LabelValue(Label* label) throw();
};





#endif // UGEN_JUCESLIDER_H