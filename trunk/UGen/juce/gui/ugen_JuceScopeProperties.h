#ifndef UGEN_JUCESCOPEPROPERTIES
#define UGEN_JUCESCOPEPROPERTIES

// this files is designed only to be included by ugen_JuceScope.cpp

static const int labelDecimalPlaces = 10;

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

class ScopeComponentIsBipolarProperty :		public BooleanPropertyComponent,
											public juce::Value::Listener
{
public:
	ScopeComponentIsBipolarProperty(ScopeControlComponent* targetScope, juce::Value value = juce::Value())
	:	BooleanPropertyComponent(value, "Polarity", "is bipolar"),
		scope(targetScope)
	{
		value.setValue(scope->getPolarity());
		value.addListener(this);
	}
	
	void valueChanged (juce::Value& value)
	{
		scope->setPolarity(value.getValue());
	}
	
private:
	ScopeControlComponent* scope;
};

#endif // UGEN_JUCESCOPEPROPERTIES

