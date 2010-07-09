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

#ifndef _UGEN_ugen_JuceMultiSlider_H_
#define _UGEN_ugen_JuceMultiSlider_H_

#include "../../core/ugen_UGen.h"
#include "../../buffers/ugen_Buffer.h"

class ScopedIgnoreMouse
{
public:
	ScopedIgnoreMouse(Component *c) : comp(c)
	{
		comp->setInterceptsMouseClicks(false, false);
	}
	
	~ScopedIgnoreMouse()
	{
		comp->setInterceptsMouseClicks(true, true);
	}
	
private:
	Component *comp;
};

class Interceptor : public Component
{
public:
	Interceptor(Component* _owner, const bool _constrain = false) 
	:	owner(_owner),
		mouseIsOver(0),
		mouseIsDownOn(0),
		constrain(_constrain)
	{ 
		setInterceptsMouseClicks(true, true);
	}
	
	void mouseEnter (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		mouseMove(e);
	}
	
	void mouseExit (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		
		if(mouseIsDownOn)
			mouseIsDownOn->mouseUp(e.getEventRelativeTo(mouseIsDownOn));
		
		mouseIsDownOn = 0;
		
		if(mouseIsOver)
			mouseIsOver->mouseExit(e.getEventRelativeTo(mouseIsOver));
		
		mouseIsOver = 0;
	}
	
	void mouseMove (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		
		Component *comp = owner->getComponentAt(e.x, e.y);
		
		if(comp == mouseIsOver)
		{
			if(mouseIsOver)
				mouseIsOver->mouseMove(e.getEventRelativeTo(mouseIsOver));
		}
		else
		{
			if(mouseIsOver)
				mouseIsOver->mouseExit(e.getEventRelativeTo(mouseIsOver));
			
			mouseIsOver = comp;
			
			if(mouseIsOver)
				mouseIsOver->mouseEnter(e.getEventRelativeTo(mouseIsOver));
		}
	}
	
	void mouseDown (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		
		if(mouseIsOver)
		{
			mouseIsDownOn = mouseIsOver;
			mouseIsDownOn->mouseDown(e.getEventRelativeTo(mouseIsDownOn));
			
			lastDragX = e.x;
			lastDragY = e.y;
		}
	}
	
	void mouseUp (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		
		if(mouseIsDownOn)
		{
			mouseIsDownOn->mouseUp(e.getEventRelativeTo(mouseIsDownOn));
			mouseIsDownOn = 0;
		}
	}
	
	void mouseDrag (const MouseEvent& e)
	{
		ScopedIgnoreMouse m(this);
		
		Component *comp = owner->getComponentAt(e.x, e.y);
		
		if(comp == mouseIsDownOn)
		{
			if(mouseIsDownOn)
				mouseIsDownOn->mouseDrag(e.getEventRelativeTo(mouseIsDownOn));
		}
		else
		{
			int deltaX = e.x - lastDragX;
			int deltaY = e.y - lastDragY;
			double incX, incY;
			int steps;
			
			if(deltaX == 0)
			{
				incX = 0;
				incY = e.y > lastDragY ? 1.0 : -1.0;
				steps = std::abs(deltaY);
			}
			else if(deltaY == 0)
			{
				incX = e.x > lastDragX ? 1.0 : -1.0;
				incY = 0;
				steps = std::abs(deltaX);
			}
			else if(std::abs(deltaX) > std::abs(deltaY))
			{
				incX = e.x > lastDragX ? 1.0 : -1.0;;
				incY = (double)deltaY / std::abs(deltaX);
				steps = std::abs(deltaX);
			}
			else
			{
				incX = (double)deltaX / std::abs(deltaY);
				incY = e.y > lastDragY ? 1.0 : -1.0;
				steps = std::abs(deltaY);
			}
			
			double x = lastDragX;
			double y = lastDragY;
						
			while(steps--)
			{
				MouseEvent eventCopy = e.withNewPosition(Point<int>(constrain ? jlimit(1, getWidth()-2, (int)x) : (int)x, 
																	constrain ? jlimit(1, getHeight()-2, (int)y) : (int)y));
								
				Component *comp = owner->getComponentAt(eventCopy.x, eventCopy.y);
				if(comp == mouseIsDownOn)
				{
					if(mouseIsDownOn)
						mouseIsDownOn->mouseDrag(eventCopy.getEventRelativeTo(mouseIsDownOn));
				}
				else
				{
					if(mouseIsDownOn)
					{
						mouseIsDownOn->mouseUp(eventCopy.getEventRelativeTo(mouseIsDownOn));
						mouseIsDownOn->mouseExit(eventCopy.getEventRelativeTo(mouseIsDownOn));
					}
					
					mouseIsDownOn = comp;
					
					if(mouseIsDownOn)
					{
						mouseIsDownOn->mouseEnter(eventCopy.getEventRelativeTo(mouseIsDownOn));
						mouseIsDownOn->mouseDown(eventCopy.getEventRelativeTo(mouseIsDownOn));
					}
				}
				
				x += incX;
				y += incY;
			}
		}
		
		lastDragX = e.x;
		lastDragY = e.y;
	}	
	
private:
	Component* const owner;
	Component* mouseIsOver;
	Component* mouseIsDownOn;
	int lastDragX, lastDragY;
	bool constrain;
};


