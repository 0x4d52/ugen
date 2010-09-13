#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <AudioToolbox/AudioQueue.h>
#include "../../UGen/UGen.h"

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define FRAME_COUNT 512
#define AUDIO_BUFFERS 3

typedef struct AQCallbackStruct 
{
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
	UGen synth;
} AQCallbackStruct;

int playbuffer();
void AQBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB);

int main(int argc, char *argv[]) 
{
	// this initialised the UGen system, it should be called at the start of your app.
	UGen::initialise();
	
    int error = playbuffer();
	
	// shutdown the UGen system
	UGen::shutdown();
	
	return error;
}


int playbuffer() 
{
    AQCallbackStruct aqc;
    UInt32 err, bufferSize;
	
    aqc.mDataFormat.mSampleRate = SAMPLE_RATE;
    aqc.mDataFormat.mFormatID = kAudioFormatLinearPCM;
	aqc.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
	int sampleSize = sizeof(float);
	aqc.mDataFormat.mBitsPerChannel = 8 * sampleSize;
	aqc.mDataFormat.mChannelsPerFrame = NUM_CHANNELS;
    aqc.mDataFormat.mBytesPerPacket = aqc.mDataFormat.mChannelsPerFrame * sampleSize;
    aqc.mDataFormat.mFramesPerPacket = 1;
    aqc.mDataFormat.mBytesPerFrame = aqc.mDataFormat.mBytesPerPacket * aqc.mDataFormat.mFramesPerPacket;
    
    aqc.frameCount = FRAME_COUNT;
	
    err = AudioQueueNewOutput(&aqc.mDataFormat,
							  AQBufferCallback,
							  &aqc,
							  NULL,
							  kCFRunLoopCommonModes,
							  0,
							  &aqc.queue);
    if (err)
        return err;
	
    aqc.frameCount = FRAME_COUNT;
    bufferSize = aqc.frameCount * aqc.mDataFormat.mBytesPerFrame;
	
	// before processing any audio provide a sample rate, blocksize and (optionally) a control-rate blocksize
	UGen::prepareToPlay(aqc.mDataFormat.mSampleRate, 
						aqc.frameCount, 
						aqc.frameCount);
	
	// test signal..
	aqc.synth = LPF::AR(LFSaw::AR(400, 0, 0.2), SinOsc::AR(4, 0, 200, 800));
	
    for (int i = 0; i < AUDIO_BUFFERS; i++) 
	{
        err = AudioQueueAllocateBuffer(aqc.queue, bufferSize,
									   &aqc.mBuffers[i]);
        if (err)
            return err;
		
        AQBufferCallback(&aqc, aqc.queue, aqc.mBuffers[i]);
    }
	
    err = AudioQueueStart(aqc.queue, NULL);
	
    if (err)
        return err;
		
    while(1) sleep(1);
	
    return 0;
}

void AQBufferCallback(void *in,
					  AudioQueueRef inQ,
					  AudioQueueBufferRef outQB)
{
    AQCallbackStruct *aqc  = (AQCallbackStruct *) in;
    float *coreAudioBuffer = (float*) outQB->mAudioData;

    if (aqc->frameCount > 0) 
	{
        outQB->mAudioDataByteSize = aqc->mDataFormat.mBytesPerPacket * aqc->frameCount;
        
		int blockID = UGen::getNextBlockID(aqc->frameCount);
		
		aqc->synth.setOutput(coreAudioBuffer, aqc->frameCount, 0);
		aqc->synth.prepareAndProcessBlock(aqc->frameCount, blockID, 0);
        
		AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
    }
}

