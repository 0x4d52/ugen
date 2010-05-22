// $Id: ugen_DiskOut.cpp 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/io/ugen_DiskOut.cpp $

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

#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_DiskOut.h"
#include "../ugen_JuceUtility.h"


DiskOutUGenInternal::DiskOutUGenInternal(File const& file, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
:	ProxyOwnerUGenInternal(NumInputs, input.getNumChannels()-1),
	audioFormatWriter(0),
	bufferData(0),
	numInputChannels(input.getNumChannels())
{
	ugen_assert(bitDepth >= 16);
	
	inputs[Input] = input;
	
	bufferData = new float*[numInputChannels];
	memset(bufferData, 0, numInputChannels * sizeof(float*));
	
	File outputFile(file);
	
	if(outputFile.getFileExtension().isEmpty())
		outputFile = outputFile.withFileExtension(T("wav"));
	
	if(overwriteExisitingFile == true && outputFile.exists())
		outputFile.deleteFile();
	else if(outputFile.exists())
	{
		ugen_assertfalse;
		return;
	}
	
	if(outputFile.hasFileExtension(".wav"))
	{
		WavAudioFormat wavAudioFormat;
		FileOutputStream* fileOutputStream = outputFile.createOutputStream();
		
		if(fileOutputStream)
			audioFormatWriter = wavAudioFormat.createWriterFor(fileOutputStream, 
															   UGen::getSampleRate(), 
															   numInputChannels, 
															   bitDepth, 0, 0);
	}
	else if(outputFile.hasFileExtension(".aif") 
			|| outputFile.hasFileExtension(".aiff"))
	{
		AiffAudioFormat aiffAudioFormat;
		FileOutputStream* fileOutputStream = outputFile.createOutputStream();
		
		if(fileOutputStream)
			audioFormatWriter = aiffAudioFormat.createWriterFor(fileOutputStream, 
																UGen::getSampleRate(), 
																numInputChannels, 
																bitDepth, 0, 0);
	}
		
}

DiskOutUGenInternal::~DiskOutUGenInternal() throw()
{
	delete audioFormatWriter;
	delete [] bufferData;
}

void DiskOutUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	if(audioFormatWriter == 0) return;
	
	int blockSize = uGenOutput.getBlockSize();
	
	for(int i = 0; i < numInputChannels; i++)
	{
		float *inputSamples = inputs[Input].processBlock(shouldDelete, blockID, i);
		float *outputSamples = proxies[i]->getSampleData();
		memcpy(outputSamples, inputSamples, blockSize * sizeof(float));
		bufferData[i] = inputSamples;
	}
		
	// should really buffer this data in larger chunks that the block size before writing
	AudioSampleBuffer audioSampleBuffer(bufferData, numInputChannels, blockSize);
	audioSampleBuffer.writeToAudioWriter(audioFormatWriter, 0, blockSize);
}

DiskOut::DiskOut(File const& file, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
{	
	initWithJuceFile(file, input, overwriteExisitingFile, bitDepth);
}

DiskOut::DiskOut(String const& path, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
{	
	File file(path);
	initWithJuceFile(file, input, overwriteExisitingFile, bitDepth);
}

DiskOut::DiskOut(const File::SpecialLocationType directory, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
{
	initWithJuceFile(File::getSpecialLocation(directory), input, overwriteExisitingFile, bitDepth);
}

void DiskOut::initWithJuceFile(File const& file, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw()
{
	DiskOutUGenInternal* internal;
	
	if(file.isDirectory()) 
	{
		internal = new DiskOutUGenInternal(file.getChildFile(getFileNameWithTimeIdentifier(T("DiskOut"))), 
										   input, 
										   overwriteExisitingFile, 
										   bitDepth);
	}
	else
	{
		internal = new DiskOutUGenInternal(file, input, overwriteExisitingFile, bitDepth);
	}
	
	initInternal(input.getNumChannels());
	generateFromProxyOwner(internal);
}

END_UGEN_NAMESPACE

#endif
