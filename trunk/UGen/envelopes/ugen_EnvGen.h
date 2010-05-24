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

#ifndef _UGEN_ugen_EnvGen_H_
#define _UGEN_ugen_EnvGen_H_

#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"
#include "ugen_Env.h"

/** @ingroup UGenInternals */
class EnvGenUGenInternal : public ReleasableUGenInternal
{
public:
	EnvGenUGenInternal(Env const& env, const UGen::DoneAction doneAction) throw();
	UGenInternal* getKr() throw();	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void release() throw();
	void steal() throw();
	
protected:
	Env env_;
	const UGen::DoneAction doneAction_; 
	int currentSegment;
	double currentValue;
	int stepsUntilTarget;
	double grow, a2, b1, y1, y2;
	EnvCurve currentCurve;
	const bool shouldDeleteValue;
	
	bool setSegment(const int segment, const double stepsPerSecond) throw();
	
};

/** @ingroup UGenInternals */
class EnvGenUGenInternalK : public EnvGenUGenInternal 
{ 
public: 
	EnvGenUGenInternalK (Env const& env, const UGen::DoneAction doneAction) throw();
	UGenInternal* getKr() throw() { incrementRefCount(); return this; } 
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw(); 
};

#define EnvGen_Docs		@param	env			The envelope specified by an Env object.						\
						@param	doneAction	The DoneAction to perform after the envelope has finished, the	\
											default is to delete the UGen graph.

/** Generate breakpoint envelopes from an Env specification. 
 @ingroup AllUGens EnvUGens
 */
DirectMulAddUGenDeclaration(EnvGen,		(env, doneAction),
										(env, doneAction, MulAdd_ArgsCall), 
										(Env const& env, const UGen::DoneAction doneAction = UGen::DeleteWhenDone),
										(Env const& env, const UGen::DoneAction doneAction = UGen::DeleteWhenDone, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS EnvGen_Docs MulAddArgs_Docs);


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define EnvGen EnvGen()
#endif


#endif // _UGEN_ugen_EnvGen_H_
