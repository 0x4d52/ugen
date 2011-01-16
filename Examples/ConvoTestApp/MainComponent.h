#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>
#include <UGen/UGen.h>


class MainComponent  :  public Component,
						public JuceIOHost,
						public ButtonListener,
						public FilenameComponentListener,
						public Timer
{
    TooltipWindow tooltipWindow;	
    TextButton* audioSettingsButton;

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
				
public:
	MainComponent()
    {
								
		
		addAndMakeVisible (audioSettingsButton = new TextButton (T("show audio settings..."),
																 T("click here to change the audio device settings")));
		audioSettingsButton->addListener (this);
		
		
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
		inputFilePlayButton->addListener(this);
		addAndMakeVisible(inputFileStopButton = new TextButton(T("Stop Input File")));
		inputFileStopButton->addListener(this);
		
		addAndMakeVisible(fileGainSlider = new Slider(T("fileGainSlider")));
		fileGainSlider->setRange(0.0, 1.0, 0.0);
		fileGainSlider->setValue(1.0);
		
		addAndMakeVisible(fileGainLabel = new Label(T("fileGainLabel"), T("File Gain")));
		
		addAndMakeVisible(outputGainSlider = new Slider(T("gainSlider")));
		outputGainSlider->setRange(0.0, 1.0, 0.0);
		//gainSlider->setValue(0.5, false, false);
		
		addAndMakeVisible(outputGainLabel = new Label(T("gainLabel"), T("Output Gain")));
		addAndMakeVisible(outputMute = new ToggleButton(T("Mute")));
		
		startTimer(100);
	}
	
	~MainComponent()
	{
		deleteAllChildren();
	}
			
	UGen constructGraph(UGen const& input)
	{
		fileOutput = Plug::AR(UGen::emptyChannels(2));
		processInput = input * inputSlider * Invert::AR(inputMute) + (fileOutput * fileGainSlider);
		convolve = Plug::AR(processInput);
		return convolve * outputGainSlider * Invert::AR(outputMute);
	}
	
	
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
			AudioDeviceSelectorComponent audioSettingsComp (getAudioDeviceManager(),
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
			fileOutput.setSource(UGen::getNull(), true);
		}
		else if(filenameComponent == impulseFileChooser)
		{
			File currentImpulseFile = impulseFileChooser->getCurrentFile();
			
			if(currentImpulseFile != File::nonexistent)
			{
				UGen newConvolver = ZeroLatencyConvolve::AR(processInput, currentImpulseFile);
				const ScopedLock sl(lock);
				convolve.fadeSourceAndRelease(newConvolver, 0.5);
			}
		}
	}
		
	void timerCallback()
	{
		cpuUsageLabel->setText(String(getCpuUsage()*100.0, 2) + T(" %"), false);
	}
};

#endif//_MAINCOMPONENT_H_ 