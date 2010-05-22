/** 
 @page		Tutorial_01_02		Tutorial 01.02: Using the JuceIOHost

 The JuceIOHost class wraps up the required interfacing with the AudioDeviceManager and the AudioIODeviceCallback providing some inputs and outputs. You need to implement the pure virtual function JuceIOHost::constructGraph() which returns a UGen graph. Most of the following examples/tutorials will use this method in order to focus on UGen++-specific issues.
 
 @includelineno 01_02_UsingJuceIOHost/MainComponent.h
 
 This example also plays a single 440Hz sine wave at an ampltude of 0.1, exactly the same as @link Tutorial_01_01 Tutorial 01.01 @endlink.
 
 @image html 01_02_UsingJuceIOHost/doc/01_02_image_01.png "Figure 01.02.01: A simple UGen graph"
 
 @dontinclude 01_02_UsingJuceIOHost/MainComponent.h
 Here our class inherits from Juce Component AND the UGen++ class JuceIOHost:
 @skip class
 @until {
 
 The JuceIOHost handles the UGen++ configuration (e.g., input and output) described in the 
 @ref Tutorial_01_01 "previous tutorial". The JuceIOHost can be configured for a particular 
 number of inputs and outputs on construction:
 @skipline MainComponent
 
 And the UGen graph is built by implementing the pure virtual function JuceIOHost::constructGraph() which is called
 when the JuceIOHost needs to to build the UGen graph to play. This is passed a UGen containing the inputs to the
 JuceIOHost, this should have a number channels equal to the first argument passed to the JuceIOHost constructor (0
 in this case). The @ref JuceIOHost::constructGraph() "constructGraph()" function must return the UGen graph to play. If 
 the number channels returned is less than the number of output channels the JuceIOHost has (i.e., the second argument 
 to the JuceIOHost constructor 1 in this case) the output will leave the additional channels empty. If the number of 
 channels returned by the @ref JuceIOHost::constructGraph() "constructGraph()" is greater than the number channels the 
 JuceIOHost has, these will be processed but only the number of output channels specified will be sent to the host.
 @skip constructGraph
 @until };
 
 
 prev: @ref Tutorial_01_01	"Tutorial 01.01: An introduction to UGen++"
 
 next: @ref Tutorial_01_03	"Tutorial 01.03: Building more complex UGen graphs"
*/