class MultiSliderBase : public Component
{
public:
	MultiSliderBase(const int numSliders = 1, 
					const bool canDragAcross = true, 
					const bool slidersAreHorizontal = true,
					const double sliderMinimum = 0.0,
					const double sliderMaximum = 1.0) throw()
	:	interceptor(0), horizontal(true)
	{
		setNumSlidersInternal(numSliders, canDragAcross, slidersAreHorizontal);
	}
	
	~MultiSliderBase()
	{
		deleteAllChildren();
	}
	
	
	int getNumSliders() const throw() { return sliders.size(); }

	void setNumSliders(const int numSliders, 
					   const bool canDragAcross = true,
					   const bool slidersAreHorizontal = true,
					   const double sliderMinimum = 0.0,
					   const double sliderMaximum = 1.0) throw()
	{
		setNumSlidersInternal(numSliders, canDragAcross, slidersAreHorizontal, 
							  sliderMinimum, sliderMaximum);
		resized();
	}
	
	void setNumSlidersInternal(const int numSliders, 
							   const bool canDragAcross = true,
							   const bool slidersAreHorizontal = true,
							   const double sliderMinimum = 0.0,
							   const double sliderMaximum = 1.0) throw()
	{
		if(numSliders != sliders.size())
		{
			deleteAllChildren();
			sliders.clear();
			interceptor = 0;
			
			horizontal = slidersAreHorizontal;
			
			for(int i = 0; i < numSliders; i++)
			{
				Slider *slider = new Slider(String("Multislider")+String(i));
				slider->setSliderStyle(horizontal ? Slider::LinearHorizontal : Slider::LinearVertical);
				slider->setTextBoxStyle(Slider::NoTextBox, 0,0,0);
				slider->setRange(sliderMinimum, sliderMaximum, 0.0);
				addAndMakeVisible(slider);
				sliders.add(slider);
			}
			
			if(canDragAcross)
				addAndMakeVisible(interceptor = new Interceptor(this, true));
			
			return;
		}
		
		if(canDragAcross == true && interceptor == 0)
		{
			addAndMakeVisible(interceptor = new Interceptor(this, true));
		}
		else if(canDragAcross == false && interceptor != 0)
		{
			removeChildComponent(interceptor);
			deleteAndZero(interceptor);
		}
		
		if(slidersAreHorizontal != horizontal)
		{
			horizontal = slidersAreHorizontal;
			
			for(int i = 0; i < sliders.size(); i++)
			{
				sliders[i]->setSliderStyle(horizontal ? Slider::LinearHorizontal : Slider::LinearVertical);
			}
		}
		
	}
	
	void setRange(const double newMinimum,
				  const double newMaximum,
				  const double newInterval = 0) 
	{
		for(int i = 0; i < sliders.size(); i++)
		{
			sliders[i]->setRange(newMinimum, newMaximum, newInterval);
		}
	}
	
	void resized()
	{
		if(interceptor)
			interceptor->setBounds(0, 0, getWidth(), getHeight());
		
		if(horizontal)
		{
			const float sliderHeight = (float)getHeight() / sliders.size();
			float y = 0.f;
			for(int i = 0; i < sliders.size(); i++)
			{
				sliders[i]->setBounds(0, roundFloatToInt(y), getWidth(), sliderHeight);
				y += sliderHeight;
			}
		}
		else
		{
			const float sliderWidth = (float)getWidth() / sliders.size();
			float x = 0.f;
			for(int i = 0; i < sliders.size(); i++)
			{
				sliders[i]->setBounds(roundFloatToInt(x), 0, sliderWidth, getHeight());
				x += sliderWidth;
			}
		}
	}
	
	Slider* getSlider(const int index) 
	{
		return sliders[index];
	}
	
	double getValue(const int index) 
	{
		Slider* slider = sliders[index];
		return slider ? slider->getValue() : 0.0;
	}
	
