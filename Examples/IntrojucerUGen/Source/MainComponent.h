/*
  ==============================================================================

    MainComponent.h
    Created: 16 Apr 2011 10:40:15am
    Author:  Martin Robinson

  ==============================================================================
*/

#ifndef __MAINCOMPONENT_H_857C3E44__
#define __MAINCOMPONENT_H_857C3E44__

#include "../../../UGen/UGen.h"

class MainComponent :	public Component,
						public JuceIOHost
{
public:
	MainComponent()
	{
	}

	~MainComponent()
	{
	}

	void paint(Graphics& g)
	{
		g.fillAll(Colours::red);
	}
	
	UGen constructGraph(UGen const& input)
	{
		return SinOsc::AR(1000, 0, 0.1);
	}
};


#endif  // __MAINCOMPONENT_H_857C3E44__
