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

BEGIN_UGEN_NAMESPACE

#include "ugen_Buffer.h"
#include "../core/ugen_UGen.h"
#include "../core/ugen_Random.h"
#include "../core/ugen_Value.h"
#include "../basics/ugen_UnaryOpUGens.h"
#if defined(UGEN_IPHONE) || defined(DOXYGEN)
	#include "../iphone/ugen_NSUtilities.h"
#endif

//#define BUFFERTESTMEMORY 1

#ifdef BUFFERTESTMEMORY
static inline void reportDataPtr(const float* ptr, const int size) throw()
{
	printf("BufferChannelInternal data=%p(%d) size=%p(%d)\n", ptr, ptr, size, size);
}
#endif

BufferChannelInternal::BufferChannelInternal(const unsigned int size, bool zeroData) throw()
:	data(0),
	size_(size),
	allocatedSize(size),
	currentWriteBlockID(-1),
	circularHead(-1), previousCircularHead(-1)
{
//	ugen_assert(size > 0);
	
	if(size_ > 0)
	{
		data = new float[size_];
		
		if(zeroData)
		{	
			memset(data, 0, size_ * sizeof(float));
		}
	}
	
#ifdef BUFFERTESTMEMORY
	reportDataPtr(data, size_);
#endif
}

BufferChannelInternal::BufferChannelInternal(const unsigned int size, 
											 const unsigned int sourceDataSize, 
											 float* sourceData,
											 const bool copyTheData) throw()
:	data(0),
	size_(size),
	allocatedSize(0),
	currentWriteBlockID(-1),
	circularHead(-1), previousCircularHead(-1)
{
	ugen_assert(size > 0);
	ugen_assert(sourceDataSize > 0);
	ugen_assert(sourceData != 0);
	
	if(copyTheData)
	{
		allocatedSize = size_;
		data = new float[allocatedSize];
		
		if(size > sourceDataSize)
		{
			memcpy(data, sourceData, sourceDataSize * sizeof(float));
			memset(data + sourceDataSize, 0, (size - sourceDataSize) * sizeof(float));
		}
		else
		{
			memcpy(data, sourceData, size * sizeof(float));
		}
	}
	else
	{
		allocatedSize = 0;
		data = sourceData;
	}

#ifdef BUFFERTESTMEMORY
	reportDataPtr(data, size_);
#endif	
}

BufferChannelInternal::BufferChannelInternal(const unsigned int size, const double start, const double end) throw()
:	size_(size),
	allocatedSize(size),
	currentWriteBlockID(-1),
	circularHead(-1), previousCircularHead(-1)
{
	ugen_assert(size >= 2);
	
	data = new float[size_];
	double inc = (end - start) / (size_ - 1);
	double currentValue = start;
	float *outputSamples = data;
	
	int numSamples = size_;
	while(--numSamples >= 0)
	{
		*outputSamples++ = (float)currentValue;
		currentValue += inc;
	}	
	
#ifdef BUFFERTESTMEMORY
	reportDataPtr(data, size_);
#endif	
}

BufferChannelInternal::~BufferChannelInternal() throw()
{
	if(allocatedSize > 0)
		delete [] data;
	
	data = 0;
	size_= 0;
	allocatedSize = 0;
	currentWriteBlockID = -1;
	circularHead = -1;
	previousCircularHead = -1;
}

Buffer::Buffer() throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
}

Buffer::Buffer(BufferSpec const& spec) throw()
:	numChannels_(spec.numChannels_),
	size_(spec.size_)
{
	const bool zeroData = spec.zeroData_;
	
	channels = new BufferChannelInternal*[numChannels_];
	
	for(int i = 0; i < numChannels_; i++)
	{
		channels[i] = new BufferChannelInternal(size_, zeroData);
	}
}

Buffer::Buffer(int i00) throw()
:	numChannels_(1),
	size_(1)
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, false);
	channels[0]->data[0] = (float)i00;
}

Buffer::Buffer(int i00, int i01) throw()
:	numChannels_(1),
	size_(2)
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, false);
	channels[0]->data[0] = (float)i00;
	channels[0]->data[1] = (float)i01;
}

Buffer::Buffer(int i00, int i01, int i02) throw()
:	numChannels_(1),
	size_(3)
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, false);
	channels[0]->data[0] = (float)i00;
	channels[0]->data[1] = (float)i01;
	channels[0]->data[2] = (float)i02;
}


Buffer::Buffer(double i00) throw()
:	numChannels_(1),
	size_(1)
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(1, false);
	channels[0]->data[0] = (float)i00;
}

Buffer::Buffer(double i00,
			   double i01, 
			   double i02, 
			   double i03, 
			   double i04, 
			   double i05, 
			   double i06, 
			   double i07, 
			   double i08, 
			   double i09, 
			   double i10, 
			   double i11, 
			   double i12, 
			   double i13, 
			   double i14, 
			   double i15, 
			   double i16, 
			   double i17, 
			   double i18, 
			   double i19, 
			   double i20, 
			   double i21, 
			   double i22, 
			   double i23, 
			   double i24, 
			   double i25, 
			   double i26, 
			   double i27, 
			   double i28, 
			   double i29, 
			   double i30, 
			   double i31) throw()
:	numChannels_(1)
{
	int count = 2;
	
	if(i02 == INFINITY) goto init; else count++;
	if(i03 == INFINITY) goto init; else count++;
	if(i04 == INFINITY) goto init; else count++;
	if(i05 == INFINITY) goto init; else count++;
	if(i06 == INFINITY) goto init; else count++;
	if(i07 == INFINITY) goto init; else count++;
	if(i08 == INFINITY) goto init; else count++;
	if(i09 == INFINITY) goto init; else count++;
	if(i10 == INFINITY) goto init; else count++;
	if(i11 == INFINITY) goto init; else count++;
	if(i12 == INFINITY) goto init; else count++;
	if(i13 == INFINITY) goto init; else count++;
	if(i14 == INFINITY) goto init; else count++;
	if(i15 == INFINITY) goto init; else count++;
	if(i16 == INFINITY) goto init; else count++;
	if(i17 == INFINITY) goto init; else count++;
	if(i18 == INFINITY) goto init; else count++;
	if(i19 == INFINITY) goto init; else count++;
	if(i20 == INFINITY) goto init; else count++;
	if(i21 == INFINITY) goto init; else count++;
	if(i22 == INFINITY) goto init; else count++;
	if(i23 == INFINITY) goto init; else count++;
	if(i24 == INFINITY) goto init; else count++;
	if(i25 == INFINITY) goto init; else count++;
	if(i26 == INFINITY) goto init; else count++;
	if(i27 == INFINITY) goto init; else count++;
	if(i28 == INFINITY) goto init; else count++;
	if(i29 == INFINITY) goto init; else count++;
	if(i30 == INFINITY) goto init; else count++;
	if(i31 == INFINITY) goto init; else count++;
	
init:
	
	size_ = count;
	
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, false);
	
	channels[0]->data[0] = (float)i00;
	channels[0]->data[1] = (float)i01;
	
	if(i02 == INFINITY) return; else channels[0]->data[ 2] = (float)i02;
	if(i03 == INFINITY) return; else channels[0]->data[ 3] = (float)i03;
	if(i04 == INFINITY) return; else channels[0]->data[ 4] = (float)i04;
	if(i05 == INFINITY) return; else channels[0]->data[ 5] = (float)i05;
	if(i06 == INFINITY) return; else channels[0]->data[ 6] = (float)i06;
	if(i07 == INFINITY) return; else channels[0]->data[ 7] = (float)i07;
	if(i08 == INFINITY) return; else channels[0]->data[ 8] = (float)i08;
	if(i09 == INFINITY) return; else channels[0]->data[ 9] = (float)i09;
	if(i10 == INFINITY) return; else channels[0]->data[10] = (float)i10;
	if(i11 == INFINITY) return; else channels[0]->data[11] = (float)i11;
	if(i12 == INFINITY) return; else channels[0]->data[12] = (float)i12;
	if(i13 == INFINITY) return; else channels[0]->data[13] = (float)i13;
	if(i14 == INFINITY) return; else channels[0]->data[14] = (float)i14;
	if(i15 == INFINITY) return; else channels[0]->data[15] = (float)i15;
	if(i16 == INFINITY) return; else channels[0]->data[16] = (float)i16;
	if(i17 == INFINITY) return; else channels[0]->data[17] = (float)i17;
	if(i18 == INFINITY) return; else channels[0]->data[18] = (float)i18;
	if(i19 == INFINITY) return; else channels[0]->data[19] = (float)i19;
	if(i20 == INFINITY) return; else channels[0]->data[20] = (float)i20;
	if(i21 == INFINITY) return; else channels[0]->data[21] = (float)i21;
	if(i22 == INFINITY) return; else channels[0]->data[22] = (float)i22;
	if(i23 == INFINITY) return; else channels[0]->data[23] = (float)i23;
	if(i24 == INFINITY) return; else channels[0]->data[24] = (float)i24;
	if(i25 == INFINITY) return; else channels[0]->data[25] = (float)i25;
	if(i26 == INFINITY) return; else channels[0]->data[26] = (float)i26;
	if(i27 == INFINITY) return; else channels[0]->data[27] = (float)i27;
	if(i28 == INFINITY) return; else channels[0]->data[28] = (float)i28;
	if(i29 == INFINITY) return; else channels[0]->data[29] = (float)i29;
	if(i30 == INFINITY) return; else channels[0]->data[30] = (float)i30;
	if(i31 == INFINITY) return; else channels[0]->data[31] = (float)i31;
}

