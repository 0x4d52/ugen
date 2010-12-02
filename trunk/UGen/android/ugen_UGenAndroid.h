#ifndef UGEN_ANDROID_H
#define UGEN_ANDROID_H


#define GenerateMangledName0(a,b) a ## _ ## b
#define GenerateMangledName(a,b) GenerateMangledName0(a,b)

#define DEFAULTSR 44100.0
#define DEFAULTBLOCKSIZE 512

#include <pthread.h>

class AndroidIOHost
{
public:
	AndroidIOHost(const double sampleRate = 44100.0, const int numInputs = 2, const int numOutputs = 2, const int preferredBufferSize = 0) throw();
	virtual ~AndroidIOHost();
	
	void init() throw();
	int processBlock(const int bufferLength, short *shortBuffer) throw();
	int processBlockOutputOnly(const int bufferLength, short *shortBuffer) throw();
	
	void lock() throw();
	void unlock() throw();
	bool tryLock() throw();

	virtual UGen constructGraph(UGen const& input) throw();
	virtual int sendTrigger(const int index) throw() { return 0; }
	virtual int setParameter(const int index, const float value) throw() { return 0; }
	virtual int sendBytes(const int index, const int size, const char* data) throw() { return 0; }
	
private:
	const double sampleRate;
	const int numInputs, numOutputs;
	const int blockSize;
	float *floatBuffer;
	int currentBlockID;
	
	pthread_mutex_t mutex;
	
	UGen output;
};
		
	
#endif // UGEN_ANDROID_H
