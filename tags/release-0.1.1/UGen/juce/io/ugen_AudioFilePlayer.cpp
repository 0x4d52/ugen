// $Id: ugen_AudioFilePlayer.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/io/ugen_AudioFilePlayer.cpp $

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

#ifdef UGEN_JUCE

BEGIN_UGEN_NAMESPACE

#include "ugen_AudioFilePlayer.h"

AudioFilePlayer::AudioFilePlayer()
:	currentAudioFileSource(0)
{	
}

AudioFilePlayer::AudioFilePlayer(const String& path)
:	currentAudioFileSource(0)
{
	File audioFile(path);
	setFile(audioFile);
}

AudioFilePlayer::AudioFilePlayer(const File& audioFile)
:	currentAudioFileSource(0)
{
	setFile(audioFile);
}

AudioFilePlayer::~AudioFilePlayer()
{
	setSource (0);
	deleteAndZero (currentAudioFileSource);
}

void AudioFilePlayer::startFromZero()
{
	if(currentAudioFileSource == 0) return;
	
	setPosition (0.0);
	start();
}

bool AudioFilePlayer::setFile(const File& audioFile, const int readAheadBufferSize)
{
	stop();
	setSource (0);
	deleteAndZero (currentAudioFileSource);
	
	AudioFormatReader* reader = audioFormatReaderFromFile(audioFile);
	
	if (reader != 0)
	{										
		currentAudioFileSource = new AudioFormatReaderSource (reader, true);
		
		setSource (currentAudioFileSource, readAheadBufferSize, reader->sampleRate);
		
		return true;
	}
	
	return false;
}

const File& AudioFilePlayer::getFile()
{
	return currentFile;
}

AudioFormatReader* AudioFilePlayer::audioFormatReaderFromFile(const File& audioFile)
{	
	AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	
	currentFile = audioFile;
	
	return formatManager.createReaderFor (audioFile);
}

END_UGEN_NAMESPACE

#endif