Buffer::Buffer(ValueArray const& array) throw()
:	numChannels_(1),
	size_(array.size())
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, false);
	for(int i = 0; i < size_; i++)
	{
		channels[0]->data[i] = array[i].getValue();
	}
}

Buffer Buffer::line(const int size, const double start, const double end) throw()
{
	Buffer newBuffer(BufferSpec(size, 1));
		
	double inc = (end - start) / (size - 1);
	double currentValue = start;
	float *outputSamples = newBuffer.getData(0);
	
	int numSamples = size;
	while(--numSamples >= 0)
	{
		*outputSamples++ = (float)currentValue;
		currentValue += inc;
	}
	
	return newBuffer;
}

Buffer Buffer::series(const int size, const double start, const double grow) throw()
{
	Buffer newBuffer(BufferSpec(size, 1));
	float currentValue = start;
	float *outputSamples = newBuffer.getData(0);
	
	int numSamples = size;
	while(--numSamples >= 0)
	{
		*outputSamples++ = currentValue;
		currentValue += grow;
	}
	
	return newBuffer;
}

Buffer Buffer::geom(const int size, const double start, const double grow) throw()
{
	Buffer newBuffer(BufferSpec(size, 1));
	float currentValue = start;
	float *outputSamples = newBuffer.getData(0);
	
	int numSamples = size;
	while(--numSamples >= 0)
	{
		*outputSamples++ = currentValue;
		currentValue *= grow;
	}
	
	return newBuffer;
}

Buffer Buffer::newClear(const int size, const int numChannels, bool zeroData) throw()
{
	return Buffer(BufferSpec(size, numChannels, zeroData));
}

Buffer Buffer::withSize(const int size, const int numChannels, bool zeroData) throw()
{
	return Buffer(BufferSpec(size, numChannels, zeroData));
}

Buffer::Buffer(const int size, float* sourceData, const bool copyTheData) throw()
:	numChannels_(1),
	size_(size)
{
	channels = new BufferChannelInternal*[numChannels_]; //	numChannels_ = 1 always in this case
	channels[0] = new BufferChannelInternal(size_, size_, sourceData, copyTheData);
}

Buffer::Buffer(const int size, const int numChannels, float** sourceDataArray, const bool copyTheData) throw()
:	numChannels_(numChannels),
	size_(size)
{
	channels = new BufferChannelInternal*[numChannels_];
	for(int i = 0; i < numChannels_; i++)
	{
		channels[i] = new BufferChannelInternal(size_, size_, sourceDataArray[i], copyTheData);
	}
}

Buffer::Buffer(BufferChannelInternal *internalToUse) throw()
:	numChannels_(1),
	size_(internalToUse->size_)
{
	// NB internalToUse can't be zero as this would be ambiguous with another constructor
	channels = new BufferChannelInternal*[numChannels_];
	internalToUse->incrementRefCount();
	channels[0] = internalToUse;
}

#if defined(UGEN_JUCE)
#include "../juce/ugen_JuceUtility.h"
Buffer::Buffer(AudioSampleBuffer& audioSampleBuffer, const bool copyTheData) throw()
:	numChannels_(audioSampleBuffer.getNumChannels()),
	size_(audioSampleBuffer.getNumSamples())
{
	channels = numChannels_ <= 0 ? 0 : new BufferChannelInternal*[numChannels_];
	for(int i = 0; i < numChannels_; i++)
	{
		channels[i] = new BufferChannelInternal(size_, size_, audioSampleBuffer.getSampleData(i), copyTheData);
	}
}

Buffer::Buffer(String const& audioFilePath) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	File audioFile (audioFilePath);
	double sampleRate = initFromJuceFile(audioFile);
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));		
}

Buffer::Buffer(String const& audioFilePath, double& sampleRate) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	File audioFile (audioFilePath);
	sampleRate = initFromJuceFile(audioFile);
}

Buffer::Buffer(const char *audioFilePath) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	File audioFile (audioFilePath);
	double sampleRate = initFromJuceFile(audioFile);
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));		
}

Buffer::Buffer(Text const& audioFilePath) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	File audioFile (audioFilePath.getArray());
	double sampleRate = initFromJuceFile(audioFile);
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));	
}

Buffer::Buffer(Text const& audioFilePath, double& sampleRate) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	File audioFile (audioFilePath.getArray());
	sampleRate = initFromJuceFile(audioFile);
}

Buffer::Buffer(const File& audioFile) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	double sampleRate = initFromJuceFile(audioFile);
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));
}

Buffer::Buffer(const File& audioFile, double& sampleRate) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	sampleRate = initFromJuceFile(audioFile);
}

double Buffer::initFromJuceFile(const File& audioFile) throw()
{
	if((audioFile == File::nonexistent) || (audioFile.exists() == false))
	{
		ugen_assertfalse;
		return 0.0;
	}
	
	AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	
//#if JUCE_QUICKTIME
//	formatManager.registerFormat(new QuickTimeAudioFormat(), false);
//#endif
	
	AudioFormatReader* audioFormatReader = formatManager.createReaderFor (audioFile);
	
	if(audioFormatReader == 0) return 0.0;
	
	double sampleRate = audioFormatReader->sampleRate;
	numChannels_ = audioFormatReader->numChannels;
	size_ = (int)audioFormatReader->lengthInSamples;
	channels = new BufferChannelInternal*[numChannels_];
	
	float** bufferData = new float*[numChannels_];
	
	for(int i = 0; i < numChannels_; i++)
	{
		channels[i] = new BufferChannelInternal(size_, false);
		bufferData[i] = channels[i]->data;
	}
	
	if(size_ > 0)
	{
		AudioSampleBuffer audioSampleBuffer(bufferData, numChannels_, size_);
		audioSampleBuffer.readFromAudioReader(audioFormatReader, 0, size_, 0, true, true);
	}
	else
	{
		sampleRate = 0.0;
	}
	
	delete audioFormatReader;
	delete [] bufferData;
	return sampleRate;
}

bool Buffer::write(Text const& audioFilePath, 
				   bool overwriteExisitingFile, 
				   int bitDepth) throw()
{
	File audioFile(audioFilePath.getArray());
	return initFromJuceFile(audioFile, overwriteExisitingFile, bitDepth);
}

bool Buffer::write(const File& audioFile, 
				   bool overwriteExisitingFile, 
				   int bitDepth) throw()
{
	return initFromJuceFile(audioFile, overwriteExisitingFile, bitDepth);
}

bool Buffer::write(const File::SpecialLocationType directory, 
				   bool overwriteExisitingFile, 
				   int bitDepth) throw()
{
	return initFromJuceFile(File::getSpecialLocation(directory), overwriteExisitingFile, bitDepth);
}

bool Buffer::initFromJuceFile(const File& audioFile, 
							  bool overwriteExisitingFile, 
							  int bitDepth) throw()
{	
	File outputFile;
	
	if(audioFile.isDirectory()) 
		outputFile = audioFile.getChildFile(getFileNameWithTimeIdentifier(T("Buffer")));
	else
		outputFile = audioFile;
		
	if(outputFile.getFileExtension().isEmpty())
		outputFile = outputFile.withFileExtension(T("wav"));
	
	if(overwriteExisitingFile == true && outputFile.exists())
		outputFile.deleteFile();
	else if(outputFile.exists())
		return false;
	
	AudioFormatWriter* audioFormatWriter;
	
	if(outputFile.hasFileExtension(".wav"))
	{
		WavAudioFormat wavAudioFormat;
		FileOutputStream* fileOutputStream = outputFile.createOutputStream();
		
		if(!fileOutputStream) return false;
		
		audioFormatWriter = wavAudioFormat.createWriterFor(fileOutputStream, 
														   UGen::getSampleRate(), 
														   getNumChannels(), 
														   bitDepth, 0, 0);
	}
	else if(outputFile.hasFileExtension(".aif") || outputFile.hasFileExtension(".aiff"))
	{
		AiffAudioFormat aiffAudioFormat;
		FileOutputStream* fileOutputStream = outputFile.createOutputStream();
		
		if(!fileOutputStream) return false;
		
		audioFormatWriter = aiffAudioFormat.createWriterFor(fileOutputStream, 
															UGen::getSampleRate(), 
															getNumChannels(), 
															bitDepth, 0, 0);
	}
	
	float **bufferData = new float*[getNumChannels()];
	memset(bufferData, 0, getNumChannels() * sizeof(float*));
	
	for(int i = 0; i < getNumChannels(); i++)
	{
		bufferData[i] = getData(i);
	}
	
	AudioSampleBuffer audioSampleBuffer(bufferData, getNumChannels(), size());
	audioSampleBuffer.writeToAudioWriter(audioFormatWriter, 0, size());
	
	delete audioFormatWriter;
	delete [] bufferData;
	
	return true;
}

