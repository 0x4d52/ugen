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


#include "UGenPlugin.h"
#include "UGenEditorComponent.h"


#define UGENIR_IRREFRESHTIME 400

//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UGenPlugin();
}

static Env getDefaultEnv()
{
    return Env(Buffer(1.0, 1.0), Buffer(1.0), EnvCurve::Linear);
}

//==============================================================================
UGenPlugin::UGenPlugin()
{
	UGen::initialise();
	
	parameters = new float[UGenInterface::Parameters::Count];
	for(int index = 0; index < UGenInterface::Parameters::Count; index++)
	{
		parameters[index] = calculateNormalisedParameter(index, getParameterNominal(index));
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
    
    selectedTab = -1;
    
    ampEnv = getDefaultEnv();
    filterEnv = getDefaultEnv();
    
    processManager.addBufferReceiver(this);
}

UGenPlugin::~UGenPlugin()
{
    processManager.removeBufferReceiver(this);

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
        
        if (!isParameterAutomatable(index))
            startTimer(UGENIR_IRREFRESHTIME);
	}    
}

bool UGenPlugin::isParameterAutomatable (int index)
{
    return UGenInterface::Parameters::Ranges[index].automatable;
}

float UGenPlugin::calculateNormalisedParameter(int index, float mappedValue)
{
    float normalisedValue;
	
	if(getParameterWarp(index))
	{
		normalisedValue = explin(mappedValue,
								 getParameterMin(index), getParameterMax(index),
								 0.f, 1.f);
	}
	else
	{
		normalisedValue = linlin(mappedValue,
								 getParameterMin(index), getParameterMax(index),
								 0.f, 1.f);
	}

    return normalisedValue;
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

void UGenPlugin::setMappedParameter(int index, float newValue)
{	
	setParameter(index, calculateNormalisedParameter(index, newValue));
}

void UGenPlugin::setMappedParameterNotifyingHost(int index, float newValue)
{
	setParameterNotifyingHost(index, calculateNormalisedParameter(index, newValue));
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

float UGenPlugin::getParameterNominal(int index) const
{
    return (float)UGenInterface::Parameters::Ranges[index].nominal;
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
	return String (getMappedParameter(index), 2);
}

const String UGenPlugin::getParameterDescription (int index)
{
    return String(UGenInterface::Parameters::Descriptions[index]);
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

bool UGenPlugin::silenceInProducesSilenceOut() const
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
	UGen wet = getMappedParameterControl(UGenInterface::Parameters::Wet).lag();
	UGen dry = getMappedParameterControl(UGenInterface::Parameters::Dry).lag();

    plug = Plug::AR(UGen::emptyChannels(getNumOutputChannels()));
    
    if(irBuffer.isNull())
        plug.setSource(inputUGen);
    else
        plug.setSource(getConv());
    
    return (plug * wet.dbamp() + input * dry.dbamp());
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
	
    outputLock.enter();
	outputUGen.prepareAndProcessBlock(numSamples, blockID, -1);
    outputLock.exit();
	
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
	//const ScopedLock sl(meterLock);
	meterLevels[index] = value;
}

void UGenPlugin::buttonClicked(int buttonIndex)
{
    UGenEditorComponent* editor = static_cast<UGenEditorComponent*> (getActiveEditor());
    
    switch ((const int)buttonIndex)
    {
        case UGenInterface::Buttons::ResetAmp: {
            ampEnv = getDefaultEnv();
            
            if (editor != 0)
            {
                editor->getIRComponent()->unsetEnvLocks();
                editor->getIRComponent()->getAmpEnvEditor()->setEnv(getDefaultEnv());
                editor->getIRComponent()->setEnvLocks();
                //editor->getIRComponent()->postCommandMessage(IRComponent::SetEnvLocks);
            }

        } break;
            
        case UGenInterface::Buttons::ResetFilter: {
            filterEnv = getDefaultEnv();
            
            if (editor != 0)
            {
                editor->getIRComponent()->unsetEnvLocks();
                editor->getIRComponent()->getFilterEnvEditor()->setEnv(getDefaultEnv());
                editor->getIRComponent()->setEnvLocks();
                //editor->getIRComponent()->postCommandMessage(IRComponent::SetEnvLocks);
                
            }
            
        } break;
        default: return;
    }
    
//    sendChangeMessage();
    startTimer(UGENIR_IRREFRESHTIME);
}

void UGenPlugin::setMenuItem(int menuItemIndex)
{
	menuItem = menuItemIndex;
    startTimer(UGENIR_IRREFRESHTIME);
}

int UGenPlugin::getMenuItem()
{
	return menuItem;
}

void UGenPlugin::loadIR(File const& newIRFile)
{
    irFile = newIRFile;
    
    DBG(String("irPath: ") + irFile.getFullPathName());
    
    originalBuffer = Buffer(irFile);
    
    replaceIR(originalBuffer);
}

void UGenPlugin::replaceIR(Buffer const& newIRBuffer)
{
    irBuffer = newIRBuffer;
    
    UGenEditorComponent* editor = static_cast<UGenEditorComponent*> (getActiveEditor());
    
    if (editor != 0)
        editor->setIRDisplay(irBuffer);
    
    UGen conv = getConv();

    const ScopedLock sl(outputLock);
    plug.fadeSourceAndRelease(conv, 0.1);
}

UGen UGenPlugin::getConv()
{
    Env pause = Env(Buffer(0.0, 0.0, 1.0, 0.0),
                    Buffer(0.5, 0.1, 0.0),
                    EnvCurve::Linear, 2);
    
    return ZeroLatencyConvolve::AR(inputUGen, irBuffer) * 0.005 * EnvGen::AR(pause);
}

void UGenPlugin::handleBuffer(Buffer const& buffer, const double value1, const int value2)
{
    replaceIR(buffer.zap().normalise());
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
	const String pluginName = UGenUtility::stringToSafeName(JucePlugin_Name);
	
    XmlElement xmlState (pluginName);

    // add some attributes to it..
    xmlState.setAttribute ("pluginVersion", 1);
	
	for(int index = 0; index < UGenInterface::Parameters::Count; index++)
	{
		String parameterName = UGenUtility::stringToSafeName(String(UGenInterface::Parameters::Names[index]));
		parameterName += "_";
		parameterName += String(index);		// try to ensure the name is unique, not foolproof
		xmlState.setAttribute (parameterName, parameters[index]);
	}
	xmlState.setAttribute ("menuItem", menuItem);
	
    const String relative = irFile.getRelativePathFrom(File::getSpecialLocation(File::userHomeDirectory));
    xmlState.setAttribute("irFile", relative);
//    xmlState.setAttribute("lastPath", lastPath);
    xmlState.setAttribute("selectedTab", selectedTab);
    
    const Buffer& ampEnvLevels = ampEnv.getLevels();
    const Buffer& ampEnvTimes = ampEnv.getTimes();
    
    xmlState.setAttribute("ampEnvNumLevels", ampEnvLevels.size());
    for (int i = 0; i < ampEnvLevels.size(); i++)
        xmlState.setAttribute(String("ampEnvLevel")+String(i), (double)ampEnvLevels.getSampleUnchecked(i));

    xmlState.setAttribute("ampEnvNumTimes", ampEnvTimes.size());
    for (int i = 0; i < ampEnvTimes.size(); i++)
        xmlState.setAttribute(String("ampEnvTime")+String(i), (double)ampEnvTimes.getSampleUnchecked(i));
    
    const Buffer& filterEnvLevels = filterEnv.getLevels();
    const Buffer& filterEnvTimes = filterEnv.getTimes();
    
    xmlState.setAttribute("filterEnvNumLevels", filterEnvLevels.size());
    for (int i = 0; i < filterEnvLevels.size(); i++)
        xmlState.setAttribute(String("filterEnvLevel")+String(i), (double)filterEnvLevels.getSampleUnchecked(i));
    
    xmlState.setAttribute("filterEnvNumTimes", filterEnvTimes.size());
    for (int i = 0; i < filterEnvTimes.size(); i++)
        xmlState.setAttribute(String("filterEnvTime")+String(i), (double)filterEnvTimes.getSampleUnchecked(i));

    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
    
    //DBG(xmlState.getAllSubText());
}

void UGenPlugin::setStateInformation (const void* data, int sizeInBytes)
{
	const String pluginName = UGenUtility::stringToSafeName(JucePlugin_Name);
	
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
				parameterName += "_";
				parameterName += String(index);		// try to ensure the name is unique, not foolproof
				parameters[index] = (float) xmlState->getDoubleAttribute (parameterName,
                                                                          calculateNormalisedParameter(index, getParameterNominal(index)));
			}
            
			menuItem = xmlState->getIntAttribute("menuItem", UGenInterface::MenuOptions::LowPass);
            
            const String relative = xmlState->getStringAttribute("irFile", String::empty);
            if(relative.isNotEmpty())
                loadIR(File::getSpecialLocation(File::userHomeDirectory).getChildFile(relative));
            
//            lastPath = xmlState->getStringAttribute("lastPath", String::empty);
            selectedTab = xmlState->getIntAttribute("selectedTab", -1);
            
            
            const int ampEnvNumLevels = xmlState->getIntAttribute("ampEnvNumLevels", 0);
            const int ampEnvNumTimes = xmlState->getIntAttribute("ampEnvNumTimes", 0);
            
            if ((ampEnvNumTimes >= 1) && (ampEnvNumLevels == (ampEnvNumTimes + 1)))
            {
                Buffer ampEnvLevels = BufferSpec(ampEnvNumLevels);
                Buffer ampEnvTimes  = BufferSpec(ampEnvNumTimes);
                
                for (int i = 0; i < ampEnvLevels.size(); i++)
                    ampEnvLevels.setSampleUnchecked(i, (float)xmlState->getDoubleAttribute(String("ampEnvLevel")+String(i), 0.0));
                
                for (int i = 0; i < ampEnvTimes.size(); i++)
                    ampEnvTimes.setSampleUnchecked(i, (float)xmlState->getDoubleAttribute(String("ampEnvTime")+String(i), 0.0));
                
                ampEnv = Env(ampEnvLevels, ampEnvTimes, EnvCurve::Linear);

            }
            else ampEnv = getDefaultEnv();
            
            const int filterEnvNumLevels = xmlState->getIntAttribute("filterEnvNumLevels", 0);
            const int filterEnvNumTimes = xmlState->getIntAttribute("filterEnvNumTimes", 0);
            
            if ((filterEnvNumTimes >= 1) && (filterEnvNumLevels == (filterEnvNumTimes + 1)))
            {
                Buffer filterEnvLevels = BufferSpec(filterEnvNumLevels);
                Buffer filterEnvTimes  = BufferSpec(filterEnvNumTimes);
                
                for (int i = 0; i < filterEnvLevels.size(); i++)
                    filterEnvLevels.setSampleUnchecked(i, (float)xmlState->getDoubleAttribute(String("filterEnvLevel")+String(i), 0.0));
                
                for (int i = 0; i < filterEnvTimes.size(); i++)
                    filterEnvTimes.setSampleUnchecked(i, (float)xmlState->getDoubleAttribute(String("filterEnvTime")+String(i), 0.0));
                
                filterEnv = Env(filterEnvLevels, filterEnvTimes, EnvCurve::Linear);
                
            }
            else filterEnv = getDefaultEnv();

            processEnvs();
            
            sendChangeMessage();
        }

        delete xmlState;
    }
}

