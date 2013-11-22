
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
		case Freq:	freq = value;			return 0;
		case Amp:	amp = value;			return 0;
		case On:	on = value;				return 0;
	}

	return -1; // param not found
}

UGen MyAndroidAudio::constructGraph(UGen const& input) throw()
{
//	return SinOsc::AR(freq.ar().lag(), 0, (amp * on).ar.lag());
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "MyAndroidAudio::constructGraph()");

	return SinOsc::AR(1000, 0, 0.2);
}

//
//// the JNI code needs to link to this function to create the host
//AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize)
//{
//	return new MyAndroidAudio(sampleRate, numInputs, numOutputs, preferredBufferSize);
//}
//
//MyAndroidAudio::MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw()
//:	AndroidIOHost(sampleRate, numInputs, numOutputs, preferredBufferSize)
//{
//}
//
//int MyAndroidAudio::setParameter(const int index, const float value) throw()
//{	
//	int idIndex = ids.indexOf(index);
//
//	if(idIndex < 0) return -1; // not found
//
//	freqs[idIndex].setValue(value); // get the Value object and set its value
//
//	return 0;
//}
//
//int MyAndroidAudio::sendTrigger(const int index) throw()
//{
//	if(index == 0)
//	{
//		return -1; // invalid must not be zero as -0 is 0
//	}
//	else if(index > 0)
//	{
//		int idIndex = ids.indexOf(index);
//
//		if(idIndex >= 0) return -1; // already exists
//
//		Value freq = 0; // use Value for a variable control
//		UGen freqControl = freq;
//		UGen event = SinOsc::AR(freqControl.kr(), 0, 0.1); // kr Value UGens do linear ramping between values to smooth out clicks automatically
//
//		lock();
//
//		ids.add(index); // add a look-up index
//		freqs.add(freq); // add it to the array of freq
//		events.add(event); // add event
//
//		unlock();
//	}
//	else
//	{
//		int deleteIndex = -index;
//
//		int idIndex = ids.indexOf(deleteIndex);
//
//		if(idIndex < 0) return -1; // invalid nothing to delete
//
//		lock();
//
//		events.remove(idIndex);
//		freqs.remove(idIndex);
//		ids.remove(idIndex);
//		
//		unlock();
//	}
//
//	return 0;
//}
//
//UGen MyAndroidAudio::constructGraph(UGen const& input) throw()
//{
//	return Mix::AR(events, false);	
//}
