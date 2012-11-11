/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-12 The University of the West of England.
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


#include "UGenPlugin.h"
#include "UGenEditorComponent.h"

#define UGENIR_VERSION "beta v" JucePlugin_VersionString

static const char* ugen_IR_aboutHeading =
    "UGen++ IR Convolution " UGENIR_VERSION "\n";


static const char* ugen_IR_aboutText =
    "\n"
    "This plug-in is designed to be a simple impulse-response convolution plug-in.\n"
    "\n"
    " - Simply select a file via the 'IR File' page to load an impulse response\n"
    "    from a WAV or AIFF file.\n"
    " - The wet/dry mix can be set or automated via your DAW.\n"
    " - Amplitude and filter envelopes can be applied to the impulse response. \n"
    " - Select which envelope to edit using the menu at the bottom of the 'IR Display' page.\n"
    " - When either of the envelopes are change the impulse response is re-processed\n"
    "    and normalized.\n"
    " - Depending on the length of the impulse response, this may take a short time. \n"
    " - The plug-in stores the file path to the impulse response relative to the user's\n"
    "    home directory.\n"
    "\n"
    "What's new:\n"
    " - [0.1.3] Added reset buttons for the envelopes.\n"
    " - [0.1.3] Improved the layour of the about box.\n"
    " - [0.1.3] Added tooltips to the sliders.\n"
    " - [0.1.3] Now with adjustable speed control for the impulse response.\n"
    " - [0.1.3] Now with a choice of filter types: LPF, HPF, BPF, BRF.\n"
    " - [0.1.2] Now with amplitude and filter envelope editors.\n"
    " - [0.1.1] (Fixed) Deployment target on OS X.\n"
    "\n"
    "To do:\n"
    " - add optional channel modes for mono, stereo and true stereo;\n"
    "\n"
    "License\n"
    "This software is part of the UGEN++ library and uses Juce (www.juce.com)\n"
    "\n"
    "Copyright 2008-12 The University of the West of England.\n"
    " by Martin Robinson\n"
    "\n"
    " UGEN++ can be redistributed and/or modified under the terms of the\n"
    " GNU General Public License, as published by the Free Software Foundation;\n"
    " either version 2 of the License, or (at your option) any later version.\n"
    "\n"
    " UGEN++ is distributed in the hope that it will be useful,\n"
    " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    " GNU General Public License for more details.\n"
    "\n"
    " You should have received a copy of the GNU General Public License\n"
    " along with UGEN++; if not, visit www.gnu.org/licenses or write to the\n"
    " Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n"
    " Boston, MA 02111-1307 USA\n"
    "\n"
;

//==============================================================================

IRAmpLegendComponent::IRAmpLegendComponent(UGenEditorComponent* o, Text const& defaultText)
:   EnvelopeLegendComponent(defaultText),
    owner(o)
{
}

double IRAmpLegendComponent::mapTime(double time)
{
    return owner->getIRDuration() * time;
}

double IRAmpLegendComponent::mapValue(double value)
{
    return ampdb(value);
}

IRFilterLegendComponent::IRFilterLegendComponent(UGenEditorComponent* o, Text const& defaultText)
:   EnvelopeLegendComponent(defaultText),
    owner(o)
{
}

double IRFilterLegendComponent::mapTime(double time)
{
    return owner->getIRDuration() * time;
}

double IRFilterLegendComponent::mapValue(double value)
{
    return linexp(value, 0.0, 1.0, owner->getPlugin()->getFilterMin(), owner->getPlugin()->getFilterMax());
}

//==============================================================================

