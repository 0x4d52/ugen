/** 
 @page		Tutorial_01_03		Tutorial 01.03: Building more complex UGen graphs

 This tutorial shows how more complex UGen graphs can be constructed very easily. The
 output of this example is a two channel frequency modulated sine wave. This demonstrates
 the flexibility of UGen inputs which also of type UGen. This type of input may be a constant,
 another UGen and may be multichannel. If an input to a UGen is also type UGen and multichannel, 
 the destination UGen itself automatically becomes multichannel as well. (There are some exceptions 
 to this but these are documented in the relevant UGen e.g., Pan2.)

 @includelineno 01_03_UGenGraphs/MainComponent.h
 
 Here we create another, intermediate, UGen to use as a modulator:
 @skipline modulator
 
 Its first input (frequency) is a multichannel constant UGen created using the macro U (which is
 simply a shorthand for UGen).
 @skipline 4.9
 
 The other arguments are just single constants:
 @skip 0
 @until ;
 
 The modulator is then a two channel modulator with the same depth and centre values but with a 
 different frequency. An equivalent (but more verbose) method would be:
 
 @code
 UGen modulator = (SinOsc::AR(4.9, 0, 40, 440), SinOsc::AR(5.1, 0, 40, 440)); 
 @endcode
 
 Notice here the parentheses around the comma-separated expression since this is using another
 method to create multichannel UGen instances by using the comma operator to concatenate channels i.e.,
 UGen::operator,(). This may be useful if the two channels are completely different (which in 
 this case they aren't).
 
 The next line connects the modulator sine oscillators to the "carrier" sine oscillators' frequency
 inputs; then the output is returned:
 @skip output
 @until return
 
 This entire example could have been written as a single expression returned from constructGraph():
 @code
 return SinOsc::AR(SinOsc::AR(U(4.9, 5.1), 0, 40, 440), 0, 0.1);
 @endcode

 The UGen graph produced is shown by Figure 01.03.01 below.
 @image html 01_03_UGenGraphs/doc/01_03_image_01.png "Figure 01.03.01 More complex UGen graphs"
 
 prev: @ref Tutorial_01_02	"Tutorial 01.02: Using the JuceIOHost"
 
 next: @ref Tutorial_01_04	"Tutorial 01.04: More examples"
*/