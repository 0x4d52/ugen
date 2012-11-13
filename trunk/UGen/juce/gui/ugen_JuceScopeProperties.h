#ifndef UGEN_JUCESCOPEPROPERTIES
#define UGEN_JUCESCOPEPROPERTIES

#ifndef UGEN_ANDROID // fixme

#include "../ugen_JuceUtility.h"
#include "../../core/ugen_Bits.h"

// this file is designed only to be included by ugen_JuceScope.cpp

static const int labelDecimalPlaces = 10;

/** A base class for boolean properties */
class BooleanValuePropertyComponent :		public BooleanPropertyComponent,
											public juce::Value::Listener
{
public:
	BooleanValuePropertyComponent(const String& propertyName, 
								  const String& buttonText, 
								  juce::Value initValue = juce::Value())
	:	BooleanPropertyComponent(initValue, propertyName, buttonText),
		value(initValue)
	{
		value.addListener(this);
	}
	
	void valueChanged (juce::Value& changedValue)
	{
		if(changedValue.refersToSameSourceAs(value))
		{
			stateChanged(changedValue.getValue());
		}
	}	
	
	virtual void stateChanged(const bool state) = 0;
	
private:
	juce::Value value;
};


/** A base class for colour properties */
class ColourPropertyComponent : public TextPropertyComponent
{
public:
	class ColourPropertyInterceptor :	public Interceptor,
										public ChangeListener
	{
	public:
		ColourPropertyInterceptor(ColourPropertyComponent* o) 
		:	Interceptor(o),
			owner(o)
		{
			colourSelector.addChangeListener(this);
		}
		
		~ColourPropertyInterceptor()
		{
			colourSelector.removeChangeListener(this);
		}
		
		void mouseDown(const MouseEvent& e)
		{
			if(e.mods.isPopupMenu())
			{
				showColourComponent();
			}
			else
			{
				Interceptor::mouseDown(e);
			}
		}
		
		void showColourComponent()
		{
			colourSelector.setSize (240, 320);
			colourSelector.setCurrentColour(Colour::fromString(owner->getText()));	

#if UGEN_JUCE_NEWCALLOUTBOX
            CallOutBox callOut (colourSelector, owner->getBounds(), 0);
#else
			CallOutBox callOut (colourSelector, *owner, 0);
#endif
            
			callOut.runModalLoop();
		}
		
		void changeListenerCallback (ChangeBroadcaster* /*source*/)
		{
			owner->setText(colourSelector.getCurrentColour().toString());
			repaint();
		}
		
		void paintOverChildren (Graphics& g)
		{
			int size = getHeight()-8;
			
			Rectangle<int> rect(getWidth()-size-4, 4, size, size-1);
			g.fillCheckerBoard(rect, rect.getWidth()*0.5+0.5, rect.getHeight()*0.5+0.5, 
							   Colour(0xffdddddd), 
							   Colour(0xffffffff));
		
			g.setColour(Colour::fromString(owner->getText()));
			g.fillRect(rect);
		}
		
	private:
		ColourPropertyComponent* owner;
		ColourSelector colourSelector;
	};
	
	ColourPropertyComponent(const String& propertyName)
	:	TextPropertyComponent(propertyName, 8, false),
		interceptor(this)
	{
		addAndMakeVisible(&interceptor);
	}
	
	void resized()
	{
		TextPropertyComponent::resized();
		interceptor.setBounds(0, 0, getWidth(), getHeight());
	}
		
private:
	ColourPropertyInterceptor interceptor;
};

/** ScopeGUI : Is Bipolar Property*/
class ScopeComponentIsBipolarProperty :	public BooleanValuePropertyComponent
{
public:
	ScopeComponentIsBipolarProperty(ScopeControlComponent* targetScope)
	:	BooleanValuePropertyComponent("Polarity", "is bipolar"),
		scope(targetScope)
	{
		setState(scope->getPolarity());
	}
	
	void stateChanged(const bool state)
	{
		scope->setPolarity(state);
	}
	
private:
	ScopeControlComponent* scope;
};