IRComponent::IRComponent(UGenEditorComponent* o)
:   owner(o)
{
    addAndMakeVisible(irScope = new ScopeComponent());
    irScope->setScaleY(ScopeGUI::LabelYAmplitude);
    irScope->setScopeColour(ScopeGUI::Trace, RGBAColour(0.8f, 0.7f, 0.7f, 0.8f));
    irScope->setScopeColour(ScopeGUI::LabelMarks, RGBAColour(0.1f, 0.9f, 0.1f, 0.5f));
    irScope->setScopeColour(ScopeGUI::ZeroLine, RGBAColour(0.1f, 0.7f, 0.1f, 0.5f));
    
    addAndMakeVisible(filterEnvEditor = new EnvelopeContainerComponent());
    filterEnvEditor->setEnvColour(EnvelopeComponent::Background, RGBAColour(0.1f, 0.1f, 0.1f, 0.4f));
    filterEnvEditor->setEnvColour(EnvelopeComponent::Node, RGBAColour(0.9f, 0.7f, 0.7f, 0.6f));
    filterEnvEditor->setEnvColour(EnvelopeComponent::Line, RGBAColour(1.0f, 0.7f, 0.7f, 0.6f));
    filterEnvEditor->setAllowNodeEditing(false);
    filterEnvEditor->setAllowCurveEditing(false);
    filterEnvEditor->setEnv(owner->getPlugin()->getIRFilterEnv());
    filterEnvEditor->setLegendComponent(new IRFilterLegendComponent (owner, ""));

    addAndMakeVisible(ampEnvEditor = new EnvelopeContainerComponent());
    ampEnvEditor->setEnvColour(EnvelopeComponent::Background, RGBAColour(0.1f, 0.1f, 0.1f, 0.4f));
    ampEnvEditor->setEnvColour(EnvelopeComponent::Node, RGBAColour(0.7f, 0.9f, 0.7f, 0.6f));
    ampEnvEditor->setEnvColour(EnvelopeComponent::Line, RGBAColour(0.7f, 1.0f, 0.7f, 0.6f));
    ampEnvEditor->setAllowNodeEditing(false);
    ampEnvEditor->setAllowCurveEditing(false);
    ampEnvEditor->setEnv(owner->getPlugin()->getIRAmpEnv());
    ampEnvEditor->setLegendComponent(new IRAmpLegendComponent (owner, ""));
    
    setEnvLocks();
    filterEnvEditor->addListener(owner);
    ampEnvEditor->addListener(owner);

    addAndMakeVisible(envSelect = new ComboBox());
    envSelect->addItem("Amplitude Envelope", SelectAmpEnv);
    envSelect->addItem("Filter Envelope", SelectFilterEnv);
    envSelect->addListener(this);
    envSelect->setSelectedId(SelectAmpEnv);
}

IRComponent::~IRComponent()
{
    deleteAllChildren();
}

void IRComponent::resized()
{
    const int legendHeight = ampEnvEditor->getLegendComponent()->getHeight();
    irScope->setBounds(0, 0, getWidth(), getHeight() - legendHeight - legendHeight);
    ampEnvEditor->setBounds(0, 0, getWidth(), getHeight() - legendHeight);
    filterEnvEditor->setBounds(0, 0, getWidth(), getHeight() - legendHeight);
    envSelect->setBounds(0, getHeight()-legendHeight, getWidth(), legendHeight);
}

void IRComponent::setEnvLocks()
{
    ampEnvEditor->getEnvelopeComponent()->getHandle(0)->lockTime(0.0);
    ampEnvEditor->getEnvelopeComponent()->getHandle(ampEnvEditor->getEnvelopeComponent()->getNumHandles()-1)->lockTime(1.0);
    ampEnvEditor->getEnvelopeComponent()->setMinMaxNumHandles(2, 50);
    
    filterEnvEditor->getEnvelopeComponent()->getHandle(0)->lockTime(0.0);
    filterEnvEditor->getEnvelopeComponent()->getHandle(filterEnvEditor->getEnvelopeComponent()->getNumHandles()-1)->lockTime(1.0);
    filterEnvEditor->getEnvelopeComponent()->setMinMaxNumHandles(2, 50);
}

void IRComponent::unsetEnvLocks()
{
    ampEnvEditor->getEnvelopeComponent()->setMinMaxNumHandles(0, 0xffffff);
    ampEnvEditor->getEnvelopeComponent()->getHandle(0)->unlockTime();
    ampEnvEditor->getEnvelopeComponent()->getHandle(ampEnvEditor->getEnvelopeComponent()->getNumHandles()-1)->unlockTime();
    
    filterEnvEditor->getEnvelopeComponent()->setMinMaxNumHandles(0, 0xffffff);
    filterEnvEditor->getEnvelopeComponent()->getHandle(0)->unlockTime();
    filterEnvEditor->getEnvelopeComponent()->getHandle(filterEnvEditor->getEnvelopeComponent()->getNumHandles()-1)->unlockTime();
}

void IRComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //DBG(String("Selected: ")+ comboBoxThatHasChanged->getText());
    switch (comboBoxThatHasChanged->getSelectedId())
    {
        case SelectAmpEnv: ampEnvEditor->toFront(false); break;
        case SelectFilterEnv: filterEnvEditor->toFront(false); break;
    }
}

//void IRComponent::handleCommandMessage(int commandId)
//{
//    switch (commandId)
//    {
//        case SetEnvLocks: setEnvLocks(); break;
//        case UnsetEnvLocks: unsetEnvLocks(); break;
//    }
//}

