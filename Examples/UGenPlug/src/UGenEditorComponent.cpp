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

#include "JucePluginCharacteristics.h"
#include "includes.h"
#include "UGenEditorComponent.h"



//==============================================================================
UGenEditorComponent::UGenEditorComponent (UGenPlugin* const ownerFilter)
    :	AudioProcessorEditor (ownerFilter),
		margin(20),
		sliderHeight(22),
		sliderWidth(200),
		sliderLabelWidth(100),
		meterWidth(40),
		buttonHeight(22),
		menuLabelWidth(100),
		menuHeight(22),
		padding(2)
{
	/*
	 Create our sliders and their labels by getting info from the plugin 
	 */
	
	int height = margin;
	int width = sliderWidth + sliderLabelWidth + margin;
	
	if(UGenInterface::MenuOptions::Label != 0)
	{
		addAndMakeVisible(menuLabel = new Label(T("menuLabel"), 
												String(UGenInterface::MenuOptions::Label)));
		menuLabel->setJustificationType(Justification::right);
	}
	else
	{
		menuLabel = 0;
	}
	
	if(UGenInterface::MenuOptions::Count > 0)
	{
		addAndMakeVisible(menu = new ComboBox(T("menu")));
		
		for(int index = 0; index < UGenInterface::MenuOptions::Count; index++)
		{
			menu->addItem(UGenInterface::MenuOptions::Names[index], index + MENU_ID_OFFSET);
		}
		
		menu->setText(UGenInterface::MenuOptions::Names[0], false);
		menu->addListener(this);
		
		height += menuHeight + margin;
	}
	
	if(UGenInterface::Parameters::Count > 0)
	{
		for(int index = 0; index < getPlugin()->getNumParameters(); index++)
		{
			// get the parameter name, which is used several times here
			String name = String(getPlugin()->getParameterName(index));
			
			// create the slider...
			Slider* slider;
			addAndMakeVisible(slider = new Slider(name));
			slider->addListener (this);
			slider->setRange (0.0, 1.0, 0.000001);
			slider->setValue(getPlugin()->getParameter(index), false);
			sliders.add(slider);
			
			// ...and its label
			Label* sliderLabel;
			addAndMakeVisible(sliderLabel = new Label(name, name));
			sliderLabel->setJustificationType(Justification::right);
			sliderLabels.add(sliderLabel);
			
			// keep track of the height needed to display our sliders
			height += sliderHeight;
		}
		
		width += margin;
	}
	else
	{
		height += sliderHeight;
	}
	
	if(UGenInterface::Meters::Count > 0)
	{
		for(int index = 0; index < UGenInterface::Meters::Count; index++)
		{
			// get the meter name, which is used several times here
			String name = String(UGenInterface::Meters::Names[index]);
			
			// create the meter...
			MeterComponent* meter;
			addAndMakeVisible(meter = new MeterComponent(name, 
														 getPlugin()->getMeterLevelPtr(index), 
														 getPlugin()->getCallbackLock()));
			meters.add(meter);
			
			// ...and its label
			Label* meterLabel;
			addAndMakeVisible(meterLabel = new Label(name, name));
			meterLabels.add(meterLabel);
			
			width += meterWidth;
		}
		
		width += margin;
	}
	
	if(UGenInterface::Buttons::Count > 0)
	{
		for(int index = 0; index < UGenInterface::Buttons::Count; index++)
		{
			// get the button name, which is used several times here
			String name = String(UGenInterface::Buttons::Names[index]);
			
			TextButton* button;
			addAndMakeVisible(button = new TextButton(name));
			button->setTooltip(name); // just in case there are lots of buttons, we'll see the entire name via a tooltip
			button->addButtonListener(this);
			buttons.add(button);				  
		}
		
		height += buttonHeight + margin;
	}
	
	
	height += margin;
	width += margin;
	
	if(height < 120) height = 120;
	
    setSize (width, height);

    // register ourselves with the filter - it will use its ChangeBroadcaster base class to
    // tell us when something has changed, and this will call our changeListenerCallback() method.
    getPlugin()->addChangeListener (this);
}

UGenEditorComponent::~UGenEditorComponent()
{
    getPlugin()->removeChangeListener (this);

    deleteAllChildren();
}

//==============================================================================
void UGenEditorComponent::paint (Graphics& g)
{
    // just clear the window
    g.fillAll (Colour::greyLevel (0.9f));
}

