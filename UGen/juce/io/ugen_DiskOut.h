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

#ifndef _UGEN_ugen_DiskOut_H_
#define _UGEN_ugen_DiskOut_H_


#include "../../core/ugen_UGen.h"

/** @ingroup UGenInternals */
class DiskOutUGenInternal : public ProxyOwnerUGenInternal
{
public:
	DiskOutUGenInternal(File const& file, UGen const& input, bool overwriteExisitingFile, int bitDepth) throw();
	~DiskOutUGenInternal() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
protected:	
	AudioFormatWriter* audioFormatWriter;
	float** bufferData;
	int numInputChannels;
};

/** Streams data from a UGen out to a disk file. 
 @ingroup AllUGens SoundFileUGens 
 @see DiskIn */
class DiskOut : public UGen 
{ 
public: 
	
	DiskOut () throw() : UGen() { } 
	DiskOut (File const& file, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24) throw(); 
	DiskOut (String const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24) throw(); 
	DiskOut (const File::SpecialLocationType directory, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24) throw(); 
		
	static inline UGen AR (File const& file, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (file, input, overwriteExisitingFile, bitDepth); 
	}
		
	static inline UGen AR (String const& path, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (path, input, overwriteExisitingFile, bitDepth); 
	}
		
	static inline UGen AR (const File::SpecialLocationType directory, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24)  throw()
	{ 
		return DiskOut (directory, input, overwriteExisitingFile, bitDepth); 
	}
	
	
private:
	void initWithJuceFile(File const& file, UGen const& input, bool overwriteExisitingFile = false, int bitDepth = 24) throw(); 

};



#endif // _UGEN_ugen_DiskOut_H_
