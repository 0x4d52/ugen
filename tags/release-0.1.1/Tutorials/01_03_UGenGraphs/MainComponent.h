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
		UGen modulator = SinOsc::AR(
							U(4.9, 5.1),	// two channel constant
							0,				// zero phase
							40,				// modulation depth
							440);			// modulation centre value
		UGen output = SinOsc::AR(modulator, 0, 0.1);
		return output;
	}
};

#endif //_MAINCOMPONENT_H_ 