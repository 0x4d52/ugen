/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.
   Modified 2008 Martin Robinson (UWE)
  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef UGENEDITORCOMPONENT_H
#define UGENEDITORCOMPONENT_H

#include "UGenCommon.h"
#include "UGenPlugin.h"


//==============================================================================
/**
    This is the Component that our filter will use as its UI.

    One or more of these is created by the UGenPlugin::createEditor() method,
    and they will be deleted at some later time by the wrapper code.

    To demonstrate the correct way of connecting a filter to its UI, this
    class is a ChangeListener, and our demo filter is a ChangeBroadcaster. The
    editor component registers with the filter when it's created and deregisters
    when it's destroyed. When the filter's parameters are changed, it broadcasts
    a message and this editor responds by updating its display.
*/
class UGenEditorComponent  : public AudioProcessorEditor,
							public ChangeListener,
							public SliderListener,
							public ButtonListener,
							public ComboBoxListener
{
public:
    /** Constructor.

        When created, this will register itself with the filter for changes. It's
        safe to assume that the filter won't be deleted before this object is.
    */
    UGenEditorComponent (UGenPlugin* const ownerFilter);

    /** Destructor. */
    ~UGenEditorComponent();

    //==============================================================================
    /** Our demo filter is a ChangeBroadcaster, and will call us back when one of
        its parameters changes.
    */
    void changeListenerCallback (ChangeBroadcaster* source);

    void sliderValueChanged (Slider*);
	void buttonClicked(Button* clickedButton);
	void comboBoxChanged(ComboBox* changedComboBox);

    //==============================================================================
    /** Standard Juce paint callback. */
    void paint (Graphics& g);

    /** Standard Juce resize callback. */
    void resized();


	class MeterComponent :	public Component,
							public Timer
	{
	public:
		MeterComponent(String& name, float* valueToUse, const CriticalSection& lockToUse)
		:	Component(name),
			value(valueToUse),
			lastDisplayValue(0.f),
			lock(lockToUse)
		{
			startTimer((int)(METER_UPDATE_TIME * 1000));
		}
		
		~MeterComponent()
		{
			stopTimer();
		}
		
		void paint(Graphics& g)
		{
			lock.enter();
			float currentValue = jlimit(0.f, 1.f, zap(*value));
			lock.exit();
			
			g.fillAll(Colours::black);
			g.setColour(Colour(0xFF00FF00));
			
			if(getWidth() > getHeight())
			{
				// horizontal meter
				g.fillRect(0, 0, (int)(getWidth() * currentValue), getHeight());
			}
			else
			{
				// vertical meter
				g.fillRect(0, getHeight() - (int)(getHeight() * currentValue), 
						   getWidth(), (int)(getHeight() * currentValue));
			}
		}
		
		void timerCallback()
		{
			lock.enter();
			float currentValue = *value;
			lock.exit();
			
			if(lastDisplayValue != currentValue)
			{	
				lastDisplayValue = currentValue;
				repaint();
			}
		}
		
		
	private:
		float* value;
		float lastDisplayValue;
		const CriticalSection& lock;
	};
	
private:
    //==============================================================================
    Array<Slider*> sliders;
	Array<Label*> sliderLabels;
	
	Array<MeterComponent*> meters;
	Array<Label*> meterLabels;
	
	Array<TextButton*> buttons;
	
	Label* menuLabel;
	ComboBox* menu;
	
    TooltipWindow tooltipWindow;
	
	int margin, sliderHeight, sliderWidth, 
		sliderLabelWidth, meterWidth, buttonHeight, 
		menuLabelWidth, menuHeight, padding;

    void updateParametersFromFilter();

    // handy wrapper method to avoid having to cast the filter to a UGenPlugin
    // every time we need it..
    UGenPlugin* getPlugin() const throw()       { return (UGenPlugin*) getAudioProcessor(); }
};


#endif // UGENEDITORCOMPONENT_H
