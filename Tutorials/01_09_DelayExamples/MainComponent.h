#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"

class MainComponent : public Component,
                      public JuceIOHost
{	
public:
	MainComponent () : JuceIOHost(2, 2) { }	
		
	UGen constructGraph(UGen const& input)
	{
		const float maximumDelay = 5.0;
		UGen mix = input.mix();
		UGen delay  = DelayN::AR(mix, maximumDelay, 1.0);
		return U(mix, delay);
	}
};

#endif //_MAINCOMPONENT_H_ 