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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_iPhoneAudioFileDiskIn.h"
#include "ugen_NSUtilities.h"

DiskInUGenInternal::DiskInUGenInternal(AudioFileID audioFile, 
									   AudioStreamBasicDescription const& format, 
									   const bool loopFlag, 
									   const double startTime,
									   const UGen::DoneAction doneAction) throw()
:	ProxyOwnerUGenInternal(0, format.mChannelsPerFrame - 1),
	audioFile_(audioFile),
	numChannels_(format.mChannelsPerFrame),
	loopFlag_(loopFlag),
	startTime_(startTime),
	packetCount(0),
	currentPacket(0),
	allocatedBlockSize(0),
	audioData(0),
	numPackets(0),
	bytesPerFrame(format.mBytesPerFrame),
	fileSampleRate(format.mSampleRate),
	doneAction_(doneAction),
	shouldDeleteValue(doneAction_ == UGen::DeleteWhenDone)
{
	OSStatus result;
	UInt32 dataSize;
	
	if(audioFile_)
	{
		dataSize = sizeof(packetCount);
		result = AudioFileGetProperty(audioFile, kAudioFilePropertyAudioDataPacketCount, &dataSize, &packetCount);
		if (result != noErr) 
		{
			printf("DiskIn: error: Could not get packet count: ID=%p err=%d\n", audioFile_, (int)result);
			AudioFileClose(audioFile_);
			audioFile_ = 0;
		}	
		else
		{
			currentPacket = ugen::max(0.0, startTime) * UGen::getSampleRate();
			if(currentPacket >= packetCount)
				currentPacket = 0;
			
			allocatedBlockSize = UGen::getEstimatedBlockSize();
			audioData = malloc(bytesPerFrame * allocatedBlockSize);
		}
	}
}

DiskInUGenInternal::~DiskInUGenInternal() throw()
{
	if(audioFile_)
		AudioFileClose(audioFile_);
	
	free(audioData);
}

OSStatus DiskInUGenInternal::clearOutputsAndReadData(bool& shouldDelete) throw()
{
	const int outputBlockSize = uGenOutput.getBlockSize();
	const double sampeRateRatio = 1.0;//fileSampleRate * UGen::getReciprocalSampleRate();
	
	const int inputBlockSize = outputBlockSize * sampeRateRatio;
	
	if(inputBlockSize > allocatedBlockSize)
	{
		allocatedBlockSize = inputBlockSize;
		audioData = realloc(audioData, bytesPerFrame * allocatedBlockSize);
		if(!audioData)
		{
			printf("DiskIn: error: could not allocated memory for the new buffer size\n\n");
			AudioFileClose(audioFile_);
			audioFile_ = 0;
		}
	}
	
	for(int channel = 0; channel < numChannels_; channel++)
	{
		float *outputSamples = proxies[channel]->getSampleData();
		memset(outputSamples, 0, outputBlockSize * sizeof(float));
	}	
	
	if(!audioFile_)
	{
		numPackets = 0;
		return -1;
	}
	else
	{
		numPackets = inputBlockSize;
		UInt32 numBytesRead = -1;
		OSStatus result = AudioFileReadPackets(audioFile_, false,
											   &numBytesRead, NULL, 
											   currentPacket, &numPackets, audioData); 
		
		if(result == noErr)
		{  
			currentPacket += numPackets;
			
			if(numPackets < inputBlockSize)
			{
				if(loopFlag_)
				{
					// we hit the end of the file and looping is on, we need to pull 
					// a few samples from the start of the file
					
					const int remainingPackets = inputBlockSize - numPackets;
					
					unsigned char *remainingAudioData = (unsigned char*)audioData;
					remainingAudioData += remainingPackets * bytesPerFrame;
					
					numPackets = remainingPackets;
					result = AudioFileReadPackets(audioFile_, false, 
												  &numBytesRead, NULL, 
												  0, &numPackets, audioData);
					
					if(result == noErr)
					{ 
						currentPacket = remainingPackets;
					}
				}
				else
				{
					setIsDone();
					shouldDelete = shouldDelete ? true : shouldDeleteValue;
				}
			}
		}
		
		return result;
	}
	
}

