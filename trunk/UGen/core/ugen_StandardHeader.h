// $Id$
// $HeadURL$

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

#ifndef UGEN_STANDARDHEADER_H
#define UGEN_STANDARDHEADER_H

// should make windows-only
#define _CRT_SECURE_NO_WARNINGS 1

#define UGEN_NAMESPACE ugen

#ifdef UGEN_NAMESPACE
#define BEGIN_UGEN_NAMESPACE    namespace UGEN_NAMESPACE {
#define END_UGEN_NAMESPACE      }
#else
#define BEGIN_UGEN_NAMESPACE 
#define END_UGEN_NAMESPACE   
#endif

/* if you're getting your first compile error here on iOS
 it's probably because the file your compiling which includes
 UGen++ headers is not a C++ file. To make an Obj-C file (.m) 
 an Obj-C++ file use .mm as the file extension. */
BEGIN_UGEN_NAMESPACE

/** To help raise compiler errors when function prototypes change.
 This can be used as the function return type in place of the original
 type to force the compiler to identify places in the source tree where the
 function (normally a virtual) changed in the base class but derived classes 
 are still using the old version (this may otherwise cause strange behaviour). */
struct ForceErrorStruct { int dummy; };
#define FORCE_COMPILER_ERROR_FUNCTION_PROTOTYPE_CHANGED ForceErrorStruct


//#define USEMEMSETCPY // bug in the floatptr stuff

#define U UGen
#define C EnvCurveList
#define B Buffer
#define LT Buffer
#define LL Buffer
#define Signal Buffer
#define I IntBuffer
#define IO IntBuffer
#define O IntBuffer


/** Enable/Disble LOCAL_DECLARE and LOCAL_COPY macros. */
#define UGEN_LOCAL_COPIES 1

#if UGEN_LOCAL_COPIES
	/** Declares a local variable which is a copy of a member variable. */
	#define LOCAL_DECLARE(Type, Name)		Type Name = this->Name	
	/** Copys the value from a local variable back to its member variable. */
	#define LOCAL_COPY(Name)				this->Name = Name
#else
	#define LOCAL_DECLARE(Type, Name)
	#define LOCAL_COPY(Name)
#endif

#ifdef UGEN_IO_CLIP
#define UGEN_IO_CLIPFUNC(x) ugen::clip2((x), 1.f)
#else
#define UGEN_IO_CLIPFUNC(x) x
#endif


END_UGEN_NAMESPACE

//#if (defined (_WIN32) || defined (_WIN64))
// #define       UGEN_WIN32 1
//#else
// #if defined (LINUX) || defined (__linux__)
//  #define     UGEN_LINUX 1
// #else
//  #define     UGEN_MAC 1
// #endif
//#endif

//#if !defined(WIN32) && !defined(UGEN_IPHONE)
//	#include <Accelerate/Accelerate.h>
//	#include <CoreServices/CoreServices.h>
//#endif


// release: define _NDEBUG=1 NDEBUG=1
// debug: define _DEBUG=1 DEBUG=1


#if !defined(NDEBUG) || defined(_DEBUG)
	#define UGEN_DEBUG 1
#endif



#ifdef UGEN_INTROJUCER
    #define UGEN_JUCE 1
#endif


// UGEN_JUCE should be defined in the target/project's preprocessor macros or
// replace "#ifdef UGEN_JUCE" with "#if 1" to force dependency on Juce
#ifdef UGEN_JUCE
	#if !defined(_NDEBUG) && !defined(NDEBUG) && !defined(_DEBUG) && !defined(DEBUG)
		#warning You should define appropriate preprocessor macros in Debug (_DEBUG=1 DEBUG=1) and Release (_NDEBUG=1 NDEBUG=1)
	#endif

	#ifdef UGEN_INTROJUCER
        #ifdef UGEN_ANDROID
            #include "../../../JuceLibraryCode/JuceHeader.h"
        #else
            #include "JuceHeader.h"
        #endif
	#else
		/* 
		 If you get an error here make sure the directory which contains 'juce' is
		 in the search path. In the Mac project this directory should be assigned
		 to JUCE_SOURCE in the Xcode -> Preferences.. -> Source Trees (just make
		 "JUCE_SOURCE" the setting name and display name).
		 */
		#include <juce/juce.h>
	#endif

    #if JUCE_MAJOR_VERSION < 2
        #define removeFirstMatchingValue removeValue
    #endif

#else

	#ifdef UGEN_ANDROID
		#include <stdlib.h>
		#include <limits.h>
		#include <math.h>
		#include <wchar.h>
		#include <typeinfo>
		#include <string.h>
		#include <stdio.h>
	#else
		#include <cstdlib>
		#include <cstdarg>
		#include <climits>
		#include <cmath>
		#include <cwchar>
		#include <stdexcept>
		#include <typeinfo>
		#include <cstring>
		#include <cstdio>
	#endif
	
	// in case some of these macros are left in the code..
	#ifndef T
		#define T(a)
	#endif
	#ifndef DBG_PRINTF
		#define DBG_PRINTF(a)
	#endif
#endif

#ifndef UGEN_ANDROID
	#include <iostream>
#endif