#elif defined(UGEN_IPHONE) // else if so we don't use these if using Juce on the iPhone

Buffer::Buffer(const char *audioFilePath) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	double sampleRate = initFromAudioFile(audioFilePath);
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));	
}

Buffer::Buffer(Text const& audioFilePath) throw()
:	numChannels_(0),
	size_(0),
	channels(0)
{
	double sampleRate = initFromAudioFile(audioFilePath.getArray());
	double currentSampleRate = UGen::getSampleRate();
	
	if((sampleRate != 0.0) && (sampleRate != currentSampleRate))
		operator= (changeSampleRate(sampleRate, currentSampleRate));	
}

Buffer::Buffer(Text const& audioFilePath, double& sampleRate) throw()
{
	sampleRate = initFromAudioFile(audioFilePath.getArray());
}

double Buffer::initFromAudioFile(const char* audioFilePath) throw()
{
	Text path; // this needs to be here so it doesn't get garbage collected too early
	
	if(audioFilePath[0] != '/')
	{	
		path = NSUtilities::pathInDirectory(NSUtilities::Bundle, audioFilePath);
		audioFilePath = path.getArray();
	}
	
	if (audioFilePath == 0 || audioFilePath[0] == 0) 
	{
		printf("Buffer: File path is null (perhaps it doesn't exist?\n");
		return 0.0;
	}
	
	OSStatus result;
	UInt32 dataSize;
	
	CFURLRef audioFileURL;
	audioFileURL = CFURLCreateFromFileSystemRepresentation(NULL,
														   (const UInt8*)audioFilePath, 
														   strlen(audioFilePath), 
														   false);
	
	AudioFileID	audioFile;
	result = AudioFileOpenURL (audioFileURL, kAudioFileReadPermission, 0, &audioFile);
	CFRelease(audioFileURL);
	if (result != noErr) 
	{
		printf("Buffer: Could not open file: %s err=%d\n", audioFilePath, (int)result);
		return 0.0;
	}
	
	SInt64 packetCount;
	dataSize = sizeof(packetCount);
	result = AudioFileGetProperty(audioFile, kAudioFilePropertyAudioDataPacketCount, &dataSize, &packetCount);
	if (result != noErr) 
	{
		printf("Buffer: Could not get packet count: %s err=%d\n", audioFilePath, (int)result);
		AudioFileClose(audioFile);
		return 0.0;
	}
	
	AudioStreamBasicDescription format;
	dataSize = sizeof(format);
	result = AudioFileGetProperty(audioFile, kAudioFilePropertyDataFormat, &dataSize, &format);
	if (result != noErr) 
	{
		printf("Buffer: Could not get data format: %s err=%d\n", audioFilePath, (int)result);
		AudioFileClose(audioFile);
		return 0.0;
	}
	
	void* audioData = malloc(format.mBytesPerFrame * packetCount);
	
	if(audioData)
	{
		UInt32 packetsRead = packetCount;
		UInt32 numBytesRead = -1;
		result = AudioFileReadPackets(audioFile, false, &numBytesRead, NULL, 0, &packetsRead, audioData); 
		
		if (result != noErr) 
		{
			printf("Buffer: Could not read audio packets: %s err=%d\n", audioFilePath, (int)result);
			free(audioData);
			AudioFileClose(audioFile);
			return 0.0;
		}
		
		numChannels_ = format.mChannelsPerFrame;
		size_ = packetsRead;
		channels = new BufferChannelInternal*[numChannels_];
		
		for(int i = 0; i < numChannels_; i++)
		{
			channels[i] = new BufferChannelInternal(size_, false);
		}
		
		if(format.mFormatID != kAudioFormatLinearPCM)
		{
			printf("Buffer: Only PCM formats supported\\n");
			clear();
		}
		else if((format.mFormatFlags & kAudioFormatFlagIsFloat) != 0)
		{
			// float format, just need to deinterleave
			for(int channel = 0; channel < numChannels_; channel++)
			{
				float* bufferSamples = channels[channel]->data;
				float* audioFileSamples = (float*)audioData + channel;
				int numSamplesToProcess = size_;
				
				if((format.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0)
				{
					// big endian float
					while(numSamplesToProcess--)
					{
						*bufferSamples = bigEndianFloat((const char*)audioFileSamples);
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
				else
				{
					while(numSamplesToProcess--)
					{
						*bufferSamples = *audioFileSamples;
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
			}
		}
		else if((format.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0)
		{
			// aif and other big edian?
			if(format.mBitsPerChannel == 16)
			{
				const float factor = 1.0 / 0x7FFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					SInt16* audioFileSamples = (SInt16*)audioData + channel;
					int numSamplesToProcess = size_;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)bigEndian16Bit((const char*)audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
			}
			else if(format.mBitsPerChannel == 32)
			{
				const float factor = 1.0 / 0x7FFFFFFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					SInt32* audioFileSamples = (SInt32*)audioData + channel;
					int numSamplesToProcess = size_;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)bigEndian32Bit((const char*)audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
			}
			else if(format.mBitsPerChannel == 24)
			{
				const float factor = 1.0 / 0x7FFFFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					char* audioFileSamples = (char*)audioData + (channel * 3);
					int numSamplesToProcess = size_;
					
					const int intInc = numChannels_ * 3;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)bigEndian24Bit(audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += intInc;
					}
				}
			}
			else
			{
				printf("Buffer: Sound file format not yet supported.");
				clear();
			}
		}
		else
		{
			// wav and other little edian?
			if(format.mBitsPerChannel == 16)
			{
				const float factor = 1.0 / 0x7FFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					SInt16* audioFileSamples = (SInt16*)audioData + channel;
					int numSamplesToProcess = size_;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)(*audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
			}
			else if(format.mBitsPerChannel == 32)
			{
				const float factor = 1.0 / 0x7FFFFFFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					SInt32* audioFileSamples = (SInt32*)audioData + channel;
					int numSamplesToProcess = size_;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)(*audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += numChannels_;
					}
				}
			}
			else if(format.mBitsPerChannel == 24)
			{
				const float factor = 1.0 / 0x7FFFFF;
				for(int channel = 0; channel < numChannels_; channel++)
				{
					float* bufferSamples = channels[channel]->data;
					char* audioFileSamples = (char*)audioData + (channel * 3);
					int numSamplesToProcess = size_;
					
					const int intInc = numChannels_ * 3;
					
					while(numSamplesToProcess--)
					{
						*bufferSamples = (float)littleEndian24Bit(audioFileSamples) * factor;
						bufferSamples++;
						audioFileSamples += intInc;
					}
				}
			}
			else
			{
				printf("Buffer: Sound file format not yet supported.");
				clear();
			}
		}
	}
	else
	{
		printf("Buffer: Could not allocate memory for audio : %s err=%d\n", audioFilePath, (int)result);
	}
	
	free(audioData);
	AudioFileClose(audioFile);
	
	return format.mSampleRate;
}

bool Buffer::write(Text const& audioFilePath, 
				   bool overwriteExisitingFile, 
				   int bitDepth) throw()
{
	ugen_assert(bitDepth >= 16);
	
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
		
	if(type == AIFF)
	{
		if(bitDepth == 24)
			return initFromAudioFileAiff24(pathChecked.getArray(), overwriteExisitingFile);
		else if(bitDepth == 32)
			return initFromAudioFileAiff32(pathChecked.getArray(), overwriteExisitingFile);
		else
		{
			if(bitDepth != 16)
				printf("Buffer: warning: bit depth of %d not supported, using 16\n", bitDepth);
			return initFromAudioFileAiff16(pathChecked.getArray(), overwriteExisitingFile);
		}
	}
	else if(type == WAV)
	{
		if(bitDepth == 24)
			return initFromAudioFileWav24(pathChecked.getArray(), overwriteExisitingFile);
		else if(bitDepth == 32)
			return initFromAudioFileWav32(pathChecked.getArray(), overwriteExisitingFile);
		else
		{
			if(bitDepth != 16)
				printf("Buffer: warning: bit depth of %d not supported, using 16\n", bitDepth);
			return initFromAudioFileWav16(pathChecked.getArray(), overwriteExisitingFile);
		}
	}
	
	return false;
}

bool Buffer::initFromAudioFileWav16(const char* audioFilePath, 
									bool overwriteExisitingFile) throw()
{	
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	SInt16 intValues[numChannels];
	
	const float factor = 0x7FFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			intValues[channel] = (SInt16)(getSampleUnchecked(channel, sample) * factor);
		}
				
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   intValues);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

bool Buffer::initFromAudioFileAiff16(const char* audioFilePath, 
									 bool overwriteExisitingFile) throw()
{
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	SInt16 intValues[numChannels];
	
	const float factor = 0x7FFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			intValues[channel] = (SInt16)(getSampleUnchecked(channel, sample) * factor);
			intValues[channel] = bigEndian16Bit((const char*)&intValues[channel]);
		}
		
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   intValues);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

