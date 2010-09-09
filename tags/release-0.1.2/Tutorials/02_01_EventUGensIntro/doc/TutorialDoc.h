/** 
 @page		Tutorial_02_01		Tutorial 02.01: Introduction to event-based UGen classes

 This tutorial introduces UGen++'s concept of event-based UGen classes. An event class is 
 a class which can generate new UGen graph events and connect them to the running audio engine.
 The new UGen graph events might be generated according to time (e.g., a new event every
 second) or according to a trigger (e.g., a signal exceeding a threshold, or a button click on 
 a GUI). The idea is closely developed around SuperCollider 2's '%Spawn' and in fact one of the
 event-based UGen classes is Spawn in UGen++. Each event-based UGen has a corresponding event class
 which is used to generate events for that event-based UGen. There is also a generic event class
 which can be used with most of the event-based UGen classes although this limits the possibilities.
 
 This tutorial uses the generic event class to illustrate some of the operations which are possible
 using event-based UGen classes. Later tutorials show more complex systems, some of which illustrate 
 the benefits of using the dedicated event class for the event-based UGen in question.
 
 (As with some of the previous introductory tutorials the examples here can be used with the iPhone
 example project. Simply replace the entire @c Factory class at the top of the @c main.mm file.)
 
 @includelineno 02_01_EventUGensIntro/MainComponent.h
 
 This example extracts the UGen generation classes into a separate class, @c Factory (it could have any name
 this is not a built-in UGen++ class). The @c Factory class has a @c constructGraph() function which
 is called by the main @c MainComponent / JuceIOHost class. The @c Factory also has a @c createEvent() function which is
 a pure virtual function of the GenericEventBase class. (GenericEventDefault is the default version of
 GenericEventBase, again using a templated version of GenericEventBase will be shown in later tutorials.)
 
 In this first example MixFill is used. This is one of the simplest event-based UGen classes since it only
 makes use of the event class when it is created (rather than spawning new events while it is running).
 
 @code
	class Factory : public GenericEventDefault
	{
	public:
		UGen createEvent(const int count)
		{
			// random frequency
			return SinOsc::AR(exprand(100, 1000), 0, 0.2);
		}
		
		UGen constructGraph(UGen const& input)
		{
			int n = 8;
			return MixFill<Factory>::AR(n) / n;
		}
	};
 @endcode
 
 Here the @c createEvent() function is called eight times when the MixFill UGen is created. This creates eight sine
 waves with random frequencies (exponentially distrubuted between 100Hz and 1kHz using the UGen++ random function exprand() ). 
 Notice that the MixFill class requires the event class to be be provided as a template argument (i.e, between the angle 
 brackets @c < @c >). In this case the event class is @c Factory, which happens to be the same class in which we are calling
 our @c constructGraph() function. (In more complex examples this might not be the case, it is done this way in this tutorial
 to keep everything in one place to aid copying/pasting examples from this tutorial document.)
 
 MixFill takes a single integer argument to specify how many events to mix together. In this example 
 the value is also used to scale down the amplitude by the same factor (this uses division in the code but in fact UGen++ 
 converts this to a more efficient multiplication operation internally).
 
 <P><HR><P>
 
 XFadeTexture: this applies a fade-in and fade-out amplitude envelope to the generated event, crossfading from one instance of the
 event to another. Here the crossfade time is 0.5s and the sustain time is 1.0s (making the total time between events 1.5s).
 
 It is important to reiterate that XFadeTexture is applying the amplitude envelope. The most important part of this is that once 
 the envelope reaches its endpoint (i.e., when it has faded out) <b>the event is destroyed</b>. This is critical since the 
 computer's memory would eventually fill up if exhasuted events were kept in memory after they are no longer needed. Some event-based
 UGen classes <b>do not</b> apply this envelope and it is the user's responsibility to do this to avoid memory problems (e.g, Spawn, TSpawn).
 
 @code
	class Factory : public GenericEventDefault
	{
	public:
		UGen createEvent(const int count)
		{
			return SinOsc::AR(exprand(100, 1000), 0, 0.2);
		}
		
		UGen constructGraph(UGen const& input)
		{
			float sustainTime = 1.0;
			float transitionTime = 0.5;
			return XFadeTexture<Factory>::AR(sustainTime, transitionTime);
		}
	};
 @endcode
 
 <P><HR><P>

 OverlapTexture: this is similar to XFadeTexture since events are provided a fade-in and fade out. There is
 an additional parameter to control the <b>density</b> of the events (i.e., how many events will sound at any
 one time. In fact a density of 2 makes OverlapTexture behave identically to XFadeTexture.
 
 Similar the the MixFill example the density is used to scale down the overall amplitude to avoid overloading
 when there a many events sounding simultaneously.

 @code
	class Factory : public GenericEventDefault
	{
	public:
		UGen createEvent(const int count)
		{
			return SinOsc::AR(exprand(100, 1000), 0, 0.2);
		}
		
		UGen constructGraph(UGen const& input)
		{
			float sustainTime = 1.0;
			float transitionTime = 0.5;
			int density = 8;
			return OverlapTexture<Factory>::AR(sustainTime, transitionTime, density) / density;
		}
	};
 @endcode
 
 prev: @ref Tutorials_01	"Tutorials 01: Fundamentals"
 
 <!--next: @ref Tutorial_XX_XX	"Tutorial XX.XX: Name"-->
*/