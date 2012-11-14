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

#ifndef _UGEN_ugen_PlayBuf_H_
#define _UGEN_ugen_PlayBuf_H_


#include "../core/ugen_UGen.h"
#include "ugen_Buffer.h"

#ifdef Trig
#undef Trig
#endif

class MetaDataReceiver;
typedef ObjectArray<MetaDataReceiver*> MetaDataReceiverArray;

class MetaDataSender
{
public:
	MetaDataSender() throw();
	virtual ~MetaDataSender();
	
	void addMetaDataReceiver(MetaDataReceiver* const receiver) throw();
	void removeMetaDataReceiver(MetaDataReceiver* const receiver) throw();
	
	void sendMetaData(Buffer const& buffer, MetaData const& metaData, MetaData::Type type, int channel = -1, int index = -1);

private:
	MetaDataReceiverArray receivers;
};

/** These receive meta data from MetaDataSender objects during playback. 
 @param buffer		The buffer to which this meta data relates.
 @param metaData	The entire meta data being used (use type and index to determine further).
 @param type		The type of meta data.
 @param channel		The channel to which this refers (-1 for no particular channel).
 @param index		The index of the meta data where there is an array (e.g., CuePoint). */
class MetaDataReceiver
{
public:
	MetaDataReceiver() {}
	virtual ~MetaDataReceiver() {}
	virtual void handleMetaData(Buffer const& buffer, MetaData const& metaData, MetaData::Type type, int channel, int index) = 0;
};

/** A UGenInternal which can playback a Buffer.
 
 This is a ProxyOwnerUGenInternal so creates a number of proxy outputs
 depending on the number of channels in the Buffer. All other inputs should be
 a single channel (and will be mixed to mono if they aren't before use).
 
 @see PlayBuf
 @ingroup UGenInternals */
class PlayBufUGenInternal :	public ProxyOwnerUGenInternal,
							public DoneActionSender,
							public MetaDataSender
{
public:
	PlayBufUGenInternal(Buffer const& buffer, 
						UGen const& rate, 
						UGen const& trig, 
						UGen const& offset, 
						UGen const& loop, 
						const UGen::DoneAction doneAction,
						MetaData const& metaData) throw();
	~PlayBufUGenInternal();
	UGenInternal* getChannel(const int channel) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();	
	
	void checkMetaDataCuePoints(const double currentPosition, 
								const double previousPosition, 
								const int channel, 
								const int numCuePoints,
								const bool forwards) throw();
	
	enum Inputs { Rate, Trig, Offset, Loop, NumInputs };
	
protected:
	Buffer buffer_;
	double bufferPos;
	float lastTrig;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
	MetaData metaData;
	DoubleArray prevPosArray;
};

#define PlayBuf_Docs	@param buffer	The Buffer to play, this number of channels witll determin the					\
										the number of channels of this PlayBuf.											\
						@param rate		The rate of playback where 1 is normal speed. The may be set to 0 and			\
										the @c offset input can be modulated manually instead (e.g., using the			\
										LoopPoints UGen).																\
						@param trig		A trigger that will send the playbakc head back to the offset. Just				\
										use 0 for normal use.															\
						@param offset	A modulatable offset into the Buffer in samples. Cab be used to driver the		\
										Buffer playback manually instead of or in addition to the @c rate input			\
						@param loop		A loop flag to indicate the Buffer should loop (1) or just play one-shot (0).	\
						@param doneAction If looping is off and the done action is UGen::DeleteWhenDone then this		\
										  UGen will fire a delete action when playback reaches the end of the Buffer.	\
						@param metaData An optional collection of metaData associated with the Buffer			
	

/** A UGen which can playback a Buffer.
 
 This should have a number of channels equal to that in the Buffer. 
 All other inputs should be a single channel (and will be mixed to mono 
 if they aren't before use).
 
 @ingroup AllUGens SoundFileUGens
 @see PlayBufUGenInternal */
UGenSublcassDeclaration(PlayBuf, (buffer, rate, trig, offset, loop, doneAction, metaData),
						(Buffer const& buffer, 
						 UGen const& rate = UGen::get1(), 
						 UGen const& trig = UGen::get0(), 
						 UGen const& offset = UGen::get0(), 
						 UGen const& loop = UGen::get0(),
						 const UGen::DoneAction doneAction = UGen::DeleteWhenDone,
						 MetaData const& metaData = MetaData()), COMMON_UGEN_DOCS PlayBuf_Docs);



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


class LoopPointsUGenInternal :	public UGenInternal,
								public DoneActionSender,
								public MetaDataSender
{
public:
	LoopPointsUGenInternal(Buffer const& buffer, 
						   UGen const& rate, 
						   UGen const& start, 
						   UGen const& end, 
						   UGen const& loop,
						   UGen const& startAtZero,
						   UGen const& playToEnd,
						   const UGen::DoneAction doneAction,
						   MetaData const& metaData) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Rate, Start, End, Loop, StartAtZero, PlayToEnd, NumInputs };
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();		
	
	void checkMetaDataCuePoints(const float currentPosition, 
								const float previousPosition, 
								const int numCuePoints,
								const bool forwards) throw();	
	
private:
	Buffer b;
	float currentValue;
	bool lastLoop;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
	MetaData metaData;
	float prevValue;
};



/** A UGen for controlling PlayBuf loop points via its offset input. 
 You need to provide the buffer, the rate of playback (1=normal) then the start
 and end loop points as a fraction 0.0-1.0 */
UGenSublcassDeclaration(LoopPoints, 
						(buffer, rate, start, end, loop, startAtZero, playToEnd, doneAction, metaData), 
						(Buffer const& buffer, 
						 UGen const& rate, 
						 UGen const& start, 
						 UGen const& end, 
						 UGen const& loop = 1.f, 
						 UGen const& startAtZero = 0.f,
						 UGen const& playToEnd = 1.f,
						 const UGen::DoneAction doneAction = UGen::DoNothing,
						 MetaData const& metaData = MetaData()), 
						COMMON_UGEN_DOCS);



#endif // _UGEN_ugen_PlayBuf_H_