bool Buffer::initFromAudioFileWav24(const char* audioFilePath, 
									bool overwriteExisitingFile) throw()
{	
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	char data[numChannels*3];
	
	const float factor = 0x7FFFFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			int *intValue = (int*)(data + (channel * 3));
			*intValue = (int)(getSampleUnchecked(channel, sample) * factor);
		}
		
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   data);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

bool Buffer::initFromAudioFileAiff24(const char* audioFilePath, 
									 bool overwriteExisitingFile) throw()
{
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	char data[numChannels*3];
	
	const float factor = 0x7FFFFFFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			char *intValue = data + (channel * 3);
			int sampleConvert = (int)(getSampleUnchecked(channel, sample) * factor);
			char *samplePtr = (char*)&sampleConvert;
			intValue[0] = samplePtr[3];
			intValue[1] = samplePtr[2];
			intValue[2] = samplePtr[1];
		}
		
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   data);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

bool Buffer::initFromAudioFileWav32(const char* audioFilePath, 
									bool overwriteExisitingFile) throw()
{	
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	SInt32 intValues[numChannels];
	
	const float factor = 0x7FFFFFFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			intValues[channel] = (SInt32)(getSampleUnchecked(channel, sample) * factor);
		}
		
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   intValues);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

bool Buffer::initFromAudioFileAiff32(const char* audioFilePath, 
									 bool overwriteExisitingFile) throw()
{
	ugen_assert(audioFilePath != 0);
	
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
	
	const int numChannels = getNumChannels();
	
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
	
	if(status) return false;
	
	UInt32 sizeInBytes = format.mBytesPerFrame;
	SInt32 intValues[numChannels];
	
	const float factor = 0x7FFFFFFF;
	
	const int numSamples = size();
	for(int sample = 0; sample < numSamples; sample++)
	{
		for(int channel = 0; channel < numChannels; channel++)
		{
			intValues[channel] = (SInt32)(getSampleUnchecked(channel, sample) * factor);
			intValues[channel] = bigEndian32Bit((const char*)&intValues[channel]);
		}
		
		UInt32 ioNumPackets = 1;
		status = AudioFileWritePackets(audioFile,
									   false,
									   sizeInBytes,
									   NULL,
									   sample,
									   &ioNumPackets,
									   intValues);
		
		if(ioNumPackets == 0)
		{
			printf("Buffer: error: writing file\n");
			AudioFileClose(audioFile);
			return false;
		}
	}
	
	AudioFileClose(audioFile);
	return true;
}

#endif



Buffer::~Buffer() throw()
{
	decrementInternals();
	delete [] channels;
}

void Buffer::incrementInternals() throw()
{
	if(channels != 0)
	{
		for(int i = 0; i < numChannels_; i++)
		{
			channels[i]->incrementRefCount();
		}
	}
}

void Buffer::decrementInternals() throw()
{
	if(channels != 0)
	{
		for(int i = 0; i < numChannels_; i++)
		{
			channels[i]->decrementRefCount();
		}
	}
}

Buffer Buffer::interleave() throw()
{
	if(size_ < 1 || numChannels_ < 1) return Buffer();
	
	Buffer intervleavedBuffer = Buffer::withSize(size_ * numChannels_, 1, false);
	
	for(int channel = 0; channel < numChannels_; channel++)
	{
		int numSamplesToProcess = size_;
		const float * srcSamples = getData(channel);
		float* dstSamples = intervleavedBuffer.getData() + channel;
		
		while(numSamplesToProcess--)
		{
			*dstSamples = *srcSamples++;
			dstSamples += numChannels_;
		}
	}
	
	return intervleavedBuffer;
}

Buffer Buffer::deinterleave(const int numInterleavedChannels) throw()
{
	ugen_assert(numChannels_ == 1);
	ugen_assert((size_ % numInterleavedChannels) == 0);
	
	const int newSize = size_ / numInterleavedChannels;
	Buffer deintervleavedBuffer = Buffer::withSize(newSize, 
												   numInterleavedChannels, 
												   false);
	
	for(int channel = 0; channel < numInterleavedChannels; channel++)
	{
		int numSamplesToProcess = newSize;
		const float * srcSamples = getData() + channel;
		float* dstSamples = deintervleavedBuffer.getData(channel);
		
		while(numSamplesToProcess--)
		{
			*dstSamples++ = *srcSamples;
			srcSamples += numInterleavedChannels;
		}
	}
	
	return deintervleavedBuffer;	
}

void Buffer::copyFrom(Buffer const& source) throw()
{
	copyFrom(source, 0, 0, ugen::min(size(), source.size()));
}

void Buffer::copyFrom(Buffer const& source, const int sourceOffset, const int destOffset, const int numSamples) throw()
{
	ugen_assert(getNumChannels() > 0);
	ugen_assert(source.getNumChannels() > 0);
	ugen_assert(size() > 0);
	ugen_assert(source.size() > 0);
	ugen_assert((destOffset+numSamples) <= size());
	ugen_assert((sourceOffset+numSamples) <= source.size());
	
	for(int channel = 0; channel < getNumChannels(); channel++)
	{
		float *destData = getData(channel);
		const float* sourceData = source.getData(channel % source.getNumChannels());
		
		memcpy(destData, sourceData, sizeof(float) * numSamples);
	}
}

double Buffer::duration() const throw()												
{ 
	return (double)size_ * UGen::getReciprocalSampleRate(); 
}

Buffer::Buffer(Buffer const& copy) throw()
:	numChannels_(copy.numChannels_),
	size_(copy.size_)
{
	channels = new BufferChannelInternal*[numChannels_];
	
	for(int i = 0; i < numChannels_; i++)
	{
		copy.channels[i]->incrementRefCount();
		channels[i] = copy.channels[i];
	}
}

Buffer Buffer::copy() const throw()
{
	Buffer newBuffer = Buffer::withSize(size_, numChannels_, false);
	
	if(size_ > 0 && numChannels_ > 0) 
	{
		for(int i = 0; i < numChannels_; i++) 
		{
			memcpy(newBuffer.getData(i), getData(i), size_ * sizeof(float));
		}
	}
	
	return newBuffer;
}

bool Buffer::operator== (Buffer const& other) const throw()
{
	if(size_ != other.size_) return false;
	if(numChannels_ != other.numChannels_) return false;
	
	for(int i = 0; i < numChannels_; i++)
	{
		if(channels[i] != other.channels[i]) return false;
	}
	
	return true;
}

bool Buffer::operator!= (Buffer const& other) const throw()
{
	return !operator== (other);
}

Buffer& Buffer::operator= (Buffer const& _other) throw()
{
	if (this != &_other)
    {		
		Buffer other = _other;
		other.incrementInternals();
		decrementInternals();
		delete [] channels;
		
		numChannels_ = other.numChannels_;
		size_ = other.size_;
		channels = new BufferChannelInternal*[numChannels_];
		
		for(int i = 0; i < numChannels_; i++)
		{
			channels[i] = other.channels[i];
		}
    }
	
    return *this;
}

Buffer& Buffer::operator+= (Buffer const& other) throw()
{
	if(isNull())
		operator= (other);
	else
		operator= (*this + other);
	
	return *this;
}

Buffer& Buffer::operator-= (Buffer const& other) throw()
{
	if(isNull())
		operator= (other);	
	else
		operator= (*this - other);
	
	return *this;
}

Buffer& Buffer::operator*= (Buffer const& other) throw()
{
	if(isNull())
		operator= (other);
	else
		operator= (*this * other);	
	
	return *this;
}

Buffer& Buffer::operator/= (Buffer const& other) throw()
{
	if(isNull())
		operator= (other);
	else
		operator= (*this / other);
	
	return *this;
}

Buffer& Buffer::operator<<= (Buffer const& other) throw()
{
	if(isNull())
		operator= (other);	
	else
		operator= (Buffer(*this, other));
	
	return *this;
}


Buffer Buffer::append(Buffer const& other) const throw()
{
	const int newNumChannels = numChannels_ > other.numChannels_ ? numChannels_ : other.numChannels_;
	const int newSize = size_ + other.size_;
	
	Buffer newBuffer = Buffer::withSize(newSize, newNumChannels, false);
	
	for(int i = 0; i < numChannels_; i++)
	{
		memcpy(newBuffer.channels[i]->data, 
			   channels[i % newNumChannels]->data, 
			   size_ * sizeof(float));
	}
	
	for(int i = 0; i < other.numChannels_; i++)
	{
		memcpy(newBuffer.channels[i]->data + size_, 
			   other.channels[i % other.numChannels_]->data, 
			   other.size_ * sizeof(float));
	}
	
	return newBuffer;
}

