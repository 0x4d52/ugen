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


#ifndef UGENPLUGIN_H
#define UGENPLUGIN_H

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../../UGen/UGen.h"
#include "UGenCommon.h"


class UGenPlugin  :	public AudioProcessor,
					public ChangeBroadcaster,
                    public BufferReceiver,
                    public Timer
{
public:
    //==============================================================================
    UGenPlugin();
    ~UGenPlugin();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

	void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

	void clearExtraChannels(AudioSampleBuffer& buffer);
	
    //==============================================================================
    AudioProcessorEditor* createEditor();
	bool hasEditor() const { return true; }
    //==============================================================================
    const String getName() const;

    int getNumParameters();

	const float* getParameterPtr (int index) const;
    float getParameter (int index);
    void setParameter (int index, float newValue);
	bool isParameterAutomatable (int parameterIndex);
    
    float calculateNormalisedParameter(int index, float mappedValue);
    
	float getMappedParameter(int index);
	void setMappedParameter(int index, float newValue);
	void setMappedParameterNotifyingHost(int index, float newValue);
	UGen getMappedParameterControl(int index) const;

	float getParameterMin(int index) const;
	float getParameterMax(int index) const;
	float getParameterNominal(int index) const;
	bool getParameterWarp(int index) const;
	const String getParameterUnits(int index) const;
	
    const String getParameterName (int index);
    const String getParameterText (int index);	

    const String getParameterDescription (int index);

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;

    //==============================================================================
    int getNumPrograms()                                        { return 0; }
    int getCurrentProgram()                                     { return 0; }
    void setCurrentProgram (int index)                          { }
    const String getProgramName (int index)                     { return String::empty; }
    void changeProgramName (int index, const String& newName)   { }

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    //==============================================================================
	
	float* getMeterLevelPtr(int index) { return meterLevels + index; }
	void setMeterLevel(int index, float value);
	void buttonClicked(int buttonIndex);
	void setMenuItem(int menuItemIndex);
	int getMenuItem();
    
//    void setLastPath(String const& path) { lastPath = path; }
//    String getLastPath() { return lastPath; }
    void setSelectedTab(const int tab) { selectedTab = tab; }
    int getSelectedTab() { return selectedTab; }
    
    void loadIR(File const& irFile);
    File getIRFile() { return irFile; }
    void replaceIR(Buffer const& irBuffer);
    UGen getConv();
    inline Buffer getIRBuffer() { return irBuffer; }
    inline Buffer getOriginalBuffer() { return originalBuffer; }
    
    inline Env getIRAmpEnv() { return ampEnv; }
    inline void setIRAmpEnv(Env const& env) { ampEnv = env; }
    inline Env getIRFilterEnv() { return filterEnv; }
    inline void setIRFilterEnv(Env const& env) { filterEnv = env; }

    const double getFilterMin() const
    {
        const double lowest = UGen::getSampleRate() * 0.0005;
        
        double min = 0.0;
        
        while (min < lowest)
            min += 5;
        
        return min;
    }

    const double getFilterMax() const
    {
        const double nyquist = UGen::getSampleRate() * 0.5 * 0.95;
        
        double max = 0.0;
        
        while (max < nyquist)
            max += 1000.0;

        return max - 1000.0;
    }
    
    void timerCallback();
    void processEnvs();
    
    void handleBuffer(Buffer const& buffer, const double value1, const int value2);
	
	UGen constructGraph(UGen const& input);
		
    juce_UseDebuggingNewOperator

private:
    float *parameters;	// an array of parameters here
	float *meterLevels; // you must use setMeterLevel() to modify these meterLevels values
	
	float channelLevel0, channelLevel1;
	int numMeasurements;
	
	int menuItem;
	
	UGen inputUGen;
	UGen outputUGen;
	Buffer inputBuffer;
	
	int blockID;
    
    UGen plug;
    File irFile;
    Buffer irBuffer;
    Buffer originalBuffer;
    Env ampEnv, filterEnv;
//    String lastPath;
    int selectedTab;
    
    CriticalSection outputLock;
    //CriticalSection meterLock;
    CriticalSection bufferLock;
    CriticalSection backgroundLock;
    
    BufferProcess processManager;
};


#endif // UGENPLUGIN_H
