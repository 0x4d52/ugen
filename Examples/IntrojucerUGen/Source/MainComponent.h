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

#if UGEN_IPHONE
 #define RESOURCES ""
#else
 #define RESOURCES "../Resources"
#endif

class MainComponent :	public Component,
public JuceIOHost,
public Timer
{
private:
	Label cpu;
    Slider slider;    
    
public:
	MainComponent()
	{		
		addAndMakeVisible(&cpu);
		addAndMakeVisible(&slider);
        slider.setRange(0, 5000);
		startTimer(100);
	}
    
	~MainComponent()
	{
	}
	
	void resized()
	{
		cpu.setBounds(10, 10, 80, 20);
		slider.setBounds(10, 40, 200, 20);
	}
	
	void timerCallback()
	{
		cpu.setText(String(getCpuUsage()*100.0, 2)+"%", false);
	}
	
	UGen constructGraph(UGen const& input)
	{
		return SinOsc::AR(Lag::AR(&slider), 0, 0.2);
	}
};


//class MainComponent :	public Component,
//						public JuceIOHost,
//						public Timer
//{
//private:
//	Label cpu;
//	ScopeControlComponent scope;
//	Buffer sound;
//	
//public:
//	MainComponent()
//	:	scope(lock)
//	{
//		File file = File::getSpecialLocation(File::currentExecutableFile)
//						.getParentDirectory()
//						.getChildFile(RESOURCES);
//		sound = Buffer(file.getChildFile("beat.wav"));
//		
//		addAndMakeVisible(&cpu);
//		addAndMakeVisible(&scope);
//		scope.setAudioBuffer(sound);
//		startTimer(100);
//	}
//
//	~MainComponent()
//	{
//	}
//	
//	void resized()
//	{
//		cpu.setBounds(10, 10, 80, 20);
//		scope.setBounds(10, 40, getWidth()-20, getHeight()-50);
//	}
//	
//	void timerCallback()
//	{
//		cpu.setText(String(getCpuUsage()*100.0, 2)+"%", false);
//	}
//	
//	UGen constructGraph(UGen const& input)
//	{
//		return PlayBuf::AR(sound, 1, 0, 0, 1);
//		
//		
//		//return SinOsc::AR(1000, 0, 0.1);
//	}
//};


#endif  // __MAINCOMPONENT_H_857C3E44__