Buffer::Buffer(Buffer const& channels0, Buffer const& channels1) throw()
:	numChannels_(channels0.numChannels_ + channels1.numChannels_)
{
	channels = new BufferChannelInternal*[numChannels_];
	
	if(channels0.size_ > channels1.size_)
	{
		size_ = channels0.size_;
		int newIndex = 0;
		for(int i = 0; i < channels0.numChannels_; i++, newIndex++)
		{
			channels0.channels[i]->incrementRefCount();
			channels[newIndex] = channels0.channels[i];
		}
		
		for(int i = 0; i < channels1.numChannels_; i++, newIndex++)
		{
			channels[newIndex] = new BufferChannelInternal(size_, 
														   channels1.size_, 
														   channels1.channels[i]->data,
														   true);
		}
	}
	else if(channels0.size_ < channels1.size_)
	{
		size_ = channels1.size_;
		int newIndex = 0;
		for(int i = 0; i < channels0.numChannels_; i++, newIndex++)
		{
			channels[newIndex] = new BufferChannelInternal(size_, 
														   channels0.size_, 
														   channels0.channels[i]->data,
														   true);
		}
		
		for(int i = 0; i < channels1.numChannels_; i++, newIndex++)
		{
			channels1.channels[i]->incrementRefCount();
			channels[newIndex] = channels1.channels[i];
		}
	}
	else
	{
		size_ = channels0.size_;
		int newIndex = 0;
		for(int i = 0; i < channels0.numChannels_; i++, newIndex++)
		{
			channels0.channels[i]->incrementRefCount();
			channels[newIndex] = channels0.channels[i];
		}
		
		for(int i = 0; i < channels1.numChannels_; i++, newIndex++)
		{
			channels1.channels[i]->incrementRefCount();
			channels[newIndex] = channels1.channels[i];
		}
	}
}

Buffer Buffer::operator<<(Buffer const& newChannels) const throw()
{
	return Buffer(*this, newChannels);
}

Buffer Buffer::operator,(Buffer const& other) const throw()
{
	return append(other);
}


//Buffer Buffer::addChannels(Buffer const& newChannels) const throw()
//{
//	return Buffer(*this, newChannels);
//}

Buffer Buffer::getChannel(const int channel) const throw()
{
	if(channel < 0 || channel >= numChannels_) 
	{
		ugen_assertfalse;
		return Buffer();
	}
	
	if(numChannels_ == 1) return *this;
	
	return Buffer(channels[channel]);
}


Buffer Buffer::getRegion(const int startSample, 
						 const int endSample, 
						 const int startChannel, 
						 const int endChannel) const throw()
{
	int newSize, newNumChannels;
	int sampleStep = 1, channelStep = 1;
	int currentChannel;
	
	if(endSample == -1)
		newSize = size_ - startSample;
	else if(endSample > startSample)
		newSize = endSample - startSample + 1;
	else
	{
		newSize = startSample - endSample + 1;
		sampleStep = -1;
	}
	
	if(startChannel == -1)
	{
		currentChannel = 0;
		newNumChannels = numChannels_;
	}
	else
	{
		currentChannel = startChannel;
		
		if(endChannel == -1)
			newNumChannels = numChannels_ - startChannel;
		else if(endChannel > startChannel)
			newNumChannels = endChannel - startChannel + 1;
		else
		{
			newNumChannels = startChannel - endChannel + 1;
			channelStep = -1;
		}
	}
	
	Buffer newBuffer = Buffer::withSize(newSize, newNumChannels, false);
	
	for(int newChannelIndex = 0; newChannelIndex < newNumChannels; newChannelIndex++, currentChannel += channelStep)
	{
		const float *inputSamples = getData(currentChannel) + startSample;
		
		float *outputSamples = newBuffer.getData(newChannelIndex);
		int numSamples = newSize;
		
		while(numSamples--)
		{
			*outputSamples++ = *inputSamples;
			inputSamples += sampleStep;
		}
	}
	
	return newBuffer;
}

Buffer& Buffer::shrinkSize(const int amount) throw()
{
	if(amount > 0)
	{
		for(int i = 0; i < numChannels_; i++)
		{
			channels[i]->size_ -= amount;
		}
		
		size_ -= amount;
	}
	
	return *this;
}

float Buffer::sum(const int channel) const throw()
{
	if(numChannels_ == 0 || channel < 0 || channel >= numChannels_) 
	{
		ugen_assertfalse;
		return 0.f;
	}
	
	int numSamples = size_;
	float* inputSamples = channels[channel]->data;
	float total = 0.f;
	
	while(--numSamples >= 0) {
		total += *inputSamples++;
	}
	
	return total;
}

float Buffer::sum() const throw()
{
	if(numChannels_ == 0) 
	{
		ugen_assertfalse;
		return 0.f;
	}
	
	float total = 0.f;
	
	for(int channel = 0; channel < numChannels_; channel++)
	{
		total += sum(channel);
	}
	
	return total;
}

Buffer Buffer::normalise() const throw()
{
	float peak = findPeak();
	
	if(peak > 0.f)
		return *this * (1.f / peak);
	else
		return *this;
}

Buffer Buffer::normaliseEach() const throw()
{
	Buffer newBuffer;
	
	if(numChannels_ > 0 && size_ > 0)
	{
		for(int channel = 0; channel < numChannels_; channel++)
		{
			newBuffer <<= getChannel(channel).normalise();
		}
	}
	
	return newBuffer;
}

float Buffer::findMaximum(const int channel) const throw()
{
	if(numChannels_ == 0 || size_ == 0) 
	{
		ugen_assertfalse;
		return 0.f;
	}
	
	float value;
	
	if(channel >= 0)
	{
		value = getSampleUnchecked(channel, 0);
		for(int i = 0; i < size_; i++)
		{
			float newValue = getSampleUnchecked(channel, i);
			value = newValue > value ? newValue : value;
		}
	}
	else
	{
		value = getSampleUnchecked(0, 0);
		for(int i = 0; i < numChannels_; i++)
		{
			float newValue = findMaximum(i);
			value = newValue > value ? newValue : value;
		}
	}
	
	return value;
}

float Buffer::findMinimum(const int channel) const throw()
{
	if(numChannels_ == 0 || size_ == 0) 
	{
		ugen_assertfalse;
		return 0.f;
	}
	
	float value;
	
	if(channel >= 0)
	{
		value = getSampleUnchecked(channel, 0);
		for(int i = 0; i < size_; i++)
		{
			float newValue = getSampleUnchecked(channel, i);
			value = newValue < value ? newValue : value;
		}
	}
	else
	{
		value = getSampleUnchecked(0, 0);
		for(int i = 0; i < numChannels_; i++)
		{
			float newValue = findMinimum(i);
			value = newValue < value ? newValue : value;
		}
	}
	
	return value;	
}

float Buffer::findPeak(const int channel) const throw()
{
	if(numChannels_ == 0 || size_ == 0) 
	{
		ugen_assertfalse;
		return 0.f;
	}
	
	float value;
	
	if(channel >= 0)
	{
		value = getSampleUnchecked(channel, 0);
		for(int i = 0; i < size_; i++)
		{
			float newValue = ugen::abs(getSampleUnchecked(channel, i));
			value = newValue > value ? newValue : value;
		}
	}
	else
	{
		value = getSampleUnchecked(0, 0);
		for(int i = 0; i < numChannels_; i++)
		{
			float newValue = findPeak(i);
			value = newValue > value ? newValue : value;
		}
	}
	
	return value;	
}

Buffer Buffer::blend(Buffer const& other, double dfraction) const throw()
{
	if(size_ == 0 || numChannels_ == 0) return other;
		
	ugen_assert(dfraction >= -1.0 && dfraction <= 1.0);
	
	const int newNumChannels = ugen::max(numChannels_, other.numChannels_);
	const int maxSize = ugen::max(size_, other.size_);
	const int minSize = ugen::min(size_, other.size_);

	Buffer newBuffer = Buffer::withSize(maxSize, newNumChannels, false);
	
	if(dfraction >= 0.0)
	{		
		float fraction = ugen::clip((float)dfraction, 0.f, 1.f);
		
		for(int channel = 0; channel < newNumChannels; channel++)
		{
			const int thisChannel = channel % getNumChannels();
			const int otherChannel = channel % other.getNumChannels();
			
			for(int sample = 0; sample < minSize; sample++)
			{
				float thisValue = getSampleUnchecked(thisChannel, sample);
				float otherValue = other.getSampleUnchecked(otherChannel, sample);
				float newValue = thisValue * (1.f-fraction) + otherValue * fraction;
				newBuffer.setSampleUnchecked(channel, sample, newValue);
			}
			
			if(size_ < other.size_)
			{
				for(int sample = minSize; sample < maxSize; sample++)
				{
					float otherValue = other.getSampleUnchecked(otherChannel, sample);
					float newValue = otherValue * fraction;
					newBuffer.setSampleUnchecked(channel, sample, newValue);
				}
			}
			else
			{
				for(int sample = minSize; sample < maxSize; sample++)
				{
					float thisValue = getSampleUnchecked(thisChannel, sample);
					float newValue = thisValue * (1.f-fraction);
					newBuffer.setSampleUnchecked(channel, sample, newValue);
				}			
			}
		}
	}
	else
	{
		float fraction = ugen::neg(ugen::clip((float)dfraction, -1.f, 0.f));
		
		for(int channel = 0; channel < newNumChannels; channel++)
		{
			const int thisChannel = channel % getNumChannels();
			const int otherChannel = channel % other.getNumChannels();
			
			for(int sample = 0; sample < minSize; sample++)
			{
				float thisValue = getSampleUnchecked(thisChannel, sample);
				float otherValue = other.getSampleUnchecked(otherChannel, sample);
				float newValue = otherValue > thisValue ?  // peak hold
					otherValue : thisValue * (1.f-fraction) + otherValue * fraction;
				newBuffer.setSampleUnchecked(channel, sample, newValue);
			}
			
			if(size_ < other.size_)
			{
				for(int sample = minSize; sample < maxSize; sample++)
				{
					float otherValue = other.getSampleUnchecked(otherChannel, sample);
					float newValue = otherValue;
					newBuffer.setSampleUnchecked(channel, sample, newValue);
				}
			}
			else
			{
				for(int sample = minSize; sample < maxSize; sample++)
				{
					float thisValue = getSampleUnchecked(thisChannel, sample);
					float newValue = thisValue * (1.f-fraction);
					newBuffer.setSampleUnchecked(channel, sample, newValue);
				}			
			}
		}		
	}
	
	return newBuffer;
}


