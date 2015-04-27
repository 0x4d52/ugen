# Introduction #

This guide follows tutorial 1.3 in the UGen source tree and give a quick example of how to get started making sounds with UGen and the Juce audio engine. For speed and to avoid you having to set up Juce and a corresponding application please open the tutorial files located at UGen++/Tutorals/01\_03\_UGenGraphs.


# Details #

As this practical uses Juce for all the audio interfacing with the computer we will be using the handy `JuceIOHost` class that wraps a Juce AudioIODeviceCallback and automatically sets up the device as required. Once you have the Tutorial 1.3 project open, open the MainComponent.h file which is where we will be doing all of our audio work.

Firstly we need to include header guards and both UGen and Juce main header files.

```
#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"
```

Next we need to declare our main component which is what is held in the main window (in this case it is just a blank component) and inherit from Juce's `Component` class and UGen's `JuceIOHost` for all the audio configurations.

```
class MainComponent : public Component,
                      public JuceIOHost
{	
```

Our constructor doesn't have to take any arguments but we do need to initialise the `JuceIOHost` base class with the number or inputs and outputs we require, in this case just 2 outputs.

```
public:
	MainComponent () : JuceIOHost(0, 2) { }	
```

Now we can construct our UGen processing graph with the constructGraph method defined by JuceIOHost. This example is a very simple vibrato acting on a 440Hz sine wave. It uses two SinOSC UGens, one for the input signal and one to modulate it.

Firstly we need to declare the modulator wave. We actually declare a stereo modulator here using the standard UGen macro U, with the left channel oscillating at 4.9Hz and the right at 5.1Hz. This is the first input to our modulator declared as a Sine wave UGen, SinOsc. The next three arguments are explained in the code showing that we want zero phase, and a depth of 40Hz centring around 440Hz.

Now we are ready to create our audible output, another sine wave with its input our stereo modulating wave, again 0 phase and a level of 0.1.

Finally we need to return our constructed graph to the JuceIOHost to begin pulling audio through it.

```
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
```

The result of our graph is a stereo output with the left channel outputting a sine wave centred around 440Hz with a frequency modulation depth of 40Hz and a rate of 4.9Hz. The right channel has the same output except that the rate is 5.1Hz. Using these two different rates the waves will oscillate in and out of phase.

The last step is to close our class and the associated header guard.

```
};

#endif //_MAINCOMPONENT_H_ 
```

Hopefully this guide will help you to quickly start using UGen. Try to experiment with chaining different UGens to create new sounds. For more working examples look in the Tutorials folder of the source tree.

Have fun!