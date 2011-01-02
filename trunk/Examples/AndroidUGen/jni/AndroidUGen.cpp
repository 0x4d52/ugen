
#include "AndroidUGen.h"

// the JNI code needs to link to this function to create the host
AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize)
{
	return new MyAndroidAudio(sampleRate, numInputs, numOutputs, preferredBufferSize);
}

MyAndroidAudio::MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw()
:	AndroidIOHost(sampleRate, numInputs, numOutputs, preferredBufferSize),
	freq(0), amp(0)
{
}

int MyAndroidAudio::setParameter(const int index, const float value) throw()
{	
	switch(index)
	{
		case Freq:	freq = value;			return 0;
		case Amp:	amp = value;			return 0;
		case On:	
		{
			if(value >= 0.5)
			{
				Env env = Env::linen(0.2, 4, 0.5, amp);
				UGen envgen = EnvGen::AR(env);
				UGen sine = SinOsc::AR(freq, 0, envgen);
					
				lock();
				events.add(sine);
				unlock();
			}
			
			return 0;
		}
	}
		
	return -1; // param not found
}

UGen MyAndroidAudio::constructGraph(UGen const& input) throw()
{
	return Mix::AR(events, false);	
}
