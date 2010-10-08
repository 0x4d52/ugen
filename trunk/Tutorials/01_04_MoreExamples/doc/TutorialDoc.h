/** 
 @page		Tutorial_01_04		Tutorial 01.04: More examples

 This tutorial gives a list of simple examples to try out in the @c constructGraph() function. 
 
 @includelineno 01_04_MoreExamples/MainComponent.h
 
 Notice the JuceIOHost is set up for 2-in and 2-out.
 
 You can copy and paste each of these examples to replace the entire body of the @c constructGraph()
 function. In fact you can use these examples to test the iPhone project too (use the 
 @c iPhone_UGen_AU/UGenRemoteIO.xcodeproj). In this case the @c constructGraph() is an Objective-C
 function but works in a similar way to the Juce project.
 
 Note that you can click on the name of a UGen (e.g., SinOsc) and it should to take you to its
 more detailed documentation.
 
 %Amplitude modulation:
 @code
	UGen mod = SinOsc::AR(4, 0, 0.1, 0.2);
	UGen output = SinOsc::AR(1000, 0, mod);
	return output;
 @endcode
 
 <P><HR><P>
 
 Frequency modulation:
 @code
	UGen mod = SinOsc::AR(4, 0, 100, 1000);
	UGen output = SinOsc::AR(mod, 0, 0.3);
	return output;
 @endcode
 
 <P><HR><P>
 
 Sawtooth:
 @code
	UGen output = LFSaw::AR(200, 0, 0.3);
	return output;
 @endcode
 
 <P><HR><P>
 
 Subtractive (low pass filter):
 @code
	UGen saw = LFSaw::AR(200, 0, 0.3);
	UGen output = BLowPass::AR(saw, 400, 0.2); // 0.2 is a Q of 1/0.2 = 5
	return output;
 @endcode
 
 <P><HR><P>
 
 Modulated filter:
 @code
	UGen saw = LFSaw::AR(200, 0, 0.3);
	UGen mod = SinOsc::AR(4, 0, 100, 400);
	UGen output = BLowPass::AR(saw, mod, 0.2);
	return output;
 @endcode
 
 <P><HR><P>
 
 Multichannel expansion (multichannel inputs force UGens lower down the chain to be multichannel too) as
 described in the earlier tutorial @ref Tutorial_01_03	"Tutorial 01.03: Building more complex UGen graphs":
 @code
	UGen saw = LFSaw::AR(200, 0, 0.3);
	UGen modFreq = U(4.9, 5.1); // U() is handy a macro for multichannel constant "arrays"
	UGen mod = SinOsc::AR(modFreq, 0, 100, 400);
	UGen output = BLowPass::AR(saw, mod, 0.2);
	return output;
 @endcode
 
 <P><HR><P>
 
 Mixing: you can use the standard arithmetic operator @c + to do this (you can also use the @c * operator
 to do multiplication with UGens and other aritmetic operators too).
 @code
	 UGen sine1 = SinOsc::AR(200, 0, 0.05);
	 UGen sine2 = SinOsc::AR(555, 0, 0.05);
	 UGen sine3 = SinOsc::AR(1000, 0, 0.05);
	 UGen sine4 = SinOsc::AR(1234, 0, 0.05);
	 UGen output = sine1 + sine2 + sine3 + sine4;
	 return output;
 @endcode
 
 <P><HR><P>
 
 More mixing (exactly the same result as the example above but more efficient especially with many channels):
 @code
	 UGen sines = SinOsc::AR(U(200, 555, 1000, 1234), 0, 0.05);
	 UGen output = Mix::AR(sines);
	 return output;
 @endcode
 
 <P><HR><P> 
 
 Yet more mixing (absolutely identical to the example above mix() is a convenience function):
 @code
	 UGen sines = SinOsc::AR(U(200, 555, 1000, 1234), 0, 0.05);
	 UGen output = sines.mix();
	 return output;
 @endcode
 
 <P><HR><P> 
 
 Grouping and mixing:
 @code
	 UGen sines = SinOsc::AR(U(200, 555, 1000, 1234), 0, 0.05);
	 UGen output = sines.groupMix(2); // 200 & 1000 on left, 555 & 1234 on right
	 return output;
 @endcode
 
 <P><HR><P> 
 
 Soundfile playback using a memory-based buffer. Here you may need to change the path to the soundfile. If you
 are testing this in the iPhone project the @c guitar.wav sample is included in the project and the iPhone UGen++
 wrapper automatically searches for soundfiles in the application bundle (if the path is NOT already a full path).
 On the Juce (Mac/Windows) projects you will need to specify the full path to an AIFF or WAV file (16 or 24 bits).
 E.g., @c "/Users/JoeBloggs/Desktop/guitar.wav" or @c "~/Desktop/guitar.wav" on the Mac, or 
 @c "C:\Documents and Settings\JoeBloggs\Desktop\guitar.wav" on Windows.
 @code
	Buffer sound("guitar.wav");
	float rate = 1.0;
	float trigger = 0.0;
	float startPos = 0.0;
	float loop = 1.0;
	UGen output = PlayBuf::AR(sound, rate, trigger, startPos, loop);
	return output;
 @endcode
 
 <P><HR><P>
 
 Same as above but using a @c rate of @c 0.0 and deriving your own "phasor" (a rising sawtooth signal) to drive the 
 "playback head" using the @c startPos input. The rate needs to be the reciprocal of the duration of the buffer 
 (so a 2 second buffer would need a rate of 0.5Hz to play back at its normal speed) and the phasor needs to run 
 from zero to the index of the last sample  (i.e., one less than the buffer size). Here we use LinLin to map a linear 
 range onto another linear range.
 @code
	 Buffer sound("guitar.wav");
	 float rate = 0.0;
	 float trigger = 0.0;
	 UGen startPos = LFSaw::AR(1.0/sound.duration(), 0, 1);
	 startPos = LinLin::AR(startPos, -1, 1, 0, sound.size()-1);
	 float loop = 1.0;
	 UGen output = PlayBuf::AR(sound, rate, trigger, startPos, loop);
	 return output;
 @endcode
 
 <P><HR><P>
 
 Simple delay (all the delay UGens require a maximum delay time to be specified in order to allocate any internal buffers):
 @code
	Buffer sound("guitar.wav");
	float rate = 1.0;
	float trigger = 0.0;
	float startPos = 0.0;
	float loop = 1.0;
	UGen playbuf = PlayBuf::AR(sound, rate, trigger, startPos, loop); 
	
	const float maximumDelay = 2.0;
	UGen output = DelayL::AR(playbuf, maximumDelay, U(0.0, 0.5));
	
	return output;
 @endcode
 
 <P><HR><P>
 
 Modulated delay:
 @code
	Buffer sound("guitar.wav");
	float rate = 1.0;
	float trigger = 0.0;
	float startPos = 0.0;
	float loop = 1.0;
	UGen playbuf = PlayBuf::AR(sound, rate, trigger, startPos, loop);
	
	const float maximumDelay = 2.0;
	UGen delayTime = SinOsc::AR(U(2.9, 3.1), 0, 0.001, 0.002);
	UGen output = DelayL::AR(playbuf, maximumDelay, delayTime);
 
	return output;
 @endcode
 
 <P><HR><P>
 
 Rericulating/feedback delay (feedback amount is specified as a time in seconds for the recirculated signal 
 to decay by 60dBs):
 @code
	Buffer sound("guitar.wav");
	float rate = 1.0;
	float trigger = 0.0;
	float startPos = 0.0;
	float loop = 1.0;
	UGen playbuf = PlayBuf::AR(sound, rate, trigger, startPos, loop);
 
	const float maximumDelay = 2.0;
	float decayTime = 3.0;
	UGen output = CombL::AR(playbuf, maximumDelay, U(0.49, 0.51), decayTime);
 
	return output;
 @endcode
 
 <P><HR><P>
 
 Audio input: an audio input (AudioIn) UGen is passed in as the argument to @c constructGraph() function.
 This should have an appropriate number of channels as set by the host (you can use the getNumChannels() function
 to find out).
 
 e.g., simple (0.5s) delay:
 @code	 
	 UGen output = DelayL::AR(input, 0.5, 0.5);
	 return output;
 @endcode
 
 <P><HR><P>
 
 White noise:
 @code
	UGen output = WhiteNoise::AR(0.1);
	return output;
 @endcode
 
 <P><HR><P>
 
 Random modulation, and using linear to exponential mapping:
 @code
	UGen noise = WhiteNoise::AR(0.1);
	UGen mod = LFNoise2::AR(2); // -1...+1
	mod = LinExp::AR(mod, -1, 1, 200, 10000); // linear to exponential: 200...10000
	UGen output = BLowPass::AR(noise, mod, 0.2);
	return output;
 @endcode
 
 <P><HR><P>
 
 Panning:
 @code
	UGen mod = SinOsc::AR(0.5, 0, 1);		// pan position -1...+1
	UGen mono = SinOsc::AR(1000, 0, 0.2);
	UGen output = Pan2::AR(mono, mod);		// constant power pan
	return output;
 @endcode
 
 
 prev: @ref Tutorial_01_03	"Tutorial 01.03: Building more complex UGen graphs"
 
 next: @ref Tutorial_01_05	"Tutorial 01.05: Maths examples"
*/