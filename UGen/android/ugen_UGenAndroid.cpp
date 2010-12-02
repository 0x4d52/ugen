#include "../UGen.h"
#include "../core/ugen_StandardHeader.h"
#include "../basics/ugen_BinaryOpUGens.cpp"
#include "../basics/ugen_Chain.cpp"
#include "../basics/ugen_MappingUGens.cpp"
#include "../basics/ugen_MixUGen.cpp"
#include "../basics/ugen_MulAdd.cpp"
#include "../basics/ugen_Pause.cpp"
#include "../basics/ugen_Plug.cpp"
#include "../basics/ugen_RawInputUGens.cpp"
#include "../basics/ugen_ScalarUGens.cpp"
#include "../basics/ugen_Thru.cpp"
#include "../basics/ugen_UnaryOpUGens.cpp"
#include "../basics/ugen_WrapFold.cpp"
#include "../buffers/ugen_Buffer.cpp"
#include "../buffers/ugen_PlayBuf.cpp"
#include "../core/ugen_Arrays.cpp"
#include "../core/ugen_Bits.cpp"
#include "../core/ugen_Deleter.cpp"
#include "../core/ugen_ExternalControlSource.cpp"
#include "../core/ugen_Random.cpp"
#include "../core/ugen_SmartPointer.cpp"
#include "../core/ugen_Text.cpp"
#include "../core/ugen_UGen.cpp"
#include "../core/ugen_UGenArray.cpp"
#include "../core/ugen_UGenInternal.cpp"
#include "../core/ugen_Value.cpp"
#include "../envelopes/ugen_Env.cpp"
#include "../envelopes/ugen_EnvCurve.cpp"
#include "../envelopes/ugen_EnvGen.cpp"
#include "../envelopes/ugen_Lines.cpp"
#include "../filters/control/ugen_Lag.cpp"
#include "../oscillators/wavetable/ugen_TableOsc.cpp"

//BEGIN_UGEN_NAMESPACE

#include "ugen_UGenAndroid.h"


AndroidIOHost::AndroidIOHost(const double sampleRateToUse, const int numInputsToUse, const int numOutputsToUse, const int preferredBufferSize) throw()
:	sampleRate(sampleRateToUse <= 0.0 ? DEFAULTSR : sampleRateToUse),
	numInputs(ugen::clip(numInputsToUse, 1, 2)),
	numOutputs(ugen::clip(numOutputsToUse, 1, 2)),
	blockSize(preferredBufferSize <= 0 ? DEFAULTBLOCKSIZE : preferredBufferSize),
	floatBuffer(new float[ugen::max(numInputs, numOutputs) * blockSize]),
	currentBlockID(-1)
{
	UGen::initialise();
	UGen::prepareToPlay(sampleRate, blockSize, ugen::min(blockSize, 64));

	output = Plug::AR(UGen::emptyChannels(numOutputs), false);
}

AndroidIOHost::~AndroidIOHost()
{
	UGen::shutdown();

	delete [] floatBuffer;
}

void AndroidIOHost::init() throw()
{
	output.setSource(constructGraph(0), true); // no inputs yet
}

int AndroidIOHost::processBlock(const int bufferLength, short *shortBuffer) throw()
{
	float *floatBufferData[2];
	floatBufferData[0] = floatBuffer;
	floatBufferData[1] = floatBuffer + blockSize;
	
	static const float upScale = 32767.f;
	static const float downScale = 1.f / upScale;
	
	for(int channel = 0; channel < numInputs; channel++)
	{
		short *shortBufferChannel = shortBuffer + channel;
		float *floatBufferChannel = floatBufferData[channel];
		
		int numSamplesToProcess = blockSize;
		
		while(numSamplesToProcess--)
		{
			*floatBufferChannel = (float)(*shortBufferChannel) * downScale;
			
			floatBufferChannel++;
			shortBufferChannel += numInputs;
		}
	}
	
	// set inputs...
	//..
	// set outputs
	output.setOutputs(floatBufferData, blockSize, numOutputs);
	
	currentBlockID = UGen::getNextBlockID(blockSize);
	output.prepareAndProcessBlock(blockSize, currentBlockID, -1);
	
	for(int channel = 0; channel < numOutputs; channel++)
	{
		short *shortBufferChannel = shortBuffer + channel;
		float *floatBufferChannel = floatBufferData[channel];
		
		int numSamplesToProcess = blockSize;
		
		while(numSamplesToProcess--)
		{
			*shortBufferChannel = (short)(*floatBufferChannel * upScale);
						
			floatBufferChannel++;
			shortBufferChannel += numInputs;
		}
	}
	
	return 0;
}

int AndroidIOHost::processBlockOutputOnly(const int bufferLength, short *shortBuffer) throw()
{
	float *floatBufferData[2];
	floatBufferData[0] = floatBuffer;
	floatBufferData[1] = floatBuffer + blockSize;
	
	static const float upScale = 32767.f;
	static const float downScale = 1.f / upScale;
	
	// set outputs
	output.setOutputs(floatBufferData, blockSize, numOutputs);
	
	currentBlockID = UGen::getNextBlockID(blockSize);
	output.prepareAndProcessBlock(blockSize, currentBlockID, -1);
	
	for(int channel = 0; channel < numOutputs; channel++)
	{
		short *shortBufferChannel = shortBuffer + channel;
		float *floatBufferChannel = floatBufferData[channel];
		
		int numSamplesToProcess = blockSize;
		
		while(numSamplesToProcess--)
		{
			*shortBufferChannel = (short)(*floatBufferChannel * upScale);
			
			floatBufferChannel++;
			shortBufferChannel += numInputs;
		}
	}
	
	return 0;
}


UGen AndroidIOHost::constructGraph(UGen const& input) throw()
{
	return 0;
}

//END_UGEN_NAMESPACE


































