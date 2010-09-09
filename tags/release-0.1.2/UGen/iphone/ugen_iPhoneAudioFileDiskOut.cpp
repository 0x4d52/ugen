// $Id:$
// $HeadURL:$

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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_iPhoneAudioFileDiskOut.h"
#include "ugen_NSUtilities.h"

DiskOutUGenInternal::DiskOutUGenInternal(AudioFileID audioFile, AudioStreamBasicDescription const& format, UGen const& input) throw()
:	ProxyOwnerUGenInternal(NumInputs, input.getNumChannels() - 1),
	audioFile_(audioFile),
	numChannels(input.getNumChannels()),
	currentPacket(0),
	allocatedBlockSize(0),
	allocatedBlockSizeInBytes(0),
	audioData(0),
	bytesPerFrame(format.mBytesPerFrame)
{
	inputs[Input] = input;
	
	if(audioFile_)
	{
		allocatedBlockSize = UGen::getEstimatedBlockSize();
		allocatedBlockSizeInBytes = allocatedBlockSize * bytesPerFrame;
		audioData = malloc(allocatedBlockSizeInBytes + 4); // pad a little for 24 bit
	}	
}

DiskOutUGenInternal::~DiskOutUGenInternal() throw()
{	
	if(audioFile_)
		AudioFileClose(audioFile_);
	
	free(audioData);
}

void DiskOutUGenInternal::checkBufferSize(const int requiredBufferSize) throw()
{
	if(audioData && requiredBufferSize > allocatedBlockSize)
	{
		allocatedBlockSize = requiredBufferSize;
		allocatedBlockSizeInBytes = allocatedBlockSize * bytesPerFrame;
		audioData = realloc(audioData, allocatedBlockSizeInBytes + 4); // pad a litte for 24 bit
		if(!audioData)
		{
			AudioFileClose(audioFile_);
			audioFile_ = 0;
		}
	}
}

void DiskOutUGenInternal::writeBuffer(const int bufferSize) throw()
{
	UInt32 ioNumPackets = bufferSize;
	OSStatus status = AudioFileWritePackets(audioFile_, false,
											allocatedBlockSizeInBytes, NULL,
											currentPacket,&ioNumPackets,
											audioData);
	
	if((status != noErr) || (ioNumPackets == 0))
	{
		printf("DiskOut: error: writing file\n");
		AudioFileClose(audioFile_);
		audioFile_ = 0;
	}
	else
	{
		currentPacket += ioNumPackets;
	}	
}

DiskOutUGenInternalWav16::DiskOutUGenInternalWav16(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalWav16::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFF;
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			SInt16* audioFileSamples = (SInt16*)audioData + channel;
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				*audioFileSamples = (SInt16)(*inputSamples * factor);
				inputSamples++;
				audioFileSamples += numChannels;
			}
		}		
		
		writeBuffer(blockSize);
	}
}

DiskOutUGenInternalAiff16::DiskOutUGenInternalAiff16(AudioFileID audioFile, 
													 AudioStreamBasicDescription const& format, 
													 UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalAiff16::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFF;
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			SInt16* audioFileSamples = ((SInt16*)audioData) + channel;
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				SInt16 sample = (SInt16)(*inputSamples * factor);
				*audioFileSamples = bigEndian16Bit((const char*)&sample);
				inputSamples++;
				audioFileSamples += numChannels;
			}
		}		
		
		writeBuffer(blockSize);
	}	
}

DiskOutUGenInternalWav24::DiskOutUGenInternalWav24(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalWav24::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFFFF;
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			char* audioFileSamples = ((char*)audioData) + (channel*3);
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				*(int*)audioFileSamples = (int)(*inputSamples * factor);
				inputSamples++;
				audioFileSamples += (numChannels * 3);
			}
		}		
		
		writeBuffer(blockSize);
	}
}

DiskOutUGenInternalAiff24::DiskOutUGenInternalAiff24(AudioFileID audioFile, 
													 AudioStreamBasicDescription const& format, 
													 UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalAiff24::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFFFFFF; // try 32 bit... and take the 3 MSBs
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			char* audioFileSamples = ((char*)audioData) + (channel*3);
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				int sample = (int)(*inputSamples * factor);
				char *samplePtr = (char*)&sample;
				audioFileSamples[0] = samplePtr[3];
				audioFileSamples[1] = samplePtr[2];
				audioFileSamples[2] = samplePtr[1];
				inputSamples++;
				audioFileSamples += (numChannels * 3);
			}
		}		
		
		writeBuffer(blockSize);
	}
}

DiskOutUGenInternalWav32::DiskOutUGenInternalWav32(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalWav32::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFFFFFF;
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			SInt32* audioFileSamples = ((SInt32*)audioData) + channel;
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				*audioFileSamples = (SInt32)(*inputSamples * factor);
				inputSamples++;
				audioFileSamples += numChannels;
			}
		}		
		
		writeBuffer(blockSize);
	}
}

DiskOutUGenInternalAiff32::DiskOutUGenInternalAiff32(AudioFileID audioFile, 
													 AudioStreamBasicDescription const& format, 
													 UGen const& input) throw()
