// $Id: ugen_BasicPan.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/pan/ugen_BasicPan.h $

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

#ifndef UGEN_BASICPAN_H
#define UGEN_BASICPAN_H


#include "../core/ugen_UGen.h"
#include "../buffers/ugen_Buffer.h"

/** @ingroup UGenInternals */
class LinPan2UGenInternal : public ProxyOwnerUGenInternal
{
public:
	LinPan2UGenInternal(UGen const& input, UGen const& position, UGen const& level) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Position, Level, NumInputs };
	
protected:
};

#define Pan_Doc		@param input		The (mono) input source											\
					@param position		-1.0 is left, 0.0 is centre and 1.0 is right.					\
										This should be a single value or a single channel				\
										(multichannel UGen instances will be mixed to mono).			\
					@param level		The amplitude. This should be a single value or a				\
										single channel (multichannel UGen instances will be				\
										mixed to mono).

/** A linear panner UGen across two channels. 
 @ingroup AllUGens ControlUGens
 @see Pan2 */
UGenSublcassDeclaration(LinPan2, (input, position, level),
					    (UGen const& input, UGen const& position = 0.f, UGen const& level = 1.f), 
						COMMON_UGEN_DOCS Pan_Doc);

/** @ingroup UGenInternals */
class Pan2UGenInternal : public ProxyOwnerUGenInternal
{
public:
	Pan2UGenInternal(UGen const& input, UGen const& position, UGen const& level) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Position, Level, NumInputs };
	
	const Buffer& getTable() const throw() { return panTable; }
	
protected:
	Buffer panTable;
	float lastPos, last0, last1;
};


/** A constant power panner UGen across two channels. 
 @ingroup AllUGens ControlUGens
 @see LinPan2 */
UGenSublcassDeclaration(Pan2, (input, position, level),
					    (UGen const& input, UGen const& position = 0.f, UGen const& level = 1.f), 
						COMMON_UGEN_DOCS Pan_Doc);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define LinPan2 LinPan2()
#define Pan2 Pan2()
#endif

#endif // UGEN_BASICPAN_H