/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-12 The University of the West of England.
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

#ifndef UGENCOMMON_H
#define UGENCOMMON_H

#ifndef UNUSED_NOWARN
	#if defined(JUCE_MAC)
		// enable supression of unused variable is GCC
		#define UNUSED_NOWARN __attribute__((unused))

	#elif defined(JUCE_MSVC)
		#define UNUSED_NOWARN

		// disable unused variable warnings in MSVC (Windows)
		#pragma warning( push )
		#pragma warning( disable : 4705 )
	#else
		#define UNUSED_NOWARN
	#endif
#endif // #ifndef UNUSED_NOWARN

/*
 Use this file to confiure the plugin parameters (sliders), buttons and menu options.
 Make sure the Names[] arrays here match the enums in each of these nested namespaces.
 The enums should all have one extra item at the end "Count".
 Your interface will be built based on this info, so edit it carefully!
 */

#define METER_UPDATE_TIME 0.020
#define MENU_ID_OFFSET 1000

#define UGenAppendNamesInternal(a, b) a ## b
#define UGenAppendNames(a, b) UGenAppendNamesInternal(a, b)

// define the class names ID
//#define UGenPluginID			PROJECT_NAME

// define the class names for the component and plugin
//#define UGenPlugin				UGenAppendNames(UGenPlugin, UGenPluginID)
//#define UGenEditorComponent		UGenAppendNames(UGenEditorComponent, UGenPluginID)
//#define UGenInterface			UGenAppendNames(UGenInterface, UGenPluginID)

#define LINEAR false
#define EXPONENTIAL true
#define NONAUTOMATABLE false
#define AUTOMATABLE true

struct UGenRange
{
	double minimum;
	double maximum;
    double nominal;
	bool warp;		// true for exponential, false for linear
	char units[64];	// use 0 for no units, e.g., "Hz" for freqency
    bool automatable;
};

/**
 Some convenient enums and name arrays for paramteres, buttons etc.
 */
namespace UGenInterface
{
	namespace Parameters
	{
		static const char UNUSED_NOWARN *Names[] = { 
			"Wet", 
			"Dry",
            "Resonance",
            "Speed"
		};
        
        static const char UNUSED_NOWARN *Descriptions[] = {
			"Wet mix control",
			"Dry mix control",
            "Resonance control for the filter applied to the impulse response",
            "Speed control for the playback of the impulse response (normal speed = 1)"
		};
		
		static const UGenRange Ranges[] = {
			{-80, 18, 0, LINEAR, " dB", AUTOMATABLE},
			{-80, 18, 0, LINEAR, " dB", AUTOMATABLE},
            {0.5, 100, 1, EXPONENTIAL, " Q", NONAUTOMATABLE},
            {0.02, 50, 1, EXPONENTIAL, " x", NONAUTOMATABLE}
		};
				
		enum { 
			Wet,
			Dry,
            Resonance,
            Speed,
		
			Count 
		};
	}
	
	namespace Meters
	{
		static const char UNUSED_NOWARN *Names[] = { 
			"Out L", 
			"Out R" 
		};
		
		enum { 
			OutL, 
			OutR,				
		
			Count 
		};
	}
	
	namespace Buttons
	{
		static const char UNUSED_NOWARN *Names[] = { 0
//            "Reset Amp",
//            "Reset Filter"
        };
		
        enum {
//            ResetAmp,
//            ResetFilter,
            
            Count
        };
	}
	
	namespace MenuOptions
	{
		static const char UNUSED_NOWARN *Label = "Filter Type";
		static const char UNUSED_NOWARN *Names[] = {
            "Low Pass",
            "High Pass",
//            "Peak-Notch",
            "Band Pass",
            "Band Reject",
        };
		
		enum {
            LowPass,
            HighPass,
//            PeakNotch,
            BandPass,
            BandReject,
            
            Count
        };
	}
	
}

/**
 Some convenient enums for audio channels.
 */
class UGenAudio
{
public:
	enum InputChannels {
		InputLeft = 0, InputRight,
		
		Input0 = 0, Input1, Input2, Input3,
		Input4, Input5, Input6, Input7,
		Input8, Input9, Input10, Input11,
		Input12, Input13, Input14, Input15
	};
	
	enum OutputChannels {
		OutputLeft = 0, OutputRight,
		
		Output0 = 0, Output1, Output2, Output3,
		Output4, Output5, Output6, Output7,
		Output8, Output9, Output10, Output11,
		Output12, Output13, Output14, Output15
	};
};

/**
 Some utility functions.
 */
class UGenUtility
{
public:
	/**
	 Removes spaces and unsafe chars from a name for XML storage (for example).
	 */
	static String stringToSafeName(const String& input)
	{
		return input
			.replaceCharacter (' ', '_')
			.replaceCharacter ('.', '_')
			.retainCharacters ("abcdefghijklmnopqrstuvwxyz_0123456789"); 
	}
	
};


#if defined(JUCE_WIN32)
	#pragma warning( pop )
#endif


#endif // UGENCOMMON_H