#if (defined (_WIN32) || defined (_WIN64))
	#define snprintf _snprintf
	#pragma warning(disable : 4244) // loss of precision
	#pragma warning(disable : 4702) // unreachable code
	#pragma warning(disable : 4355) // use of 'this' in base member init
	#pragma warning(disable : 4127) // conditional expression is constant - dumb MSVC wouldn't let me replace these with compile time ones!
	#pragma warning(disable : 4800) // bool int nonsense
#endif

#ifdef UGEN_IPHONE
	#ifdef __OBJC__
		#import <Foundation/Foundation.h>
		#import <UIKit/UIKit.h>

		#ifdef UGEN_IOS_COREMIDI
			#import <CoreMIDI/CoreMIDI.h>
			#import <CoreMIDI/MIDINetworkSession.h>
		#endif
	#else
        #ifndef UISlider
            class UISlider;
        #endif

        #ifndef UIButton
            class UIButton;
        #endif
    #endif

	//class UIScopeView;
	
	#include <sys/sysctl.h>
	#include <AudioToolbox/AudioQueue.h>
	#include <AudioToolbox/AudioServices.h>
	#include <AudioToolbox/AudioToolbox.h>
	#include <AudioUnit/AudioUnit.h>
	
	#include <CoreGraphics/CGColor.h>

	#include <TargetConditionals.h> 
	#if defined(UGEN_VFP) && (TARGET_IPHONE_SIMULATOR == 1)
		#undef UGEN_VFP
	#endif
	#if defined(UGEN_NEON) && (TARGET_IPHONE_SIMULATOR == 1)
		#undef UGEN_NEON
	#endif

	#define UGEN_ALIGN __attribute__ ((aligned))

#else
	// vfp only valid on iPhone / iPod touch - consider using iOS4 and UGEN_VDSP
	#undef UGEN_VFP
	#undef UGEN_NEON
#endif

#ifdef UGEN_VDSP // mac/iphone(4) only and must not be used in conjunction with UGEN_VFP or UGEN_NEON
#include <Accelerate/Accelerate.h>
#endif

#define UGEN_MAJOR_VERSION      0
#define UGEN_MINOR_VERSION      1
#define UGEN_BUILDNUMBER        7


#if defined (UGEN_DEBUG) && UGEN_DEBUG
	#if defined(UGEN_JUCE) && UGEN_JUCE && defined(juce_breakDebugger)
		#define ugen_breakDebugger				juce_breakDebugger
		#define ugen_isRunningUnderDebugger		juce_isRunningUnderDebugger
		#define ugen_breakRunningUnderDebugger	{ if (JUCE_NAMESPACE::juce_isRunningUnderDebugger()) juce_breakDebugger; }

	#elif defined(UGEN_IPHONE) && UGEN_IPHONE
		#define ugen_breakDebugger				kill (0, SIGTRAP);

		static bool ugen_isRunningUnderDebugger() throw()
		{
			static char testResult = 0;
			
			if (testResult == 0)
			{
				struct kinfo_proc info;
				int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
				size_t sz = sizeof (info);
				sysctl (m, 4, &info, &sz, 0, 0);
				testResult = ((info.kp_proc.p_flag & P_TRACED) != 0) ? 1 : -1;
			}
			
			return testResult > 0;
		}

		#define ugen_breakRunningUnderDebugger { if (ugen_isRunningUnderDebugger()) ugen_breakDebugger; }

	#else
		#define ugen_breakDebugger
		#define ugen_breakRunningUnderDebugger
	#endif

	#define ugen_assertfalse																			\
		{																								\
			printf ("UGen++ Assertion failure in %s, line %d\n", __FILE__, __LINE__);					\
			ugen_breakRunningUnderDebugger;																\
		}

	#define ugen_assert(expression)		{ if (! (expression)) ugen_assertfalse }
	#define ugen_debugprintf(args)		printf args

#else
	#define ugen_breakDebugger

	#define ugen_assertfalse			{ }
	#define ugen_assert(expression)		{ }
	#define ugen_debugprintf(args)
#endif



#ifndef INFINITY
union MSVC_EVIL_FLOAT_HACK
{
   unsigned __int8 Bytes[4];
   float Value;
};
static union MSVC_EVIL_FLOAT_HACK INFINITY_HACK = {{0x00, 0x00, 0x80, 0x7F}};
#define INFINITY (INFINITY_HACK.Value)
#endif

#define GenerateClassName0(a,b,c) a ## _ ## b ## _ ## c
#define GenerateClassName(a,b,c) GenerateClassName0(a,b,c)


#ifdef __VEC__
#include <Accelerate/Accelerate.h>
#define UGEN_ALIGN __attribute__ ((aligned))
typedef union 
{ 
	float	flt[4];
	vFloat	vec;
} VecSplat;
#endif // __VEC__


inline int quantiseUp(const int a, const int q) 
{ 
	if(a % q)
		return a / q * q + q;
	else
		return a / q * q;
}

inline int quantiseDown(const int a, const int q) 
{ 
	return a / q * q; 
}

#ifndef numElementsInArray
	#define numElementsInArray(a)   ((int) (sizeof (a) / sizeof ((a)[0])))
#endif

#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define BYTE_ORDER LITTLE_ENDIAN
// may need to add Big Endian support

// define ALIGN to do nothing if it's not defined...
#ifndef UGEN_ALIGN
#define UGEN_ALIGN
#endif

#endif // UGEN_STANDARDHEADER_H