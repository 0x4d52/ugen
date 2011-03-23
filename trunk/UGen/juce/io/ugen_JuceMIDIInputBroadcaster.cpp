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
 
 ==============================================================================
 */

#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceMIDIInputBroadcaster.h"

JuceMIDIInputListener::JuceMIDIInputListener(const bool activate) throw()
:	active(activate)
{
	if(active)
	{
		JuceMIDIInputBroadcaster *broadcaster = JuceMIDIInputBroadcaster::getInstance();
		broadcaster->addListener(dynamic_cast<JuceMIDIInputListener*> (this));
	}
}

JuceMIDIInputListener::~JuceMIDIInputListener() //throw()
{
	if(active)
	{
		JuceMIDIInputBroadcaster::getInstance()->removeListener(dynamic_cast<JuceMIDIInputListener*> (this));
	}
}

JuceMIDIInputBroadcaster::JuceMIDIInputBroadcaster() throw()
{
	const int numDevices = MidiInput::getDevices().size();
	for(int i = 0; i < numDevices; i++)
	{
		MidiInput* midiInput = MidiInput::openDevice(i, this);
		midiInput->start();
		midiInputs.add(midiInput);
	}
}

JuceMIDIInputBroadcaster::~JuceMIDIInputBroadcaster() throw()
{
	for(int i = 0; i < midiInputs.size(); i++)
	{
		MidiInput* midiInput = midiInputs.getUnchecked(i);
		midiInput->stop();
		delete midiInput;
	}
}

juce_ImplementSingleton (JuceMIDIInputBroadcaster);

void JuceMIDIInputBroadcaster::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw()
{
	sendIncomingMidiMessage(source, message);
}

void JuceMIDIInputBroadcaster::addListener (JuceMIDIInputListener* const newListener) throw()
{		
	if (newListener != 0)
		midiInputListeners.add (newListener);
}

void JuceMIDIInputBroadcaster::removeListener (JuceMIDIInputListener* const listener) throw()
{		
	if(midiInputListeners.contains(listener))
		midiInputListeners.removeValue(listener);
}

void JuceMIDIInputBroadcaster::sendIncomingMidiMessage(MidiInput* source, const MidiMessage& message) throw()
{
	for (int i = midiInputListeners.size(); --i >= 0;)
	{
		JuceMIDIInputListener* const ml = (JuceMIDIInputListener*) midiInputListeners[i];
		
		if (ml != 0)
		{
			ml->handleIncomingMidiMessage(source, message);
		}
	}
}

END_UGEN_NAMESPACE

#endif