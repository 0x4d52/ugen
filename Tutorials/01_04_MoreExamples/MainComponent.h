#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"

class MainComponent : public Component,
public JuceIOHost
{	
public:
	MainComponent () : JuceIOHost(0, 2) { }	
	
	UGen constructGraph(UGen const& input)
	{
		// replace the entire body of this function with the examples in the HTML tutorial
		return SinOsc::AR(1000, 0, 0.1);
	}
};

#endif //_MAINCOMPONENT_H_ 