void DiskInUGenInternal::prepareForBlock(const int actualBlockSize, const unsigned int blockID) throw()
{
	senderUserData = userData;
	if(isDone()) sendDoneInternal();
}

DiskInUGenInternalWav16::DiskInUGenInternalWav16(AudioFileID audioFile, 
												 AudioStreamBasicDescription const& format, 
												 const bool loopFlag, 
												 const double startTime,
												 const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalWav16::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFF;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			SInt16* audioFileSamples = (SInt16*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)(*audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}

DiskInUGenInternalAiff16::DiskInUGenInternalAiff16(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   const bool loopFlag, 
												   const double startTime,
												   const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalAiff16::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFF;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			SInt16* audioFileSamples = (SInt16*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)bigEndian16Bit((const char*)audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}

DiskInUGenInternalWav24::DiskInUGenInternalWav24(AudioFileID audioFile, 
												 AudioStreamBasicDescription const& format, 
												 const bool loopFlag, 
												 const double startTime,
												 const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalWav24::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFFFF;
	const int intInc = numChannels_ * 3;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			char* audioFileSamples = (char*)audioData + (channel * 3);
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)littleEndian24Bit(audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += intInc;
			}
		}
	}	
}

DiskInUGenInternalAiff24::DiskInUGenInternalAiff24(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   const bool loopFlag, 
												   const double startTime,
												   const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalAiff24::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFFFF;
	const int intInc = numChannels_ * 3;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			char* audioFileSamples = (char*)audioData + (channel * 3);
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)bigEndian24Bit(audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += intInc;
			}
		}
	}	
}

