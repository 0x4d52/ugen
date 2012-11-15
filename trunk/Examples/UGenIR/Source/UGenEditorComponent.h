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


#ifndef UGENEDITORCOMPONENT_H
#define UGENEDITORCOMPONENT_H

#include "../JuceLibraryCode/JuceHeader.h"
#include "UGenCommon.h"
#include "UGenPlugin.h"

class UGenEditorComponent;

class IRAmpLegendComponent : public EnvelopeLegendComponent
{
public:
    IRAmpLegendComponent(UGenEditorComponent* owner, Text const& defaultText = Text::empty);
    
    double mapTime(double time);
    String getTimeUnits() { return "s"; }
    double mapValue(double value);
    String getValueUnits() { return "dB"; }

    
private:
    UGenEditorComponent* owner;
};

class IRFilterLegendComponent : public EnvelopeLegendComponent
{
public:
    IRFilterLegendComponent(UGenEditorComponent* owner, Text const& defaultText = Text::empty);
    
    double mapTime(double time);
    String getTimeUnits() { return "s"; }
    double mapValue(double value);
    String getValueUnits() { return "Hz"; }

    
private:
    UGenEditorComponent* owner;
};


class IRComponent : public Component,
                    public ComboBox::Listener
{
public:
    IRComponent(UGenEditorComponent* owner);
    ~IRComponent();
    
    void resized();
    
    void setEnvLocks();
    void unsetEnvLocks();
    
//    enum Messages
//    {
//        SetEnvLocks,
//        UnsetEnvLocks
//    };
    
    inline ScopeComponent* getIRScope() { return irScope; }
    inline EnvelopeContainerComponent* getAmpEnvEditor() { return ampEnvEditor; }
    inline EnvelopeContainerComponent* getFilterEnvEditor() { return filterEnvEditor; }
    
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    
//    void handleCommandMessage(int commandId);
    
    enum EnvSelect
    {
        SelectAmpEnv = 1,
        SelectFilterEnv = 2
    };
    
private:
    UGenEditorComponent* owner;
    ScopeComponent* irScope;
    EnvelopeContainerComponent* ampEnvEditor;
    EnvelopeContainerComponent* filterEnvEditor;
    ComboBox* envSelect;

};


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
							public ComboBoxListener,
                            public FileBrowserListener,
                            public EnvelopeComponentListener
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
    
    void envelopeChanged(EnvelopeComponent* changedEnvelope);
    void envelopeStartDrag(EnvelopeComponent* changedEnvelope);
    void envelopeEndDrag(EnvelopeComponent* changedEnvelope);
    
    void selectionChanged();
    void fileClicked (const File& file, const MouseEvent& e);
    void fileDoubleClicked (const File& file);
    void browserRootChanged (const File& newRoot);
    void setFile (const File& file);
    
    void setIRDisplay(Buffer const& irBuffer);
    double getIRDuration();
    
    IRComponent* getIRComponent() { return irDisplay; }

    //==============================================================================
    /** Standard Juce paint callback. */
    void paint (Graphics& g);

    /** Standard Juce resize callback. */
    void resized();
	
    friend class IRComponent;
    friend class IRFilterLegendComponent;

private:
    //==============================================================================
    Array<PluginSlider*> sliders;
	Array<Label*> sliderLabels;
	
	Array<MeterComponent*> meters;
	Array<Label*> meterLabels;
	
	Array<TextButton*> buttons;
	
	Label* menuLabel;
	ComboBox* menu;
	Label* menu2Label;
	ComboBox* menu2;

    
    TabbedComponent* tabs;
    TabbedButtonBar* tabButtons;

    const WildcardFileFilter fileFilter;
    FileBrowserComponent* fileBrowser;
    
    IRComponent* irDisplay;
    
    TooltipWindow tooltipWindow;
	
	int margin, sliderHeight, sliderWidth, 
		sliderLabelWidth, meterWidth, buttonHeight, 
		menuLabelWidth, menuHeight, padding, tabsHeight;

    void updateParametersFromFilter();

    // handy wrapper method to avoid having to cast the filter to a UGenPlugin
    // every time we need it..
    UGenPlugin* getPlugin() const throw()       { return (UGenPlugin*) getAudioProcessor(); }
};


#endif // UGENEDITORCOMPONENT_H
