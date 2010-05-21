// $Id: ugen_AudioFilePlayer.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/io/ugen_AudioFilePlayer.h $

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


#ifndef UGEN_AUDIOFILEPLAYER_H
#define UGEN_AUDIOFILEPLAYER_H

/** 
 A wrapper class for playing audio files.
 
 Being an AudioTransportSource this controls the playback of a positionable audio stream, 
 handling the starting/stopping and sample-rate conversion.
 */
class AudioFilePlayer : public AudioTransportSource
{
public:
	
	/** Constuct an empty AudioFilePlayer.
	 */
	AudioFilePlayer();
	
	/** Constuct a new AudioFilePlayer with a given path.
	 
	 @param		path	The path of the audio file.
	 */
	AudioFilePlayer(const String& path);
	AudioFilePlayer(const File& audioFile);
	
	~AudioFilePlayer();
	
	/** Play the audio file from the start.
	 */
	void startFromZero();
	
	/** Open and get ready to play a given audio file path.
	 */
	bool setFile(const File& audioFile, const int readAheadBufferSize = 32768);
	const File& getFile();

	juce_UseDebuggingNewOperator
	
private:	
	AudioFormatReader* audioFormatReaderFromFile(const File& audioFile);
	
	// this is the actual stream that's going to read from the audio file.
	AudioFormatReaderSource* currentAudioFileSource;
	
	File currentFile;
	
};

#endif // UGEN_AUDIOFILEPLAYER_H