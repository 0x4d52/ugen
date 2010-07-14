// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-10 by Martin Robinson www.miajo.co.uk
 
 ------------------------------------------------------------------------------
 
 UGEN++ can be redistributed and/or modified under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.
 
 UGEN++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with UGEN++; if not, visit www.gnu.org/licenses or write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA
 
 The idea for this project and code in the UGen implementations is
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef UGEN_NOEXTGPL // can't be bothered to remove the use of Env/EnvGen - this file should probably be deprecated anyway..

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_AudioQueueIOHost.h"
#include "../basics/ugen_RawInputUGens.h"
#include "../core/ugen_Bits.h"
#include "../envelopes/ugen_EnvGen.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

// 2048 for input?
#define FRAME_COUNT 512

// 7 for input?
#define AUDIO_BUFFERS 3
#define FLOATBUFFER_SIZE (FRAME_COUNT*NUM_CHANNELS)

typedef struct AQIOStruct
{
	AudioQueueRef queue;
	AudioStreamBasicDescription dataFormat;
	AudioQueueBufferRef buffers[AUDIO_BUFFERS];
	float floatBuffer[FLOATBUFFER_SIZE*AUDIO_BUFFERS];
	int writeIndex, readIndex;
	UGen graph;
	
} AQIOStruct;


typedef struct AQCallbackStruct 
{
	AudioQueueIOHostController* owner;
	UInt32 frameCount;
	
	AQIOStruct inputInfo;
	AQIOStruct outputInfo;
	
} AQCallbackStruct;

inline void AQInputBufferCallback(void									*in, 
								  AudioQueueRef							inQ, 
								  AudioQueueBufferRef					inQB, 
								  const AudioTimeStamp					*timestamp,
								  unsigned long							frameSize,
								  const AudioStreamPacketDescription	*dataFormat) throw()
{
	//printf("AQInputBufferCallback inQB=%p frameSize=%d mAudioDataByteSize=%d\n", inQB, frameSize, inQB->mAudioDataByteSize);
	
    AQCallbackStruct *aqc  = (AQCallbackStruct *) in;
	[aqc->owner audioQueueInputCallback:inQ bufferRef:inQB];
}

inline void AQOutputBufferCallback(void					*in, 
								   AudioQueueRef		inQ, 
								   AudioQueueBufferRef	outQB) throw()
{
    AQCallbackStruct *aqc  = (AQCallbackStruct *) in;
	[aqc->owner audioQueueOutputCallback:inQ bufferRef:outQB];
}



@implementation AudioQueueIOHostController
- (id)init
{	
	aqc = new AQCallbackStruct();
	
	return self;
}

