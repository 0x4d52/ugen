/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#ifdef UGEN_ANDROID

#ifndef UGEN_JUCE

#include <string.h>
#include <android/log.h>
#include <jni.h>

#include "../UGen.h"
#include "../android/ugen_UGenAndroid.h"

// should be able to use JNIEnv::RegisterNatives() in JNI_OnLoad()
// or perhaps Android requires the similar AndroidRuntime::registerNativeMethods()?

static AndroidIOHost* host = 0;

// this function must be defined in user code
AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize);

extern "C" void JNICALL Java_uk_co_miajo_UGen_UGenAudio_createIOHost(JNIEnv *env, jobject thiz, jdouble sr, jint numInputs, jint numOutputs, jint blockSize)
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "createIOHost %f %d %d %d", sr, numInputs, numOutputs, blockSize);
	
	if(host == 0)
	{
		host = createHost(sr, numInputs, numOutputs, blockSize);
				
		host->init();
		
		__android_log_print(ANDROID_LOG_DEBUG, "ugen", "host->init()");

	}
	else
	{
		__android_log_print(ANDROID_LOG_DEBUG, "ugen", "createIOHost failed, AndroidIOHost already created");
	}
}

extern "C" void JNICALL Java_uk_co_miajo_UGen_UGenAudio_destroyIOHost(JNIEnv *env, jobject thiz)
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "destroyIOHost");

	delete host;
	host = 0;
}

/** Gets called every audio block on the audio thread. */
extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_processIOHost(JNIEnv *env, jobject thiz, jshortArray jShorts)
{	
	short *shortBuffer = env->GetShortArrayElements(jShorts, NULL);
	int result = host->processBlock(-1, shortBuffer);
	env->ReleaseShortArrayElements(jShorts, shortBuffer, 0);
	return result;
}

// no need for this now.. Java_uk_co_miajo_UGen_UGenAudio_processIOHost is the same
extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_processIOHostOutputOnly(JNIEnv *env, jobject thiz, jshortArray jShorts)
{	
	short *shortBuffer = env->GetShortArrayElements(jShorts, NULL);
	int result = host->processBlock(-1, shortBuffer);
	env->ReleaseShortArrayElements(jShorts, shortBuffer, 0);
	return result;
}

extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_sendTrigger(JNIEnv *env, jobject thiz, jint index)
{
	int result;
	result = host->sendTrigger(index);
	return result;
}

extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_setParameter(JNIEnv *env, jobject thiz, jint index, jfloat value)
{
	int result;
	result = host->setParameter(index, value);
	return result;
}

extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_sendBytes(JNIEnv *env, jobject thiz, jint index, jint size, jbyteArray jBytes)
{
	if(size < 1) return -1;
	
	char *bytes = new char[size];
	env->GetByteArrayRegion(jBytes, 0, size, (jbyte*)bytes);
	int result = host->sendBytes(index, size, bytes);
	delete [] bytes;
	return result;
}

#endif

#endif // UGEN_ANDROID


