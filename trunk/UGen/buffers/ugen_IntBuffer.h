// $Id: ugen_IntBuffer.h 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/buffers/ugen_IntBuffer.h $

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

#ifndef _UGEN_ugen_IntBuffer_H_
#define _UGEN_ugen_IntBuffer_H_

/** Not currently used. */
class IntBuffer
{
public:
	IntBuffer() throw();
	IntBuffer(int i00) throw();
	IntBuffer(int i00,
			  int i01, 
			  int i02 = 0x7FFFFFFF, 
			  int i03 = 0x7FFFFFFF, 
			  int i04 = 0x7FFFFFFF, 
			  int i05 = 0x7FFFFFFF, 
			  int i06 = 0x7FFFFFFF, 
			  int i07 = 0x7FFFFFFF, 
			  int i08 = 0x7FFFFFFF, 
			  int i09 = 0x7FFFFFFF, 
			  int i10 = 0x7FFFFFFF, 
			  int i11 = 0x7FFFFFFF, 
			  int i12 = 0x7FFFFFFF, 
			  int i13 = 0x7FFFFFFF, 
			  int i14 = 0x7FFFFFFF, 
			  int i15 = 0x7FFFFFFF, 
			  int i16 = 0x7FFFFFFF, 
			  int i17 = 0x7FFFFFFF, 
			  int i18 = 0x7FFFFFFF, 
			  int i19 = 0x7FFFFFFF, 
			  int i20 = 0x7FFFFFFF, 
			  int i21 = 0x7FFFFFFF, 
			  int i22 = 0x7FFFFFFF, 
			  int i23 = 0x7FFFFFFF, 
			  int i24 = 0x7FFFFFFF, 
			  int i25 = 0x7FFFFFFF, 
			  int i26 = 0x7FFFFFFF, 
			  int i27 = 0x7FFFFFFF, 
			  int i28 = 0x7FFFFFFF, 
			  int i29 = 0x7FFFFFFF, 
			  int i30 = 0x7FFFFFFF, 
			  int i31 = 0x7FFFFFFF) throw();
	
	int* getData() throw()		{ return data;		}
	int size() const throw()	{ return size_;		}
	
private:
	
	int data[32];
	unsigned int size_;
};

#endif // _UGEN_ugen_IntBuffer_H_
