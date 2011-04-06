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
#define UGenPluginID			PROJECT_NAME

// define the class names for the component and plugin
#define UGenPlugin				UGenAppendNames(UGenPlugin, UGenPluginID)
#define UGenEditorComponent		UGenAppendNames(UGenEditorComponent, UGenPluginID)
#define UGenInterface			UGenAppendNames(UGenInterface, UGenPluginID)

#define LINEAR false
#define EXPONENTIAL true

struct UGenRange
{
	double minimum;
	double maximum;
	bool warp;		// true for exponential, false for linear
	char units[64];	// use 0 for no units, e.g., "Hz" for freqency
};

/**
 Some convenient enums and name arrays for paramteres, buttons etc.
 */
namespace UGenInterface
{
	namespace Parameters
	{
		static const char UNUSED_NOWARN *Names[] = { 
			"Gain", 
			"Pan",
			"Cut-off" 
		};
		
		static const UGenRange Ranges[] = {
			{0, 1, LINEAR, ""},
			{-1, 1, LINEAR, ""},
			{50, 18000, EXPONENTIAL, "Hz"}
		};
				
		enum { 
			Gain, 
			Pan, 
			Cutoff,	
		
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
		static const char UNUSED_NOWARN *Names[] = { 
			"Swap", 
			"Centre", 
			"Left", 
			"Right" 
		};
		
		enum { 
			Swap, 
			Centre, 
			Left, 
			Right,			
		
			Count 
		};
	}
	
	namespace MenuOptions
	{
		static const char UNUSED_NOWARN *Label = "Pan Law";
		static const char UNUSED_NOWARN *Names[] = { 
			"-6dB linear pan", 
			"-3dB constant power pan" 
		};
		
		enum { 
			LinearPan, 
			ConstantPowerPan, 
		
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
			.retainCharacters (T("abcdefghijklmnopqrstuvwxyz_0123456789")); 
	}
	
};


#if defined(JUCE_WIN32)
	#pragma warning( pop )
#endif


#endif // UGENCOMMON_H