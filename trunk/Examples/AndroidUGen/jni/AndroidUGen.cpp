
#include "AndroidUGen.h"

// the JNI code needs to link to this function to create the host
AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize)
{
	return new MyAndroidAudio(sampleRate, numInputs, numOutputs, preferredBufferSize);
}

MyAndroidAudio::MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw()
:	AndroidIOHost(sampleRate, numInputs, numOutputs, preferredBufferSize)
{
}

int MyAndroidAudio::setParameter(const int index, const float value) throw()
{
	switch(index)
	{
		case Freq:	freq = value;			return 0;
		case Amp:	amp = value;			return 0;
		case On:	on = (value >= 0.5f);	return 0;
	}
	
	return -1; // param not found
}

UGen MyAndroidAudio::constructGraph(UGen const& input) throw()
{
	UGen freqControl = Lag::AR(&freq);
	UGen ampControl = Lag::AR(&amp);
	UGen onControl = Lag::AR(&on);
	
	return SinOsc::AR(freqControl, 0, ampControl) * onControl;
}
