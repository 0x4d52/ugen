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

// NB same name as SC but no derived code, the helper event classes do have derived code though

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_VoicerBase.h"
#include "../basics/ugen_MixUGen.h"
#include "../core/ugen_UGenArray.h"


VoicerBaseUGenInternal::VoicerBaseUGenInternal(const int numChannels, const int numVoices, const bool forcedSteal) throw()
:	SpawnBaseUGenInternal(0, numChannels, 0),
	numVoices_(numVoices),
	stealMode_(StealOldest),
	forcedSteal_(forcedSteal),
	ageCounter(0)
{
	ugen_assert(numChannels > 0);
	ugen_assert(numVoices >= 0);
}

bool VoicerBaseUGenInternal::sendMidiNote(const int midiChannel, 
										  const int midiNote,
										  const int velocity) throw()
{
	const int userData = createUserData(midiChannel, midiNote);
	
	ugen_assert(userData != UGen::defaultUserData);
	ugen_assert(velocity >= 0);

	if(velocity > 0)
	{
		if(numVoices_ > 0)
		{
			const int voicesUsed = countNonstealingVoices();
			if(voicesUsed >= numVoices_)
			{
				UGen stealee = chooseStealee();
					
				if(stealee.isNotNull())
				{
					stealee.userData = stealingUserData;
					stealee.steal(forcedSteal_);
				}
			}
		}
		
		// stop double notes, AU lab was sending two ons but only one off 
		// stealNote(midiChannel, midiNote, false, true);  // let's only do this in the Juce version..
		
		UGen newEvent = spawnEvent(*this, currentEventIndex++, midiChannel, midiNote, velocity);
		newEvent.userData = userData;
		events.add(newEvent);
		events.removeNulls();
	}
	else
	{
		UGen releasee = chooseReleasee(midiChannel, midiNote);
		
		if(releasee.isNotNull())
		{
			//releasee.userData = UGen::defaultUserData; // need to rethink why I really wanted to do this
			releasee.release();
		}
	}
	
	return true;
}

bool VoicerBaseUGenInternal::stealNote(const int midiChannel, 
									   const int midiNote, 
									   const bool forcedSteal,
									   const bool stealAll) throw()
{
	const int userData = createUserData(midiChannel, midiNote);
	bool didSteal = false;
	
	for(int i = 0; i < events.size(); i++)
	{
		UGen& event = events[i];
		if(event.userData == userData)
		{
			event.userData = stealingUserData;
			event.steal(forcedSteal);
			if(stealAll)
				didSteal = true;
			else
				return true;
		}
	}
	
	return didSteal;
}

int VoicerBaseUGenInternal::countNonstealingVoices() const throw()
{
	const int numEvents = events.size();
	int nonStealingVoices = 0;
	
	for(int i = 0; i < numEvents; i++)
	{
		if(events[i].userData != stealingUserData)
			nonStealingVoices++;
	}
	
	return nonStealingVoices;
}

const UGen& VoicerBaseUGenInternal::chooseStealee() throw()
{
	// could use different methods in here to select the voice to steal
	
	const int numEvents = events.size();
	
	for(int i = 0; i < numEvents; i++)
	{
		UGen& stealee = events[i];
		if(stealee.userData != stealingUserData && stealee.isNotNull())
			return stealee;
	}
	
	return UGen::getNull();
}

const UGen& VoicerBaseUGenInternal::chooseReleasee(const int midiChannel, const int midiNote) throw()
{
	ugen_assert(midiChannel == char(midiChannel));
	ugen_assert(midiNote == char(midiNote));

	const int numEvents = events.size();
	
	for(int i = numEvents-1; i >= 0; i--) // release in reverse ?
	{
		UGen& releasee = events[i];
		VoicerUserDataStruct data = getUserData(releasee.userData);
		if((data.midiChannel == midiChannel)
		   && (data.midiNote == midiNote) 
		   // && (releasee.isReleasing() == false // something like this would be useful but tricky to implement
		   )
		{
			return releasee;
		}
	}
	
	return UGen::getNull();
}

int VoicerBaseUGenInternal::createUserData(const int midiChannel, const int midiNote) throw()
{
	ugen_assert(midiChannel == char(midiChannel));
	ugen_assert(midiNote == char(midiNote));

	VoicerUserDataUnion data;
	
	data.i = 0;
	data.s.midiChannel = midiChannel;
	data.s.midiNote = midiNote;
	
	return data.i;
}

void VoicerBaseUGenInternal::parseUserData(const int userData, int& midiChannel, int& midiNote) throw()
{
	VoicerUserDataUnion data;
	
	data.i = userData;
	midiChannel = data.s.midiChannel;
	midiNote = data.s.midiNote;
}

VoicerUserDataStruct VoicerBaseUGenInternal::getUserData(const int userData) throw()
{
	VoicerUserDataUnion data;
	
	data.i = userData;
	return data.s;
}

const int VoicerBaseUGenInternal::stealingUserData = 0x7FFFFFFE; // used to label a voice that is being stolen


END_UGEN_NAMESPACE
