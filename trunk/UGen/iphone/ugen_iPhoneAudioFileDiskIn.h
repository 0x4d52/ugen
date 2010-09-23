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

#ifndef _UGEN_ugen_iPhoneAudioFileDiskIn_H_
#define _UGEN_ugen_iPhoneAudioFileDiskIn_H_


#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class DiskInUGenInternal :	public ProxyOwnerUGenInternal,
							public DoneActionSender
{
public:
	DiskInUGenInternal(AudioFileID audioFile, 
					   AudioStreamBasicDescription const& format, 
					   const bool loopFlag = false, 
					   const double startTime = 0.0,
					   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	~DiskInUGenInternal() throw();
	OSStatus clearOutputsAndReadData(bool& shouldDelete) throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw();
	
protected:
	AudioFileID	audioFile_;
	const int numChannels_;
	const bool loopFlag_;
	const double startTime_;
	SInt64 packetCount;
	SInt64 currentPacket;
	int allocatedBlockSize;
	void *audioData;
	UInt32 numPackets;
	UInt32 bytesPerFrame;
	double fileSampleRate;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};

class DiskInUGenInternalWav16 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalWav16(AudioFileID audioFile, 
							AudioStreamBasicDescription const& format, 
							const bool loopFlag = false, 
							const double startTime = 0.0,
							const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalAiff16 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalAiff16(AudioFileID audioFile, 
							 AudioStreamBasicDescription const& format, 
							 const bool loopFlag = false, 
							 const double startTime = 0.0,
							 const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalWav24 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalWav24(AudioFileID audioFile, 
							AudioStreamBasicDescription const& format, 
							const bool loopFlag = false, 
							const double startTime = 0.0,
							const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalAiff24 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalAiff24(AudioFileID audioFile, 
							 AudioStreamBasicDescription const& format, 
							 const bool loopFlag = false, 
							 const double startTime = 0.0,
							 const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalWav32 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalWav32(AudioFileID audioFile, 
							AudioStreamBasicDescription const& format, 
							const bool loopFlag = false, 
							const double startTime = 0.0,
							const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalAiff32 : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalAiff32(AudioFileID audioFile, 
							 AudioStreamBasicDescription const& format, 
							 const bool loopFlag = false, 
							 const double startTime = 0.0,
							 const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalFloatBigEndian : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalFloatBigEndian(AudioFileID audioFile, 
									 AudioStreamBasicDescription const& format, 
									 const bool loopFlag = false, 
									 const double startTime = 0.0,
									 const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class DiskInUGenInternalFloatLittleEndian : public DiskInUGenInternal
{
public: 
	DiskInUGenInternalFloatLittleEndian(AudioFileID audioFile, 
										AudioStreamBasicDescription const& format, 
										const bool loopFlag = false, 
										const double startTime = 0.0,
										const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};



/** Streams a soundfile from disk.
 @ingroup AllUGens SoundFileUGens
 @see PlayBuf, DiskOut */
class DiskIn : public UGen 
{ 
public: 
	DiskIn () throw() : UGen() { } 
	DiskIn (Text const& path, 
			const bool loopFlag = false, 
			const double startTime = 0.0, 
			const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw(); 
		
	static inline UGen AR (Text const& path, 
						   const bool loopFlag = false, 
						   const double startTime = 0.0,
						   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw() 
	{ 
		return DiskIn (path, loopFlag, startTime, doneAction); 
	} 	
					
private:
	void initWithAudioFile(const char* audioFilePath, 
						   const bool loopFlag = false, 
						   const double startTime = 0.0,
						   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
};



#endif // _UGEN_ugen_iPhoneAudioFileDiskIn_H_
