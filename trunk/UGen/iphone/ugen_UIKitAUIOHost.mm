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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_UIKitAUIOHost.h"
#include "../basics/ugen_RawInputUGens.h"
#include "../basics/ugen_Plug.h"
#include "../core/ugen_Random.h"

#ifdef UGEN_VFP
	#include "armasm/ugen_vfp_clobbers.h"
	#include "armasm/ugen_vfp_macros.h"
#endif

#ifdef UGEN_NEON
	#include "armasm/ugen_neon_Utilities.h"
#endif


END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

#define NUM_CHANNELS 2
//#define DEFAULT_BUFFERSIZE 2048


static OSStatus	Render(void							*inRefCon, 
					   AudioUnitRenderActionFlags 	*ioActionFlags, 
					   const AudioTimeStamp 		*inTimeStamp, 
					   UInt32 						inBusNumber, 
					   UInt32 						inNumberFrames, 
					   AudioBufferList				*ioData)
{	
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init]; 
	UIKitAUIOHost *host = (UIKitAUIOHost *)inRefCon;
	OSStatus result =  [host renderCallback:inNumberFrames 
							withActionFlags:ioActionFlags 
								atTimeStamp:inTimeStamp
								withBuffers:ioData];
	[pool release]; 
	return result;
	
}

void PropertyListener(void *                  inClientData,
					  AudioSessionPropertyID  inID,
					  UInt32                  inDataSize,
					  const void *            inPropertyValue)
{
//	printf("Property changed!\n");
	
	UIKitAUIOHost *host = (UIKitAUIOHost *)inClientData;
	[host propertyCallback:inID 
				  withSize:inDataSize 
			  withProperty:inPropertyValue];;
}

void InterruptionListener(void *inClientData, UInt32 inInterruption)
{
//	printf("Session interrupted! --- %s ---\n", inInterruption == kAudioSessionBeginInterruption ? "Begin Interruption" : "End Interruption");
	
	UIKitAUIOHost *host = (UIKitAUIOHost *)inClientData;
	[host interruptionCallback:inInterruption];
}

@implementation UIKitAUIOHost


- (id)init
{
	if (self = [super init])
	{
		deleter = 0;
		nsLock = [[NSLock alloc] init];
		fadeInTime = 0.5;
		preferredBufferSize = 1024;
		hwSampleRate = 0.0; // let the hardware choose
		cpuUsage = 0.0;
	}
	return self;
}

- (void)setFormat
{
	memset(&format, 0, sizeof(AudioStreamBasicDescription));
	format.mSampleRate = hwSampleRate;
	format.mFormatID = kAudioFormatLinearPCM;
	int sampleSize = sizeof(AudioSampleType);
	format.mFormatFlags = kAudioFormatFlagsCanonical;
	format.mBitsPerChannel = 8 * sampleSize;
	format.mChannelsPerFrame = NUM_CHANNELS;
	format.mFramesPerPacket = 1;
	format.mBytesPerPacket = format.mBytesPerFrame = sampleSize;
	format.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;		
}

- (int)setupRemoteIO
{
//	if(rioUnit) AudioComponentInstanceDispose(rioUnit);
	
	// Open the output unit
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	AudioComponent comp = AudioComponentFindNext(NULL, &desc);
	AudioComponentInstanceNew(comp, &rioUnit);
	
	const UInt32 one = 1;
	AudioUnitSetProperty(rioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof(one));	
	AudioUnitSetProperty(rioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof(inputProc));
	
	AudioUnitSetProperty(rioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format));
	AudioUnitSetProperty(rioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof(format));
	
//	Float64 sr = 22050.0;
//	AudioUnitSetProperty(rioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Global, 0, &sr, sizeof(Float64));
	
	AudioUnitInitialize(rioUnit);
	
	return 0;	
}

#pragma mark float<->short conversion routines

