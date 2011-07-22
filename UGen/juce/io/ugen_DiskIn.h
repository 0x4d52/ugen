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

#ifndef _UGEN_ugen_DiskIn_H_
#define _UGEN_ugen_DiskIn_H_


#include "../../core/ugen_UGen.h"
#include "ugen_AudioFilePlayer.h"

/** @ingroup UGenInternals */
class DiskInUGenInternal :	public ProxyOwnerUGenInternal,
							public ChangeListener,
							public DoneActionSender
{
public:
	DiskInUGenInternal(File const& file, 
					   const int numChannels, 
					   bool loopFlag, 
					   const double startTime, 
					   const int numFrames,
					   const UGen::DoneAction doneAction) throw();
	~DiskInUGenInternal() throw();
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	void changeListenerCallback (ChangeBroadcaster*);
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();
	
	
protected:
	
	File file_;
	bool loopFlag_;
	double startTime_;
	AudioFilePlayer filePlayer;
	float** bufferData;
	const UGen::DoneAction doneAction_;
	const bool shouldDeleteValue;	
};


/** Streams a soundfile from disk.
 @ingroup AllUGens SoundFileUGens
 @see PlayBuf, DiskOut */
class DiskIn : public UGen 
{ 
public: 
	DiskIn () throw() : UGen() { } 
	DiskIn (File const& file, 
			bool loopFlag = false, 
			const double startTime = 0.0, 
			const int numFrames = 32768, 
			const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw(); 
	DiskIn (String const& path, 
			bool loopFlag = false, 
			const double startTime = 0.0, 
			const int numFrames = 32768,
			const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw(); 
		
	static inline UGen AR (File const& file, 
						   bool loopFlag = false, 
						   const double startTime = 0.0, 
						   const int numFrames = 32768,
						   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw() 
	{ 
		return DiskIn (file, loopFlag, startTime, numFrames, doneAction); 
	} 	
		
	static inline UGen AR (String const& file, 
						   bool loopFlag = false, 
						   const double startTime = 0.0, 
						   const int numFrames = 32768,
						   const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw() 
	{ 
		return DiskIn (file, loopFlag, startTime, numFrames, doneAction); 
	} 		
		
private:
	void initWithJuceFile(File const& file, 
						  bool loopFlag = false, 
						  const double startTime = 0.0, 
						  const int numFrames = 32768,
						  const UGen::DoneAction doneAction = UGen::DeleteWhenDone) throw();
};



#endif // _UGEN_ugen_DiskIn_H_
