#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../../UGen/UGen.h"

class MainComponent : public Component,
                      public AudioIODeviceCallback
{
private:
	AudioDeviceManager audioDeviceManager;
	UGen synth;
		
public:
	MainComponent ()
	{
		UGen::initialise();

		String error = audioDeviceManager.initialise (0, 1, 0, true);
		
		if (error.isNotEmpty())
		{
			AlertWindow::showMessageBox (AlertWindow::WarningIcon, 
										 T(PROJECT_NAME), 
										 T("AudioDeviceManager failed \n\n") + error);
		}
		else audioDeviceManager.addAudioCallback (this);
	}
	
	~MainComponent ()
	{		
		audioDeviceManager.removeAudioCallback(this);
		deleteAllChildren();
		UGen::shutdown();
	}
	
	void audioDeviceIOCallback (const float** inputChannelData,
								int numInputChannels, 
								float** outputChannelData, 
								int numOutputChannels, 
								int numSamples)
	{
		int blockID = UGen::getNextBlockID(numSamples);
		synth.setOutputs(outputChannelData, numSamples, numOutputChannels);
		synth.prepareAndProcessBlock(numSamples, blockID);
	}
	
	void audioDeviceAboutToStart (AudioIODevice* device)
	{
		UGen::prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
		
		synth = SinOsc::AR(440, 0, 0.1);
	}
	
	void audioDeviceStopped() { }
	
};

#endif //_MAINCOMPONENT_H_ 