#ifdef UGEN_VFP
static inline void audioFloatToShort(float *src, short* dst, unsigned int length)
{
	static const float scale = 32767.f;
	int temp[16] UGEN_ALIGN; 
	int *tempa = temp;
	int *tempb = temp+8;
	
	unsigned int numVectors = length >> 4U;
	unsigned int numScalars = length & 15U;
	
	VFP_FIXED_16_VECTOR_MUL_SCALAR_FTI_LOAD(&scale);
	
	while(numVectors--)
	{
		VFP_FIXED_16_VECTOR_MUL_SCALAR_FTI_SAVE(src, tempa, tempb);
		
		dst[ 0] = (short)temp[ 0];
		dst[ 1] = (short)temp[ 1];
		dst[ 2] = (short)temp[ 2];
		dst[ 3] = (short)temp[ 3];
		dst[ 4] = (short)temp[ 4];
		dst[ 5] = (short)temp[ 5];
		dst[ 6] = (short)temp[ 6];
		dst[ 7] = (short)temp[ 7];
		dst[ 8] = (short)temp[ 8];
		dst[ 9] = (short)temp[ 9];
		dst[10] = (short)temp[10];
		dst[11] = (short)temp[11];
		dst[12] = (short)temp[12];
		dst[13] = (short)temp[13];
		dst[14] = (short)temp[14];
		dst[15] = (short)temp[15];
		dst += 16;
	}
	
	while(numScalars--)
	{
		*dst++ = (short)(*src++ * scale);
	}	
}

static inline void audioFloatToShortChannels(float *src[], AudioBufferList* dst, unsigned int length, unsigned int numChannels)
{
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(7);
	
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)dst->mBuffers[channel].mData;		
		audioFloatToShort(src[channel], audioUnitBuffer, length);
	}
	
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
}

static inline void audioShortToFloat(short *src, float* dst, unsigned int length)
{
	static const float scale = 1.f / 32767.f;
	int temp[16] UGEN_ALIGN; 
	int *tempa = temp;
	int *tempb = tempa+8;
	
	unsigned int numVectors = length >> 4U;
	unsigned int numScalars = length & 15U;
	
	VFP_FIXED_16_VECTOR_ITF_MUL_SCALAR_LOAD(&scale);
	
	while(numVectors--)
	{
		temp[ 0] = (int)src[ 0];
		temp[ 1] = (int)src[ 1];
		temp[ 2] = (int)src[ 2];
		temp[ 3] = (int)src[ 3];
		temp[ 4] = (int)src[ 4];
		temp[ 5] = (int)src[ 5];
		temp[ 6] = (int)src[ 6];
		temp[ 7] = (int)src[ 7];
		temp[ 8] = (int)src[ 8];
		temp[ 9] = (int)src[ 9];
		temp[10] = (int)src[10];
		temp[11] = (int)src[11];
		temp[12] = (int)src[12];
		temp[13] = (int)src[13];
		temp[14] = (int)src[14];
		temp[15] = (int)src[15];
		src += 16;
		
		VFP_FIXED_16_VECTOR_ITF_MUL_SCALAR_SAVE(tempa, tempb, dst);
	}
	
	while(numScalars--)
	{
		*dst++ = (float)(*src++) * scale;
	}	
}

static inline void audioShortToFloatChannels(AudioBufferList* src, float* dst[], unsigned int length, unsigned int numChannels)
{
	VFP_SWITCH_TO_ARM_ASM;
	VFP_VECTOR_LENGTH_ASM(7);

	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)src->mBuffers[0].mData; // need this other than 0?...		
		audioShortToFloat(audioUnitBuffer, dst[channel], length);
	}	
	
	VFP_VECTOR_LENGTH_ZERO_ASM;
	VFP_SWITCH_TO_THUMB_ASM;
}

#elif defined(UGEN_NEON)

