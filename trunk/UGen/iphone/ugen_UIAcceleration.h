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

#ifndef _UGEN_ugen_UIAcceleration_H_
#define _UGEN_ugen_UIAcceleration_H_

#include "../basics/ugen_ScalarUGens.h"
#include "../basics/ugen_MulAdd.h"

/** This listens to the global accelerometer.
 Accel... UGen classes use this as a value source. Because there can be only one
 Accelerometer delegate (?) if you change the delegate directly, all the Accel... 
 UGen classes will break!
 @todo	Make this a broadcaster which can broadcast to listeners as an alternative
		way of using the Accelerometer for purposes other than UGen control in the app. 
 @see AccelX, AccelY, AccelZ, AccelDeltaX, AccelDeltaY, AccelDeltaZ */
class UIAccelerationSingleton
{
public:
	~UIAccelerationSingleton();
	
	enum Directions
	{
		AccelerationX,
		AccelerationY,
		AccelerationZ,
		DeltaX,
		DeltaY,
		DeltaZ,
		DeltaDeltaX,
		DeltaDeltaY,
		DeltaDeltaZ
	};
	
	static UIAccelerationSingleton& getInstance();
	
	void set(double newX, double newY, double newZ) throw();
	double get(Directions direction) throw();
	double* getPtr(Directions direction) throw();
	
private:
	UIAccelerationSingleton();
	void* voidPeer;
	double x, y, z;
	double dx, dy, dz;
	double ddx, ddy, ddz;
};


/** @ingroup UGenInternals */
class UIAccelerationUGenInternal : public DoublePtrUGenInternal
{	
public:
	UIAccelerationUGenInternal(UIAccelerationSingleton::Directions direction) throw();
};

/** A UGen which takes its value source from a UIAccelerometer (iPhone) X, Y & Z directions.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(Accel, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) X direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelX, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) Y direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelY, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) Z direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelZ, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) delta X, Y & Z directions.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDelta, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) delta X direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaX, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) delta Y direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaY, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) delta Z direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaZ, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) change of delta X, Y & Z directions.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaDelta, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) change of delta X direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaDeltaX, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) change of delta Y direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaDeltaY, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);

/** A UGen which takes its value source from a UIAccelerometer (iPhone) change of delta Z direction.
 @ingroup ControlUGens */
DirectMulAddUGenDeclaration(AccelDeltaDeltaZ, (), (MulAdd_ArgsCall), (), (MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS MulAddArgs_Docs);




#endif // _UGEN_ugen_UIAcceleration_H_