//Buffer Buffer::loopFade(const float fadeTime, 
//						EnvCurve const& fadeInShape, 
//						EnvCurve const& fadeOutShape) const throw()
//{
//	const int fadeSize = fadeTime * UGen::getSampleRate();
//	
//	if(fadeSize >= size_) return *this;
//	
//	const int newSize = size_ - fadeSize;
//	
//	Buffer newBuffer(BufferSpec(newSize, numChannels_, false));
//	for(int channel = 0; channel < numChannels_; channel++)
//	{
//		int numSamplesToProcess;
//		
//		float *outputSamples;
//		
//		outputSamples = newBuffer.getData(channel);
//		memcpy(outputSamples, getDataReadOnly(channel), newSize * sizeof(float));
//		
//		double grow, a2, b1, y1, y2;
//		
//		// do fade in
//		numSamplesToProcess = fadeSize;
//		switch(fadeInShape.getType())
//		{
//			case EnvCurve::Numerical: {
//				float curveValue = fadeInShape.getCurve();
//				if(std::fabs(curveValue) > 0.001)
//				{
//					
//					break;
//				}
//				// else fall through
//			}
//			case EnvCurve::Exponential: // can't do exp since it hits 0
//			case EnvCurve::Linear: {
//				float fadeInLevel = 0.f;
//				float fadeSlope = 1.f / fadeSize;
//				
//				while(numSamplesToProcess--)
//				{
//					*outputSamples++ *= fadeInLevel;						
//					fadeInLevel += fadeSlope;
//				}
//			} break;
//			case EnvCurve::Sine: {
//				w = pi / (double)fadeSize;
//				
//				a2 = 0.5;
//				b1 = 2.0 * std::cos(w);
//				y1 = 0.5;
//				y2 = y1 * std::sin(piOverTwo - w);
//				currentValue = a2 - y1;
//				
//			} break;
//			case EnvCurve::Welch:
//			case EnvCurve::Empty:
//			case EnvCurve::Step:
//			default:
//		}
//		// do fade out
//		
//		outputSamples = newBuffer.getData(channel);
//		const float* inputSamples = getDataReadOnly(channel) + newSize;
//		
//		numSamplesToProcess = fadeSize;
//		
//		float fadeOutLevel = 1.f;
//		
//		while(numSamplesToProcess--)
//		{			
//			*outputSamples++ += *inputSamples++ * fadeOutLevel;			
//			fadeOutLevel -= fadeSlope;
//		}
//	}
//	
//	return newBuffer;
//}


Buffer Buffer::loopFade(const float fadeTime, 
						EnvCurve const& fadeInShape, 
						EnvCurve const& fadeOutShape) const throw()
{
	ugen_assert(fadeTime >= 0.f);
	
	const int fadeSize = fadeTime * UGen::getSampleRate();
	
	if(fadeSize >= size_ || fadeSize == 0) return *this;
	
	const int newSize = size_ - fadeSize;
	
	Buffer newBuffer = Buffer::withSize(newSize, numChannels_, false);
	for(int channel = 0; channel < numChannels_; channel++)
	{
		float *outputSamples = newBuffer.getData(channel);
		memcpy(outputSamples, getData(channel), newSize * sizeof(float));
		
		const float* inputSamples = getData(channel) + newSize;
		
		int numSamplesToProcess = fadeSize;
		float fadeInLevel = 0.f;
		float fadeOutLevel = 1.f;
		float fadeSlope = 1.f / fadeSize;
		
		while(numSamplesToProcess--)
		{
			float fadeIn = *outputSamples;
			float fadeOut = *inputSamples;
			
			*outputSamples = fadeIn * fadeInLevel + fadeOut * fadeOutLevel;			
						
			fadeInLevel += fadeSlope;
			fadeOutLevel -= fadeSlope;
			inputSamples++;
			outputSamples++;
		}
	}
	
	return newBuffer;
}

Buffer Buffer::resample(const int newSize) const throw()
{
	ugen_assert(newSize > 0);
	
	if(size_ == 0 || size_ == newSize)
	{
		return *this;
	}
	else
	{
		Buffer newBuffer = Buffer::withSize(newSize, numChannels_, false);
		
		double reciprocalNewSize = 1.0 / (double)(newSize-1);
		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			for(int sample = 0; sample < newSize; sample++)
			{
				float value = lookup(channel, (double)sample * reciprocalNewSize);
				newBuffer.setSampleUnchecked(channel, sample, value);
			}
		}
		
		return newBuffer;
	}
}

Buffer Buffer::changeSampleRate(const double oldSampleRate, const double newSampleRateIn) const throw()
{
	const double newSampleRate = newSampleRateIn == 0.0 ? UGen::getSampleRate() : newSampleRateIn;
	
	if(oldSampleRate == newSampleRate)
	{
		return *this;
	}
	else
	{
		return resample(size_ * (newSampleRate / oldSampleRate));
	}
}

#ifndef UGEN_ANDROID
Buffer Buffer::rand(const int size, const double lower, const double upper, const int numChannels) throw()
{
	Buffer newBuffer = Buffer::withSize(size, numChannels, false);
	
	double range = upper-lower;
	double rangeFactor = range / RAND_MAX;
	
	for(int channel = 0; channel < numChannels; channel++)
	{
		for(int sample = 0; sample < size; sample++)
		{
			newBuffer.setSampleUnchecked(channel, sample, std::rand() * rangeFactor + lower);
		}
	}
	
	return newBuffer;
}
#else
Buffer Buffer::rand(const int size, const double lower, const double upper, const int numChannels) throw()
{
	Buffer newBuffer = Buffer::withSize(size, numChannels, false));
	
	double range = upper-lower;
	
	for(int channel = 0; channel < numChannels; channel++)
	{
		for(int sample = 0; sample < size; sample++)
		{
			newBuffer.setSampleUnchecked(channel, sample, Ran088::defaultGenerator().nextDouble(range) + lower);
		}
	}
	
	return newBuffer;
}
#endif


Buffer Buffer::rand2(const int size, const double positive, const int numChannels) throw()
{
	return Buffer::rand(size, -positive, positive, numChannels);
}

#ifndef UGEN_NOEXTGPL
Buffer Buffer::exprand(const int size, const double lower, const double upper, const int numChannels) throw()
{
	Buffer newBuffer = Buffer::withSize(size, numChannels, false);
	Ran088& random(Ran088::defaultGenerator());
		
	for(int channel = 0; channel < numChannels; channel++)
	{
		for(int sample = 0; sample < size; sample++)
		{
			newBuffer.setSampleUnchecked(channel, sample, (float)random.nextExpRandRange(lower, upper));
		}
	}
	
	return newBuffer;
}

Buffer Buffer::linrand(const int size, const double lower, const double upper, const int numChannels) throw()
{
	Buffer newBuffer = Buffer::withSize(size, numChannels, false);
	Ran088& random(Ran088::defaultGenerator());
	
	double range = upper-lower;
	
	for(int channel = 0; channel < numChannels; channel++)
	{
		for(int sample = 0; sample < size; sample++)
		{
			newBuffer.setSampleUnchecked(channel, sample, (float)random.nextDouble(range) + lower);
		}
	}
	
	return newBuffer;
}
#endif // gpl

Buffer Buffer::sineTable(const int size, const float repeats) throw()
{
	return Buffer::line(size, 0.0, twoPi * repeats).sin();
}

Buffer Buffer::cosineTable(const int size, const float repeats) throw()
{
	return Buffer::line(size, 0.0, twoPi * repeats).cos();
}

Buffer Buffer::cosineWindow(const int size, const float repeats) throw()
{
	return Buffer::line(size, 0.0, pi * repeats).sin();
}