//==============================================================================
UGenEditorComponent::UGenEditorComponent (UGenPlugin* const ownerFilter)
    :	AudioProcessorEditor (ownerFilter),
        fileFilter("*.aif;*.aiff;*.wav", "", "Audio files"),
		margin(20),
		sliderHeight(22),
		sliderWidth(360),
		sliderLabelWidth(100),
		meterWidth(40),
		buttonHeight(22),
		menuLabelWidth(100),
		menuHeight(22),
		padding(2),
        tabsHeight(300)
{
	/*
	 Create our sliders and their labels by getting info from the plugin 
	 */
	
	int height = margin;
	int width = sliderWidth + sliderLabelWidth + margin;
	
	if(UGenInterface::MenuOptions::Label != 0)
	{
		addAndMakeVisible(menuLabel = new Label("menuLabel",
												String(UGenInterface::MenuOptions::Label)));
		menuLabel->setJustificationType(Justification::right);
	}
	else
	{
		menuLabel = 0;
	}
	
	if(UGenInterface::MenuOptions::Count > 0)
	{
		addAndMakeVisible(menu = new ComboBox("menu"));
		
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
			float minimum = getPlugin()->getParameterMin(index);
			float maximum = getPlugin()->getParameterMax(index);
            float nominal = getPlugin()->getParameterNominal(index);
			bool warp = getPlugin()->getParameterWarp(index);
			const String units = getPlugin()->getParameterUnits(index);
			
			// create the slider...
			PluginSlider* slider = new PluginSlider(name, minimum, maximum, nominal, warp, units);
			addAndMakeVisible(slider);
			slider->addListener (this);
                        
#ifdef __JUCE_NOTIFICATIONTYPE_JUCEHEADER__
            slider->setValue(getPlugin()->getParameter(index), dontSendNotification);
#else
			slider->setValue(getPlugin()->getParameter(index), false);
#endif
			
            sliders.add(slider);
			
			// ...and its label
			Label* sliderLabel;
			addAndMakeVisible(sliderLabel = new Label(name, name));
			sliderLabel->setJustificationType(Justification::right);
			sliderLabels.add(sliderLabel);
			
			// keep track of the height needed to display our sliders
			height += sliderHeight;
            
            String desc = getPlugin()->getParameterDescription(index);
            if (desc.length() > 0)
            {
                slider->setTooltip(desc);
                sliderLabel->setTooltip(desc);
            }
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
			button->addListener(this);
			buttons.add(button);				  
		}
		
		height += buttonHeight + margin;
	}
    
    
    addAndMakeVisible(tabs = new TabbedComponent(TabbedButtonBar::TabsAtLeft));
    tabButtons = &tabs->getTabbedButtonBar();
    tabButtons->addChangeListener(this);
        
    height += tabsHeight + margin + margin;

    // about tab
    {
        TextEditor* text = new TextEditor();
        text->setMultiLine(true);
        text->setReadOnly(true);
        text->setFont(Font(18.f, Font::italic|Font::bold));
        text->insertTextAtCaret(ugen_IR_aboutHeading);
        text->setFont(Font(13.f, Font::plain));
        text->insertTextAtCaret(ugen_IR_aboutText);
        tabs->addTab("About", Colour::greyLevel(0.75f).withAlpha(0.5f), text, true);
    }

    // ir view
    {
        irDisplay = new IRComponent(this);

        Text filename ((const char*)getPlugin()->getIRFile().getFullPathName().toUTF8());
        irDisplay->getIRScope()->setChannelLabels(filename + Text(":ch%d"));

        tabs->addTab("IR View", Colour::greyLevel(0.75f).withAlpha(0.5f), irDisplay, true);
        setIRDisplay(getPlugin()->getIRBuffer());
    }
    
    // file browser tab
    {
        int options =   FileBrowserComponent::openMode |
                        FileBrowserComponent::canSelectFiles |
                        FileBrowserComponent::filenameBoxIsReadOnly;
        
        File root = File::getSpecialLocation(File::userHomeDirectory);
//        if(!getPlugin()->getLastPath().isEmpty())
//            root = root.getSiblingFile(getPlugin()->getLastPath());
//        DBG(String("root: ")+root.getFullPathName());
        
        fileBrowser = new FileBrowserComponent(options, root, &fileFilter, 0);
//        fileBrowser->setFilenameBoxLabel("IR");
        fileBrowser->addListener(this);
        tabs->addTab("IR File", Colour::greyLevel(0.75f).withAlpha(0.5f), fileBrowser, true);
	}
    
    
    const int selectedTab = getPlugin()->getSelectedTab();
    if (selectedTab >= 0)
        tabs->setCurrentTabIndex(selectedTab);
    
	height += margin;
	width += margin;
	
	if(height < 120) height = 120;
	
    tooltipWindow.setMillisecondsBeforeTipAppears(1500);
    
    setSize (width, height);

    // register ourselves with the filter - it will use its ChangeBroadcaster base class to
    // tell us when something has changed, and this will call our changeListenerCallback() method.
    getPlugin()->addChangeListener (this);
}

