#include "../../../UGen/UGen.h"
#include "../../../UGen/android/ugen_UGenAndroid.h"

//class MyAndroidAudio : public AndroidIOHost
//{
//public:
//	MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw();
//	UGen constructGraph(UGen const& input) throw();
//	int setParameter(const int index, const float value) throw();
//
//	enum Params { Freq = 0, Amp = 1, On = 2 };
//	
//private:
//	float freq;
//	float amp;
//	UGenArray events;
//};

class MyAndroidAudio : public AndroidIOHost
{
public:
	MyAndroidAudio(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize) throw();
	UGen constructGraph(UGen const& input) throw();
	int setParameter(const int index, const float value) throw();
	int sendTrigger(const int index) throw();
	
private:
	ObjectArray<int> ids;
	ObjectArray<Value> freqs;
	UGenArray events;
	
};