static inline void audioFloatToShort(float *src, short* dst, unsigned int length)
{
	static const float scale = 32767.f;
	static const vFloat scaleVec = vdupq_n_f32(scale);
	vInt tempVec;
	int *temp = (int*)&tempVec;
	
	unsigned int numVectors = length >> 2U;
	unsigned int numScalars = length & 3U;
	
	vFloat *srcVec = (vFloat*)src;
	
	while(numVectors--)
	{		
		tempVec = vcvtq_s32_f32(vmulq_f32(*srcVec++, scaleVec));
		
		dst[ 0] = (short)temp[ 0];
		dst[ 1] = (short)temp[ 1];
		dst[ 2] = (short)temp[ 2];
		dst[ 3] = (short)temp[ 3];
		dst += 4;
	}
	
	src = (float*)srcVec;
	
	while(numScalars--)
	{
		*dst++ = (short)(*src++ * scale);
	}	
}

static inline void audioFloatToShortChannels(float *src[], AudioBufferList* dst, unsigned int length, unsigned int numChannels)
{	
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)dst->mBuffers[channel].mData;		
		audioFloatToShort(src[channel], audioUnitBuffer, length);
	}
}

static inline void audioShortToFloat(short *src, float* dst, unsigned int length)
{
	static const float scale = 1.f / 32767.f;
	static const vFloat scaleVec = vdupq_n_f32(scale);
	vInt tempVec;
	int *temp = (int*)&tempVec;

	unsigned int numVectors = length >> 2U;
	unsigned int numScalars = length & 3U;
	
	vFloat *dstVec = (vFloat*)dst;
		
	while(numVectors--)
	{
		temp[ 0] = (int)src[ 0];
		temp[ 1] = (int)src[ 1];
		temp[ 2] = (int)src[ 2];
		temp[ 3] = (int)src[ 3];
		src += 4;
		
		*dstVec++ = vmulq_f32(vcvtq_f32_s32(tempVec), scaleVec);
	}
	
	dst = (float*)dstVec;
	
	while(numScalars--)
	{
		*dst++ = (float)(*src++) * scale;
	}	
}

static inline void audioShortToFloatChannels(AudioBufferList* src, float* dst[], unsigned int length, unsigned int numChannels)
{
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)src->mBuffers[0].mData; // need this other than 0?...		
		audioShortToFloat(audioUnitBuffer, dst[channel], length);
	}	
}

#elif defined(UGEN_VDSP) 

static inline void audioFloatToShort(float *src, short* dst, unsigned int length)
{
	static float scale = 32767.f;
	static float zero = 0.f;
	vDSP_vsmsa(src, 1, &scale, &zero, src, 1, length);	
	vDSP_vfix16(src, 1, dst, 1, length);
}

static inline void audioFloatToShortChannels(float *src[], AudioBufferList* dst, unsigned int length, unsigned int numChannels)
{
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)dst->mBuffers[channel].mData;		
		audioFloatToShort(src[channel], audioUnitBuffer, length);
	}
}

static inline void audioShortToFloat(short *src, float* dst, unsigned int length)
{
	static float scale = 1.f / 32767.f;
	static float zero = 0.f;
	vDSP_vflt16(src, 1, dst, 1, length);
	vDSP_vsmsa(dst, 1, &scale, &zero, dst, 1, length);
}

static inline void audioShortToFloatChannels(AudioBufferList* src, float* dst[], unsigned int length, unsigned int numChannels)
{
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)src->mBuffers[0].mData; // need this other than 0?...		
		audioShortToFloat(audioUnitBuffer, dst[channel], length);
	}	
}

#else // normal code...

static inline void audioFloatToShort(float *src, short* dst, unsigned int length)
{
	static const float scale = 32767.f;
	
	while(length--)
	{
		*dst++ = (short)(*src++ * scale);
	}		
}

static inline void audioFloatToShortChannels(float *src[], AudioBufferList* dst, unsigned int length, unsigned int numChannels)
{
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)dst->mBuffers[channel].mData;		
		audioFloatToShort(src[channel], audioUnitBuffer, length);
	}
}

