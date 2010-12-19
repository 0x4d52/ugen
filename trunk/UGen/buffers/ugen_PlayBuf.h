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

#ifndef _UGEN_ugen_PlayBuf_H_
#define _UGEN_ugen_PlayBuf_H_


#include "../core/ugen_UGen.h"
#include "ugen_Buffer.h"

#ifdef Trig
#undef Trig
#endif

/** A UGenInternal which can playback a Buffer.
 
 This is a ProxyOwnerUGenInternal so creates a number of proxy outputs
 depending on the number of channels in the Buffer. All other inputs should be
 a single channel (and will be mixed to mono if they aren't before use).
 
 @see PlayBuf
 @ingroup UGenInternals */
class PlayBufUGenInternal :	public ProxyOwnerUGenInternal,
//							public Seekable,
							public DoneActionSender	
{
public:
	PlayBufUGenInternal(Buffer const& buffer, 
						UGen const& rate, 
						UGen const& trig, 
						UGen const& offset, 
						UGen const& loop, 
						const UGen::DoneAction doneAction) throw();
	UGenInternal* getChannel(const int channel) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();	
	
	enum Inputs { Rate, Trig, Offset, Loop, NumInputs };
	
protected:
	Buffer buffer_;
	double bufferPos;
	float lastTrig;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};

/** A UGen which can playback a Buffer.
 
 This should have a number of channels equal to that in the Buffer. 
 All other inputs should be a single channel (and will be mixed to mono 
 if they aren't before use).
 
 @ingroup AllUGens SoundFileUGens
 @see PlayBufUGenInternal */
UGenSublcassDeclaration(PlayBuf, (buffer, rate, trig, offset, loop, doneAction),
						(Buffer const& buffer, 
						 UGen const& rate = 1.f, 
						 UGen const& trig = 0.f, 
						 UGen const& offset = 0.f, 
						 UGen const& loop = 0.f,
						 const UGen::DoneAction doneAction = UGen::DeleteWhenDone), COMMON_UGEN_DOCS);



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


class RecordBufUGenInternal :	public ProxyOwnerUGenInternal,
//								public Seekable,
								public DoneActionSender	
{
public:
	RecordBufUGenInternal(UGen const& input,
						  Buffer const& buffer, 
						  UGen const& recLevel,
						  UGen const& preLevel,
						  UGen const& loop, 
						  const UGen::DoneAction doneAction) throw();
	UGenInternal* getChannel(const int channel) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();	
	
	enum Inputs { Input, RecLevel, PreLevel, Loop, NumInputs };
	
protected:
	Buffer buffer_;
	int bufferPos;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};

/** A UGen which writes to a buffer. 
 @ingroup AllUGens SoundFileUGens
 @see PlayBufUGen */
UGenSublcassDeclaration(RecordBuf, (input, buffer, recLevel, preLevel, loop, doneAction),
						(UGen const& input,
						 Buffer const& buffer, 
						 UGen const& recLevel = 1.f,
						 UGen const& preLevel = 0.f,
						 UGen const& loop = 0.f, 
						 const UGen::DoneAction doneAction = UGen::DeleteWhenDone), COMMON_UGEN_DOCS);

#if 1

class LoopPointsUGenInternal :	public UGenInternal//,
//								public Seekable
{
public:
	LoopPointsUGenInternal(Buffer const& buffer, 
						   UGen const& rate, 
						   UGen const& start, 
						   UGen const& end, 
						   UGen const& loop,
						   UGen const& startAtZero,
						   UGen const& playToEnd) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Rate, Start, End, Loop, StartAtZero, PlayToEnd, NumInputs };
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();		
	
private:
	Buffer b;
	float currentValue;
	bool lastLoop;
};

/** A UGen for controlling PlayBuf loop points via its offset input. 
 You need to provide the buffer, the rate of playback (1=normal) then the start
 and end loop points as a fraction 0.0-1.0 */
UGenSublcassDeclaration(LoopPoints, 
						(buffer, rate, start, end, loop, startAtZero, playToEnd), 
						(Buffer const& buffer, 
						 UGen const& rate, 
						 UGen const& start, 
						 UGen const& end, 
						 UGen const& loop = 1.f, 
						 UGen const& startAtZero = 0.f,
						 UGen const& playToEnd = 1.f), 
						COMMON_UGEN_DOCS);

#endif // 0

#endif // _UGEN_ugen_PlayBuf_H_