- (int) initAudioQueue:(BOOL)isOutputOnly; 
{
	
	// tried something like
	//[NSThread detachNewThreadSelector:@selector(initAudioQueue) toTarget:self withObject:nil];
	// to create a new thread but it looks like a lot of management
	
	const int sampleSize = sizeof(short);
    UInt32 err;
	
	UGen::initialise();
	deleter = new NSDeleter();
	UGen::setDeleter(deleter);
	
	// init common struct members
	
	aqc->owner = self;
	
    aqc->outputInfo.dataFormat.mSampleRate = SAMPLE_RATE;
    aqc->outputInfo.dataFormat.mFormatID = kAudioFormatLinearPCM;
	aqc->outputInfo.dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	aqc->outputInfo.dataFormat.mBitsPerChannel = 8 * sampleSize;
	aqc->outputInfo.dataFormat.mChannelsPerFrame = NUM_CHANNELS;
    aqc->outputInfo.dataFormat.mBytesPerPacket = aqc->outputInfo.dataFormat.mChannelsPerFrame * sampleSize;
    aqc->outputInfo.dataFormat.mFramesPerPacket = 1;
    aqc->outputInfo.dataFormat.mBytesPerFrame = aqc->outputInfo.dataFormat.mBytesPerPacket * aqc->outputInfo.dataFormat.mFramesPerPacket;
    
	aqc->inputInfo = aqc->outputInfo;
	//aqc->inputInfo.dataFormat.mFormatFlags = kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	
    aqc->frameCount = FRAME_COUNT;
	const UInt32 bufferSize = aqc->frameCount * aqc->outputInfo.dataFormat.mBytesPerFrame;
	
	// init UGen++
	aqc->inputInfo.graph = AudioIn::AR(NUM_CHANNELS);
	UGen output = [self constructGraph:aqc->inputInfo.graph];
	aqc->outputInfo.graph = output * EnvGen::KR(Env::asr(0.1, 1.0, 0.1, 1.0, 0.0));
	UGen::prepareToPlay(aqc->outputInfo.dataFormat.mSampleRate, aqc->frameCount, 64);
	
	
	aqc->inputInfo.writeIndex = 0;
	aqc->inputInfo.readIndex = AUDIO_BUFFERS-1;
	memset(aqc->inputInfo.floatBuffer, 0, (FLOATBUFFER_SIZE*AUDIO_BUFFERS) * sizeof(float));
	
	// init input queue
	
	//printf("audioQueueInputCallback ASSUMING 2 CHANNELS !!!!!!\n");
	
//	UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
//	AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride), &audioRouteOverride);
//	AudioSessionSetActive(true);
	
	if(isOutputOnly == NO)
	{
		err = AudioQueueNewInput (&aqc->inputInfo.dataFormat,
								  AQInputBufferCallback,
								  aqc,
								  NULL,//CFRunLoopGetCurrent(),
								  kCFRunLoopCommonModes,
								  0,
								  &aqc->inputInfo.queue);
		
		for (int i = 0; i < AUDIO_BUFFERS; i++) 
		{
			err = AudioQueueAllocateBuffer (aqc->inputInfo.queue, bufferSize, 
											&aqc->inputInfo.buffers[i]);
			
			if (err) return err;
			
			AudioQueueEnqueueBuffer (aqc->inputInfo.queue, aqc->inputInfo.buffers[i], 0, NULL);
		}
		
		err = AudioQueueStart(aqc->inputInfo.queue, NULL);
		if (err) return err;
	}
	
	// init output queue
    err = AudioQueueNewOutput(&aqc->outputInfo.dataFormat,
							  AQOutputBufferCallback,
							  aqc,
							  NULL,//CFRunLoopGetCurrent(), //NULL,
							  kCFRunLoopCommonModes,
							  0,
							  &aqc->outputInfo.queue);
	
    if (err) return err;
	
    for (int i = 0; i < AUDIO_BUFFERS; i++) 
	{
        err = AudioQueueAllocateBuffer(aqc->outputInfo.queue, bufferSize,
									   &aqc->outputInfo.buffers[i]);
		
        if (err) return err;
		
        AQOutputBufferCallback(aqc, aqc->outputInfo.queue, aqc->outputInfo.buffers[i]);
    }
	
    err = AudioQueueStart(aqc->outputInfo.queue, NULL);
    if (err) return err;
	
    return 0;
}

- (void) audioQueueInputCallback: (AudioQueueRef) inQ bufferRef: (AudioQueueBufferRef) inQB;
{
	static const float oneOverMaxVal = 1.f / (float) 0x7ffe;
	
	short * const coreAudioBuffer = (short*) inQB->mAudioData;
	float * const floatBuffer = aqc->inputInfo.floatBuffer + (FLOATBUFFER_SIZE * aqc->inputInfo.writeIndex);
	
//	// temp to stop unused warnings
//	(void)coreAudioBuffer;
//	(void)floatBuffer;
//	(void)oneOverMaxVal;
//	
//	// need to copy coreAudioBuffer -> floatBuffer
//	printf("audioQueueInputCallback: %p\n", inQB);
	
//	printf("audioQueueInputCallback ASSUMING 2 CHANNELS !!!!!!\n");
	
	int numFramesToProcess = aqc->frameCount;
	short* inChannel0 = coreAudioBuffer;
	float* ugenChannel0 = floatBuffer;
	
	switch(NUM_CHANNELS)
	{
		case 1: {
			while(numFramesToProcess--)
			{
				*ugenChannel0 = (float)(*inChannel0) * oneOverMaxVal;
				inChannel0++;
				ugenChannel0++;
			}
		} break;
		case 2: {
			short* inChannel1 = coreAudioBuffer+1;
			float* ugenChannel1 = floatBuffer+numFramesToProcess;
			
			while(numFramesToProcess--)
			{
				*ugenChannel0 = (float)(*inChannel0) * oneOverMaxVal;
				*ugenChannel1 = (float)(*inChannel1) * oneOverMaxVal;
				
				inChannel0 += NUM_CHANNELS;
				inChannel1 += NUM_CHANNELS;
				ugenChannel0++;
				ugenChannel1++;
			}
		}
	}
	
	aqc->inputInfo.writeIndex++;
	if(aqc->inputInfo.writeIndex == AUDIO_BUFFERS)
		aqc->inputInfo.writeIndex = 0;
	
	AudioQueueEnqueueBuffer (inQ, inQB, 0, NULL);
}

