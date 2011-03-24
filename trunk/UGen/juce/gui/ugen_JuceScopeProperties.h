#ifndef UGEN_JUCESCOPEPROPERTIES
#define UGEN_JUCESCOPEPROPERTIES

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


class ScopeComponentYMaximumProperty : public TextPropertyComponent
{
public:
	ScopeComponentYMaximumProperty(ScopeControlComponent* targetScope)
	:	TextPropertyComponent("Y Maximum", 10, false),
		scope(targetScope)
	{
	}
	
	void setText (const String& newText)
	{
		scope->setYMaximum(newText.getFloatValue());
	}
	
	const String getText() const
	{
		return String(scope->getYMaximum(), labelDecimalPlaces);
	}
	
private:
	ScopeControlComponent* scope;
};



class ScopeComponentIsBipolarProperty :		public BooleanValuePropertyComponent
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

#endif // UGEN_JUCESCOPEPROPERTIES

