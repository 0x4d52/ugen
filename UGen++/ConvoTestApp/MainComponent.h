#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include "../UGen/UGen.h"
#define _FILEID_ _MAINCOMPONENT_H_


class MainComponent  :  public Component,
						public ButtonListener,
						public FilenameComponentListener,
						public SliderListener,
						public AudioIODeviceCallback,
						public Timer
{
    TooltipWindow tooltipWindow;
	
	//==============================================================================
    TextButton* audioSettingsButton;
	
    //==============================================================================
    // this wraps the actual audio device
    AudioDeviceManager audioDeviceManager;
	
	Label* cpuUsageLabel;
	
	Slider* inputSlider;
	Label* inputLabel;
	ToggleButton* inputMute;
	FilenameComponent* inputFileChooser;
	FilenameComponent* impulseFileChooser;
	TextButton* inputFilePlayButton;
	TextButton* inputFileStopButton;
	Slider* fileGainSlider;
	Label* fileGainLabel;
	Slider* outputGainSlider;
	Label* outputGainLabel;
	ToggleButton* outputMute;
			
	UGen output;
	UGen fileOutput;
	UGen processInput;
	UGen convolve;
	UGen input;
	File currentImpulseFile, currentInputFile;
				
	CriticalSection lock;
	
public:
	//==============================================================================
	MainComponent()
    {
		setName (T("UGen Test"));
						
		
		//==============================================================================
		
		addAndMakeVisible (audioSettingsButton = new TextButton (T("show audio settings..."),
																 T("click here to change the audio device settings")));
		audioSettingsButton->addButtonListener (this);
		
		
		addAndMakeVisible(cpuUsageLabel = new Label(T("CPU usage"), T("0.00 %")));
		cpuUsageLabel->setJustificationType(Justification::right);
					
		addAndMakeVisible(inputSlider = new Slider(T("inputSlider")));
		inputSlider->setRange(0.0, 1.0, 0.0);
		
		addAndMakeVisible(inputLabel = new Label(T("inputLabel"), T("Input Gain")));
		addAndMakeVisible(inputMute = new ToggleButton(T("Mute")));
		
		addAndMakeVisible(impulseFileChooser = new FilenameComponent(T("impulseFileChooser"),
																	 File::nonexistent,
																	 false,false,false,
																	 T("*.wav;*.aif;*.aiff"),
																	 String::empty,
																	 T("Choose an Impulse File...")));
		impulseFileChooser->addListener(this);
		
		addAndMakeVisible(inputFileChooser = new FilenameComponent(T("inputFileChooser"),
																   File::nonexistent,
																   false,false,false,
																   T("*.wav;*.aif;*.aiff"),
																   String::empty,
																   T("Choose an Input File...")));
		inputFileChooser->addListener(this);
		
		addAndMakeVisible(inputFilePlayButton = new TextButton(T("Play Input File")));
		inputFilePlayButton->addButtonListener(this);
		addAndMakeVisible(inputFileStopButton = new TextButton(T("Stop Input File")));
		inputFileStopButton->addButtonListener(this);
		
		addAndMakeVisible(fileGainSlider = new Slider(T("fileGainSlider")));
		fileGainSlider->setRange(0.0, 1.0, 0.0);
		fileGainSlider->setValue(1.0);
		
		addAndMakeVisible(fileGainLabel = new Label(T("fileGainLabel"), T("File Gain")));
		
		addAndMakeVisible(outputGainSlider = new Slider(T("gainSlider")));
		outputGainSlider->setRange(0.0, 1.0, 0.0);
		//gainSlider->setValue(0.5, false, false);
		
		addAndMakeVisible(outputGainLabel = new Label(T("gainLabel"), T("Output Gain")));
		addAndMakeVisible(outputMute = new ToggleButton(T("Mute")));
		
		// and initialise the device manager with no settings so that it picks a
		// default device to use.
		const String error (audioDeviceManager.initialise (2, /* number of input channels */
														   2, /* number of output channels */
														   0, /* no XML settings.. */
														   true  /* select default device on failure */));
		
		if (error.isNotEmpty())
		{
			AlertWindow::showMessageBox (AlertWindow::WarningIcon,
										 T("JuceAudioTemplateApp"),
										 T("Couldn't open an output device!\n\n") + error);
		}
		else
		{
			audioDeviceManager.addAudioCallback (this);	
		}	
	}
	
	~MainComponent()
	{
		audioDeviceManager.removeAudioCallback (this);
		deleteAllChildren();
	}
	
	//==============================================================================
	void audioDeviceIOCallback (const float** inputChannelData,
								int totalNumInputChannels,
								float** outputChannelData,
								int totalNumOutputChannels,
								int numSamples)
	{				
		const ScopedLock sl(lock);
		
		int blockID = UGen::getNextBlockID(numSamples);
		input.setInput(inputChannelData[0], numSamples, 0);
		output.setOutputs(outputChannelData, numSamples, 2);
		output.prepareAndProcessBlock(numSamples, blockID);
	}
	
	void audioDeviceAboutToStart (AudioIODevice* device)
	{
		const ScopedLock sl(lock);
		
		startTimer(50);
		UGen::prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples(), 64);
		
		input = AudioIn::AR(1);		// mono input
		input = U(input, input);	// double mono
		fileOutput = Plug::AR(UGen::emptyChannels(2));
		processInput = input * inputSlider * Invert::AR(inputMute) + (fileOutput * fileGainSlider);
		convolve = Plug::AR(processInput);
		output = convolve * outputGainSlider * Invert::AR(outputMute);
	}
	
	void audioDeviceStopped()
	{
		stopTimer();
	}
	
	//==============================================================================
	
	
	void resized()
	{		
		audioSettingsButton->setBounds (10, 10, 200, 24);
		audioSettingsButton->changeWidthToFitText();
		
		cpuUsageLabel->setBounds(getWidth()-100, 10, 80, 24);
		
		inputSlider->setBounds(10, 50, 300, 24);
		inputLabel->setBounds(320, 50, 100, 24);
		inputMute->setBounds(430, 50, 80, 24);

		impulseFileChooser->setBounds(10, 80, 500, 24);
		inputFileChooser->setBounds(10, 110, 500, 24);
		inputFilePlayButton->setBounds(10, 140, 500, 24);
		inputFileStopButton->setBounds(10, 170, 500, 24);
		
		fileGainSlider->setBounds(10, 200, 300, 24);
		fileGainLabel->setBounds(320, 200, 100, 24);
			
		outputGainSlider->setBounds(10, 230, 300, 24);
		outputGainLabel->setBounds(320, 230, 100, 24);
		outputMute->setBounds(430, 230, 80, 24);
	}
	
	
	void buttonClicked (Button* button)
	{
		if (button == audioSettingsButton)
		{			
			// Create an AudioDeviceSelectorComponent which contains the audio choice widgets...
			AudioDeviceSelectorComponent audioSettingsComp (audioDeviceManager,
															0, 2,
															2, 2,
															true,
															false,
															true,
															false);
			
			// ...and show it in a DialogWindow...
			audioSettingsComp.setSize (500, 400);
			
			DialogWindow::showModalDialog (T("Audio Settings"),
										   &audioSettingsComp,
										   this,
										   Colours::azure,
										   true);
		}
		else if(button == inputFilePlayButton)
		{
			if(inputFileChooser->getCurrentFile()==File::nonexistent) return;
			
			const ScopedLock sl(lock);
			fileOutput.fadeSourceAndRelease(DiskIn::AR(inputFileChooser->getCurrentFile(), true), 0.1);
		}
		else if(button == inputFileStopButton)
		{
			if(inputFileChooser->getCurrentFile()==File::nonexistent) return;
			
			const ScopedLock sl(lock);
			fileOutput.fadeSourceAndRelease(UGen::emptyChannels(2), 0.1);
		}
	}
	
	void filenameComponentChanged (FilenameComponent* filenameComponent)
	{
		if(filenameComponent == inputFileChooser)
		{
			const ScopedLock sl(lock);
			fileOutput.setSource(UGen::null, true);
		}
		else if(filenameComponent == impulseFileChooser)
		{
			currentImpulseFile = impulseFileChooser->getCurrentFile();
			if(currentImpulseFile != File::nonexistent)
			{
				UGen newConvolver = ZeroLatencyConvolve::AR(processInput, currentImpulseFile);
				const ScopedLock sl(lock);
				convolve.fadeSourceAndRelease(newConvolver, 0.5);
			}
			
		}
	}
	
	void sliderValueChanged (Slider* slider)
	{

	}
	
	void timerCallback()
	{
		cpuUsageLabel->setText(String(audioDeviceManager.getCpuUsage()*100.0, 2) << T(" %"), false);
	}
		
	
	
	
	
	
	
	
};

#undef _FILEID_
#endif//_MAINCOMPONENT_H_ 