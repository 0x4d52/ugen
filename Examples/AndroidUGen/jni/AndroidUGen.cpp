
#include "AndroidUGen.h"
#include <android/log.h>

// the JNI code needs to link to this function to create the host
AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize)
{
	return new MyAndroidAudio(sampleRate, numInputs, numOutputs, preferredBufferSize);
}

MyAndroidAudio::MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw()
:	AndroidIOHost(sampleRate, numInputs, numOutputs, preferredBufferSize),
	freq(0), amp(0), on(0)
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "MyAndroidAudio::MyAndroidAudio()");
}

int MyAndroidAudio::setParameter(const int index, const float value) throw()
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "MyAndroidAudio::setParameter(%d, %f)", index, value);

	switch(index)
	{
		case Freq:	freq.setValue(value);			return 0;
		case Amp:	amp.setValue(value);			return 0;
		case On:	on.setValue(value);				return 0;
	}

	return -1; // param not found
}

UGen MyAndroidAudio::constructGraph(UGen const& input) throw()
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "MyAndroidAudio::constructGraph()");

	return SinOsc::AR(UGen(freq).lag(), 0, UGen(amp * on).lag());
}


