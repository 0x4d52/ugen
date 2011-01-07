/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.
   Modified 2008 Martin Robinson (UWE)

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "JucePluginCharacteristics.h"
#include "includes.h"
#include "UGenPlugin.h"
#include "UGenEditorComponent.h"


//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UGenPlugin();
}

//==============================================================================
UGenPlugin::UGenPlugin()
{
	UGen::initialise();
	
	parameters = new float[UGenInterface::Parameters::Count];
	for(int index = 0; index < UGenInterface::Parameters::Count; index++)
	{
		parameters[index] = 0.f;
	}
	
	meterLevels = new float[UGenInterface::Meters::Count];
	for(int index = 0; index < UGenInterface::Meters::Count; index++)
	{
		meterLevels[index] = 0.f;
	}
	
	channelLevel0 = channelLevel1 = 0.f;
	numMeasurements = 0;
	
	menuItem = 0;
	blockID = 0;
}

UGenPlugin::~UGenPlugin()
{
	delete [] parameters;
	parameters = 0;
	delete [] meterLevels;
	meterLevels = 0;
	
	UGen::shutdown();
}

//==============================================================================
const String UGenPlugin::getName() const
{
	// This #define is in "JucePluginCharacteristics.h"
    return JucePlugin_Name;
}

int UGenPlugin::getNumParameters()
{
    return UGenInterface::Parameters::Count;
}

float UGenPlugin::getParameter (int index)
{
    return parameters[index];
}

void UGenPlugin::setParameter (int index, float newValue)
{
	if(parameters[index] != newValue)
	{
		parameters[index] = newValue;
		
		// if this is changing a parameter, broadcast a change message which
		// our editor will pick up.
		sendChangeMessage();
	}
}

const String UGenPlugin::getParameterName (int index)
{
	return String(UGenInterface::Parameters::Names[index]);
}

const String UGenPlugin::getParameterText (int index)
{
	return String (parameters[index], 2);
}

const String UGenPlugin::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String UGenPlugin::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool UGenPlugin::isInputChannelStereoPair (int index) const
{
    return true;
}

bool UGenPlugin::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool UGenPlugin::acceptsMidi() const
{
    return false;
}

bool UGenPlugin::producesMidi() const
{
    return false;
}

//==============================================================================
void UGenPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // do your pre-playback setup stuff here..
	// create filters here... for example since we now have the sample rate
	
	UGen::prepareToPlay(sampleRate, samplesPerBlock);
	inputBuffer = Buffer::newClear(samplesPerBlock, getNumInputChannels(), true);
	
	int numChannels = getNumInputChannels();
	
	DBG(String("numChannels = ")+String(numChannels));
	
	inputUGen = AudioIn::AR(numChannels);//getNumInputChannels());	
	outputUGen = constructGraph(inputUGen);
}

UGen UGenPlugin::constructGraph(UGen const& input)
{
	// for most things you'll just need to edit this function and the UGenCommon.h file
	
	// get pointers to the params
	float *gain = parameters + UGenInterface::Parameters::Gain;
	float *pan = parameters + UGenInterface::Parameters::Pan;
	float *cutoffLinear = parameters + UGenInterface::Parameters::Cutoff;
	
	// map the 0-1 range to exponential for the frequency
	UGen cutoff = LinExp::AR(cutoffLinear, 0, 1, 50, 18000);
	cutoff = cutoff.lag();
	
	// filter the input signal
	UGen filter = LPF::AR(input, cutoff);
	
	// map the 0-1 control to -1...+1
	UGen panControl = UGen(pan) * 2 - 1; // or you could use LinLin
	panControl = panControl.lag();
	
	UGen gainControl = gain;
	gainControl = gainControl.lag();
	
	// create the two pans
	UGen constantPan = Pan2::AR(filter, panControl, gainControl);
	UGen linearPan = LinPan2::AR(filter, panControl, gainControl);
	
	// use the menu as control
	UGen panSelector = &menuItem;
	
	// wrap the pans in a Pause to make sure only one uses cpu
	constantPan = Pause::AR(constantPan, panSelector);
	linearPan = Pause::AR(linearPan, Invert::AR(panSelector));
		
	// mix the pans for the output
	UGen mix = constantPan + linearPan;
	return mix;
}

void UGenPlugin::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.	
}

