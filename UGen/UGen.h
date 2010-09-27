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

/*
 Current line count (27/9/2010)

 cloc --force-lang="Objective C",mm .
 
 254 text files.
 254 unique files.                                          
 760 files ignored.
 
 http://cloc.sourceforge.net v 1.52  T=1.0 s (254.0 files/s, 65680.0 lines/s)
 -------------------------------------------------------------------------------
 Language                     files          blank        comment           code
 -------------------------------------------------------------------------------
 C++                            108           5989           4106          20297
 C/C++ Header                   138           6640           7537          18067
 Objective C                      8            701            378           1965
 -------------------------------------------------------------------------------
 SUM:                           254          13330          12021          40329
 -------------------------------------------------------------------------------
 */

#ifndef UGEN_H
#define UGEN_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "core/ugen_StandardHeader.h"


BEGIN_UGEN_NAMESPACE


#ifdef UGEN_SCSTYLE
	#warning UGEN_SCSTYLE: SC style was too complex to maintain and support has been removed
#endif

#ifdef UGEN_NOEXTGPL
	#warning UGEN_NOEXTGPL: any linker errors might be due to your use of externally GPL'd code
	#ifdef UGEN_JUCE
		#warning UGEN_NOEXTGPL: NB that Juce is GPL unless you have a closed source license!
	#endif
#endif

#include "core/ugen_UGen.h"
#include "core/ugen_UGenInternal.h"
#include "core/ugen_UGenArray.h"
#include "core/ugen_Constants.h"
#include "core/ugen_Random.h"
#include "core/ugen_Bits.h"
#include "core/ugen_Value.h"
#include "core/ugen_Arrays.h"
#include "core/ugen_Collections.h"
#include "core/ugen_TextFile.h"
#include "basics/ugen_ScalarUGens.h"
#include "basics/ugen_UnaryOpUGens.h"
#include "basics/ugen_BinaryOpUGens.h"
#include "basics/ugen_MixUGen.h"
#include "basics/ugen_Plug.h"
#include "basics/ugen_RawInputUGens.h"
#include "basics/ugen_MappingUGens.h"
#include "basics/ugen_Temporary.h"
#include "basics/ugen_Pause.h"
#include "basics/ugen_MulAdd.h"
#include "basics/ugen_Thru.h"
#include "basics/ugen_Chain.h"
#include "basics/ugen_WrapFold.h"
#include "envelopes/ugen_Lines.h"
#include "envelopes/ugen_Env.h"
#include "envelopes/ugen_EnvGen.h"
#include "envelopes/ugen_ASR.h"
#include "analysis/ugen_Amplitude.h"
#include "analysis/ugen_Maxima.h"
#include "analysis/ugen_Poll.h"
#include "analysis/ugen_Schmidt.h"
#include "analysis/ugen_Trig.h"
#include "analysis/ugen_TrigProcess.h"
#include "analysis/ugen_DataRecorder.h"
#include "pan/ugen_BasicPan.h"
#include "buffers/ugen_Buffer.h"
#include "buffers/ugen_PlayBuf.h"
#include "buffers/ugen_XFadePlayBuf.h"
#include "oscillators/ugen_OscillatorUGens.h"
#include "oscillators/simple/ugen_LFSaw.h"
#include "oscillators/simple/ugen_LFPulse.h"
#include "oscillators/simple/ugen_Impulse.h"
#include "oscillators/simple/ugen_FSinOsc.h"
#include "oscillators/simple/ugen_Triggers.h"
#include "oscillators/wavetable/ugen_TableOsc.h"
#include "noise/ugen_WhiteNoise.h"
#include "noise/ugen_PinkNoise.h"
#include "noise/ugen_BrownNoise.h"
#include "noise/ugen_Dust.h"
#include "noise/ugen_LFNoise.h"
#include "filters/ugen_SOS.h"
#include "filters/ugen_BEQ.h"
#include "filters/ugen_LeakDC.h"
#include "filters/simple/ugen_LPF.h"
#include "filters/simple/ugen_HPF.h"
#include "filters/control/ugen_Lag.h"
#include "filters/control/ugen_Decay.h"
#include "filters/dynamics/ugen_Normaliser.h"
#include "delays/ugen_Delay.h"
#include "delays/ugen_BlockDelay.h"
#include "spawn/ugen_Spawn.h"
#include "spawn/ugen_TSpawn.h"
#include "spawn/ugen_VoicerBase.h"
#include "spawn/ugen_Textures.h"
#include "gui/ugen_Scope.h"
#include "fft/ugen_FFTEngine.h"
#include "fft/ugen_FFTMagnitude.h"
#include "fft/ugen_FFTMagnitudeSelection.h"
#include "neuralnet/ugen_NeuralNetwork.h"
#include "neuralnet/ugen_NeuralNetworkUGen.h"