/** ScopeGUI : Y Maximum Property */
class ScopeComponentYMaximumProperty : public TextPropertyComponent
{
public:
	ScopeComponentYMaximumProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Y Maximum", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setYMaximum(jmax(0.00000001f, newText.getFloatValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getYMaximum(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Lower Margin Property */
class ScopeComponentLowerMarginProperty : public TextPropertyComponent
{
public:
	ScopeComponentLowerMarginProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Lower Margin", 10, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setLowerMargin(newText.getFloatValue());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getLowerMargin(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale X Units */
class ScopeComponentScaleXProperty : public ChoicePropertyComponent
{
public:
	ScopeComponentScaleXProperty(ScopeControlComponent* targetScope)
	:	ChoicePropertyComponent("Scale X Units"),
		scope(targetScope) 
	{
		choices.add("None");
		choices.add("Marks");
		choices.add("Samples");
		choices.add("Time");
		choices.add("Frequency");
	}

	void setIndex (int newIndex)
	{
		const String& choice = getChoices()[newIndex];
		
		const bool labelFirst = scope->getLabelFirstX();
		
		if(choice == "None")			scope->setScaleX(ScopeGUI::LabelXNone, -1, -1, labelFirst);
		else if(choice == "Marks")		scope->setScaleX(ScopeGUI::LabelXMarks, -1, -1, labelFirst);
		else if(choice == "Samples")	scope->setScaleX(ScopeGUI::LabelXSamples, -1, -1, labelFirst);
		else if(choice == "Time")		scope->setScaleX(ScopeGUI::LabelXTime, -1, -1, labelFirst);
		else if(choice == "Frequency")	scope->setScaleX(ScopeGUI::LabelXFrequency, -1, -1, labelFirst);
	}
	