void UGenEditorComponent::resized()
{
	/* 
	 this is a good place to position the components
	 even if their position is fixed
	 */
	
	int numMenuItems = UGenInterface::MenuOptions::Count;
	
	if(numMenuItems > 0)
	{
		if(menuLabel == 0)
			menu->setBounds(margin, margin, getWidth() - margin - margin, menuHeight - padding);
		else
		{
			menuLabel->setBounds(margin, margin,
								 menuLabelWidth - padding, menuHeight - padding);
			menu->setBounds(margin + menuLabelWidth, margin, 
							getWidth() - margin - margin - menuLabelWidth, menuHeight - padding);
		}
	}
	
	for(int index = 0; index < sliders.size(); index++)
	{
		int y = (index+1) * sliderHeight - padding;
		if(numMenuItems > 0)
			y += menuHeight + margin;
		sliders[index]->setBounds (sliderLabelWidth + margin + margin, y, sliderWidth, sliderHeight - padding);
		sliderLabels[index]->setBounds (margin, y, sliderLabelWidth, sliderHeight - padding);
	}
	
	for(int index = 0; index < meters.size(); index++)
	{
		int height = getPlugin()->getNumParameters() * sliderHeight;
		
		if(height < sliderHeight)
			height = sliderHeight;
		
		int y = margin * 0.5;
		if(numMenuItems > 0)
			y += menuHeight + margin;
		
		meters[index]->setBounds (meterWidth * index + sliderWidth + sliderLabelWidth + margin + margin + margin, 
								  y, 
								  meterWidth - padding, 
								  height - padding);
		meterLabels[index]->setBounds (meterWidth * index + sliderWidth + sliderLabelWidth + margin + margin + margin, 
									   y + height + padding, 
									   meterWidth, 
									   sliderHeight);
	}
	
	int numButtons = buttons.size();
	if(numButtons > 0)
	{
		int width = (getWidth() - margin - margin) / numButtons;
		int y = getPlugin()->getNumParameters() * sliderHeight;
		if(y < sliderHeight)
			y = sliderHeight;
		
		y += sliderHeight + margin;
		
		if(numMenuItems > 0)
			y += menuHeight + margin;
		
		for(int index = 0; index < numButtons; index++)
		{
			buttons[index]->setBounds(width * index + margin, y, width - padding, buttonHeight - padding);
		}
	}
}

//==============================================================================
void UGenEditorComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    // this is the filter telling us that it's changed
    updateParametersFromFilter();
}

void UGenEditorComponent::sliderValueChanged (Slider* changedSlider)
{
	// find the slider index
    int index = sliders.indexOf(changedSlider);
	
	// set the value in the plugin and notify the host of the change
	getPlugin()->setParameterNotifyingHost (index, (float) changedSlider->getValue());
}

void UGenEditorComponent::buttonClicked(Button* clickedButton)
{
	ScopedLock(getPlugin()->getCallbackLock());
	getPlugin()->buttonClicked(buttons.indexOf((TextButton*)clickedButton));
}

void UGenEditorComponent::comboBoxChanged(ComboBox* changedComboBox)
{
	ScopedLock(getPlugin()->getCallbackLock());
	getPlugin()->setMenuItem(changedComboBox->getSelectedId()-MENU_ID_OFFSET);
}

//==============================================================================
void UGenEditorComponent::updateParametersFromFilter()
{
	
	for(int index = 0; index < getPlugin()->getNumParameters(); index++)
	{
		// the value might be modified in a different thread, so lock the value 
		getPlugin()->getCallbackLock().enter();

		const float newValue = getPlugin()->getParameter (index);
	
		// ..release the lock ASAP
		getPlugin()->getCallbackLock().exit();

		/* Update our slider.

		   (note that it's important here to tell the slider not to send a change
		   message, because that would cause it to call the filter with a parameter
		   change message again, and the values would drift out.
		*/
		sliders[index]->setValue (newValue, false);
	}
	
	// get the menu selection, if we have any menu items
	if(UGenInterface::MenuOptions::Count > 0)
	{
		getPlugin()->getCallbackLock().enter();
		const int newMenuItem = getPlugin()->getMenuItem();
		getPlugin()->getCallbackLock().exit();
		menu->setSelectedId(newMenuItem + MENU_ID_OFFSET, false);
	}
}