#ifdef UGEN_JUCE
	#include "juce/ugen_JuceUtility.h"
	#include "juce/ugen_JuceSlider.h"
	#include "juce/ugen_JuceTimerDeleter.h"
	#include "juce/ugen_JuceVoicer.h"
	#include "juce/gui/ugen_JuceScope.h"
	#include "juce/gui/ugen_JuceMultiSlider.h"
	#include "juce/gui/ugen_JuceEnvelopeComponent.h"
	#include "juce/io/ugen_DiskIn.h"
	#include "juce/io/ugen_DiskOut.h"
	#include "juce/io/ugen_JuceMIDIController.h"
	#include "juce/io/ugen_JuceMIDIMostRecentNote.h"
	#include "juce/io/ugen_JuceMIDIInputBroadcaster.h"
	#include "juce/io/ugen_JuceIOHost.h"
	#ifdef UGEN_CONVOLUTION
		#ifndef WIN32
		END_UGEN_NAMESPACE
			#include <Accelerate/Accelerate.h>
		BEGIN_UGEN_NAMESPACE
		#endif
		#include "convolution/ugen_Convolution.h"
		#include "convolution/ugen_SimpleConvolution.h"
	#endif // UGEN_CONVOLUTION

	#define Value ugen::Value

#elif defined(UGEN_IPHONE)
	// don't include these if we're using Juce on the iPhone
	#include "iphone/ugen_iPhoneAudioFileDiskIn.h"
	#include "iphone/ugen_iPhoneAudioFileDiskOut.h"
	#include "iphone/ugen_UISlider.h"
	#include "iphone/ugen_UIBUtton.h"
	#include "iphone/ugen_AudioQueueIOHost.h"
	#include "iphone/ugen_UIKitAUIOHost.h"
	#include "iphone/ugen_NSDeleter.h"
	#include "iphone/ugen_ScopeView.h"
	
	#ifdef UGEN_VDSP
		#include <Accelerate/Accelerate.h>
	#endif

	#ifdef UGEN_CONVOLUTION
		#include "convolution/ugen_Convolution.h"
		#include "convolution/ugen_SimpleConvolution.h"
	#endif
#endif

#ifdef UGEN_IPHONE
	// include these on the iPhone even if we're using Juce
	#include "iphone/ugen_NSUtilities.h"
	#include "iphone/ugen_UIAcceleration.h"
#endif

/** A debugging function for reporting the sizes of various objects. */
inline static void ugen_reportObjectSizes() throw()
{
	ReportSize(UGen);
	ReportSize(UGenInternal);
	ReportSize(UGenArray);
	ReportSize(UGenArray::Internal);
	ReportSize(Buffer);
	ReportSize(BufferChannelInternal);
	ReportSize(ScalarUGenInternal);
	ReportSize(SmartPointer);
}


END_UGEN_NAMESPACE



#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif


//**************** main page docs *****************//

/** 
 @mainpage UGen Documentation
 
 @section Introduction Introduction
 UGen++ is a audio processing and synthesis framework in C++. It provides oscillators, filters, envelopes and event management etc.
 There are starter projects for Mac (using Juce), Windows (using Juce) and iPhone.
 
 @section Installation Installation
 Check out the source code using SVN using the command:
 @code svn co http://ugen.googlecode.com/svn/trunk/ ugen @endcode
 In Xcode on the Mac for the Juce projects the directory containing Juce (i.e., juce.h) should be defined 
 as a Source Tree "JUCE_SOURCE" such that it can be found using:
 @code #include <juce/juce.h> @endcode
 
 To use the iPhone project this is not necessary, just use the @c iPhone_UGen_AU/UGenRemoteIO.xcodeproj to get started.
 
 @section GettingStarted Getting started
 The best places to start are with the @link Tutorials tutorials @endlink and/or looking through the @link AllUGens available UGen classes @endlink.
 The UGen derived classes (and other classes) are categorised on the main <a href="modules.html">modules</a> page. 
 
 */



//**************** tutorial docs *****************//