	int getIndex() const
	{
		const StringArray& choices = getChoices();
		
		switch(scope->getScaleX())
		{
			case ScopeGUI::LabelXNone:		return choices.indexOf("None");
			case ScopeGUI::LabelXMarks:		return choices.indexOf("Marks");
			case ScopeGUI::LabelXSamples:	return choices.indexOf("Samples");
			case ScopeGUI::LabelXTime:		return choices.indexOf("Time");
			case ScopeGUI::LabelXFrequency: return choices.indexOf("Frequency");
			default: return -1;
		}
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale X Mark Spacing Property */
class ScopeComponentMarkSpacingXProperty : public TextPropertyComponent
{
public:
	ScopeComponentMarkSpacingXProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Mark Spacing X", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		const bool labelFirst = scope->getLabelFirstX();
		scope->setScaleX(ScopeGUI::LabelInvalid, jmax(1.0, newText.getDoubleValue()), -1, labelFirst);
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getMarkSpacingX(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Mark X Height Property */
class ScopeComponentMarkXHeightProperty : public TextPropertyComponent
{
public:
	ScopeComponentMarkXHeightProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Mark X Height", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setMarkXHeight(jmax(0, newText.getIntValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getMarkXHeight());
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Text Size X Property */
class ScopeComponentTextSizeXProperty : public TextPropertyComponent
{
public:
	ScopeComponentTextSizeXProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Text Size X", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setTextSizeX(jmax(0.f, newText.getFloatValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getTextSizeX(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};
		
/** ScopeGUI : Scale X Label Hop Property */
class ScopeComponentLabelHopXProperty : public TextPropertyComponent
{
public:
	ScopeComponentLabelHopXProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Label Hop X", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		const bool labelFirst = scope->getLabelFirstX();
		scope->setScaleX(ScopeGUI::LabelInvalid, -1, jmax(0, newText.getIntValue()), labelFirst);
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getLabelHopX());
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale X Label First Property */
class ScopeComponentLabelFirstXProperty :	public BooleanValuePropertyComponent
{
public:
	ScopeComponentLabelFirstXProperty(ScopeControlComponent* targetScope)
	:	BooleanValuePropertyComponent("Scale X", "label first mark"),
		scope(targetScope)
	{
		setState(scope->getLabelFirstX());
	}
	
	void stateChanged(const bool state)
	{
		scope->setScaleX(ScopeGUI::LabelInvalid, -1, -1, state);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale Y Units */
class ScopeComponentScaleYProperty : public ChoicePropertyComponent
{
public:
	ScopeComponentScaleYProperty(ScopeControlComponent* targetScope)
	:	ChoicePropertyComponent("Scale Y Units"),
		scope(targetScope) 
	{
		choices.add("None");
		choices.add("Marks");
		choices.add("Amplitude");
		choices.add("Percent");
		choices.add("Decibels");
	}
	
	void setIndex (int newIndex)
	{
		const String& choice = getChoices()[newIndex];
				
		if(choice == "None")			scope->setScaleY(ScopeGUI::LabelYNone, -1, -1, -1);
		else if(choice == "Marks")		scope->setScaleY(ScopeGUI::LabelYMarks, -1, -1, -1);
		else if(choice == "Amplitude")	scope->setScaleY(ScopeGUI::LabelYAmplitude, -1, -1, -1);
		else if(choice == "Percent")	scope->setScaleY(ScopeGUI::LabelYPercent, -1, -1, -1);
		else if(choice == "Decibels")	scope->setScaleY(ScopeGUI::LabelYDecibels, -1, -1, -1);
	}
	
	int getIndex() const
	{
		const StringArray& choices = getChoices();
		
		switch(scope->getScaleY())
		{
			case ScopeGUI::LabelYNone:			return choices.indexOf("None");
			case ScopeGUI::LabelYMarks:			return choices.indexOf("Marks");
			case ScopeGUI::LabelYAmplitude:		return choices.indexOf("Amplitude");
			case ScopeGUI::LabelYPercent:		return choices.indexOf("Percent");
			case ScopeGUI::LabelYDecibels:		return choices.indexOf("Decibels");
			default: return -1;
		}
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale Y Mark Spacing Property */
class ScopeComponentMarkSpacingYProperty : public TextPropertyComponent
{
public:
	ScopeComponentMarkSpacingYProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Mark Spacing Y", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setScaleY(ScopeGUI::LabelInvalid, jmax(0.00000001, newText.getDoubleValue()), -1, -1);
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getMarkSpacingY(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Mark Y Width Property */
class ScopeComponentMarkYWidthProperty : public TextPropertyComponent
{
public:
	ScopeComponentMarkYWidthProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Mark Y Width", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setMarkYWidth(jmax(0, newText.getIntValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getMarkYWidth());
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Text Size Y Property */
class ScopeComponentTextSizeYProperty : public TextPropertyComponent
{
public:
	ScopeComponentTextSizeYProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Text Size Y", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setTextSizeY(jmax(0.f, newText.getFloatValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getTextSizeY(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale Y Label Hop Property */
class ScopeComponentLabelHopYProperty : public TextPropertyComponent
{
public:
	ScopeComponentLabelHopYProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Label Hop Y", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setScaleY(ScopeGUI::LabelInvalid, -1, jmax(0, newText.getIntValue()), -1);
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getLabelHopY());
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Scale Y Decimal Places Property */
class ScopeComponentDecimalPlacesYProperty : public TextPropertyComponent
{
public:
	ScopeComponentDecimalPlacesYProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Decimal Places Y", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setScaleY(ScopeGUI::LabelInvalid, -1, -1, jmax(0, newText.getIntValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		int places = scope->getDecimalPlacesY();
		return places == 0 ? "Scientific" :String(places);
	}
	
private:
	ScopeControlComponent* scope;
};
			
/** ScopeGUI : Colour Property */
class ScopeComponentColourProperty : public ColourPropertyComponent
{
public:
	ScopeComponentColourProperty(ScopeControlComponent* targetScope, 
								 String const& name, 
								 ScopeGUI::ScopeColours colourID)
	:	ColourPropertyComponent(name),
		scope(targetScope),
		cid(colourID)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		scope->setScopeColour(cid, Colour::fromString(text.substring(0, 8)));
		refresh();
	}	
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(scope->getScopeColour(cid).get32bitColour()).toString();
	}
	
private:
	ScopeControlComponent* scope;
	ScopeGUI::ScopeColours cid;
};

/** ScopeGUI : Text Size Channel Property */
class ScopeComponentTextSizeChannelProperty : public TextPropertyComponent
{
public:
	ScopeComponentTextSizeChannelProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Text Size Channel", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setTextSizeChannel(jmax(0.f, newText.getFloatValue()));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return String(scope->getTextSizeChannel(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Label Channels Property*/
class ScopeComponentLabelChannelsProperty :	public BooleanValuePropertyComponent
{
public:
	ScopeComponentLabelChannelsProperty(ScopeControlComponent* targetScope)
	:	BooleanValuePropertyComponent("Label Channels", "on/off"),
		scope(targetScope)
	{
		setState(scope->getLabelChannelsState());
	}
	
	void stateChanged(const bool state)
	{
		scope->setLabelChannelsState(state);
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Channel Labels Property */
class ScopeComponentChannelLabelsProperty : public TextPropertyComponent
{
public:
	ScopeComponentChannelLabelsProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Channel Labels", 0, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		TextArray labels;

		if(newText != "Ch%d")
		{
			StringArray items;
			items.addTokens(newText, ",", String::empty);
			
			for(int i = 0; i < items.size(); i++)
			{
				labels.add((const char*)items[i].trim().toUTF8());
			}
		}
		
		scope->setChannelLabels(labels, scope->getChannelLabelOffset());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{		
		const TextArray& labels = scope->getChannelLabels();
		
		if(labels.length() == 0)
		{
			return "Ch%d";
		}
		else
		{
			String text;
			
			for(int i = 0; i < labels.length(); i++)
			{
				String label = labels[i];
								
				if(i > 0) text += ", ";
				
				text += label;
			}
			
			return text;
		}
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeGUI : Channel Label Offset Property */
class ScopeComponentChannelLabelOffsetProperty : public TextPropertyComponent
{
public:
	ScopeComponentChannelLabelOffsetProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Channel Label Offset", labelDecimalPlaces, false),
		scope(targetScope) { }
	
	void setText (const String& newText)
	{
		scope->setChannelLabels(scope->getChannelLabels(), newText.getIntValue());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(scope->getChannelLabelOffset());
	}
	
private:
	ScopeControlComponent* scope;
};

/** ScopeControlComponent : Colour Property */
class ScopeControlComponentColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlComponentColourProperty(ScopeControlComponent* targetScope, 
										String const& name, 
										ScopeControlComponent::ControlColours colourID)
	:	ColourPropertyComponent(name),
		scope(targetScope),
		cid(colourID)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		scope->getColour(cid) = Colour::fromString(text.substring(0, 8));		
		refresh();
	}	
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
    String getText() const
	{
		return Colour(scope->getColour(cid).get32bitColour()).toString();
	}
	
private:
	ScopeControlComponent* scope;
	ScopeControlComponent::ControlColours cid;
};

/** ScopeControlComponent :  Display Options Properties */
class ScopeComponentDisplayOptionProperty :	public BooleanValuePropertyComponent
{
public:
	ScopeComponentDisplayOptionProperty(ScopeControlComponent* targetScope, 
										String const& name, 
										int option)
	:	BooleanValuePropertyComponent(name, "on/off"),
		scope(targetScope),
		thisOption(option),
		otherOptions(~thisOption)
	{
		ugen_assert(Bits::countOnes(thisOption) == 1);
		
		setState((scope->getDisplayOptions() & thisOption) != 0); // FIXME bool issue
	}
	
	void stateChanged(const bool state)
	{
		int currentOptions = scope->getDisplayOptions();
		
		if(state)
		{
			scope->setDisplayOptions((currentOptions & otherOptions) | thisOption);
		}
		else
		{
			scope->setDisplayOptions((currentOptions & otherOptions));
		}
	}
	
private:
	ScopeControlComponent* scope;
	const int thisOption;
	const int otherOptions;
};





class ScopeControlCuePointLineColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlCuePointLineColourProperty(ScopeCuePointComponent* targetPoint, String const& name)
	:	ColourPropertyComponent(name),
		point(targetPoint)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
				
		point->setColours(Colour::fromString(text.substring(0, 8)), point->getTextColour());
		refresh();
	}	
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(point->getLineColour().get32bitColour()).toString();
	}
	
private:
	ScopeCuePointComponent* point;
};

class ScopeControlCuePointTextColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlCuePointTextColourProperty(ScopeCuePointComponent* targetPoint, String const& name)
	:	ColourPropertyComponent(name),
		point(targetPoint)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		point->setColours(point->getLineColour(), Colour::fromString(text.substring(0, 8)));
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(point->getTextColour().get32bitColour()).toString();
	}
	
private:
	ScopeCuePointComponent* point;
};

class ScopeCuePointComponentLabelProperty : public TextPropertyComponent
{
public:
	ScopeCuePointComponentLabelProperty(ScopeCuePointComponent* targetPoint, String const& name)
	:	TextPropertyComponent(name, 0, false),
		point(targetPoint) { }
	
	void setText (const String& newText)
	{
		point->setLabel((const char*)newText.toUTF8());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String((const char*)point->getLabel());
	}
	
private:
	ScopeCuePointComponent* point;
};

class ScopeCuePointComponentCommentProperty : public TextPropertyComponent
{
public:
	ScopeCuePointComponentCommentProperty(ScopeCuePointComponent* targetPoint, String const& name)
	:	TextPropertyComponent(name, 0, true),
		point(targetPoint) { }
	
	void setText (const String& newText)
	{
		point->setComment((const char*)newText.toUTF8());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String((const char*)point->getComment());
	}
	
private:
	ScopeCuePointComponent* point;
};


class ScopeCuePointComponentOffsetProperty : public TextPropertyComponent
{
public:
	ScopeCuePointComponentOffsetProperty(ScopeCuePointComponent* targetPoint, String const& name)
	:	TextPropertyComponent(name, labelDecimalPlaces, false),
		point(targetPoint) { }
	
	void setText (const String& newText)
	{
		point->setSampleOffset(newText.getIntValue());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return String(point->getSampleOffset());
	}
	
private:
	ScopeCuePointComponent* point;
};

class ScopeControlRegionStartColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlRegionStartColourProperty(ScopeRegionComponent* targetRegion, String const& name)
	:	ColourPropertyComponent(name),
		region(targetRegion)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		region->setColours(Colour::fromString(text.substring(0, 8)), 
						   region->getEndColour(),
						   region->getTextColour(),
						   region->getFillColour());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(region->getStartColour().get32bitColour()).toString();
	}
	
private:
	ScopeRegionComponent* region;
};

class ScopeControlRegionEndColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlRegionEndColourProperty(ScopeRegionComponent* targetRegion, String const& name)
	:	ColourPropertyComponent(name),
		region(targetRegion)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		region->setColours(region->getStartColour(),
						   Colour::fromString(text.substring(0, 8)),
						   region->getTextColour(),
						   region->getFillColour());
		refresh();
	}	
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(region->getEndColour().get32bitColour()).toString();
	}
	
private:
	ScopeRegionComponent* region;
};

class ScopeControlRegionTextColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlRegionTextColourProperty(ScopeRegionComponent* targetRegion, String const& name)
	:	ColourPropertyComponent(name),
		region(targetRegion)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		region->setColours(region->getStartColour(),
						   region->getEndColour(),
						   Colour::fromString(text.substring(0, 8)),
						   region->getFillColour());
		refresh();
	}
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(region->getTextColour().get32bitColour()).toString();
	}
	
private:
	ScopeRegionComponent* region;
};

class ScopeControlRegionFillColourProperty : public ColourPropertyComponent
{
public:
	ScopeControlRegionFillColourProperty(ScopeRegionComponent* targetRegion, String const& name)
	:	ColourPropertyComponent(name),
		region(targetRegion)
	{
	}
	
