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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_NSUtilities_H_
#define _UGEN_ugen_NSUtilities_H_

#include "../core/ugen_Text.h"

class NSUtilities
{
public:
	enum Locations
	{
		Bundle,		///< In the \<MyApp\>.app/ bundle, files can be placed here during compilation in Xcode.
		Documents,	///< The application's Documents directory, files here will be backed up during iTunes sync.
		Temporary	///< The application's tmp directory, NB this is cleared when the device is restarted.
	};
	
	/** Returns the absolute path of a file on the iPhone.
	 NB Paths requested for files in the Bundle MUST exist (as this is read only) otherwise 
	 empty Text is returned. This may be used for creating paths to files in the application's
	 /Documents or /tmp directories for reading and/or writing. */
	static Text pathInDirectory(Locations location, const char *filename) throw();
	
	static Text stringToText(CFStringRef string) throw();

};


inline int littleEndian24Bit (const char* const bytes) throw()                          
{ 
	return (((int) bytes[2]) << 16) | (((UInt32) (UInt8) bytes[1]) << 8) | ((UInt32) (UInt8) bytes[0]); 
}

inline int bigEndian24Bit (const char* const bytes) throw()                             
{ 
	return (((int) bytes[0]) << 16) | (((UInt32) (UInt8) bytes[1]) << 8) | ((UInt32) (UInt8) bytes[2]); 
}

inline SInt16 bigEndian16Bit (const char* const bytes) throw()                             
{ 
	return (((int) bytes[0]) << 8) | (((UInt16) (UInt8) bytes[1])); 
}

inline SInt32 bigEndian32Bit (const char* const bytes) throw()                             
{ 
	return	(((int) bytes[0]) << 24) | 
			(((UInt32) (UInt8) bytes[1]) << 16) |
			(((UInt32) (UInt8) bytes[2]) << 8)	|
			(((UInt32) (UInt8) bytes[3])); 
}

inline float bigEndianFloat (const char* const bytes) throw()                             
{ 
	UInt32 temp =	(((UInt32) (UInt8) bytes[0]) << 24) | 
					(((UInt32) (UInt8) bytes[1]) << 16) |
					(((UInt32) (UInt8) bytes[2]) << 8)	|
					(((UInt32) (UInt8) bytes[3])); 
	
	return *((float*)(&temp));
}


#endif // _UGEN_ugen_NSUtilities_H_