/**	@defgroup Tutorials Tutorials main page
	The UGen++ tutorials main page. These tutorials are grouped into sections. Each individual tutorial document here
	relates to a project folder in the UGen++ source tree. Most tutorials are based in Juce (for Mac/Windows). This is because
	the UGen++ code for other platforms will be largely similar. There are also some other platform-specific tutorials
	e.g., the for the iPhone.
		
	- @subpage Tutorials_01 "Tutorials 01: Fundamentals"
	- @subpage Tutorials_02 "Tutorials 02: More complex issues"
	- @subpage Tutorials_03 "Tutorials 03: Juce-specific support"
	- @subpage Tutorials_04 "Tutorials 04: iPhone-specific support"
	- @subpage Tutorials_05 "Tutorials 05: Other platforms"
	- @subpage Tutorials_06 "Tutorials 06: Developing custom UGens"

 */


/** @page Tutorials_01 Tutorials 01: Fundamentals
		
	prev: @ref Tutorials

	- @subpage	Tutorial_01_01	"Tutorial 01.01: An introduction to UGen++"
	- @subpage	Tutorial_01_02	"Tutorial 01.02: Using the JuceIOHost"
	- @subpage	Tutorial_01_03	"Tutorial 01.03: Building more complex UGen graphs"
	- @subpage	Tutorial_01_04	"Tutorial 01.04: More examples"
	- @subpage	Tutorial_01_05	"Tutorial 01.05: Maths examples"
	- TBA 01.06 Oscillators
	- TBA 01.07 Noise and random
	- TBA 01.08 Buffers
	- @subpage	Tutorial_01_09	"Tutorial 01.09: Delay examples"
	- TBA 01.10 Filters and convolution
	- TBA 01.11 Array examples
	- TBA 01.12 Value examples
	- TBA 01.13 Text and text files
	

	next: @ref Tutorials_02	"Tutorials 02: More complex issues"
*/

/** @page Tutorials_02 Tutorials 02: More complex issues
 
	prev: @ref Tutorials_01 "Tutorials 01: Fundamentals"
 
	[ in progress ]
	
	- @subpage	Tutorial_02_01	"Tutorial 02.01: Introduction to event-based UGen classes"
	- TBA 02.02 Chain and Bank
	- TBA 02.03 Envelope examples
	- TBA 02.04 Spawn
	- TBA 02.05 XFadeTexture
	- TBA 02.06 OverlapTexture
	- TBA 02.07 TSpawn
	- TBA 02.08 TrigXFade
	- TBA 02.09 Plug
	- TBA 02.10 FFT intro
	- TBA 02.11 FFT examples
	- TBA 02.12 DataRecorder
	- TBA 02.13 Neural Network
 
 
	next: @ref Tutorials_03	"Tutorials 03: Juce-specific support"
 */

/** @page Tutorials_03 Tutorials 03: Juce-specific support
 
	prev: @ref Tutorials_02 "Tutorials 02: More complex issues"
 
	[ in progress ]
 
	- TBA 03.01 More on the JuceIOHost (addOther and threads)
	- TBA 03.02 DiskIn
	- TBA 03.03 DiskOut
	- TBA 03.04 Sliders
	- TBA 03.05 MultiSlider
	- TBA 03.06 Buttons
	- TBA 03.07 Scope
	- TBA 03.08 MIDI
	- TBA 03.09 Voicer
	- TBA 03.10 Custom UGens by combining other UGens
	
	next: @ref Tutorials_04	"Tutorials 04: iPhone-specific support"
 */

/** @page Tutorials_04 Tutorials 04: iPhone-specific support
 
	prev: @ref Tutorials_03 "Tutorials 03: Juce-specific support"
 
	- TBA 04.01 The UIKitAUIOHost (addOther and threads)
	- TBA 04.02 Sliders
	- TBA 04.03 Using other UIKit controls to control UGens
	- TBA 04.04 Scope
	- TBA 04.05 Accelerometer
	
	next: @ref Tutorials_05	"Tutorials 05: Other platforms"
 */

/** @page Tutorials_05 Tutorials 05: Other platforms
 
	prev: @ref Tutorials_04 "Tutorials 04: iPhone-specific support"
 
	TBA
 
	next: @ref Tutorials_06	"Tutorials 06: Developing custom UGens"
 */

/** @page Tutorials_06 Tutorials 06: Developing custom UGens
 
	prev: @ref Tutorials_05 "Tutorials 05: Other platforms"
 
	TBA
 
 */

#endif // UGEN_H

