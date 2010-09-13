// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-9 by Martin Robinson www.miajo.co.uk
 
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

#ifndef _UGEN_UGenInstrumentEvent_H_
#define _UGEN_UGenInstrumentEvent_H_


#include "../../../UGen/UGen.h"
#include "UGenCommon.h"

class UGenPlugin;

class UGenInstrumentEvent : public VoicerEvent <UGenPlugin>
{
public:
	UGenInstrumentEvent(UGenPlugin* owner);
	UGen spawnEvent(VoicerUGenInternal& voicer, 
					const int eventCount,
					const int midiChannel,
					const int midiNote,
					const int velocity);
		
private:
	UGenInstrumentEvent();
};


#endif // _UGEN_UGenInstrumentEvent_H_