static inline void audioShortToFloat(short *src, float* dst, unsigned int length)
{
	static const float scale = 1.f / 32767.f;
	
	while(length--)
	{
		*dst++ = (float)(*src++) * scale;
	}
}

static inline void audioShortToFloatChannels(AudioBufferList* src, float* dst[], unsigned int length, unsigned int numChannels)
{
	for (UInt32 channel = 0; channel < numChannels; channel++)
	{
		AudioSampleType *audioUnitBuffer = (AudioSampleType*)src->mBuffers[0].mData; // need this other than 0?...		
		audioShortToFloat(audioUnitBuffer, dst[channel], length);
	}	
}
#endif

#pragma mark callbacks

- (OSStatus)renderCallback:(UInt32)inNumberFrames 
		   withActionFlags:(AudioUnitRenderActionFlags*)ioActionFlags
			   atTimeStamp:(const AudioTimeStamp*)inTimeStamp 
			   withBuffers:(AudioBufferList*)ioData
{
	OSStatus err = 0;
	
	double renderTime = [[NSDate date] timeIntervalSince1970];
	
	if(inNumberFrames > bufferSize)
	{
		delete [] floatBuffer;
		bufferSize = inNumberFrames;
		
		floatBuffer = new float[inNumberFrames * NUM_CHANNELS];
	}
	
	long blockID = UGen::getNextBlockID(inNumberFrames);
	
	float *floatBufferData[2];
	floatBufferData[0] = floatBuffer;
	floatBufferData[1] = floatBufferData[0] + inNumberFrames;	
		
	if(audioInputIsAvailable)
	{
		err = AudioUnitRender(rioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);
		if (err) { printf("renderCallback: error %d\n", (int)err); return err; }
		
		audioShortToFloatChannels(ioData, floatBufferData, inNumberFrames, numInputChannels);
				
		[self lock];
		rawInput.getSource().setInputs((const float**)floatBufferData, inNumberFrames, numInputChannels);
	}
	else
	{
		memset(floatBuffer, 0, numInputChannels * inNumberFrames * sizeof(float));
		[self lock];
	}
	
	postFadeOutput.setOutputs(floatBufferData, inNumberFrames, 2);
	
	const int numOthers = others.size();
	for(int i = 0; i < numOthers; i++)
	{
		others[i].prepareAndProcessBlock(inNumberFrames, blockID, -1);
	}		
	
	preFadeOutput.prepareAndProcessBlock(inNumberFrames, blockID, -1);
	postFadeOutput.prepareAndProcessBlock(inNumberFrames, blockID, -1);
	
	[self unlock];
	
	audioFloatToShortChannels(floatBufferData, ioData, inNumberFrames, ioData->mNumberBuffers);
			
	renderTime = [[NSDate date] timeIntervalSince1970] - renderTime;
	
	const float timeRatio = renderTime * reciprocalBufferDuration;
	cpuUsage += 0.2f * (timeRatio - cpuUsage); 
	
	return err;	
}	

