// $Id: ugen_IntBuffer.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/buffers/ugen_IntBuffer.cpp $

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

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_IntBuffer.h"


IntBuffer::IntBuffer() throw()
:	size_(0)
{
}

IntBuffer::IntBuffer(int i00) throw()
:	size_(1)
{
	data[0] = i00;
}

IntBuffer::IntBuffer(int i00,
					 int i01, 
					 int i02, 
					 int i03, 
					 int i04, 
					 int i05, 
					 int i06, 
					 int i07, 
					 int i08, 
					 int i09, 
					 int i10, 
					 int i11, 
					 int i12, 
					 int i13, 
					 int i14, 
					 int i15, 
					 int i16, 
					 int i17, 
					 int i18, 
					 int i19, 
					 int i20, 
					 int i21, 
					 int i22, 
					 int i23, 
					 int i24, 
					 int i25, 
					 int i26, 
					 int i27, 
					 int i28, 
					 int i29, 
					 int i30, 
					 int i31) throw()
{
	int count = 2;
	
	if(i02 == 0x7FFFFFFF) goto init; else count++;
	if(i03 == 0x7FFFFFFF) goto init; else count++;
	if(i04 == 0x7FFFFFFF) goto init; else count++;
	if(i05 == 0x7FFFFFFF) goto init; else count++;
	if(i06 == 0x7FFFFFFF) goto init; else count++;
	if(i07 == 0x7FFFFFFF) goto init; else count++;
	if(i08 == 0x7FFFFFFF) goto init; else count++;
	if(i09 == 0x7FFFFFFF) goto init; else count++;
	if(i10 == 0x7FFFFFFF) goto init; else count++;
	if(i11 == 0x7FFFFFFF) goto init; else count++;
	if(i12 == 0x7FFFFFFF) goto init; else count++;
	if(i13 == 0x7FFFFFFF) goto init; else count++;
	if(i14 == 0x7FFFFFFF) goto init; else count++;
	if(i15 == 0x7FFFFFFF) goto init; else count++;
	if(i16 == 0x7FFFFFFF) goto init; else count++;
	if(i17 == 0x7FFFFFFF) goto init; else count++;
	if(i18 == 0x7FFFFFFF) goto init; else count++;
	if(i19 == 0x7FFFFFFF) goto init; else count++;
	if(i20 == 0x7FFFFFFF) goto init; else count++;
	if(i21 == 0x7FFFFFFF) goto init; else count++;
	if(i22 == 0x7FFFFFFF) goto init; else count++;
	if(i23 == 0x7FFFFFFF) goto init; else count++;
	if(i24 == 0x7FFFFFFF) goto init; else count++;
	if(i25 == 0x7FFFFFFF) goto init; else count++;
	if(i26 == 0x7FFFFFFF) goto init; else count++;
	if(i27 == 0x7FFFFFFF) goto init; else count++;
	if(i28 == 0x7FFFFFFF) goto init; else count++;
	if(i29 == 0x7FFFFFFF) goto init; else count++;
	if(i30 == 0x7FFFFFFF) goto init; else count++;
	if(i31 == 0x7FFFFFFF) goto init; else count++;
	
init:
	
	size_ = count;
		
	data[0] = i00;
	data[1] = i01;
	
	if(i02 == 0x7FFFFFFF) return; else data[ 2] = i02;
	if(i03 == 0x7FFFFFFF) return; else data[ 3] = i03;
	if(i04 == 0x7FFFFFFF) return; else data[ 4] = i04;
	if(i05 == 0x7FFFFFFF) return; else data[ 5] = i05;
	if(i06 == 0x7FFFFFFF) return; else data[ 6] = i06;
	if(i07 == 0x7FFFFFFF) return; else data[ 7] = i07;
	if(i08 == 0x7FFFFFFF) return; else data[ 8] = i08;
	if(i09 == 0x7FFFFFFF) return; else data[ 9] = i09;
	if(i10 == 0x7FFFFFFF) return; else data[10] = i10;
	if(i11 == 0x7FFFFFFF) return; else data[11] = i11;
	if(i12 == 0x7FFFFFFF) return; else data[12] = i12;
	if(i13 == 0x7FFFFFFF) return; else data[13] = i13;
	if(i14 == 0x7FFFFFFF) return; else data[14] = i14;
	if(i15 == 0x7FFFFFFF) return; else data[15] = i15;
	if(i16 == 0x7FFFFFFF) return; else data[16] = i16;
	if(i17 == 0x7FFFFFFF) return; else data[17] = i17;
	if(i18 == 0x7FFFFFFF) return; else data[18] = i18;
	if(i19 == 0x7FFFFFFF) return; else data[19] = i19;
	if(i20 == 0x7FFFFFFF) return; else data[20] = i20;
	if(i21 == 0x7FFFFFFF) return; else data[21] = i21;
	if(i22 == 0x7FFFFFFF) return; else data[22] = i22;
	if(i23 == 0x7FFFFFFF) return; else data[23] = i23;
	if(i24 == 0x7FFFFFFF) return; else data[24] = i24;
	if(i25 == 0x7FFFFFFF) return; else data[25] = i25;
	if(i26 == 0x7FFFFFFF) return; else data[26] = i26;
	if(i27 == 0x7FFFFFFF) return; else data[27] = i27;
	if(i28 == 0x7FFFFFFF) return; else data[28] = i28;
	if(i29 == 0x7FFFFFFF) return; else data[29] = i29;
	if(i30 == 0x7FFFFFFF) return; else data[30] = i30;
	if(i31 == 0x7FFFFFFF) return; else data[31] = i31;
}


END_UGEN_NAMESPACE
