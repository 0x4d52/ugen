/** 
 @page		Tutorial_01_01		Tutorial 01.01: An introduction to UGen++
 
 This tutorial introduces the basics of the UGen++ library, shows the necessary startup and shutdown code, shows building a "UGen graph" and shows how to render audio data from a UGen graph into a host environment's audio buffers. Most of the tutorial examples use Juce but this example shows the fundamental steps that would be required when using UGen++ in other contexts. You could skip forward to @ref Tutorial_01_04 "Tutorial 01.04: More examples" to get an idea of how to build a range of standard audio processing configurations in UGen++.
 
 <!-- THIS NEEDS TO START FROM SCRATCH AND INTRODUCE THE REQUIRED FUNDAMENTALS-->

 @includelineno 01_01_Introduction/MainComponent.h
 
 @dontinclude 01_01_Introduction/MainComponent.h
 Before calling any UGen++ functions or creating any UGen++ objects you must call UGen::initialise(), 
 this intialises memory required by the system (especially wavetables for things like sine generation and panning UGens):
 @skipline UGen::initialise();
 Then the Juce audio device manager is intialised:
 @skip audioDeviceManager
 @until addAudioCallback
 
 @dontinclude 01_01_Introduction/MainComponent.h
 After the audio device manager is initialised it calls @c audioDeviceAboutToStart(). Here UGen++ is being
 prepared to play with the system sample rate and buffer size. Here we are also assigning our
 UGen graph to the class member @c synth.
 @skip audioDeviceAboutToStart
 @until }
 
 @dontinclude 01_01_Introduction/MainComponent.h
 Each time the audio device manager calls our callback we process samples from our synth member. The
 UGen::getNextBlockID() function counts the number of samples processed and is used to ensure that UGens don't
 process sample blocks more than once. The @ref UGen::setOutputs() "setOutputs()" function tells the UGen where to write its
 final output, which in this case is to the @c outputChannelData argument of the callback.
 The @ref UGen::prepareAndProcessBlock() "prepareAndProcessBlock()" function tells the UGen to render its audio to the output.
 @skip audioDeviceIOCallback
 @until }
 
 @dontinclude 01_01_Introduction/MainComponent.h
 This continues until the application is quit, at which point our destructor is called. The callback
 is removed form the device mananger. Any components are deleted (there aren't any in this case but this is a harmless Juce call). 
 And importantly, UGen::shutdown() is called:
 @skip ~
 @until }
 
 This example plays a single 440Hz sine wave at an ampltude of 0.1 as shown below:
 
 @image html 01_01_Introduction/doc/01_01_image_01.png "Figure 01.01.01: A simple UGen graph"

 @section Exercises
 
 Haven't got any exercises yet, just testing this structuring mechanism for adding them soon...
 
 The actual exercises would go here...
 
 @ref Tutorial_01_01_Solutions "The solutions for these exercises are here."
 
 @section Conclusion
 
 This tutorial introduced the basics of the UGen++ library. The next tutorial wraps up most of the host handling code 
 so that users can concentrate on building the audio processing graphs.
 
 next: @ref Tutorial_01_02	"Tutorial 01.02: Using the JuceIOHost"
*/



/** 
 @page		Tutorial_01_01_Solutions		Tutorial 01.01: Solutions
 
 Haven't got any exercises yet, just testing this structuring mechanism for adding them soon.
 
 The actual solutions would go here...
 
 @ref		Tutorial_01_01					"Go back to the tutorial."
*/