static bool isEnvDefault(Env const& env)
{
    Env defaultEnv = getDefaultEnv();
    
    const Buffer& envLevels = env.getLevels();
    const Buffer& envTimes = env.getTimes();
    const EnvCurveList& envCurves = env.getCurves();
    const Buffer& defaultLevels = defaultEnv.getLevels();
    const Buffer& defaultTimes = defaultEnv.getTimes();
    const EnvCurveList& defaultCurves = defaultEnv.getCurves();

    
    if (envLevels.size() != defaultLevels.size()) return false;
    if (envTimes.size() != defaultTimes.size()) return false;
    if (envCurves.size() != defaultCurves.size()) return false;
    
    for (int i = 0; i < envLevels.size(); i++)
        if (envLevels.getSampleUnchecked(i) != defaultLevels.getSampleUnchecked(i))
            return false;
    
    for (int i = 0; i < envTimes.size(); i++)
        if (envTimes.getSampleUnchecked(i) != defaultTimes.getSampleUnchecked(i))
            return false;
    
    for (int i = 0; i < envCurves.size(); i++)
        if (envCurves.getData()[i] != defaultCurves.getData()[i])
            return false;
    
    return true;
}

static float convertQtoOctaves(float q)
{
    const float twoLn2 = 2.f / log(2.f);
    const float rTwoQ = reciprocal(2.f * q);
    return twoLn2 * asinh(rTwoQ);
}

