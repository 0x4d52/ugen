// $Id: ugen_PlayBuf.h 993 2010-03-03 21:47:50Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/buffers/ugen_PlayBuf.h $

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

#ifndef _UGEN_ugen_PlayBuf_H_
#define _UGEN_ugen_PlayBuf_H_


#include "../core/ugen_UGen.h"
#include "ugen_Buffer.h"

/** A UGenInternal which can playback a Buffer.
 
 This is a ProxyOwnerUGenInternal so creates a number of proxy outputs
 depending on the number of channels in the Buffer. All other inputs should be
 a single channel (and will be mixed to mono if they aren't before use).
 
 @see PlayBuf
 @ingroup UGenInternals */
class PlayBufUGenInternal : public ProxyOwnerUGenInternal
{
public:
	PlayBufUGenInternal(Buffer const& buffer, UGen const& rate, UGen const& trig, UGen const& offset, UGen const& loop) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Rate, Trig, Offset, Loop, NumInputs };
	
protected:
	Buffer buffer_;
	double bufferPos;
	float lastTrig;
};

/** A UGen which can playback a Buffer.
 
 This should have a number of channels equalt to that in the Buffer. 
 All other inputs should be a single channel (and will be mixed to mono 
 if they aren't before use).
 
 @ingroup AllUGens SoundFileUGens
 @see PlayBufUGenInternal */
UGenSublcassDeclaration(PlayBuf, (buffer, rate, trig, offset, loop),
						(Buffer const& buffer, 
						 UGen const& rate = 1.f, 
						 UGen const& trig = 0.f, 
						 UGen const& offset = 0.f, 
						 UGen const& loop = 0.f), COMMON_UGEN_DOCS);



class BufferValuesUGenInternal :	public ProxyOwnerUGenInternal,
									public BufferReceiver
{
public:
	BufferValuesUGenInternal(Buffer const& buffer);
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void handleBuffer(Buffer const& buffer, const double value1, const int value2) throw();
	
	enum Inputs { NumInputs };
	
protected:
	Buffer buffer;
};

UGenSublcassDeclaration(BufferValues, (buffer), (Buffer const& buffer), COMMON_UGEN_DOCS);


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define PlayBuf PlayBuf()
#endif


#endif // _UGEN_ugen_PlayBuf_H_
