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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_UIButton.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif


//@interface UISliderUGenInternalPeer : NSObject
//{
//@public
//	UISliderUGenInternal* owner_;
//}
//
//- (void) setOwner: (UISliderUGenInternal*) owner;
//- (void) updateValue: (UISlider *) slider;
//@end
//
//
//@implementation UISliderUGenInternalPeer
//- (void) setOwner: (UISliderUGenInternal*) owner
//{
//	owner_ = owner;
//}
//
//- (void) updateValue: (UISlider *) slider
//{
//	owner_->setValue([slider value]);
//}
//@end


	
BEGIN_UGEN_NAMESPACE

//UISliderUGenInternal::UISliderUGenInternal(UISlider* slider) throw()
//:	FloatPtrUGenInternal(&value),
//	slider_(slider)
//{
//	UISliderUGenInternalPeer* peer = [UISliderUGenInternalPeer alloc];
//	[peer setOwner: this];
//	
//	[slider_ retain];
//	[slider_ addTarget:peer action:@selector(updateValue:) forControlEvents:UIControlEventValueChanged];
//	
//	value = [slider_ value];
//	
//	voidPeer = (void*)peer;
//}
//
//UISliderUGenInternal::~UISliderUGenInternal() throw()
//{
//	UISliderUGenInternalPeer* peer = (UISliderUGenInternalPeer*)voidPeer;
//	
//	[slider_ removeTarget:peer action:@selector(updateValue:) forControlEvents:UIControlEventValueChanged];
//	[slider_ release];
//	[peer dealloc];
//}
//
//void UISliderUGenInternal::setValue(float newValue) 
//{ 
//	value = newValue; 
//}
//
//UISliderUGen::UISliderUGen(UISlider* slider) throw()
//{
//	initInternal(1);	
//	internalUGens[0] = new UISliderUGenInternal(slider);
//}



END_UGEN_NAMESPACE

#endif
