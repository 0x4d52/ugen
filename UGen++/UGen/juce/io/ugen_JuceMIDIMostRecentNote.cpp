// $Id: ugen_JuceMIDIMostRecentNote.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/io/ugen_JuceMIDIMostRecentNote.cpp $

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

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceMIDIMostRecentNote.h"


MIDIMostRecentNoteInternal::MIDIMostRecentNoteInternal(const int midiChannel,
													   const float minVal, const float maxVal,
													   const ExternalControlSource::Warp warp, void* port) throw()
:	ExternalControlSourceInternal(minVal, maxVal, warp),
	midiChannel_(midiChannel),
	port_(port)
{
}

void MIDIMostRecentNoteInternal::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw()
{	
	if(port_ != 0 && port_ != source)						return;
	if(message.isForChannel(midiChannel_) == false)			return;
	if(message.isNoteOn() == false)							return;
	
	setNormalisedValue(message.getNoteNumber() / 127.f);
}

MIDIMostRecentNote::MIDIMostRecentNote(const int midiChannel,
									   const float minVal, const float maxVal, 
									   const ExternalControlSource::Warp warp, 
									   void* port) throw()
{
	internal = new MIDIMostRecentNoteInternal(midiChannel, minVal, maxVal, warp, port);
}


END_UGEN_NAMESPACE

#endif