	void getValues(Array<double> &values)
	{
		values.clearQuick();
		values.ensureStorageAllocated(sliders.size());
		for(int i = 0; i < sliders.size(); i++)
		{
			values.add(sliders[i]->getValue());
		}
	}
	
	void setValue (const int index,
				   double newValue,
				   const bool sendUpdateMessage = true,
				   const bool sendMessageSynchronously = false)
	{
		Slider* slider = sliders[index];
		if(slider)
		{
			slider->setValue(newValue, sendUpdateMessage, sendMessageSynchronously);
		}
	}
	
	void setValues(Array<double> const& values,
				   const bool sendUpdateMessage = true,
				   const bool sendMessageSynchronously = false)
	{
		const int size = jmin(values.size(), sliders.size());
		for(int i = 0; i < size; i++)
		{
			sliders[i]->setValue(values[i], sendUpdateMessage, sendMessageSynchronously);
		}
	}
	
protected:
	Array<Slider*> sliders;
	Interceptor *interceptor;
	bool horizontal;
};

class MultiSlider : public MultiSliderBase,
					public BufferReceiver
{
public:
	enum CommandIDs { Update = -1 };
	
	MultiSlider(const int numSliders = 1, 
				const bool canDragAcross = true, 
				const bool slidersAreHorizontal = true,
				const double sliderMinimum = 0.0,
				const double sliderMaximum = 1.0) throw()
	:	MultiSliderBase(numSliders, canDragAcross, slidersAreHorizontal, sliderMinimum, sliderMaximum),
		attachedToMultiSliderUGenInternal(false)
	{
	}
	
	template<class NumericalType>
	NumericalArray<NumericalType> getValues()
	{
		NumericalArray<NumericalType> values = NumericalArray<NumericalType>::withSize(sliders.size());
		for(int i = 0; i < sliders.size(); i++)
		{
			NumericalArray<NumericalType>::roundCopy(sliders[i]->getValue(), values[i]);
		}
		
		return values;
	}
	
	template<class NumericalType>
	void setValues(NumericalArray<NumericalType> const& values,
				   const bool sendUpdateMessage = true,
				   const bool sendMessageSynchronously = false)
	{
		const int size = jmin(values.length(), sliders.size());
		for(int i = 0; i < size; i++)
		{
			sliders[i]->setValue((double)values[i], sendUpdateMessage, sendMessageSynchronously);
		}
	}
	
	void setValues(Buffer const& values,
				   const bool sendUpdateMessage = true,
				   const bool sendMessageSynchronously = false)
	{
		const int size = jmin(values.size(), sliders.size());
		for(int i = 0; i < size; i++)
		{
			sliders[i]->setValue((double)values.getSampleUnchecked(i), sendUpdateMessage, sendMessageSynchronously);
		}
	}
	
	bool attachToMultiSliderUGenInternal() throw()
	{
		if(attachedToMultiSliderUGenInternal)
			return false;
		else
		{
			attachedToMultiSliderUGenInternal = true;
			return true;
		}
	}
	
	bool detachFromMultiSliderUGenInternal() throw()
	{
		if(attachedToMultiSliderUGenInternal)
		{
			attachedToMultiSliderUGenInternal = false;
			return true;
		}
		else
			return false;
	}
	
	void handleBuffer(Buffer const& buffer, const double value1, const int value2)
	{
		if(lock.tryEnter())
		{
			values = buffer;
			lock.exit();
			postCommandMessage(Update);
		}
	}
	
	void handleCommandMessage(int commandID)
	{
		if(commandID == Update)
		{
			if(lock.tryEnter())
			{
				Buffer copy = values;
				lock.exit();
				setNumSliders(copy.size());
				setValues(copy);
			}
		}
	}
	
	UGen asUGen()
	{
		UGen result;
		
		for(int i = 0; i < sliders.size(); i++)
		{
			result = (result, sliders[i]);
		}
		
		return result;
	}
	
private:
	bool attachedToMultiSliderUGenInternal;
	Buffer values;
	CriticalSection lock;
};


class MultiSliderUGenInternal : public ProxyOwnerUGenInternal,
								public Timer
{
public:
	MultiSliderUGenInternal(UGen const& input, MultiSlider *sliders) throw();
	~MultiSliderUGenInternal();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void timerCallback();
	
	enum Inputs { Input, NumInputs };
	
protected:
	MultiSlider *sliders;
	double *values;
	bool oldClick, oldChildClick;
};


UGenSublcassDeclarationNoDefault(MultiSliderUGen, 
								 (input, sliders), 
								 (UGen const& input, MultiSlider *sliders),
								 COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_JuceMultiSlider_H_