void UGenPlugin::processEnvs()
{
    const float speed = getMappedParameter(UGenInterface::Parameters::Speed);
    const float duration = originalBuffer.duration() / speed;
    const float newSize = originalBuffer.size() / speed;
    
    Env ampEnvScaled = ampEnv.timeScale(duration);
    UGen player = PlayBuf::AR(originalBuffer,
                              speed,
                              0, 0, 0, UGen::DoNothing);
        
    if (!isEnvDefault(filterEnv))
    {
        Env filterEnvScaled = filterEnv.timeScale(duration);
        
        UGen envgen = EnvGen::AR(filterEnvScaled).linexp(0, 1, getFilterMin(), getFilterMax());
        
        switch (menuItem) {
            case UGenInterface::MenuOptions::LowPass:
                player = BLowPass::AR(player, envgen,
                                      reciprocal(getMappedParameter(UGenInterface::Parameters::Resonance)));
                break;
            case UGenInterface::MenuOptions::HighPass:
                player = BHiPass::AR(player, envgen,
                                     reciprocal(getMappedParameter(UGenInterface::Parameters::Resonance)));
                break;
//            case UGenInterface::MenuOptions::PeakNotch:
//                player = BPeakEQ::AR(player,
//                                      EnvGen::AR(filterEnvScaled).linexp(0, 1, getFilterMin(), getFilterMax()),
//                                      getMappedParameterControl(UGenInterface::Parameters::Resonance).reciprocal());
//                break;
            case UGenInterface::MenuOptions::BandPass:
                player = BBandPass::AR(player, envgen,
                                       convertQtoOctaves(getMappedParameter(UGenInterface::Parameters::Resonance)));
                break;
            case UGenInterface::MenuOptions::BandReject:
                player = BBandStop::AR(player, envgen,
                                       convertQtoOctaves(getMappedParameter(UGenInterface::Parameters::Resonance)));
                break;
        }
        
    }
    
    processManager.add(newSize, player * EnvGen::AR(ampEnvScaled));
}

void UGenPlugin::timerCallback()
{
    stopTimer();
    processEnvs();
}