- (void) audioQueueOutputCallback: (AudioQueueRef) inQ bufferRef: (AudioQueueBufferRef) outQB;
{
	static const float maxVal = (float) 0x7ffe;
	
	int numFramesToProcess = aqc->frameCount;
	short * const coreAudioBuffer = (short*) outQB->mAudioData;
	float * const outputFloatBuffer = aqc->outputInfo.floatBuffer;
	float * const inputFloatBuffer = aqc->inputInfo.floatBuffer + (FLOATBUFFER_SIZE * aqc->inputInfo.readIndex);
	
    if (numFramesToProcess > 0) 
	{
        outQB->mAudioDataByteSize = aqc->outputInfo.dataFormat.mBytesPerPacket * numFramesToProcess;
        
		int blockID = UGen::getNextBlockID(numFramesToProcess);
		
		for(int channel = 0; channel < aqc->outputInfo.dataFormat.mChannelsPerFrame; channel++)
		{
			aqc->inputInfo.graph.setInput(inputFloatBuffer + numFramesToProcess * channel, 
										  numFramesToProcess, 
										  channel);
			
			aqc->outputInfo.graph.setOutput(outputFloatBuffer + numFramesToProcess * channel, 
											numFramesToProcess, 
											channel);
		}
		
		aqc->outputInfo.graph.prepareAndProcessBlock(numFramesToProcess, blockID);
				
		short* outChannel0 = coreAudioBuffer;
		float* ugenChannel0 = outputFloatBuffer;
		
		switch(NUM_CHANNELS)
		{
			case 1: {
				while(numFramesToProcess--)
				{
					*outChannel0 = (short)(*ugenChannel0 * maxVal);					
					outChannel0++;
					ugenChannel0++;
				}
			} break;
			case 2: {
				short* outChannel1 = coreAudioBuffer+1;
				float* ugenChannel1 = outputFloatBuffer+numFramesToProcess;
				
				while(numFramesToProcess--)
				{
					*outChannel0 = *ugenChannel0 * maxVal;
					*outChannel1 = *ugenChannel1 * maxVal;
					
					outChannel0 += NUM_CHANNELS;
					outChannel1 += NUM_CHANNELS;
					ugenChannel0++;
					ugenChannel1++;
				}
			} break;
			default: {
				for(int sample = 0; sample < FRAME_COUNT; sample++)
				{
					for(int channel = 0; channel < NUM_CHANNELS; channel++)
					{
						int coreAudioIndex = sample * NUM_CHANNELS + channel;
						int floatIndex = sample + FRAME_COUNT * channel;
						coreAudioBuffer[coreAudioIndex] = outputFloatBuffer[floatIndex] * maxVal;
					}
				}	
			}
		}
		
		aqc->inputInfo.readIndex++;
		if(aqc->inputInfo.readIndex == AUDIO_BUFFERS)
			aqc->inputInfo.readIndex = 0;
		
		AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
    }	
}



- (UGen) constructGraph: (UGen) input
{
	(void)input;
	return UGen::emptyChannels(NUM_CHANNELS);
}

- (void) cleanUp
{
	aqc->outputInfo.graph.release();
	while(aqc->outputInfo.graph.isNotNull())
	{
		sleep(200);
	}
	
	// do this input stuff conditionally on input being there!...
	AudioQueueStop(aqc->inputInfo.queue, true);
	AudioQueueStop(aqc->outputInfo.queue, true);
	
	AudioQueueDispose(aqc->inputInfo.queue, true);
	AudioQueueDispose(aqc->outputInfo.queue, true);
	
	AudioSessionSetActive(false);
	
	UGen::shutdown();
	
	delete aqc;
	delete deleter;
}

@end



BEGIN_UGEN_NAMESPACE

// C++ functions ?
 
END_UGEN_NAMESPACE

#undef SAMPLE_RATE
#undef NUM_CHANNELS
#undef FRAME_COUNT
#undef AUDIO_BUFFERS

#endif

#endif // gpl