Buffer Buffer::triangleWindow(const int size) throw()
{
	const int rampUpSize = size/2;
	const int rampDownSize = size-rampUpSize;
	Buffer rampUp = Buffer::line(rampUpSize+2, 0.0, 1.0).shrinkSize(1).getRegion(1);
	Buffer rampDown = Buffer::line(rampDownSize+1, 1.0, 0.0).shrinkSize(1);
	return (rampUp, rampDown);
}

Buffer Buffer::bartlettWindow(const int size) throw()
{
	const int rampUpSize = size/2;
	const int rampDownSize = size-rampUpSize;
	Buffer rampUp = Buffer::line(rampUpSize+1, 0.0, 1.0).shrinkSize(1);
	Buffer rampDown = Buffer::line(rampDownSize, 1.0, 0.0);
	return (rampUp, rampDown);
}

Buffer Buffer::hannWindow(const int size) throw()
{
	return (-cosineTable(size) + 1.f) * 0.5f;
}

Buffer Buffer::hammingWindow(const int size) throw()
{
	return  -cosineTable(size) * 0.46f + 0.54f;
}

Buffer Buffer::blackmanWindow(const int size, const float alpha) throw()
{
	float a0 = (1.f-alpha) * 0.5f;
	float a1 = 0.5f;
	float a2 = alpha * 0.5f;
	
	return -cosineTable(size) * a1 + cosineTable(size, 2.f) * a2 + a0;
}



//Buffer Buffer::kaiserWindow(const int size, const float alpha) throw()
//{	
//	Buffer window = Buffer::line(size, 0.0, 2.0) - 1.f;
//	window = -window.squared() + 1.f;
//	window = window.sqrt() * alpha;
//	window = window.besselI0() / ugen::besselI0(alpha);
//	
//	return window;
//}

//Buffer Buffer::synth(const int size, UGen const& graph_) throw()
//{
//	UGen graph = graph_;	
//	ugen_assert(size > 0);
//	
//	int numChannels = graph.getNumChannels();
//	ugen_assert(numChannels > 0);
//	
//	Buffer result = Buffer::withSize(size, numChannels);
//	
//	for(int i = 0; i < numChannels; i++)
//	{
//		graph.setOutput(result.getData(i), size, i);
//	}
//	
//	graph.prepareAndProcessBlock(size, 0, 0);
//	
//	return result;	
//}

Buffer Buffer::synth(const int size, UGen const& graph, const bool allAtOnce) throw()
{
	ugen_assert(size > 0);
	
	int numChannels = graph.getNumChannels();
	ugen_assert(numChannels > 0);
	
	Buffer result = Buffer::withSize(size, numChannels);
	
	result.synthInPlace(graph, 0, size, allAtOnce);	
	
	return result;	
}


void Buffer::synthInPlace(UGen const& graph_, const int offset, const int numSamples_, const bool allAtOnce) throw()
{
	UGen graph = graph_;	
	
	int numSamples = (numSamples_ <= 0) ? size() : numSamples_;
	ugen_assert(numSamples > 0);
	
	int numChannels = ugen::min(getNumChannels(), graph.getNumChannels());
	ugen_assert(numChannels > 0);
		
	for(int i = 0; i < numChannels; i++)
	{
		graph.setOutput(getData(i) + offset, numSamples, i);
	}
			
	if(allAtOnce)
	{
		graph.prepareAndProcessBlock(numSamples, 0, -1);	
	}
	else
	{
		int numSamplesRemaining = numSamples;
		int blockSize = UGen::getEstimatedBlockSize();
		if(blockSize <= 0) blockSize = 512;
		
		int blockID = 0;
		
		while(numSamplesRemaining > 0)
		{
			if(numSamplesRemaining < blockSize)
				blockSize = numSamplesRemaining;
			
			graph.prepareAndProcessBlock(blockSize, blockID, -1);	
			
			numSamplesRemaining -= blockSize;
			blockID += blockSize;
			
#ifdef UGEN_JUCE
			Thread::yield();
#endif
		}
	}
	
	if(numChannels < getNumChannels())
	{
		for(int channel = numChannels; channel < getNumChannels(); channel++)
		{
			memcpy(getData(channel) + offset, 
				   getData(channel % numChannels) + offset, 
				   sizeof(float) * numSamples);
		}
	}	
}

void Buffer::synthAndSend(const int size, UGen const& graph, BufferReceiver* receiver, const int bufferID) throw()
{
	ugen_assert(receiver != 0);
	
	Buffer result = synth(size, graph, false);
	receiver->handleBuffer(result, 0.0, bufferID);	
}

Buffer Buffer::reciprocalExceptZero() const throw()
{
	Buffer newBuffer = Buffer::withSize(size_, numChannels_, false);
	
	for(int channelIndex = 0; channelIndex < numChannels_; channelIndex++)
	{
		int numSamples = size_;
		float* inputSamples = channels[channelIndex]->data;
		float* outputSamples = newBuffer.channels[channelIndex]->data;
		
		while(--numSamples >= 0) {
			float value = *inputSamples++;
			if(value == 0.f || value == -0.f)
				*outputSamples++ = 0.f;
			else
				*outputSamples++ = 1.f / value;
		}
	}
	
	return newBuffer;
}

Buffer Buffer::reverse() const throw()
{
	Buffer newBuffer = Buffer::withSize(size_, numChannels_, false);
	
	for(int channelIndex = 0; channelIndex < numChannels_; channelIndex++)
	{
		int numSamples = size_;
		float* inputSamples = channels[channelIndex]->data + numSamples - 1;
		float* outputSamples = newBuffer.channels[channelIndex]->data;
		
		while(--numSamples >= 0) {
			*outputSamples++ = *inputSamples--;
		}
	}
	
	return newBuffer;
}

Buffer Buffer::mix() const throw()
{
	Buffer newBuffer(BufferSpec(size_, 1, true));
	
	int channelIndex = 0;
	
	do
	{
		int numSamples = size_;
		float* inputSamples = channels[channelIndex]->data;
		float* outputSamples = newBuffer.channels[0]->data;
		
		while(--numSamples >= 0) {
			*outputSamples++ += *inputSamples++;
		}
		
		channelIndex++;
	}
	while(channelIndex < numChannels_);
	
	return newBuffer;
}

//Buffer Buffer::process(UGen const& input_, UGen const& graph_, const int offset, const int numSamples_) const throw()
//{
//	UGen input = input_;
//	UGen graph = graph_;
//	
//	int numSamples = (numSamples_ <= 0) ? size() : numSamples_;
//	ugen_assert(numSamples > 0);
//	
//	int numChannels = ugen::min(getNumChannels(), graph.getNumChannels());
//	ugen_assert(numChannels > 0);
//		
//	Buffer result = Buffer::withSize(numSamples, numChannels);
//		
//	for(int i = 0; i < numChannels; i++)
//	{
//		input.setInput(getData(i) + offset, numSamples, i);
//		graph.setOutput(result.getData(i), numSamples, i);
//	}
//		
//	int numSamplesRemaining = numSamples;
//	int blockSize = UGen::getEstimatedBlockSize();
//	if(blockSize <= 0) blockSize = 512;
//	
//	int blockID = 0;
//	
//	while(numSamplesRemaining > 0)
//	{
//		if(numSamplesRemaining < blockSize)
//			blockSize = numSamplesRemaining;
//		
//		graph.prepareAndProcessBlock(blockSize, blockID, -1);	
//		
//		numSamplesRemaining -= blockSize;
//		blockID += blockSize;
//		
//#ifdef UGEN_JUCE
//		Thread::yield();
//#endif
//	}
//	
//	
//	return result;
//}

Buffer Buffer::process(UGen const& input, UGen const& graph, const int offset, const int numSamples_, const bool allAtOnce) const throw()
{	
	int numSamples = (numSamples_ <= 0) ? size() : numSamples_;
	ugen_assert(numSamples > 0);
	
	int numChannels = ugen::min(getNumChannels(), graph.getNumChannels());
	ugen_assert(numChannels > 0);
		
	Buffer result = Buffer::withSize(numSamples, numChannels);
	
	result.processInPlace(input, graph, offset, numSamples, allAtOnce);
	
	return result;
}

void Buffer::processInPlace(UGen const& input_, UGen const& graph_, const int offset, const int numSamples_, const bool allAtOnce) throw()
{
	UGen input = input_;
	UGen graph = graph_;
	
	int numSamples = (numSamples_ <= 0) ? size() : numSamples_;
	ugen_assert(numSamples > 0);
	
	int numChannels = ugen::min(getNumChannels(), graph.getNumChannels());
	ugen_assert(numChannels > 0);
		
	for(int i = 0; i < numChannels; i++)
	{
		input.setInput(getData(i) + offset, numSamples, i);
		graph.setOutput(getData(i) + offset, numSamples, i);
	}
		
	if(allAtOnce)
	{
		graph.prepareAndProcessBlock(numSamples, 0, -1);	
	}
	else
	{
		int numSamplesRemaining = numSamples;
		int blockSize = UGen::getEstimatedBlockSize();
		if(blockSize <= 0) blockSize = 512;
		
		int blockID = 0;
		
		while(numSamplesRemaining > 0)
		{
			if(numSamplesRemaining < blockSize)
				blockSize = numSamplesRemaining;

			graph.prepareAndProcessBlock(blockSize, blockID, -1);	
			
			numSamplesRemaining -= blockSize;
			blockID += blockSize;
			
#ifdef UGEN_JUCE
			Thread::yield();
#endif
		}
	}
	
	if(numChannels < getNumChannels())
	{
		for(int channel = numChannels; channel < getNumChannels(); channel++)
		{
			memcpy(getData(channel) + offset, 
				   getData(channel % numChannels) + offset, 
				   sizeof(float) * numSamples);
		}
	}		
}