DiskInUGenInternalWav32::DiskInUGenInternalWav32(AudioFileID audioFile, 
												 AudioStreamBasicDescription const& format, 
												 const bool loopFlag, 
												 const double startTime,
												 const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalWav32::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFFFFFF;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			SInt32* audioFileSamples = (SInt32*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)(*audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}

DiskInUGenInternalAiff32::DiskInUGenInternalAiff32(AudioFileID audioFile, 
												   AudioStreamBasicDescription const& format, 
												   const bool loopFlag, 
												   const double startTime,
												   const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalAiff32::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	static const float factor = 1.0 / 0x7FFFFFFF;
	
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			SInt32* audioFileSamples = (SInt32*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = (float)bigEndian32Bit((const char*)audioFileSamples) * factor;
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}

DiskInUGenInternalFloatBigEndian::DiskInUGenInternalFloatBigEndian(AudioFileID audioFile, 
																   AudioStreamBasicDescription const& format, 
																   const bool loopFlag, 
																   const double startTime,
																   const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalFloatBigEndian::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			float* audioFileSamples = (float*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = bigEndianFloat((const char*)audioFileSamples);
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}

DiskInUGenInternalFloatLittleEndian::DiskInUGenInternalFloatLittleEndian(AudioFileID audioFile, 
																		 AudioStreamBasicDescription const& format, 
																		 const bool loopFlag, 
																		 const double startTime,
																		 const UGen::DoneAction doneAction) throw()
:	DiskInUGenInternal(audioFile, format, loopFlag, startTime, doneAction)
{
}

void DiskInUGenInternalFloatLittleEndian::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{
	if(clearOutputsAndReadData(shouldDelete) == noErr)
	{		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			float *outputSamples = proxies[channel]->getSampleData();
			float* audioFileSamples = (float*)audioData + channel;
			int numSamplesToProcess = numPackets;
			
			while(numSamplesToProcess--)
			{
				*outputSamples = *audioFileSamples;
				outputSamples++;
				audioFileSamples += numChannels_;
			}
		}
	}	
}



DiskIn::DiskIn(Text const& path, 
			   const bool loopFlag, 
			   const double startTime,
			   const UGen::DoneAction doneAction) throw()
{	
	initWithAudioFile(path.getArray(), loopFlag, startTime, doneAction);
}

void DiskIn::initWithAudioFile(const char* audioFilePath, 
							   const bool loopFlag, 
							   const double startTime,
							   const UGen::DoneAction doneAction) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Bundle, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	OSStatus result;
	UInt32 dataSize;
	
	CFURLRef audioFileURL;
	audioFileURL = CFURLCreateFromFileSystemRepresentation(NULL,
														   (const UInt8*)audioFilePath, 
														   strlen(audioFilePath), 
														   false);
	
	AudioFileID	audioFile = 0;
	result = AudioFileOpenURL (audioFileURL, kAudioFileReadPermission, 0, &audioFile);
	CFRelease(audioFileURL);
	if (result != noErr) 
	{
		printf("DiskIn: error: Could not open file: %s err=%d\n", audioFilePath, (int)result);
		audioFile = 0;
	}
	
	if(audioFile)
	{
		AudioStreamBasicDescription format;
		dataSize = sizeof(format);
		result = AudioFileGetProperty(audioFile, kAudioFilePropertyDataFormat, &dataSize, &format);
		if (result != noErr) 
		{
			printf("DiskIn: error: Could not get data format: %s err=%d\n", audioFilePath, (int)result);
			AudioFileClose(audioFile);
			audioFile = 0;
		}
		else if(format.mFormatID != kAudioFormatLinearPCM)
		{
			printf("DiskIn: error: Only PCM formats supported\\n");
			AudioFileClose(audioFile);
			audioFile = 0;
		}
		else
		{
			if(format.mSampleRate != UGen::getSampleRate())
			{
				printf("DiskIn: warning: Sample rate mismatch - resampling is not yet implemented\\n");
			}
			
			if((format.mFormatFlags & kAudioFormatFlagIsFloat) != 0)
			{
				initInternal(format.mChannelsPerFrame);
				
				if((format.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0)
				{
					generateFromProxyOwner(new DiskInUGenInternalFloatBigEndian(audioFile, 
																				format, 
																				loopFlag, 
																				startTime, 
																				doneAction));
					return;
				}
				else
				{
					generateFromProxyOwner(new DiskInUGenInternalFloatLittleEndian(audioFile, 
																				   format, 
																				   loopFlag, 
																				   startTime, 
																				   doneAction));
					return;
				}
			}
			else if((format.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0)
			{
				// aif and other big edian?
				if(format.mBitsPerChannel == 16)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalAiff16(audioFile, 
																		format, 
																		loopFlag, 
																		startTime, 
																		doneAction));
					return;
				}
				else if(format.mBitsPerChannel == 32)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalAiff32(audioFile, 
																		format, 
																		loopFlag, 
																		startTime, 
																		doneAction));			
					return;
				}
				else if(format.mBitsPerChannel == 24)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalAiff24(audioFile, 
																		format, 
																		loopFlag, 
																		startTime, 
																		doneAction));	
					return;
				}
				else
				{
					printf("DiskIn: error: Sound file format not yet supported.");
					AudioFileClose(audioFile);
					audioFile = 0;
				}
			}
			else
			{
				// wav and other little edian?
				if(format.mBitsPerChannel == 16)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalWav16(audioFile, 
																	   format, 
																	   loopFlag, 
																	   startTime, 
																	   doneAction));
					return;					
				}
				else if(format.mBitsPerChannel == 32)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalWav32(audioFile, 
																	   format, 
																	   loopFlag, 
																	   startTime, 
																	   doneAction));
					return;					
				}
				else if(format.mBitsPerChannel == 24)
				{
					initInternal(format.mChannelsPerFrame);
					generateFromProxyOwner(new DiskInUGenInternalWav24(audioFile, 
																	   format, 
																	   loopFlag, 
																	   startTime, 
																	   doneAction));
					return;					
				}
				else
				{
					printf("DiskIn: error: Sound file format not yet supported.");
					AudioFileClose(audioFile);
					audioFile = 0;
				}
			}
		}
	}
}


END_UGEN_NAMESPACE

#endif // UGEN_IPHONE