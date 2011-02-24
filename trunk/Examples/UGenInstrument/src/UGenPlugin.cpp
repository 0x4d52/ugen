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
:	eventGenerator(this)
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

const float* UGenPlugin::getParameterPtr (int index) const
{
	return parameters + index;
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

float UGenPlugin::getMappedParameter(int index)
{
	if(getParameterWarp(index))
	{
		return linexp(getParameter(index), 
					  0.f, 1.f, 
					  getParameterMin(index), getParameterMax(index));
	}
	else
	{
		return linlin(getParameter(index), 
					  0.f, 1.f, 
					  getParameterMin(index), getParameterMax(index));		
	}	
}

void UGenPlugin::setMappedParameterNotifyingHost(int index, float newValue)
{
	float normalisedValue;
	
	if(getParameterWarp(index))
	{
		normalisedValue = explin(newValue, 
								 getParameterMin(index), getParameterMax(index),
								 0.f, 1.f);
	}
	else
	{
		normalisedValue = linlin(newValue, 
								 getParameterMin(index), getParameterMax(index),
								 0.f, 1.f);
	}	
	
	setParameterNotifyingHost(index, normalisedValue);
}

UGen UGenPlugin::getMappedParameterControl(int index) const
{
	if(getParameterWarp(index))
	{
		return LinExp::AR(getParameterPtr(index), 
						  0, 1, 
						  getParameterMin(index), getParameterMax(index));
	}
	else
	{
		return LinLin::AR(getParameterPtr(index), 
						  0, 1, 
						  getParameterMin(index), getParameterMax(index));	
	}
}

float UGenPlugin::getParameterMin(int index) const
{
	return (float)UGenInterface::Parameters::Ranges[index].minimum;
}

float UGenPlugin::getParameterMax(int index) const
{
	return (float)UGenInterface::Parameters::Ranges[index].maximum;
}

bool UGenPlugin::getParameterWarp(int index) const
{
	return UGenInterface::Parameters::Ranges[index].warp;
}

const String UGenPlugin::getParameterUnits(int index) const
{
	return UGenInterface::Parameters::Ranges[index].units;
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
    return false;
}

bool UGenPlugin::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool UGenPlugin::acceptsMidi() const
{
    return true;
}

bool UGenPlugin::producesMidi() const
{
    return false;
}

//==============================================================================
void UGenPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{	
	UGen::prepareToPlay(sampleRate, samplesPerBlock);	
	outputUGen = constructGraph();
}

UGen UGenPlugin::constructGraph()
{	
	// get the params and use these as UGens	
	UGen gain = getMappedParameterControl(UGenInterface::Parameters::Gain).lag();
	UGen pan = getMappedParameterControl(UGenInterface::Parameters::Pan).lag();		
	
	// assign a Voicer to voicerUGen - this spawns events for MIDI notes
	voicerUGen = Voicer<UGenInstrumentEvent>::AR(eventGenerator, 
												 getNumOutputChannels(), 
												 1,			// MIDI channel
												 0,			// no limit on number of voices
												 false,		// do not force stolen notes (n/a since previous arg is 0)
												 false);	// do not listen directly to MIDI ports, we're providing buffers of MIDI
	
	return Pan2::AR(voicerUGen, pan, gain);
}

void UGenPlugin::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.	
}

void UGenPlugin::processBlock(AudioSampleBuffer& buffer,
							  MidiBuffer& midiMessages)
{	
	buffer.clear();
	
	int numSamples = buffer.getNumSamples();
	
	voicerUGen.sendMidiBuffer(midiMessages);
		
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
	
	midiMessages.clear();
	
	blockID += numSamples;
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
			setMappedParameterNotifyingHost(UGenInterface::Parameters::Pan, 
											neg(getMappedParameter(UGenInterface::Parameters::Pan)));
			break;
		case UGenInterface::Buttons::Centre:
			setMappedParameterNotifyingHost(UGenInterface::Parameters::Pan, 0);
			break;
		case UGenInterface::Buttons::Left:
			setMappedParameterNotifyingHost(UGenInterface::Parameters::Pan, -1);
			break;
		case UGenInterface::Buttons::Right:
			setMappedParameterNotifyingHost(UGenInterface::Parameters::Pan, 1);
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