void Buffer::processAndSend(UGen const& input, 
							UGen const& graph, 
							BufferReceiver* receiver, 
							const int bufferID,
							const int offset, 
							const int numSamples) const throw()
{
	ugen_assert(receiver != 0);
	
	Buffer result = process(input, graph, offset, numSamples, false);
	receiver->handleBuffer(result, 0.0, bufferID);
}

Buffer Buffer::operator- () const throw()
{	
	Buffer newBuffer(BufferSpec(size_, numChannels_, false));
	
	for(int channelIndex = 0; channelIndex < numChannels_; channelIndex++)
	{
		int numSamples = size_;
		float* inputSamples = channels[channelIndex]->data;
		float* outputSamples = newBuffer.channels[channelIndex]->data;
		
		while(--numSamples >= 0) {
			*outputSamples++ = -(*inputSamples++);
		}
	}
	
	return newBuffer;
}

const Buffer& Buffer::getTableSine512() throw()
{
	static Buffer table = Buffer::line(513, 0.0, 2.0 * pi).sin().shrinkSize();
	return table;
}

const Buffer& Buffer::getTableSine8192() throw()
{
	static Buffer table = Buffer::line(8193, 0.0, 2.0 * pi).sin().shrinkSize();
	return table;	
}

const Buffer& Buffer::getTableCosine512() throw()
{
	static Buffer table = Buffer::line(513, 0.0, 2.0 * pi).cos().shrinkSize();
	return table;
}

const Buffer& Buffer::getTableCosine8192() throw()
{
	static Buffer table = Buffer::line(8193, 0.0, 2.0 * pi).cos().shrinkSize();
	return table;
}

static Buffer ugen_calculatePanTable512() throw()
{
	Buffer constantPanTemp;
	constantPanTemp = Buffer::line(512, -piOverFour, piOverFour) << Buffer::line(512, piOverFour, -piOverFour);
	return (constantPanTemp.cos() - constantPanTemp.sin()) * sqrt2OverTwo;
}

const Buffer& Buffer::getTableConstantPan512() throw()
{
	static Buffer table = ugen_calculatePanTable512();
	return table;
}


ComplexBuffer::ComplexBuffer(const int size, bool zeroData) throw()
:	Buffer(BufferSpec(size, 2, zeroData))
{	
}

ComplexBuffer::ComplexBuffer(const int size, const float* realSource, const float* imagSource) throw()
:	Buffer(BufferSpec(size, 2, false))
{
	float* realSamples = getDataReal();
	float* imagSamples = getDataImag();
	
	if(realSource)
		memcpy(realSamples, realSource, size_ * sizeof(float));
	else
		memset(realSamples, 0, size_ * sizeof(float));
	
	if(imagSource)
		memcpy(imagSamples, imagSource, size_ * sizeof(float));
	else
		memset(imagSamples, 0, size_ * sizeof(float));
}

ComplexBuffer::ComplexBuffer(Buffer const& realBuffer) throw()
:	Buffer(BufferSpec(realBuffer.size(), 2, false))
{
	const float* sourceSamples = realBuffer.getData(0); // ignore other channels
	
	float* realSamples = getDataReal();
	float* imagSamples = getDataImag();
	
	memcpy(realSamples, sourceSamples, size_ * sizeof(float));
	memset(imagSamples, 0, size_ * sizeof(float));
}

ComplexBuffer::~ComplexBuffer()
{
}

ComplexBuffer ComplexBuffer::operator+ (ComplexBuffer const& rightOperand) const throw()
{
	ComplexBuffer newBuffer(size_);
	
	const float *leftRealSamples = getDataRealReadOnly();
	const float *leftImagSamples = getDataImagReadOnly();
	const float *rightRealSamples = rightOperand.getDataRealReadOnly();
	const float *rightImagSamples = rightOperand.getDataImagReadOnly();
	float *newRealSamples = newBuffer.getDataReal();
	float *newImagSamples = newBuffer.getDataImag();
	
	int numSamples = size_;
	
	while(numSamples--)
	{
		*newRealSamples++ = *leftRealSamples++ + *rightRealSamples++;
		*newImagSamples++ = *leftImagSamples++ + *rightImagSamples++;
	}
	
	return newBuffer;
}

ComplexBuffer ComplexBuffer::operator- (ComplexBuffer const& rightOperand) const throw()
{
	ComplexBuffer newBuffer(size_);
	
	const float *leftRealSamples = getDataRealReadOnly();
	const float *leftImagSamples = getDataImagReadOnly();
	const float *rightRealSamples = rightOperand.getDataRealReadOnly();
	const float *rightImagSamples = rightOperand.getDataImagReadOnly();
	float *newRealSamples = newBuffer.getDataReal();
	float *newImagSamples = newBuffer.getDataImag();
	
	int numSamples = size_;
	
	while(numSamples--)
	{
		*newRealSamples++ = *leftRealSamples++ - *rightRealSamples++;
		*newImagSamples++ = *leftImagSamples++ - *rightImagSamples++;
	}
	
	return newBuffer;
}

ComplexBuffer ComplexBuffer::operator* (ComplexBuffer const& rightOperand) const throw()
{
	ComplexBuffer newBuffer(size_);
	
	const float *leftRealSamples = getDataRealReadOnly();
	const float *leftImagSamples = getDataImagReadOnly();
	const float *rightRealSamples = rightOperand.getDataRealReadOnly();
	const float *rightImagSamples = rightOperand.getDataImagReadOnly();
	float *newRealSamples = newBuffer.getDataReal();
	float *newImagSamples = newBuffer.getDataImag();
	
	int numSamples = size_;
	
	while(numSamples--)
	{
		*newRealSamples++ = (*leftRealSamples * *rightRealSamples) - (*leftImagSamples * *rightImagSamples);
		*newImagSamples++ = (*leftImagSamples * *rightRealSamples) + (*leftRealSamples * *rightImagSamples);
		leftRealSamples++;
		leftImagSamples++;
		rightRealSamples++;
		rightImagSamples++;
	}
		
	return newBuffer;
}

//ComplexBuffer ComplexBuffer::operator/ (ComplexBuffer const& rightOperand) const throw()
//{
//	//(a+ib)/(c+id)=(ac+bd+i(bc-ad))/(c2+d2) 
//	
//	ComplexBuffer newBuffer(size_);
//	
//	float *leftRealSamples = getDataReal();
//	float *leftImagSamples = getDataImag();
//	float *rightRealSamples = rightOperand.getDataReal();
//	float *rightImagSamples = rightOperand.getDataImag();
//	float *newRealSamples = newBuffer.getDataReal();
//	float *newImagSamples = newBuffer.getDataImag();
//	
//	int numSamples = size_;
//	
//	while(numSamples--)
//	{
//		// to do
//	}
//	
//	return newBuffer;
//	
//}
//


BufferSender::BufferSender() throw()
{
}

BufferSender::~BufferSender()
{
	const int size = receivers.size();
	for(int i = 0; i < size; i++)
	{
		receivers[i]->removeBufferSender(this);
	}	
}

void BufferSender::addBufferReceiver(BufferReceiver* receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	if(receivers.contains(receiver)) return;
	
	receivers.add(receiver);
	receiver->addBufferSender(this);
}

void BufferSender::removeBufferReceiver(BufferReceiver* receiver) throw()
{
	if(receiver == 0) { ugen_assertfalse; return; }
	
	receivers.removeItem(receiver);
}

void BufferSender::sendBuffer(Buffer const& buffer, const double value1, const int value2) throw()
{
	const int size = receivers.size();
	for(int i = 0; i < size; i++)
	{
		receivers[i]->handleBuffer(buffer, value1, value2);
	}
}

BufferReceiver::BufferReceiver() throw()
{
}

BufferReceiver::~BufferReceiver()
{	
	const int size = senders.size();
	for(int i = 0; i < size; i++)
	{
		senders[i]->removeBufferReceiver(this);
	}
}

void BufferReceiver::addBufferSender(BufferSender* const sender) throw()
{
	if(sender == 0) return;
	if(senders.contains(sender)) return;
	
	senders.add(sender);
}

void BufferReceiver::removeBufferSender(BufferSender* const sender) throw()
{
	senders.removeItem(sender);
}

END_UGEN_NAMESPACE