UGenEditorComponent::~UGenEditorComponent()
{
    getPlugin()->removeChangeListener (this);
    tabButtons->removeChangeListener(this);
    
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
    
    tabs->setBounds(margin, getHeight()-tabsHeight-margin-margin,
                    getWidth()-margin-margin, tabsHeight);
}

//==============================================================================
void UGenEditorComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == tabButtons)
    {
        getPlugin()->setSelectedTab(tabButtons->getCurrentTabIndex());
    }
    else // if (source == getPlugin())
    {
        updateParametersFromFilter();
    }
}

void UGenEditorComponent::sliderValueChanged (Slider* changedSlider)
{
	// find the slider index
    int index = sliders.indexOf(static_cast<PluginSlider*>(changedSlider));
	
	ugen_assert(index >= 0); // invalid slider somehow
	
	// set the value in the plugin and notify the host of the change
	getPlugin()->setParameterNotifyingHost (index, (float) changedSlider->getValue());
}

void UGenEditorComponent::buttonClicked(Button* clickedButton)
{
	const ScopedLock sl(getPlugin()->getCallbackLock());
	getPlugin()->buttonClicked(buttons.indexOf((TextButton*)clickedButton));
}

void UGenEditorComponent::comboBoxChanged(ComboBox* changedComboBox)
{
	const ScopedLock sl(getPlugin()->getCallbackLock());
	getPlugin()->setMenuItem(changedComboBox->getSelectedId()-MENU_ID_OFFSET);
}

void UGenEditorComponent::envelopeChanged(EnvelopeComponent* changedEnvelope)
{
    getPlugin()->setIRAmpEnv(irDisplay->getAmpEnvEditor()->getEnv());
    getPlugin()->setIRFilterEnv(irDisplay->getFilterEnvEditor()->getEnv());
    getPlugin()->startTimer(250);
}

void UGenEditorComponent::envelopeStartDrag(EnvelopeComponent* changedEnvelope)
{
}

void UGenEditorComponent::envelopeEndDrag(EnvelopeComponent* changedEnvelope)
{
}

void UGenEditorComponent::selectionChanged()
{
//    // set the currently viewed path...
//    File root = fileBrowser->getRoot();
//    const String path = root.getRelativePathFrom(File::getSpecialLocation(File::userHomeDirectory));
//    DBG(String("lastPath: ")+path);
//    getPlugin()->setLastPath(path);
}

void UGenEditorComponent::fileClicked (const File& file, const MouseEvent& e)
{
    setFile(file);
}

void UGenEditorComponent::fileDoubleClicked (const File& file)
{
    setFile(file);
}

void UGenEditorComponent::browserRootChanged (const File& newRoot)
{
    //..
}

void UGenEditorComponent::setFile (const File& file)
{
    if (!file.isDirectory())
        getPlugin()->loadIR(file);
}

void UGenEditorComponent::setIRDisplay(Buffer const& irBuffer)
{
    if (!irBuffer.isNull())
        irDisplay->getIRScope()->setAudioBuffer(irBuffer);
}

double UGenEditorComponent::getIRDuration()
{
    return getPlugin()->getIRBuffer().isNull() ? 1.0 : getPlugin()->getIRBuffer().duration();
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
#ifdef __JUCE_NOTIFICATIONTYPE_JUCEHEADER__
        sliders[index]->setValue (newValue, dontSendNotification);
#else
		sliders[index]->setValue (newValue, false);
#endif
	}
	
	// get the menu selection, if we have any menu items
	if(UGenInterface::MenuOptions::Count > 0)
	{
		getPlugin()->getCallbackLock().enter();
		const int newMenuItem = getPlugin()->getMenuItem();
		getPlugin()->getCallbackLock().exit();
		menu->setSelectedId(newMenuItem + MENU_ID_OFFSET, false);
	}
    
    setIRDisplay(getPlugin()->getIRBuffer());
    
//    Text filename ((const char*)getPlugin()->getIRFile().getFullPathName().toUTF8());
//    irDisplay->getIRScope()->setChannelLabels(filename + Text(":ch%d"));
    
    tabs->setCurrentTabIndex(getPlugin()->getSelectedTab());
//    fileBrowser->setRoot(File::getSpecialLocation(File::userHomeDirectory).getChildFile(getPlugin()->getLastPath()));
//    fileBrowser->refresh();
    
//    irDisplay->unsetEnvLocks();
//    irDisplay->getAmpEnvEditor()->setEnv(getPlugin()->getIRAmpEnv());
//    irDisplay->setEnvLocks();
}
