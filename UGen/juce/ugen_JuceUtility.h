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

#ifndef _UGEN_ugen_JuceUtility_H_
#define _UGEN_ugen_JuceUtility_H_

#include "../buffers/ugen_Buffer.h"

/** Get a string which is useful as a time identifier.
 @return The time string. */
inline String getCurrentTimeIdentifier() throw()
{
	String ms = String(Time::getMillisecondCounter() % 1000).paddedLeft(T('0'), 3);
	return Time::getCurrentTime().formatted(T("%Y%m%d_%H%M%S_")) + ms; 
}

/** Append a time identifier to t string useful as a filename.
 @todo Check if there's a extension and insert the identifier before the dot. */
inline String getFileNameWithTimeIdentifier(String const& filename) throw()
{
	return filename + T("_") + getCurrentTimeIdentifier();
}

/** A little Juce class used as a pop-up editor. */
class PopupComponent :	public Component,
						public Timer
{
private:
	int counter;
	const int maxCount; 	
	static int activePopups;
	
public:
	PopupComponent(const int max = 20);	
	~PopupComponent();
	void paint(Graphics &g);	
	void mouseDown(MouseEvent const& e);
	
	static int getActivePopups() { return activePopups; }

protected:
	void resetCounter();	
	void timerCallback();
	void expire();
};

/** Manages Buffer processing on a background thread. */
class BufferProcess :	public Thread,
						public BufferSender
{
public:
	BufferProcess() throw();
	~BufferProcess();
	
	/** Adds a Buffer process to the job array.
	 This processes @c buffer through @c graph (where @c input is at the top) and sends the
	 result to registered BufferReceiver objects using @c bufferID as an ID 
	 (which is passed to the third argument of the handleBuffer() callbacks). 
	 @see Buffer::process(), Buffer::processInPlace() */
	void add(Buffer const& buffer, UGen const& input, UGen const& graph, const int bufferID = 0) throw();
	
	/** Adds a Buffer synthesiser to the job array.
	 This processes @c graph for @c size samples and sends the
	 result to registered BufferReceiver objects using @c bufferID as an ID 
	 (which is passed to the third argument of the handleBuffer() callbacks). 
	 @see Buffer::synth(), Buffer::synthInPlace() */	
	void add(const int size, UGen const& graph, const int bufferID = 0) throw();
	
	
private:	
	void run() throw();
	ObjectArray<Buffer> buffers;
	ObjectArray<UGen> inputs;
	ObjectArray<UGen> graphs;
	IntArray ids;
	CriticalSection lock;
};



class AudioIOHelper
{
public:
	
	static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }
	
	static int64 getChunkPosition(AudioFormatReader* reader, const char* name);
	static CuePointArray getCuePoints(AudioFormatReader* reader);

	
private:
	static int64 getWavChunkPosition(InputStream* input, const char* name);
	static CuePointArray getWavCuePoints(InputStream* input);
	static int64 getAiffChunkPosition(InputStream* input, const char* name);
	static CuePointArray getAiffCuePoints(InputStream* input);

};



#endif // _UGEN_ugen_JuceUtility_H_