void UGenPlugin::processBlock(AudioSampleBuffer& buffer,
							  MidiBuffer& midiMessages)
{	
	clearExtraChannels(buffer); // see below
	
	int numSamples = buffer.getNumSamples();
	
	if(inputBuffer.size() < numSamples)
		inputBuffer = Buffer::newClear(numSamples, getNumInputChannels(), true);
	
	for(int i = 0; i < getNumInputChannels(); i++)
	{
		float *inputData = inputBuffer.getData(i);
		memcpy(inputData, buffer.getSampleData(i), numSamples * sizeof(float));
		inputUGen.setInput(inputData, numSamples, i);
	}
	
	const int numOutputChannels = jmin(getNumOutputChannels(), outputUGen.getNumChannels());
	for(int i = 0; i < numOutputChannels; i++)
	{
		outputUGen.setOutput(buffer.getSampleData(i), numSamples, i);
	}
	
	outputUGen.prepareAndProcessBlock(numSamples, blockID, -1);
	
	// quick and dirty metering...
	channelLevel0 += buffer.getRMSLevel(UGenAudio::Output0, 0, buffer.getNumSamples());
	channelLevel1 += buffer.getRMSLevel(UGenAudio::Output1, 0, buffer.getNumSamples());
	
	numMeasurements++;
	if(numMeasurements >= 4)
	{
		setMeterLevel(UGenInterface::Meters::OutL, channelLevel0);
		setMeterLevel(UGenInterface::Meters::OutR, channelLevel1);
		channelLevel0 = channelLevel1 = 0.f;
		numMeasurements = 0;
    }
	
	blockID += numSamples;
}

void UGenPlugin::clearExtraChannels(AudioSampleBuffer& buffer)
{
	// in case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
	// in addition, the output UGen might have fewer channels than the number of 
	// outputs the plug-in has
	
	const int numOutputChannels = jmin(getNumOutputChannels(), outputUGen.getNumChannels());
	for (int i = getNumInputChannels(); i < numOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

void UGenPlugin::setMeterLevel(int index, float value)
{
	ScopedLock sl(getCallbackLock());
	meterLevels[index] = value;
}

void UGenPlugin::buttonClicked(int buttonIndex)
{
	switch(buttonIndex)
	{
		case UGenInterface::Buttons::Swap:
			setParameterNotifyingHost(UGenInterface::Parameters::Pan, 
									  1.f-getParameter(UGenInterface::Parameters::Pan));
			break;
		case UGenInterface::Buttons::Centre:
			setParameterNotifyingHost(UGenInterface::Parameters::Pan, 0.5f);
			break;
		case UGenInterface::Buttons::Left:
			setParameterNotifyingHost(UGenInterface::Parameters::Pan, 0.f);
			break;
		case UGenInterface::Buttons::Right:
			setParameterNotifyingHost(UGenInterface::Parameters::Pan, 1.f);
			break;
	}
}

void UGenPlugin::setMenuItem(int menuItemIndex)
{
	menuItem = menuItemIndex;
}

int UGenPlugin::getMenuItem()
{
	return menuItem;
}

//==============================================================================
AudioProcessorEditor* UGenPlugin::createEditor()
{
    return new UGenEditorComponent (this);
}

//==============================================================================
void UGenPlugin::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
	const String pluginName = UGenUtility::stringToSafeName(T(JucePlugin_Name));
	
    XmlElement xmlState (pluginName);

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
	
	for(int index = 0; index < UGenInterface::Parameters::Count; index++)
	{
		String parameterName = UGenUtility::stringToSafeName(String(UGenInterface::Parameters::Names[index]));
		parameterName += T("_");
		parameterName += String(index);		// try to ensure the name is unique, not foolproof
		xmlState.setAttribute (parameterName, parameters[index]);
	}
	xmlState.setAttribute (T("menuItem"), menuItem);
	
    // you could also add as many child elements as you need to here..

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void UGenPlugin::setStateInformation (const void* data, int sizeInBytes)
{
	const String pluginName = UGenUtility::stringToSafeName(T(JucePlugin_Name));
	
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (pluginName))
        {
            // ok, now pull out our parameters..
			for(int index = 0; index < UGenInterface::Parameters::Count; index++)
			{
				String parameterName = UGenUtility::stringToSafeName(String(UGenInterface::Parameters::Names[index]));
				parameterName += T("_");
				parameterName += String(index);		// try to ensure the name is unique, not foolproof
				parameters[index] = (float) xmlState->getDoubleAttribute (parameterName, parameters[index]);
			}
			menuItem = xmlState->getIntAttribute(T("menuItem"), menuItem);
			
            sendChangeMessage ();
        }

        delete xmlState;
    }
}

