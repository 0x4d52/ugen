/** 
 @page		Tutorial_01_05		Tutorial 01.05: Maths examples

 This tutorial gives a list of examples of mathematical and arithmentic functions. See @ref MathsUGens "the full list" for more info.
   
 As in the previous tutorial you can copy and paste each of these examples to replace 
 the entire body of the @c constructGraph() function.
 
 <P><HR><P>
 
 Aritmetic operators: as already illustrated, these construct processing units based on the mathematical signal. 
 As such the @c + operator can mix signals and the @c * operator can amplify/attenuate signals. This example takes a 
 signal in the ±1 range and maps it between 220...880 using multiply and add (or scale and offset).
 
 @code
	UGen mod = SinOsc::AR(1);
	mod = mod * 330 + 550;
	UGen output = SinOsc::AR(mod, 0, 0.2);
	return output;
 @endcode
 
 <P><HR><P>
 
 There are convenience UGens for this kind of common operation as illustrated in the previous tutorial. LinLin performs
 a mapping from one linear range to another linear range. This is equivalent to the previous example.
 
 @code
	 UGen mod = SinOsc::AR(1);
	 mod = LinLin::AR(mod, -1, 1, 220, 880);
	 UGen output = SinOsc::AR(mod, 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 The UGen class has some of these "mapping" UGens built in as convenience functions. Again this is identical 
 to the above:
 
 @code
	 UGen mod = SinOsc::AR(1).linlin(-1, 1, 220, 880);
	 UGen output = SinOsc::AR(mod, 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 Exponential mapping is often more appropriate for mapping a linear range to control frequency. Using 
 linear mapping the midpoint is 550Hz (halfway between 220 and 880). However, it would be more musical
 to have 440Hz as the midpoint since this is an octave above the lower bound (double 220Hz) AND an octave below the 
 upper bound (half 880Hz).
 
 @code
	 UGen mod = SinOsc::AR(1);
	 mod = LinExp::AR(mod, -1, 1, 220, 880); // or use .linexp(...)
	 UGen output = SinOsc::AR(mod, 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 In fact we could generate this modulator in the MIDI note range then convert it to frequency to achieve the same
 effect. Here 220Hz is MIDI note 57 and 880Hz is MIDI note 81.
 
 @code
	 UGen mod = SinOsc::AR(1).linlin(-1, 1, 57, 81);
	 UGen output = SinOsc::AR(mod.midicps(), 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 The midicps() function is just one example of the unary operators in UGen++. We could synthesise a sine
 wave by generating a sawtooth wave a phasor in the ±pi range then applying a sin() function (the SinOsc
 UGen uses a lookup table rather than calculating the sine function for each sample). This also shows that
 the constant @c pi which is defined as part of UGen++.
 
 @code
	 UGen output = LFSaw::AR(440, 0, pi).sin();
	 return output;
 @endcode
 
 <P><HR><P>
 
 The unary operators can be used in the style above by passing them as a message to a UGen. Or they can be
 called like a regular function. You can use whichever style you prefer. The message passing style is often
 more flexible (since you can chain together processes quite easily without ending up with a deeply nested
 set of parentheses) although might seem unusual to use at first.
 
 @code
	 UGen output = sin( LFSaw::AR(440, 0, pi) );
	 return output;
 @endcode
 
 <P><HR><P>
 
 Take this example which uses the modulation technique from examples above but quantises pitch to the nearest
 MIDI note before converting to frequency:
 
 @code
	 UGen mod = SinOsc::AR(0.25).linlin(-1, 1, 57, 81).round(1).midicps();
	 UGen output = SinOsc::AR(mod, 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 Using the functional style the modulator definition would look like this (unless split onto separate lines):
 
 @code
	 UGen mod = midicps(round(LinLin::AR(SinOsc::AR(0.25), -1, 1, 57, 81), 1));
	 UGen output = SinOsc::AR(mod, 0, 0.2);
	 return output;
 @endcode
 
 <P><HR><P>
 
 Splitting onto separate lines is no less efficient when it comes to perform and render the audio. This
 is because all you are doing at this stage is specifying the interconnections of UGens in to a graph.
 The example above could become:
 
 @code
	UGen mod = SinOsc::AR(0.25);
	mod = LinLin::AR(mod, -1, 1, 57, 81);
	mod = round(mod, 1);
	mod = midicps(mod);
	UGen output = SinOsc::AR(mod, 0, 0.2);
	return output;
 @endcode
 
 As you can see the separate lines approach is reads in a similar way to the message passing approach.
 
 <P><HR><P>
 
 Comparison operators on UGens produce a signal with binary values @c 0.0 and @c 1.0
 evaluated for each corresponding sample in the input signals.
 
 e.g., square wave from a sine wave:
 @code
	UGen sine = SinOsc::AR(200, 0, 1.0);
	UGen square = sine > 0.0;		// 0...1 square wave
	UGen output = square.linlin(0, 1, -1, 1);
	output *= 0.2; // amplitude
	return output;
 @endcode
 
 Here when the sine wave is greater than @c 0.0 the comparison yields @c 1.0, otherwise it is @c 0.0 . This
 example also shows the use of the compound operator @c *= which works as expected on UGens in the way it
 works with simple numercial values.
 
 <P><HR><P>
 
 
 
 prev: @ref Tutorial_01_04	"Tutorial 01.04: More examples"
*/