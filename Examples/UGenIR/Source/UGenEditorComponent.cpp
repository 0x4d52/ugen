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

#define UGENIR_VERSION "alpha v" JucePlugin_VersionString

static const char* ugen_IR_aboutText =
    "UGen++ IR Convolution " UGENIR_VERSION "\n"
    "\n"
    "This plug-in is designed to be a simple impulse-response convolution plug-in.\n"
    "Simply select a file via the 'IR File' tab to load an impulse response from a WAV or AIFF file.\n"
    "The wet/dry mix can be set or automated via your DAW.\n"
    "\n"
    "The plug-in stores the file path to the impulse response relative to the user's home directory.\n"
    "\n"
    "Bug fixes:\n"
    " - Deplpoyment target on OS X\n"
    "\n"
    "To do:\n"
    " - add optional channel modes for mono, stereo and true stereo;\n"
    " - add amplitude and filter envelope overlays to the 'IR View'\n"
    "\n"
    "License\n"
    "This software is part of the UGEN++ library and uses Juce (www.juce.com)\n"
    "\n"
    "Copyright 2008-12 The University of the West of England.\n"
    "by Martin Robinson\n"
    "\n"
    "UGEN++ can be redistributed and/or modified under the terms of the\n"
    "GNU General Public License, as published by the Free Software Foundation;\n"
    "either version 2 of the License, or (at your option) any later version.\n"
    "\n"
    "UGEN++ is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with UGEN++; if not, visit www.gnu.org/licenses or write to the\n"
    "Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n"
    "Boston, MA 02111-1307 USA\n"
    "\n"
;

//==============================================================================

IRLegendComponent::IRLegendComponent(UGenEditorComponent* o, Text const& defaultText)
:   EnvelopeLegendComponent(defaultText),
    owner(o)
{
}

double IRLegendComponent::mapTime(double time)
{
    return owner->getIRDuration() * time;
}


//==============================================================================

IRComponent::IRComponent(UGenEditorComponent* o)
:   owner(o)
{
    addAndMakeVisible(irScope = new ScopeComponent());
    irScope->setScaleY(ScopeGUI::LabelYAmplitude);
    irScope->setScopeColour(ScopeGUI::Trace, RGBAColour(0.5f, 0.4f, 0.4f, 0.6f));
    irScope->setScopeColour(ScopeGUI::LabelMarks, RGBAColour(0.1f, 0.9f, 0.1f, 0.5f));
    irScope->setScopeColour(ScopeGUI::ZeroLine, RGBAColour(0.1f, 0.7f, 0.1f, 0.5f));
    
    addAndMakeVisible(ampEnvEditor = new EnvelopeContainerComponent());
    ampEnvEditor->setEnvColour(EnvelopeComponent::Background, RGBAColour(0.f, 0.f, 0.f, 0.f));
    ampEnvEditor->setAllowNodeEditing(false);
    ampEnvEditor->setAllowCurveEditing(false);
    ampEnvEditor->getEnvelopeComponent()->addHandle(0.0, 1.0, EnvCurve::Linear);
    ampEnvEditor->getEnvelopeComponent()->addHandle(1.0, 1.0, EnvCurve::Linear);
    ampEnvEditor->getEnvelopeComponent()->getHandle(0)->lockTime(0.0);
    ampEnvEditor->getEnvelopeComponent()->getHandle(1)->lockTime(1.0);
    ampEnvEditor->getEnvelopeComponent()->setMinMaxNumHandles(2, 20);
    
    ampEnvEditor->setLegendComponent(new IRLegendComponent (owner, "Amplitude Envelope"));
    ampEnvEditor->addListener(owner);
    
}

IRComponent::~IRComponent()
{
    deleteAllChildren();
}

void IRComponent::resized()
{
    irScope->setBounds(0, 0, getWidth(), getHeight() - ampEnvEditor->getLegendComponent()->getHeight());
    ampEnvEditor->setBounds(0, 0, getWidth(), getHeight());
}


//==============================================================================
UGenEditorComponent::UGenEditorComponent (UGenPlugin* const ownerFilter)
    :	AudioProcessorEditor (ownerFilter),
        fileFilter("*.aif;*.aiff;*.wav", "", "Audio files"),
		margin(20),
		sliderHeight(22),
		sliderWidth(400),
		sliderLabelWidth(60),
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
        text->setText(ugen_IR_aboutText);
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
	
    setSize (width, height);

    // register ourselves with the filter - it will use its ChangeBroadcaster base class to
    // tell us when something has changed, and this will call our changeListenerCallback() method.
    getPlugin()->addChangeListener (this);
    processManager.addBufferReceiver(getPlugin());
}

UGenEditorComponent::~UGenEditorComponent()
{
    processManager.removeBufferReceiver(getPlugin());

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
//    if (irDisplay->getAmpEnvEditor()->getEnvelopeComponent() == changedEnvelope)
//    {
//        //..
//    }
    
    Buffer originalBuffer = getPlugin()->getOriginalBuffer();
    Env ampEnv = irDisplay->getAmpEnvEditor()->getEnv().timeScale(originalBuffer.duration());
    
    UGen player = PlayBuf::AR(originalBuffer, 1.0, 0, 0, 0, UGen::DoNothing) * EnvGen::AR(ampEnv);
    
    processManager.add(originalBuffer.size(), player);
}

void UGenEditorComponent::envelopeStartDrag(EnvelopeComponent* changedEnvelope)
{
    //DBG("start drag");
}

void UGenEditorComponent::envelopeEndDrag(EnvelopeComponent* changedEnvelope)
{
    //DBG("end drag");
    
//    Buffer originalBuffer = getPlugin()->getOriginalBuffer();
//    Env ampEnv = irDisplay->getAmpEnvEditor()->getEnv().timeScale(originalBuffer.duration());
//    
//    UGen player = PlayBuf::AR(originalBuffer, 1.0, 0, 0, 0, UGen::DoNothing) * EnvGen::AR(ampEnv);
//        
//    processManager.add(originalBuffer.size(), player);
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
    
    Text filename ((const char*)getPlugin()->getIRFile().getFullPathName().toUTF8());
    irDisplay->getIRScope()->setChannelLabels(filename + Text(":ch%d"));
    
    tabs->setCurrentTabIndex(getPlugin()->getSelectedTab());
//    fileBrowser->setRoot(File::getSpecialLocation(File::userHomeDirectory).getChildFile(getPlugin()->getLastPath()));
//    fileBrowser->refresh();
}
