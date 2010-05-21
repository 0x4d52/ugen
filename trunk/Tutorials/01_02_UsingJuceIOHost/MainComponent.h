#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"

class MainComponent : public Component,
                      public JuceIOHost
{
private:

public:
	MainComponent () : JuceIOHost(0, 1) { }
	~MainComponent () {	}
		
	UGen constructGraph(UGen const& input)
	{
		return SinOsc::AR(440, 0, 0.1);
	}
};

#endif //_MAINCOMPONENT_H_ 