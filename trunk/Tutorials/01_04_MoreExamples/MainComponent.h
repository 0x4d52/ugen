#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"


class MainComponent :	public Component,
						public JuceIOHost
{	
public:
	MainComponent () : JuceIOHost(2, 2) { }	
	
	UGen constructGraph(UGen const& input)
	{
		
		// replace this test tone with one of the examples in the tutorial document
		return SinOsc::AR(SinOsc::AR(5, 0, 100, 1000), 0, 0.2);
		
	}
	
};

#endif //_MAINCOMPONENT_H_ 