- (void)propertyCallback:(AudioSessionPropertyID)inID
				withSize:(UInt32)inDataSize	
			withProperty:(const void *)inPropertyValue
{	
	if(!isRunning) return;
	
	if(inPropertyValue)
	{
		CFDictionaryRef routeChangeDictionary  = (CFDictionaryRef)inPropertyValue;
		CFNumberRef     routeChangeReasonRef   = 
		(CFNumberRef)CFDictionaryGetValue (routeChangeDictionary, 
										   CFSTR (kAudioSession_AudioRouteChangeKey_Reason));
		
		SInt32 routeChangeReason;
		CFNumberGetValue(routeChangeReasonRef, kCFNumberSInt32Type, &routeChangeReason);
		
		CFStringRef newAudioRoute;
		UInt32 propertySize = sizeof (CFStringRef);
		AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &propertySize, &newAudioRoute);
		
		printf("route=%s\n", CFStringGetCStringPtr(newAudioRoute, CFStringGetSystemEncoding()));
		
//		if (routeChangeReason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
//			[self fixAudioRouteIfSetToReceiver];
	}
	
	UInt32 size = sizeof(UInt32);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareInputNumberChannels, &size, &numInputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareOutputNumberChannels, &size, &numOutputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &size, &audioInputIsAvailable);
	
	printf("inputs=%d outputs=%d audioInputIsAvailable=%d\n", (int)numInputChannels, (int)numOutputChannels, (int)audioInputIsAvailable);
	
	if(rioUnit)	AudioComponentInstanceDispose(rioUnit);
	
	rioUnit = NULL;
	
	[self setFormat];
	[self setupRemoteIO];
	
	rawInput.setSource(AudioIn::AR(numInputChannels), true);
	postFadeOutput = Plug::AR(UGen::emptyChannels(numOutputChannels), false);
	postFadeOutput.fadeSourceAndRelease(preFadeOutput, fadeInTime);
	
	AudioSessionSetActive(true);
	AudioOutputUnitStart(rioUnit);
	
}

- (void)fixAudioRouteIfSetToReceiver
{
	CFStringRef audioRoute = 0;
	UInt32 propertySize = sizeof (audioRoute);
	if (AudioSessionGetProperty (kAudioSessionProperty_AudioRoute, &propertySize, &audioRoute) == noErr)
	{
		NSString* route = (NSString*) audioRoute;
				
		if ([route hasPrefix: @"Receiver"])
		{
			UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
			AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute, sizeof (audioRouteOverride), &audioRouteOverride);
		}
		
		CFRelease (audioRoute);
	}
}

- (void)interruptionCallback:(UInt32)inInterruption
{
	if (inInterruption == kAudioSessionEndInterruption) {
		// make sure we are again the active session
		//AudioSessionSetActive(false);
		AudioSessionSetActive(true);
		isRunning = true;
		AudioOutputUnitStart(rioUnit);
	}
	
	
	// may be needed for earlier than OS3 on iPod touch?
//	if (inInterruption == kAudioSessionBeginInterruption) {
//		isRunning = false;
//		AudioOutputUnitStop(rioUnit);
//		
//		printf("InterruptionListener audioInputIsAvailable=%d\n", audioInputIsAvailable);
//		
//		UIAlertView *baseAlert = [[UIAlertView alloc] initWithTitle:@"Audio interrupted" 
//															message:@"This could have been interrupted by another application or due to unplugging a headset:" 
//														   delegate:self
//												  cancelButtonTitle:nil
//												  otherButtonTitles:@"Resume", @"Cancel", nil];
//		[baseAlert show];
//    }	
}

#pragma mark other functions

- (id)initAudio
{	
	//[self lock];
	
	// render proc
	inputProc.inputProc = Render;
	inputProc.inputProcRefCon = self;
	
	UInt32 size = sizeof(hwSampleRate);
//	printf("UIKitAUIOHost: SR=%f\n", hwSampleRate);
	
	// session
	AudioSessionInitialize(NULL, NULL, InterruptionListener, self);
	AudioSessionSetActive(true);
	
	UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(audioCategory), &audioCategory);
	AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, PropertyListener, self);
	
	[self fixAudioRouteIfSetToReceiver];
	
	if(hwSampleRate > 0.0)
	{
		AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareSampleRate, size, &hwSampleRate);
	}		
//	else
//	{
		AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareSampleRate, &size, &hwSampleRate);
//	}
	
	bufferDuration = preferredBufferSize / hwSampleRate;
	AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(bufferDuration), &bufferDuration);
	
	size = sizeof(UInt32);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareInputNumberChannels, &size, &numInputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareOutputNumberChannels, &size, &numOutputChannels);
	AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &size, &audioInputIsAvailable);
			
	UGen::initialise();
	deleter = new NSDeleter();
	UGen::setDeleter(deleter);
	