:	DiskOutUGenInternal(audioFile, format, input)
{
}

void DiskOutUGenInternalAiff32::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 0x7FFFFFFF;
	
	const int blockSize = uGenOutput.getBlockSize();
	checkBufferSize(blockSize);
	
	if(audioFile_)
	{		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
			float *outputSamples = proxies[channel]->getSampleData();
			memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
			
			SInt32* audioFileSamples = ((SInt32*)audioData) + channel;
			int numSamplesToProcess = blockSize;
			
			while(numSamplesToProcess--)
			{
				SInt32 sample = (SInt32)(*inputSamples * factor);
				*audioFileSamples = bigEndian32Bit((const char*)&sample);
				inputSamples++;
				audioFileSamples += numChannels;
			}
		}		
		
		writeBuffer(blockSize);
	}	
}


DiskOut::DiskOut(Text const& audioFilePath, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
{	
	enum AudioFileType { WAV, AIFF };
	AudioFileType type;
	
	TextArray split = audioFilePath.split(".");
	int splitLength = split.length();
	
	Text pathChecked; 
	if(splitLength <= 1)
	{
		pathChecked = audioFilePath << ".wav";
		type = WAV;
	}
	else
	{
		pathChecked = audioFilePath;
		if(split.last().equalsIgnoreCase("aif") || 
		   split.last().equalsIgnoreCase("aiff"))
		{
			type = AIFF;
		}
		else
		{
			type = WAV;
		}
	}
	
	bool success = false;
	
	if(type == AIFF)
	{
		if(bitDepth == 24)
			success = initWithAudioFileAiff24(pathChecked.getArray(), input, overwriteExisitingFile);
		else if(bitDepth == 32)
			success = initWithAudioFileAiff32(pathChecked.getArray(), input, overwriteExisitingFile);
		else
		{
			if(bitDepth != 16)
				printf("DiskOut: warning: bit depth of %d not supported, using 16\n", bitDepth);
			success = initWithAudioFileAiff16(pathChecked.getArray(), input, overwriteExisitingFile);
		}
	}
	else if(type == WAV)
	{
		if(bitDepth == 24)
			success = initWithAudioFileWav24(pathChecked.getArray(), input, overwriteExisitingFile);
		else if(bitDepth == 32)
			success = initWithAudioFileWav32(pathChecked.getArray(), input, overwriteExisitingFile);
		else
		{
			if(bitDepth != 16)
				printf("DiskOut: warning: bit depth of %d not supported, using 16\n", bitDepth);
			success = initWithAudioFileWav16(pathChecked.getArray(), input, overwriteExisitingFile);
		}
	}
	
	
	
	// unnecessary?
//	if(!success)
//	{
//		initInternal(1);
//		internalUGens[0] = getNullInternal();		
//	}
}

bool DiskOut::initWithAudioFileWav16(const char *audioFilePath, 
									 UGen const& input, 
									 bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 16;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileWAVEType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalWav16(audioFile, format, input));
	
	return true;	
}

bool DiskOut::initWithAudioFileAiff16(const char *audioFilePath, 
									  UGen const& input, 
									  bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 16;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileAIFFType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalAiff16(audioFile, format, input));
	
	return true;	
}

bool DiskOut::initWithAudioFileWav24(const char *audioFilePath, 
									 UGen const& input, 
									 bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 24;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileWAVEType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalWav24(audioFile, format, input));
	
	return true;	
}

bool DiskOut::initWithAudioFileAiff24(const char *audioFilePath, 
									  UGen const& input, 
									  bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 24;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileAIFFType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalAiff24(audioFile, format, input));
	
	return true;	
}

bool DiskOut::initWithAudioFileWav32(const char *audioFilePath, 
									 UGen const& input, 
									 bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 32;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileWAVEType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalWav32(audioFile, format, input));
	
	return true;	
}

bool DiskOut::initWithAudioFileAiff32(const char *audioFilePath, 
									  UGen const& input, 
									  bool overwriteExisitingFile) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Documents, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, 
															   (UInt8*)audioFilePath, 
															   strlen(audioFilePath), 
															   false);
	
	AudioStreamBasicDescription format;
	
	const int numChannels = input.getNumChannels();
	
	format.mChannelsPerFrame	= numChannels;
	format.mSampleRate			= UGen::getSampleRate();
	format.mFormatID			= kAudioFormatLinearPCM;
	format.mFormatFlags			= kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBitsPerChannel		= 32;
	format.mBytesPerFrame		= format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mFramesPerPacket		= 1;
	format.mBytesPerPacket		= format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioFileID	audioFile;
	OSStatus status = AudioFileCreateWithURL(fileURL,
											 kAudioFileAIFFType,
											 &format,
											 overwriteExisitingFile ? kAudioFileFlags_EraseFile : 0,
											 &audioFile);
	
	if(status != noErr) return false;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskOutUGenInternalAiff32(audioFile, format, input));
	
	return true;	
}


END_UGEN_NAMESPACE

#endif // UGEN_IPHONE