	void setText (const String& newText)
	{
		String text = newText;
		
		while(text.length() < 8)
		{
			text << "0";
		}
		
		region->setColours(region->getStartColour(), 
						   region->getEndColour(),
						   region->getTextColour(),
						   Colour::fromString(text.substring(0, 8)));
		refresh();
	}	
	
#if JUCE_MAJOR_VERSION < 2
    const
#endif
	String getText() const
	{
		return Colour(region->getFillColour().get32bitColour()).toString();
	}
	
private:
	ScopeRegionComponent* region;
};

enum ControlPropertyType
{
	ColourTypes = 1,
	LabelTypes = 2,
	PositionTypes = 4,
	AllTypes = 0xffffff
};

class ScopeCuePointProperties : public PropertyPanel
{
public:
	ScopeCuePointProperties(ScopeCuePointComponent* target, String const& name)
	{
		Array<PropertyComponent*> props;
		props.addArray(get(target, name, AllTypes));
		addSection(name, props, true);
	}
	
	static Array<PropertyComponent*> get(ScopeCuePointComponent* target, 
										 String const& name,
										 int types = AllTypes)
	{
		Array<PropertyComponent*> props;
				
		if((types & LabelTypes) != 0)
		{
			props.add(new ScopeCuePointComponentLabelProperty(target, name + " Label"));
		}
		
		if((types & PositionTypes) != 0)
		{
			props.add(new ScopeCuePointComponentOffsetProperty(target, name + " Position"));
		}
		
		if((types & ColourTypes) != 0)
		{
			props.add(new ScopeControlCuePointLineColourProperty(target, name + " Line Colour"));
			props.add(new ScopeControlCuePointTextColourProperty(target, name + " Text Colour"));
		}
		
		if(target->belongsToRegion() == false)
		{
			if((types & LabelTypes) != 0)
			{
				props.add(new ScopeCuePointComponentCommentProperty(target, name + " Comment"));
			}
		}
		
		return props;
	}	
};

class ScopeRegionProperties : public PropertyPanel
{
public:
	ScopeRegionProperties(ScopeRegionComponent* target, String const& name)
	{
		Array<PropertyComponent*> props;
		props.addArray(get(target, name, AllTypes));
		addSection(name, props, true);
	}
	
