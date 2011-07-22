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


#ifndef _UGEN_ugen_JuceMIDIInputBroadcaster_H_
#define _UGEN_ugen_JuceMIDIInputBroadcaster_H_

#include "../../filters/control/ugen_Lag.h"

#define DeclareJuceMIDIDataSourceCommonFunctions(CLASSNAME,							\
												 CONSTRUCTOR_ARGS_CALL,				\
												 CONSTRUCTOR_ARGS_DECLARE,			\
												 LAGTIME_ARG_CALL,					\
												 CONSTRUCTOR_ARGS_DECLARE_UGEN)		\
		CLASSNAME CONSTRUCTOR_ARGS_DECLARE throw();																	\
		CLASSNAME init CONSTRUCTOR_ARGS_DECLARE throw()			{ return CLASSNAME CONSTRUCTOR_ARGS_CALL; }			\
																													\
		static UGen KR CONSTRUCTOR_ARGS_DECLARE_UGEN throw()														\
		{																											\
			return CLASSNAME CONSTRUCTOR_ARGS_CALL.krInternal(LAGTIME_ARG_CALL);									\
		}																											\
																													\
		static UGen AR CONSTRUCTOR_ARGS_DECLARE_UGEN throw()														\
		{																											\
			return Lag(CLASSNAME CONSTRUCTOR_ARGS_CALL, LAGTIME_ARG_CALL);											\
		}																											\
																													\
		void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw()


#define DeclareJuceMIDIDataSourceNoDefault(CLASSNAME,								\
										   CONSTRUCTOR_ARGS_CALL,					\
										   CONSTRUCTOR_ARGS_DECLARE,				\
										   LAGTIME_ARG_CALL,						\
										   CONSTRUCTOR_ARGS_DECLARE_UGEN)			\
	class CLASSNAME : public ExternalControlSource																	\
	{																												\
	public:																											\
		DeclareJuceMIDIDataSourceCommonFunctions(CLASSNAME,															\
												 CONSTRUCTOR_ARGS_CALL,												\
												 CONSTRUCTOR_ARGS_DECLARE,											\
												 LAGTIME_ARG_CALL,													\
												 CONSTRUCTOR_ARGS_DECLARE_UGEN);									\
	}


#define DeclareJuceMIDIDataSource(CLASSNAME,										\
								  CONSTRUCTOR_ARGS_CALL,							\
								  CONSTRUCTOR_ARGS_DECLARE,							\
								  LAGTIME_ARG_CALL,									\
								  CONSTRUCTOR_ARGS_DECLARE_UGEN)					\
	class CLASSNAME : public ExternalControlSource																	\
	{																												\
	public:																											\
		CLASSNAME () throw() : ExternalControlSource() { }															\
		DeclareJuceMIDIDataSourceCommonFunctions(CLASSNAME,															\
												 CONSTRUCTOR_ARGS_CALL,												\
												 CONSTRUCTOR_ARGS_DECLARE,											\
												 LAGTIME_ARG_CALL,													\
												 CONSTRUCTOR_ARGS_DECLARE_UGEN);									\
	}

/** Listens for MIDI data broadcasted by the JuceMIDIInputBroadcaster.
 @see JuceMIDIInputBroadcaster
 */
class JuceMIDIInputListener
{
public:
	JuceMIDIInputListener(const bool activate = true) throw();
	virtual ~JuceMIDIInputListener();// throw();
	virtual void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) = 0;
	
private:
	const bool active;
};

/** Broadcasts received MIDI data to its listeners.
 This sits in the background receiving MIDI data from all sources and
 broadcasts it to all listeners. 
 
 Since this is a singleton class, commonly you would inherit from JuceMIDIInputListener and do something like:
 @code JuceMIDIInputBroadcaster::getInstance()->addListener(dynamic_cast<JuceMIDIInputListener*> (this)); @endcode
 ..in its constuctor. Your class would then receive JuceMIDIInputListener::handleIncomingMidiMessage() callbacks
 when MIDI was received.
 
 Be sure to remove a listener before it is deleted.
 
 @see JuceMIDIInputListener
 */
class JuceMIDIInputBroadcaster : public MidiInputCallback
{
public:
	JuceMIDIInputBroadcaster() throw();
	~JuceMIDIInputBroadcaster() throw();
	
	juce_DeclareSingleton(JuceMIDIInputBroadcaster, false);
	
	void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw();
	void addListener (JuceMIDIInputListener* const newListener) throw();
	void removeListener (JuceMIDIInputListener* const listener) throw();
	void sendIncomingMidiMessage(MidiInput* source, const MidiMessage& message) throw();
	
	const Array<MidiInput*>& getMidiInputs() throw()	{ return midiInputs;		}
	MidiInput* getMidiInput(const int index) throw()	{ return midiInputs[index]; }
	
private:
	Array<MidiInput*> midiInputs;
	SortedSet <void*> midiInputListeners;
};

#endif // _UGEN_ugen_JuceMIDIInputBroadcaster_H_
