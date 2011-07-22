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

#ifdef UGEN_IPHONE

BEGIN_UGEN_NAMESPACE

#include "../core/ugen_Constants.h"
#include "ugen_UIAcceleration.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif


@interface UIAccelerationSingletonPeer : NSObject<UIAccelerometerDelegate>
{
@public
	UIAccelerationSingleton* owner_;
}
- (void)setOwner: (UIAccelerationSingleton*) owner;
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;
@end


@implementation UIAccelerationSingletonPeer
- (void) setOwner: (UIAccelerationSingleton*) owner
{
	ugen_assert(owner != 0);
	owner_ = owner;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	owner_->set([acceleration x], [acceleration y], [acceleration z]);
}
@end

	
BEGIN_UGEN_NAMESPACE

UIAccelerationSingleton::UIAccelerationSingleton()
{
	UIAccelerationSingletonPeer* peer = [UIAccelerationSingletonPeer alloc];
	[peer setOwner:this];
	
	UIAccelerometer *accel = [UIAccelerometer sharedAccelerometer];
	
	[accel setDelegate:peer]; 
	
	//printf("UIAccelerometer update interval = %f\n", [accel updateInterval]);
	
	accel.updateInterval = 1024.0 / 44100.0;
	
	printf("UIAccelerometer update interval = %f\n", [accel updateInterval]);
	
	voidPeer = (void*)peer;
}

UIAccelerationSingleton::~UIAccelerationSingleton()
{
	UIAccelerationSingletonPeer* peer = (UIAccelerationSingletonPeer*)voidPeer;
	[peer dealloc];
}

UIAccelerationSingleton& UIAccelerationSingleton::getInstance()
{
	static UIAccelerationSingleton singleton;
	return singleton;
}

void UIAccelerationSingleton::set(double newX, double newY, double newZ) throw()
{
//	printf("accel: %f %f %f\n", newX, newY, newZ);
	
	double newDx = newX-x;
	double newDy = newY-y;
	double newDz = newZ-z;
	
	ddx = newDx-dx;
	ddy = newDy-dy;
	ddz = newDz-dz;
	
	dx = newDx;
	dy = newDy;
	dz = newDz;
	
	x = newX;
	y = newY;
	z = newZ;
}

double UIAccelerationSingleton::get(Directions direction) throw()
{
	switch(direction)
	{
		case AccelerationX: return x;
		case AccelerationY: return y;
		case AccelerationZ: return z;
		case DeltaX: return dx;
		case DeltaY: return dy;
		case DeltaZ: return dz;
		case DeltaDeltaX: return ddx;
		case DeltaDeltaY: return ddy;
		case DeltaDeltaZ: return ddz;
		default: return 0.0;
	}
}

double* UIAccelerationSingleton::getPtr(Directions direction) throw()
{
	switch(direction)
	{
		case AccelerationX: return &x;
		case AccelerationY: return &y;
		case AccelerationZ: return &z;
		case DeltaX: return &dx;
		case DeltaY: return &dy;
		case DeltaZ: return &dz;
		case DeltaDeltaX: return &ddx;
		case DeltaDeltaY: return &ddy;
		case DeltaDeltaZ: return &ddz;
		default: return (double*)(&zero); // not sure about this!
	}
}

UIAccelerationUGenInternal::UIAccelerationUGenInternal(UIAccelerationSingleton::Directions direction) throw()
:	DoublePtrUGenInternal(UIAccelerationSingleton::getInstance().getPtr(direction))
{	
}

Accel::Accel() throw()
{
	initInternal(3);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationX);
	internalUGens[1] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationY);
	internalUGens[2] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationZ);
}


AccelX::AccelX() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationX);
}

AccelY::AccelY() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationY);
}

AccelZ::AccelZ() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::AccelerationZ);
}

AccelDelta::AccelDelta() throw()
{
	initInternal(3);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaX);
	internalUGens[1] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaY);
	internalUGens[2] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaZ);
}

AccelDeltaX::AccelDeltaX() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaX);
}

AccelDeltaY::AccelDeltaY() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaY);
}

AccelDeltaZ::AccelDeltaZ() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaZ);
}

AccelDeltaDelta::AccelDeltaDelta() throw()
{
	initInternal(3);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaX);
	internalUGens[1] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaY);
	internalUGens[2] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaZ);
}

AccelDeltaDeltaX::AccelDeltaDeltaX() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaX);
}

AccelDeltaDeltaY::AccelDeltaDeltaY() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaY);
}

AccelDeltaDeltaZ::AccelDeltaDeltaZ() throw()
{
	initInternal(1);	
	internalUGens[0] = new UIAccelerationUGenInternal(UIAccelerationSingleton::DeltaDeltaZ);
}




END_UGEN_NAMESPACE

#endif