	static Array<PropertyComponent*> get(ScopeRegionComponent* target,
										 String const& name,
										 int types = AllTypes)
	{
		Array<PropertyComponent*> props;
		
		props.addArray(ScopeCuePointProperties::get(target->getStartPoint(), name + " Start", types & ~(ColourTypes)));
		props.addArray(ScopeCuePointProperties::get(target->getEndPoint(), name + " End", types & ~(ColourTypes)));
		
		if((types & ColourTypes) != 0)
		{
			props.add(new ScopeControlRegionStartColourProperty(target, name + " Start Colour"));
			props.add(new ScopeControlRegionEndColourProperty(target, name + " End Colour"));
			props.add(new ScopeControlRegionTextColourProperty(target, name + " Text Colour"));
			props.add(new ScopeControlRegionFillColourProperty(target, name + " Fill Colour"));
		}
		
		return props;		
	}
};	

class ScopeControlProperties : public PropertyPanel
{
public:
	ScopeControlProperties(ScopeControlComponent* target)
	{
		addLayoutProps(target);
		addColoursProps(target);
		addControlProps(target);
		addControlColoursProps(target);
	}
	
	void addLayoutProps(ScopeControlComponent* target)
	{
		Array<PropertyComponent*> props;
		
		props.add(new ScopeComponentIsBipolarProperty(target));
		props.add(new ScopeComponentYMaximumProperty(target));
		props.add(new ScopeComponentLowerMarginProperty(target));
		
		props.add(new ScopeComponentScaleXProperty(target));
		props.add(new ScopeComponentMarkSpacingXProperty(target));
		props.add(new ScopeComponentMarkXHeightProperty(target));
		props.add(new ScopeComponentTextSizeXProperty(target));
		props.add(new ScopeComponentLabelHopXProperty(target));
		props.add(new ScopeComponentLabelFirstXProperty(target));
		
		props.add(new ScopeComponentScaleYProperty(target));
		props.add(new ScopeComponentMarkSpacingYProperty(target));
		props.add(new ScopeComponentMarkYWidthProperty(target));
		props.add(new ScopeComponentTextSizeYProperty(target));
		props.add(new ScopeComponentLabelHopYProperty(target));
		props.add(new ScopeComponentDecimalPlacesYProperty(target));
		
		props.add(new ScopeComponentTextSizeChannelProperty(target));
		props.add(new ScopeComponentLabelChannelsProperty(target));
		props.add(new ScopeComponentChannelLabelsProperty(target));
		props.add(new ScopeComponentChannelLabelOffsetProperty(target));
		
		addSection("Scope Layout", props, true);				
	}
	
