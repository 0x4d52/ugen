// $Id: ugen_DiskIn.cpp 980 2010-01-15 21:59:10Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/io/ugen_DiskIn.cpp $

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

#include "ugen_DiskIn.h"


DiskInUGenInternal::DiskInUGenInternal(File const& file, const int numChannels, bool loopFlag, const double startTime, const int numFrames) throw()
:	ProxyOwnerUGenInternal(0, numChannels - 1),
	file_(file),
	loopFlag_(loopFlag),
	startTime_(startTime < 0.0 ? 0.0 : startTime)
{	
	ugen_assert(numChannels > 0);
	ugen_assert(startTime == startTime_);
	ugen_assert(numFrames > 0);
	
	bufferData = new float*[numChannels];
	
	filePlayer.addChangeListener(this);
	filePlayer.setFile(file, numFrames);
	filePlayer.prepareToPlay(UGen::getEstimatedBlockSize(), UGen::getSampleRate());
	
	filePlayer.setPosition(startTime);
	filePlayer.start();
}

DiskInUGenInternal::~DiskInUGenInternal() throw()
{
	filePlayer.releaseResources();
	filePlayer.removeChangeListener(this);
	
	delete [] bufferData;
}


void DiskInUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numChannels = getNumChannels();
	int blockSize = uGenOutput.getBlockSize();
	for(int i = 0; i < numChannels; i++)
	{
		bufferData[i] = proxies[i]->getSampleData();
	}
	
	AudioSampleBuffer audioSampleBuffer(bufferData, numChannels, blockSize);
	
	AudioSourceChannelInfo info;
	info.buffer			= &audioSampleBuffer;
	info.numSamples		= blockSize;
	info.startSample	= 0;
	
	if(filePlayer.isPlaying())
		filePlayer.getNextAudioBlock(info);
	else
		info.clearActiveBufferRegion();
}

void DiskInUGenInternal::changeListenerCallback (void*)
{
	if(filePlayer.isPlaying() == false && loopFlag_ == true)
	{
		filePlayer.setPosition(startTime_);
		filePlayer.start();
	}
		
}



DiskIn::DiskIn(File const& file, bool loopFlag, const double startTime, const int numFrames) throw()
{	
	initWithJuceFile(file, loopFlag, startTime, numFrames);
}

DiskIn::DiskIn(String const& path, bool loopFlag, const double startTime, const int numFrames) throw()
{
	File file(path);
	initWithJuceFile(file, loopFlag, startTime, numFrames);
}

void DiskIn::initWithJuceFile(File const& file, bool loopFlag, const double startTime, const int numFrames) throw()
{
	AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	
	AudioFormatReader* reader = formatManager.createReaderFor (file);
	int numChannels = reader->numChannels;
	delete reader;
	
	initInternal(numChannels);
	generateFromProxyOwner(new DiskInUGenInternal(file, numChannels, loopFlag, startTime, numFrames));	
}


END_UGEN_NAMESPACE

#endif