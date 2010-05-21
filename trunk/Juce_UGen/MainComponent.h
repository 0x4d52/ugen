#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_


#include <juce/juce.h>
#include "../UGen/UGen.h"
#define _FILEID_ _MAINCOMPONENT_H_



class MainComponent  :  public Component,
						public ButtonListener,
						public SliderListener,
						public AudioIODeviceCallback,
						public Timer
{
    //==============================================================================
    TextButton* audioSettingsButton;
	
    //==============================================================================
    // this wraps the actual audio device
    AudioDeviceManager audioDeviceManager;
	
	Label* cpuUsageLabel;
	
	Slider* freqSlider1;
	Slider* freqSlider2;
	Slider* ampSlider1;
	Slider* ampSlider2;
	TextButton* testButton1;
	TextButton* testButton2;
	ToggleButton* toggle1;
	ToggleButton* toggle2;
	ScopeComponent* scopeComponent;
	EnvelopeContainerComponent* envComponent;
	MultiSlider* sliders1;
	MultiSlider* sliders2;
	File fileToPlay;
	
	float freqValue1, freqValue2, ampValue1, ampValue2;
		
	UGen synth;
	UGen input;
	UGen record;
	UGen scope;
	UGen lfo;
	
	Buffer penta;
				
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
		
		addAndMakeVisible(freqSlider1 = new Slider(T("Frequency1")));
		freqSlider1->setRange(50, 5000, 0.0);
		freqSlider1->setSkewFactorFromMidPoint(250.0);
		freqSlider1->addListener(this);
		freqSlider1->setValue(100, true, true);
		
		addAndMakeVisible(freqSlider2 = new Slider(T("Frequency2")));
		freqSlider2->setRange(50, 5000, 0.0);
		freqSlider2->setSkewFactorFromMidPoint(250.0);
		freqSlider2->addListener(this);
		freqSlider2->setValue(100, true, true);
		
		addAndMakeVisible(ampSlider1 = new Slider(T("Amp1")));
		ampSlider1->setRange(0.0, 1.0, 0.0);
		ampSlider1->addListener(this);
		ampSlider1->setValue(0.5, true, true);
		
		addAndMakeVisible(ampSlider2 = new Slider(T("Amp2")));
		ampSlider2->setRange(0.0, 1.0, 0.0);
		ampSlider2->addListener(this);
		ampSlider2->setValue(0.5, true, true);
		
		addAndMakeVisible(testButton1 = new TextButton(T("Test Button 1")));
		testButton1->addButtonListener(this);
		addAndMakeVisible(testButton2 = new TextButton(T("Test Button 2")));
		testButton2->addButtonListener(this);
		
		addAndMakeVisible(toggle1 = new ToggleButton(T("Toggle 1")));
		toggle1->addButtonListener(this);
		addAndMakeVisible(toggle2 = new ToggleButton(T("Toggle 2")));
		toggle2->addButtonListener(this);
		
		addAndMakeVisible(sliders1 = new MultiSlider(4));
		addAndMakeVisible(sliders2 = new MultiSlider(4));
		
		addAndMakeVisible(scopeComponent = new ScopeComponent());
		scopeComponent->setScaleX(ScopeGUI::LabelXTime, 441, 2, false);
		scopeComponent->setScaleY(ScopeGUI::LabelYAmplitude);
		
		addAndMakeVisible(envComponent = new EnvelopeContainerComponent("My Envelope"));
		Env env = Env::linen(1.0, 1.0, 1.0, 0.7, EnvCurve::Sine);
		float time = 0.f;
		float inc = 0.1f;
		for(int i = 0; i < 31; i++)
		{
			printf("time (%f) = %f\n", time, env.lookup(time));
			time += inc;
		}
		
		envComponent->setDomainRange(env.duration());
		envComponent->setEnv(env);
		
				
		// and initialise the device manager with no settings so that it picks a
		// default device to use.
		const String error (audioDeviceManager.initialise (1, /* number of input channels */
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
		audioDeviceManager.removeAudioCallback(this);
		deleteAllChildren();
		record = UGen::getNull();
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
		synth.setOutputs(outputChannelData, numSamples, 2);
		
		synth.prepareAndProcessBlock(numSamples, blockID);
		record.prepareAndProcessBlock(numSamples, blockID);
		
		scope.prepareAndProcessBlock(numSamples, blockID);
	}
	
	void audioDeviceAboutToStart (AudioIODevice* device)
	{
		const ScopedLock sl(lock);
		
		startTimer(50);
		
		UGen::prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples(), 64);
		
		input = AudioIn::AR(1);
				
		synth = constructUGenGraph();
		
		// old way
//		scope = Scope::AR(&scopeComponent, synth, U(ampSlider2) / 10); 
		
		// new way
		scope = Sender::AR(synth, U(ampSlider2) / 10);
		scope.addBufferReceiver(scopeComponent);
	}

	
	void audioDeviceStopped()
	{
		stopTimer();
		//record = UGen::null;
	}
	
	//==============================================================================
	
	
	void resized()
	{		
		audioSettingsButton->setBounds (10, 10, 200, 24);
		audioSettingsButton->changeWidthToFitText();
		
		cpuUsageLabel->setBounds(getWidth()-90, 10, 80, 24);
		
		freqSlider1->setBounds(10, 80, 200, 20);
		freqSlider2->setBounds(10, 110, 200, 20);
		ampSlider1->setBounds(10, 140, 200, 20);
		ampSlider2->setBounds(10, 170, 200, 20);
		
		testButton1->setBounds(10, 200, 200, 20);
		testButton2->setBounds(10, 230, 200, 20);
		
		toggle1->setBounds(10, 260, 200, 20);
		toggle2->setBounds(10, 290, 200, 20);
		
		sliders1->setBounds(10, 320, 200, 100);
		sliders2->setBounds(10, 450, 200, 100);
		
		scopeComponent->setBounds(230, 40, getWidth()-230-10, getHeight()-50-10-80);
		envComponent->setBounds(230, getHeight()-10-80, getWidth()-230-10, 80);
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
		else if(button == testButton1)
		{
			Env env = envComponent->getEnv();
			printf("test\n");
		}
		else if(button == testButton2)
		{
			
		}
		else if(button == toggle1)
		{
			
		}
		else if(button == toggle2)
		{

		}
	}
	
	void sliderValueChanged (Slider* slider)
	{
		//const ScopedLock sl(lock);
		
		if(slider == freqSlider1)
		{
			freqValue1 = (float)freqSlider1->getValue();
		}
		else if(slider == freqSlider2)
		{
			freqValue2 = (float)freqSlider2->getValue();
		}
		else if(slider == ampSlider1)
		{
			ampValue1 = (float)ampSlider1->getValue();
		}
		else if(slider == ampSlider2)
		{
			ampValue2 = (float)ampSlider2->getValue();
		}
	}
	
	void timerCallback()
	{
		cpuUsageLabel->setText(String(audioDeviceManager.getCpuUsage()*100.0, 2), false);
	}
			
	UGen constructUGenGraph()
	{
		return SinOsc::AR(U(freqSlider1, freqSlider2), 0, Lag::AR(ampSlider1));
	}
	
	
};

#undef _FILEID_
#endif//_MAINCOMPONENT_H_ 