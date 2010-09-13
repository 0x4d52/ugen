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

#include "JucePluginCharacteristics.h"
#include "includes.h"
#include "UGenInstrumentEvent.h"
#include "UGenPlugin.h"

UGenInstrumentEvent::UGenInstrumentEvent(UGenPlugin* owner)
:	VoicerEvent<UGenPlugin>(owner)
{
}

UGen UGenInstrumentEvent::spawnEvent(VoicerUGenInternal& voicer, 
									 const int eventCount,
									 const int midiChannel,
									 const int midiNote,
									 const int velocity)
{
	// an ADSR envelope
	Env env = Env::adsr(0.1, 0.1, 0.7, 2.0, velocity/127.0, EnvCurve::Sine);
	
	// generator for the envelope
	UGen envgen = EnvGen::AR(env);
	
	UGen note = midiNote;
	note += voicer.getPitchWheelUGen() * 2.0; // +-2 semi tone with pitch wheel
	
	// convert midi note to frequency
	UGen freq = midicps(note);
	
	
	// the wave source with the envelope used as amplitude control
	UGen wave = LFSaw::AR(freq, 0.0, envgen);
	
	// get the cutoff control 0..1
	UGen linearCutoff = getOwner()->getParameterPtr(UGenInterface::Parameters::Cutoff);
	
	// map the linear cutoff to a useful frequency range and use the envlope to vary it too
	UGen cutoff = LinExp::AR(linearCutoff * envgen, 
							 0, 1, 
							 freq*2, freq*12).lag();
	
	// apply the filter and use MIDI controller 7 for amplitude control
	return LPF::AR(wave, cutoff) * Lag::AR(voicer.getControllerUGen(7));
}
