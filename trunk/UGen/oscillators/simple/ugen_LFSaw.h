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

#ifndef _UGEN_ugen_LFSaw_H_
#define _UGEN_ugen_LFSaw_H_

#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class LFSawUGenInternal :  public UGenInternal//,
//						   public Seekable
{
public:
	LFSawUGenInternal(UGen const& freq, const float initialPhase) throw();
	UGenInternal* getChannel(const int channel) throw();									
	UGenInternal* getKr() throw();															
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();		
	
	enum Inputs { Freq, NumInputs };
	
protected:
	float currentPhase;	
};

/** @ingroup UGenInternals */
class LFSawUGenInternalK : public LFSawUGenInternal
{
public:
	LFSawUGenInternalK(UGen const& freq, const float initialPhase) throw();
	UGenInternal* getKr() throw() { incrementRefCount(); return this; }
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
private:
	float value;
};

/** Non bandlimted sawtooth.
 @ingroup AllUGens OscUGens */
DirectMulAddUGenDeclaration(LFSaw, 
							(freq, initialPhase),
							(freq, initialPhase, MulAdd_ArgsCall),
							(UGen const& freq = 440.f, Buffer const& initialPhase = 0.f),
							(UGen const& freq = 440.f, Buffer const& initialPhase = 0.f, MulAdd_ArgsDeclare), COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_LFSaw_H_