	void addColoursProps(ScopeControlComponent* target)
	{
		Array<PropertyComponent*> props;

		props.add(new ScopeComponentColourProperty(target, "Background Colour", ScopeGUI::Background));
		props.add(new ScopeComponentColourProperty(target, "Top Line Colour", ScopeGUI::TopLine));
		props.add(new ScopeComponentColourProperty(target, "Zero Line Colour", ScopeGUI::ZeroLine));
		props.add(new ScopeComponentColourProperty(target, "Label Marks Colour", ScopeGUI::LabelMarks));
		props.add(new ScopeComponentColourProperty(target, "Text X Colour", ScopeGUI::TextX));
		props.add(new ScopeComponentColourProperty(target, "Text Y Colour", ScopeGUI::TextY));
		props.add(new ScopeComponentColourProperty(target, "Text Channel Colour", ScopeGUI::TextChannel));
		props.add(new ScopeComponentColourProperty(target, "Trace Colour", ScopeGUI::Trace));		
		
		addSection("Scope Colours", props, true);				
	}
	
	void addControlColoursProps(ScopeControlComponent* target)
	{
		Array<PropertyComponent*> props;
		
		props.add(new ScopeControlComponentColourProperty(target, "Cue Point Colour", ScopeControlComponent::CuePointColour));
		props.add(new ScopeControlComponentColourProperty(target, "Cue Point Text Colour", ScopeControlComponent::CuePointTextColour));
		
		props.add(new ScopeControlComponentColourProperty(target, "Loop Point Start Colour", ScopeControlComponent::LoopPointStartColour));
		props.add(new ScopeControlComponentColourProperty(target, "Loop Point End Colour", ScopeControlComponent::LoopPointEndColour));
		props.add(new ScopeControlComponentColourProperty(target, "Loop Fill Colour", ScopeControlComponent::LoopFillColour));
		props.add(new ScopeControlComponentColourProperty(target, "Loop Text Colour", ScopeControlComponent::LoopTextColour));
		
		props.add(new ScopeControlComponentColourProperty(target, "Region Start Colour", ScopeControlComponent::RegionStartColour));
		props.add(new ScopeControlComponentColourProperty(target, "Region End Colour", ScopeControlComponent::RegionEndColour));
		props.add(new ScopeControlComponentColourProperty(target, "Region Fill Colour", ScopeControlComponent::RegionFillColour));
		props.add(new ScopeControlComponentColourProperty(target, "Region Text Colour", ScopeControlComponent::RegionTextColour));
		
//		props.add(new ScopeControlComponentColourProperty(target, "Insert Point Colour", ScopeControlComponent::InsertPointColour));
//		props.add(new ScopeControlComponentColourProperty(target, "Insert Point Text Colour", ScopeControlComponent::InsertPointTextColour));
//
//		props.add(new ScopeControlComponentColourProperty(target, "Selection Start Colour", ScopeControlComponent::SelectionStartColour));
//		props.add(new ScopeControlComponentColourProperty(target, "Selection End Colour", ScopeControlComponent::SelectionEndColour));
//		props.add(new ScopeControlComponentColourProperty(target, "Selection Fill Colour", ScopeControlComponent::SelectionFillColour));
//		props.add(new ScopeControlComponentColourProperty(target, "Selection Text Colour", ScopeControlComponent::SelectionTextColour));
		
		addSection("Default Scope Control Colours", props, false);				
	}
	
	
	void addControlProps(ScopeControlComponent* target)
	{
		Array<PropertyComponent*> props;
		
		props.addArray(ScopeCuePointProperties::get(target->getInsert(), "Insert", (ColourTypes|LabelTypes)));
		props.addArray(ScopeRegionProperties::get(target->getSelection(), "Selection", (ColourTypes|LabelTypes)));
		
		props.add(new ScopeComponentDisplayOptionProperty(target, "Show Cue Points",	ScopeControlComponent::DisplayCuePoints));
		props.add(new ScopeComponentDisplayOptionProperty(target, "Show Loop Points",	ScopeControlComponent::DisplayLoopPoints));
		props.add(new ScopeComponentDisplayOptionProperty(target, "Show Regions",		ScopeControlComponent::DisplayRegions));
		props.add(new ScopeComponentDisplayOptionProperty(target, "Show Insert Point",	ScopeControlComponent::DisplayInsert));
		props.add(new ScopeComponentDisplayOptionProperty(target, "Show Selection",		ScopeControlComponent::DisplaySelection));
				  
		addSection("Scope Controls", props, true);						
	}
	
};

#endif

#endif // UGEN_JUCESCOPEPROPERTIES

