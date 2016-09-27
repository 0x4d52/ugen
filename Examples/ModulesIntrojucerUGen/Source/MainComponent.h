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
						public JuceIOHost,
						public Timer
{
private:
	Label cpu;
	ScopeControlComponent scope;
	UGen sender;
	
public:
	MainComponent()
	:	scope(lock)
	{
        addAndMakeVisible(&cpu);
		addAndMakeVisible(&scope);
        startTimer(40);
	}

	~MainComponent()
	{
		sender.removeBufferReceiver(scope);
	}
	
	void resized()
	{
		cpu.setBounds(10, 10, 80, 20);
		scope.setBounds(10, 40, getWidth()-20, getHeight()-50);
	}
	
	void timerCallback()
	{
		cpu.setText(String(getCpuUsage()*100.0, 2)+"%", dontSendNotification);
	}
	
	UGen constructGraph(UGen const& input)
	{
		(void)input;
		UGen output = SinOsc::AR(SinOsc::AR(1, 0, 200, 1000), 0, 0.1);
		sender = Sender::AR(output, 0.01);
		sender.addBufferReceiver(scope);
		addOther(sender);
		return output;
	}
};




#endif  // __MAINCOMPONENT_H_857C3E44__
