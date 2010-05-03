/** 
 @page		Tutorial_01_09	Tutorial 01.09: Delay examples

 This tutorial intruduces some of the possibilities and techniques with UGen++'s delay-based UGen classes: 
 e.g., DelayN, DelayL, CombN, CombL, AllpassN, AllpassL. Each of these allocates its own delay buffer internally
 and the size of this buffer is specified by the @c maximumDelayTime parameter. In most of these examples the 
 maximum delay is set to 5 seconds in order to make it easier for you to experiment with delay times. Normally,
 you would set the maximum delay no larger than it needs to be.
 
 Again as with previous "examples" tutorials, just copy and paste the examples below into the @c constructGraph() 
 function (replacing the entire body of the function) to try them out.
 
 <p><hr><p>
 Simple delay: left channel dry, right channel delayed by 1 second. DelayN is a non-interpolating delay line:
 delay times are quantised to the delay times in whole samples (subsample delays are not possible). DelayN should
 be sufficient where delay times are not being modulated or where efficiency is more of a concern than quality.
 
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 UGen delay  = DelayN::AR(mix, maximumDelay, 1.0);
	 return U(mix, delay);
 @endcode
 
 <p><hr><p>
 Multitap delays with different amplitudes for each tap.
 
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 UGen delay  = DelayN::AR(mix, maximumDelay, U(0.1, 0.2, 0.3, 0.4, 0.5, 0.6));
	 delay *= U(0.01, 0.02, 0.04, 0.08, 0.16, 0.32);
	 delay = delay.mixScale();
	 return U(mix, delay); 
 @endcode
 
 <p><hr><p>
 Modulated delay (giving a pitch varying effects), here using DelayL for better quality.
 
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 float modDepth = 0.005; // depth of +- 5ms
	 float delayTime = 0.01; // constant offset delay time of 10ms
	 UGen mod = SinOsc::AR(4, 0, modDepth, delayTime);
	 UGen delay  = DelayL::AR(mix, maximumDelay, mod);
	 return delay;
 @endcode
 
 <p><hr><p>
 Same as above but using a multichannel modulator resulting in a multichannel effect.
 
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 float modDepth = 0.002; // depth of +- 2ms
	 float delayTime = 0.002; // constant offset delay time of 2ms
	 UGen mod = SinOsc::AR(U(5.9, 6.1), 0, modDepth, delayTime); // multichannel modulator
	 UGen delay  = DelayL::AR(mix, maximumDelay, mod);
	 return delay;
 @endcode 
 
 <p><hr><p>
 
 Stereo flanger:
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 float modDepth = 0.001;
	 UGen mod = LFSaw::AR(U(0.11, 0.10), 0, modDepth, modDepth); // multichannel modulator
	 UGen delay  = DelayL::AR(mix, maximumDelay, mod);
	 return delay + mix;
 @endcode
 
 <p><hr><p>
 
 Stereo chorus:
 @code
	 const float maximumDelay = 5.0;
	 UGen mix = input.mix();
	 float modDepth = 0.001;
	 
	 const int numDelays = 8;
	 FloatArray delayRates = FloatArray::rand(numDelays, 0.05, 0.15);
	 
	 UGen mod = LFNoise1::AR(delayRates, modDepth, modDepth);
	 UGen delay  = DelayL::AR(mix, maximumDelay, mod);
	 
	 UGen output; // somewhere to put the panned delays
	 
	 for(int i = 0; i < delay.getNumChannels(); i++)
	 {
		 UGen panned = Pan2::AR(delay[i], rand2(1.0)); // random but fixed-position pan
		 output <<= panned; // append channels
	 }
	 
	 // group into stereo pairs and down to mix stereo out:
	 // scaling down the level based on the number being mixed
	 return output.groupMixScale(2); 
 @endcode 
 
 <p><hr><p>
 
 Echo using a comb filter: slightly different delay times for left and right (close to 0.25s) to give a mild stereo effect.
 @code
	 const float maximumDelay = 5.0;
	 const float decayTime = 7; // time is seconds to decay by 60dB
	 return CombL::AR(input, maximumDelay, U(0.251, 0.249), decayTime);
 @endcode
 
 <p><hr><p>
 
 Same as above but with a true "comb filter" sound to it by using a delay time resulting in ringing at audio frequencies. 
 Here the delay times are close to 0.01s which give a fundamental frequency to the ringing of 100Hz (i.e., 1.0/0.01 = 100).
 Again slightly different delay times used on the left and right giving a detune effect.
 @code
	 const float maximumDelay = 5.0;
	 const float decayTime = 1.0;
	 return CombL::AR(input, maximumDelay, U(0.009, 0.011), decayTime); 
 @endcode
 
 <p><hr><p>
 
 Modulated version of the above using a sine wave modulator on the delay time. Again a slightly different rate is used on the
 left and right channel so this gives a stereo-type effect. A small variation (±0.0002) to the delay time gives the vibrato effect.
 
 @code
	 const float maximumDelay = 5.0;
	 const float decayTime = 1.0;
	 const float delay = 0.01;
	 UGen mod = SinOsc::AR(U(5.4, 5.6), 0, 0.0002, delay);
	 return CombL::AR(input, maximumDelay, mod, decayTime);
 @endcode
 
 <p><hr><p>
 
 Typical comb-based early reflection simulation using parallel comb filters with randomised delay times.
 
 @code
	 const float maximumDelay = 0.1;
	 const float decayTime = 5.0;
	 UGen mix = input.mix();
	 
	 const int numCombs = 4;
	 FloatArray delayTimes = FloatArray::exprand(numCombs, 0.01, maximumDelay);
	 UGen combs = CombL::AR(input, maximumDelay, delayTimes, decayTime);
	 
	 return combs.mixScale();
 @endcode
 
 <p><hr><p>
 
 Typcal allpass-based reverb tail simulation using a series (or chain) of allpass delays with randomised delay times.
 
 @code
	 const float decayTime = 5.0;
	 UGen mix = input.mix();
	 UGen reverb = mix; // input to the chain
	 
	 const int n = 16;
	 for(int i = 0; i < n; i++)
	 {
		 // save the output of this chain link for next time
		 // using the last link as input...
		 const float delay = rand(0.0, 0.05);
		 reverb = AllpassL::AR(reverb, delay, delay, decayTime);
	 }
	 
	 return reverb; 
 @endcode
 
 */  /*
 prev: @ref Tutorial_XX_XX	"Tutorial XX.XX: Name"
 
 next: @ref Tutorial_XX_XX	"Tutorial XX.XX: Name"
*/