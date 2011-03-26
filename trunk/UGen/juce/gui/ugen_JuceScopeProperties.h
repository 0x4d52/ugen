#ifndef UGEN_JUCESCOPEPROPERTIES
#define UGEN_JUCESCOPEPROPERTIES

#include "../ugen_JuceUtility.h"

// this files is designed only to be included by ugen_JuceScope.cpp

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
			
			CallOutBox callOut (colourSelector, *owner, 0);
			callOut.runModalLoop();			
		}
		
		void changeListenerCallback (ChangeBroadcaster* source)
		{
			owner->setText(colourSelector.getCurrentColour().toString());
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
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
	
	const String getText() const
	{
		return Colour(scope->getScopeColour(cid).get32bitColour()).toString();
	}
	
private:
	ScopeControlComponent* scope;
	ScopeGUI::ScopeColours cid;
};

class ScopeControlProperties : public PropertyPanel
{
public:
	ScopeControlProperties(ScopeControlComponent* target)
	{
		addLayoutProps(target);
		addColoursProps(target);
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
		
		addSection("Scope Layout", props, false);				
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
		
		addSection("Scope Colours", props, false);				

	}
};


#endif // UGEN_JUCESCOPEPROPERTIES

