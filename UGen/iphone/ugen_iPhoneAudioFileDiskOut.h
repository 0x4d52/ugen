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

#ifndef _UGEN_ugen_iPhoneAudioFileDiskOut_H_
#define _UGEN_ugen_iPhoneAudioFileDiskOut_H_


#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class DiskOutUGenInternal : public ProxyOwnerUGenInternal
{
public:
	DiskOutUGenInternal(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	~DiskOutUGenInternal() throw();
	void checkBufferSize(const int requiredBufferSize) throw();
	void writeBuffer(const int bufferSize) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:	
	AudioFileID audioFile_;
	const int numChannels;
	SInt64 currentPacket;
	int allocatedBlockSize;
	UInt32 allocatedBlockSizeInBytes;
	void* audioData;
	
	UInt32 bytesPerFrame;
};

class DiskOutUGenInternalWav16 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalWav16(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};		

class DiskOutUGenInternalAiff16 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalAiff16(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};		

class DiskOutUGenInternalWav24 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalWav24(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};	

class DiskOutUGenInternalAiff24 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalAiff24(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};	

class DiskOutUGenInternalWav32 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalWav32(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};	

class DiskOutUGenInternalAiff32 : public DiskOutUGenInternal
{
public:
	DiskOutUGenInternalAiff32(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};	


/** Streams data from a UGen out to a disk file. 
 @ingroup AllUGens SoundFileUGens 
 @see DiskIn */
class DiskOut : public UGen 
{ 
public: 
	
	DiskOut () throw() : UGen() { } 
	DiskOut (Text const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24) throw(); 
		
	static inline UGen AR (Text const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (path, input, overwriteExisitingFile, bitDepth); 
	}
	
	inline UGen ar (Text const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (path, input, overwriteExisitingFile, bitDepth); 
	}
				
	inline UGen operator() (Text const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (path, input, overwriteExisitingFile, bitDepth); 
	}
	
	
private:
	bool initWithAudioFileWav16(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw(); 
	bool initWithAudioFileAiff16(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw();
	bool initWithAudioFileWav24(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw(); 
	bool initWithAudioFileAiff24(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw(); 
	bool initWithAudioFileWav32(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw(); 
	bool initWithAudioFileAiff32(const char *audioFilePath, UGen const& input, bool overwriteExisitingFile = false) throw(); 

};



#endif // _UGEN_ugen_iPhoneAudioFileDiskOut_H_
