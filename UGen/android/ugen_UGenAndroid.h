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

#ifndef UGEN_ANDROID_H
#define UGEN_ANDROID_H


#define GenerateMangledName0(a,b) a ## _ ## b
#define GenerateMangledName(a,b) GenerateMangledName0(a,b)

#define DEFAULTSR 44100.0
#define DEFAULTBLOCKSIZE 512

#include <pthread.h>

class AndroidIOHost
{
public:
	AndroidIOHost(const double sampleRate = 44100.0, const int numInputs = 2, const int numOutputs = 2, const int preferredBufferSize = 0) throw();
	virtual ~AndroidIOHost();
	
	void init() throw();
	int processBlock(const int bufferLength, short *shortBuffer) throw();
	int processBlockOutputOnly(const int bufferLength, short *shortBuffer) throw();
	
	void lock() throw();
	void unlock() throw();
	bool tryLock() throw();

	virtual UGen constructGraph(UGen const& input) throw();
	virtual int sendTrigger(const int index) throw() { return 0; }
	virtual int setParameter(const int index, const float value) throw() { return 0; }
	virtual int sendBytes(const int index, const int size, const char* data) throw() { return 0; }
	
private:
	const double sampleRate;
	const int numInputs, numOutputs;
	const int blockSize;
	float *floatBuffer;
	int currentBlockID;
	
	pthread_mutex_t mutex;
	
	UGen output;
	UGen input;
};
		
	
#endif // UGEN_ANDROID_H
