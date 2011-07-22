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
 
 CoreMIDI iOS code originally by Pete Goodliffe.
 Adapted from http://cocoawithlove.com/2010/07/tips-tricks-for-conditional-ios3-ios32.html

 ==============================================================================
 */

#ifndef _UGEN_ugen_iOSVersionDetection_H_
#define _UGEN_ugen_iOSVersionDetection_H_


#ifndef kCFCoreFoundationVersionNumber_iPhoneOS_4_0
#define kCFCoreFoundationVersionNumber_iPhoneOS_4_0 550.32
#endif

#ifndef kCFCoreFoundationVersionNumber_iPhoneOS_4_1
#define kCFCoreFoundationVersionNumber_iPhoneOS_4_1 550.38
#endif

#ifndef kCFCoreFoundationVersionNumber_iPhoneOS_4_2
// NOTE: THIS IS NOT THE FINAL NUMBER
// 4.2 is not out of beta yet, so took the beta 1 build number
#define kCFCoreFoundationVersionNumber_iPhoneOS_4_2 550.47
#endif

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 40000
#define IF_IOS4_OR_GREATER(...) \
    if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber_iPhoneOS_4_0) \
    { \
        __VA_ARGS__ \
    }
#else
#define IF_IOS4_OR_GREATER(...) \
    if (false) \
    { \
    }
#endif

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 40200
#define IF_IOS4_2_OR_GREATER(...) \
    if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber_iPhoneOS_4_2) \
    { \
        __VA_ARGS__ \
    }
#else
#define IF_IOS4_2_OR_GREATER(...) \
    if (false) \
    { \
    }
#endif

#define IF_IOS_HAS_COREMIDI(...) IF_IOS4_2_OR_GREATER(__VA_ARGS__)

#endif // _UGEN_ugen_iOSVersionDetection_H_

