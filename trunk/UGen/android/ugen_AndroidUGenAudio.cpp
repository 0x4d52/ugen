/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <android/log.h>
#include <jni.h>

// weird, - paths seem to need to be relative to ./jni
#include "../../../UGen/UGen.h"
#include "../../../UGen/android/ugen_UGenAndroid.h"

// macros to save having to use fully qulaified names in the code
// should be able to use  JNIEnv::RegisterNatives() in JNI_OnLoad()
// or perhaps Android requires the similar AndroidRuntime::registerNativeMethods()?
// anyway this works..

// macro for the class name
//#define FullyQualifiedClassName Java_uk_co_miajo_UGen_UGenAudio
//
//// define function names creating fully qualified ones..
//#define createIOHost				GenerateMangledName(FullyQualifiedClassName, createIOHost)
//#define destroyIOHost				GenerateMangledName(FullyQualifiedClassName, destroyIOHost)
//#define processIOHost				GenerateMangledName(FullyQualifiedClassName, processIOHost)
//#define processIOHostOutputOnly		GenerateMangledName(FullyQualifiedClassName, processIOHostOutputOnly)
//#define sendTrigger					GenerateMangledName(FullyQualifiedClassName, sendTrigger)
//#define setParameter				GenerateMangledName(FullyQualifiedClassName, setParameter)
//#define sendBytes					GenerateMangledName(FullyQualifiedClassName, sendBytes)


static AndroidIOHost* host = 0;
//static short* shortBuffer = 0;
//static int bufferLength = 0;

// this function must be defined in user code
AndroidIOHost* createHost(const double sampleRate, const int numInputs, const int numOutputs, const int preferredBufferSize);

extern "C" void JNICALL Java_uk_co_miajo_UGen_UGenAudio_createIOHost(JNIEnv *env, jobject thiz, jdouble sr, jint numInputs, jint numOutputs, jint blockSize)
{
	__android_log_print(ANDROID_LOG_DEBUG, "ugen", "createIOHost %f %d %d %d", sr, numInputs, numOutputs, blockSize);
	
	if(host == 0)
	{
		//host = new AndroidIOHost(sr, numInputs, numOutputs, blockSize);
		host = createHost(sr, numInputs, numOutputs, blockSize);
		
//		bufferLength = 1 * numOutputs * blockSize;
//		shortBuffer = new short[bufferLength];
		
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
	
//	delete [] shortBuffer;
//	shortBuffer = 0;
//	bufferLength = 0;
}

///** Gets called every audio block on the audio thread. */
//extern "C" jint JNICALL processIOHost(JNIEnv *env, jobject thiz, jshortArray shortBufferFromJava)
//{
//	//__android_log_print(ANDROID_LOG_DEBUG, "ugen", "processIOHost"); // probably not a good idea to leave this on unless in an emergency!
//	
//	env->GetShortArrayRegion(shortBufferFromJava, 0, bufferLength, shortBuffer);
//	int result = host->processBlock(bufferLength, shortBuffer);
//	env->SetShortArrayRegion(shortBufferFromJava, 0, bufferLength, shortBuffer);
//	
//	return result;
//}
//
//
//extern "C" jint JNICALL processIOHostOutputOnly(JNIEnv *env, jobject thiz, jshortArray shortBufferFromJava)
//{
//	//__android_log_print(ANDROID_LOG_DEBUG, "ugen", "processIOHostOutputOnly"); // probably not a good idea to leave this on unless in an emergency!
//	
//	int result = host->processBlockOutputOnly(bufferLength, shortBuffer);
//	env->SetShortArrayRegion(shortBufferFromJava, 0, bufferLength, shortBuffer);
//	
//	return result;
//}


/** Gets called every audio block on the audio thread. */
extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_processIOHost(JNIEnv *env, jobject thiz, jshortArray jShorts)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "ugen", "processIOHost"); // probably not a good idea to leave this on unless in an emergency!
	
	short *shortBuffer = env->GetShortArrayElements(jShorts, NULL);
	int result = host->processBlock(-1, shortBuffer);
	env->ReleaseShortArrayElements(jShorts, shortBuffer, 0);
	
	return result;
}


extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_processIOHostOutputOnly(JNIEnv *env, jobject thiz, jshortArray jShorts)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "ugen", "processIOHostOutputOnly"); // probably not a good idea to leave this on unless in an emergency!
	
	short *shortBuffer = env->GetShortArrayElements(jShorts, NULL);
	int result = host->processBlock(-1, shortBuffer);
	env->ReleaseShortArrayElements(jShorts, shortBuffer, 0);
	
	return result;
}


extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_sendTrigger(JNIEnv *env, jobject thiz, jint index)
{
	return host->sendTrigger(index);
}

extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_setParameter(JNIEnv *env, jobject thiz, jint index, jfloat value)
{
	return host->setParameter(index, value);
}

extern "C" jint JNICALL Java_uk_co_miajo_UGen_UGenAudio_sendBytes(JNIEnv *env, jobject thiz, jint index, jint size, jbyteArray jBytes)
{
//#undef sendBytes
//	unsigned char *bytes = new unsigned char[size];
//	env->GetByteArrayRegion(jBytes, 0, size, bytes);
//	int result = host->sendBytes(index, size, bytes);
//	delete [] bytes;
//	return result;
}




