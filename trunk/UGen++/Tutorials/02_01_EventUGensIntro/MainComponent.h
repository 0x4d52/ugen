#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"


class Factory : public GenericEventDefault
{
public:
	UGen createEvent(const int count)
	{
		// random frequency
		return SinOsc::AR(exprand(100, 1000), 0, 0.2);
	}
	
	UGen constructGraph(UGen const& input)
	{
		int n = 8;
		return MixFill<Factory>::AR(n) / n;
	}
};


class MainComponent : public Component,
                      public JuceIOHost
{	
public:
	MainComponent () : JuceIOHost(2, 2) { }	
		
	UGen constructGraph(UGen const& input)
	{
		Factory factory;
		return factory.constructGraph(input);
	}
};

#endif //_MAINCOMPONENT_H_ 