#ifndef UGEN_NOEXTGPL
	Ran088::defaultGenerator().setSeed((int)[[NSDate date] timeIntervalSince1970]);
#endif
	
	UGen::prepareToPlay(hwSampleRate, preferredBufferSize);
	
	rawInput = Plug::AR(UGen::emptyChannels(NUM_CHANNELS));
	preFadeOutput = [self constructGraph: rawInput];
	
	rioUnit = NULL;
	isRunning = true;
	[self propertyCallback:0 
				  withSize:0 
			  withProperty:0]; // call directly this first time
	
	size = sizeof(format);
	AudioUnitGetProperty(rioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, &size);
	
	size = sizeof(bufferDuration);
	AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &bufferDuration);
	
	reciprocalBufferDuration = 1.f / bufferDuration; 
	
	bufferSize = (int)(hwSampleRate*bufferDuration+0.5);
	floatBuffer = new float[bufferSize * NUM_CHANNELS];
	
#if defined(UGEN_VFP)
	printf("UIKitAUIOHost: using vector floating point (vfp) ops in places					\n");
	printf("               make sure this is only used for Release builds on the device,	\n");
	printf("               and set optimisations to fastest/smallest (-Os)					\n");
	printf("               do not use:  -ftree-vectorize									\n");
	printf("                            -ffast-math											\n");
	printf("               				 Thumb Mode											\n");
#elif defined(UGEN_NEON)
	printf("UIKitAUIOHost: using arm neon instuctions for float ops in places				\n");
#else
	printf("UIKitAUIOHost: using standard floating point ops (non-vfp) \n");
#endif
	
	printf("UIKitAUIOHost: SR=%f buffer=%fs\n", hwSampleRate, bufferDuration);

	//[self unlock];
	
	return self;
}

- (void)setPreferredBufferSize:(int)size
{
	ugen_assert(size > 0);
	preferredBufferSize = size;
}

- (void)setPreferredSampleRate:(double)newSampleRate
{
	ugen_assert(newSampleRate > 0.0);
	hwSampleRate = newSampleRate;
}

- (UGen)constructGraph:(UGen)input
{
	return UGen::emptyChannels(NUM_CHANNELS);	
}

- (void)addOther:(UGen)ugen
{
	[self lock];
	others <<= ugen;
	[self unlock];
}

- (float)getCpuUsage
{
	return cpuUsage;
}

- (void)lock
{
	[nsLock lock];
}

- (void)unlock
{
	[nsLock unlock];
}

- (BOOL)tryLock
{
	return [nsLock tryLock];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	printf("buttonIndex=%d\n", buttonIndex);
	
	if(buttonIndex == 0)
	{
		// resume
		isRunning = true;
		[self propertyCallback:0 
					  withSize:0 
				  withProperty:0]; // call directly to reinitialise
	}
	
	[alertView release];
}

// no longer an app delegate, need to call shutdown from the app delegate manually
//- (void)applicationWillTerminate:(UIApplication *)application 
//{ 
//	[self shutdown]; 
//} 

- (void)isAboutToShutdown
{
	// override this to do something just before shutdown
}

- (void)shutdown
{
	[self isAboutToShutdown];
	
	// fade out before shutdown
	const double fadeTime = 0.5;
	const double pause = 0.25;
	postFadeOutput.fadeSourceAndRelease(UGen::emptyChannels(NUM_CHANNELS), fadeTime);
	[NSThread sleepForTimeInterval: fadeTime+pause];
	
	// shutdown when the fade has finished (hopefully!)
	UGen::shutdown();
	delete deleter;
	
	others			= UGen::getNull();
	rawInput		= UGen::getNull();
	postFadeOutput	= UGen::getNull();
	preFadeOutput	= UGen::getNull();
}


-(void) dealloc
{
	delete [] floatBuffer;
	[nsLock release];
	[super dealloc];
}
@end

BEGIN_UGEN_NAMESPACE

// C++ functions ?

END_UGEN_NAMESPACE

#endif
