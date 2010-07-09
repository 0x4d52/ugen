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

#ifndef _UGEN_ugen_JuceIOHost_H_
#define _UGEN_ugen_JuceIOHost_H_

/** An audio IO host for Juce projects.
 @see UIKitAUIOHost AudioQueueIOHostController 
 @ingroup Hosts */
class JuceIOHost :	public AudioIODeviceCallback,
					private Timer
{
public:
	/**
	 Constructor.
	 
	 Constructs a UGen++ JuceIOHost with the required number of inputs and outputs. This also calls
	 UGen::initialise() to setup UGen++ and creates a Juce AudioDeviceManager for handling audio IO.
	 The creation of the AudioDeviceManager is deferred until after the constructor returns so that we don't
	 crash when the constructGraph() function is called (since it's pure virtual).
	 
	 If @c preferredBufferSize is zero (or less) the default buffer size for the device will be used.

	 */
	JuceIOHost(const int numInputs = 2, const int numOutputs = 2, const int preferredBufferSize = 0, const bool useTimerDeleter = false) throw()
	:	numInputs_(numInputs < 0 ? 0 : numInputs),
		numOutputs_(numOutputs < 0 ? 0 : numOutputs),
		bufferSize(preferredBufferSize),
		juceDeleter(0)
	{
		ugen_assert(numInputs == numInputs_);
		ugen_assert(numOutputs == numOutputs_);
		ugen_assert(bufferSize == preferredBufferSize);
		
		const ScopedLock sl(lock);
		UGen::initialise();
		if(useTimerDeleter) 
		{
			juceDeleter = new JuceTimerDeleter();
			UGen::setDeleter(juceDeleter);
		}
		
#ifndef UGEN_NOEXTGPL
		Ran088::defaultGenerator().setSeed(Time::currentTimeMillis());
#endif
		startTimer(50);
		
		output_ = Plug::AR(UGen::emptyChannels(numOutputs_));
	}
	
	/**
	 Destructor.
	 
	 Cleans up the internal AudioDeviceManager and calls UGen::shutdown() to clean up UGen++
	 */
	~JuceIOHost()
	{
		audioDeviceManager.removeAudioCallback(this); // try this first to prevent deadlock? nope - now crashes here
		
		delete juceDeleter;
	}
	
	/**
	 Sets up the AudioDeviceManager shortly after the constructor exits.
	 */
	void timerCallback()
	{
		stopTimer();
		
		String error = audioDeviceManager.initialise (numInputs_, numOutputs_, 0, true);
		
		if (error.isNotEmpty())
		{
			AlertWindow::showMessageBox (AlertWindow::WarningIcon, 
										 T("JuceIOHost"), 
										 T("AudioDeviceManager init failed \n\n") + error);
			
			return;
		}

		AudioDeviceManager::AudioDeviceSetup setup;
		audioDeviceManager.getAudioDeviceSetup(setup);

		if(bufferSize < 1)
		{
			bufferSize = setup.bufferSize;
		}
		else
		{
			setup.bufferSize = bufferSize;
			audioDeviceManager.setAudioDeviceSetup(setup, false);
		}
		
		if (error.isNotEmpty())
		{
			AlertWindow::showMessageBox (AlertWindow::WarningIcon, 
										 T("JuceIOHost"), 
										 T("AudioDeviceManager setup failed \n\n") + error);
			
			return;
		}
		
		const ScopedLock sl(lock);
		
		if(numInputs_ > 0)
			input_ = AudioIn::AR(numInputs_);
		audioDeviceManager.addAudioCallback (this);
		
	}
	
	/**
	 Renders a block of audio through the UGen graph.
	 
	 The input UGen is provided with the sample data from the host environment and the the output UGen
	 is provided with a place to write its sample data output. The output is then rendered.
	 */
	void audioDeviceIOCallback (const float** inputChannelData,
								int numInputChannels, 
								float** outputChannelData, 
								int numOutputChannels, 
								int numSamples)
	{
		// may need to be a bit cleverer with the channels in here..
		const ScopedLock sl(lock);
		int blockID = UGen::getNextBlockID(numSamples);
		
		if(numInputs_ > 0)
			input_.setInputs(inputChannelData, numSamples, numInputChannels);
		
		if(numOutputs_ > 0)
		{
			output_.setOutputs(outputChannelData, numSamples, numOutputChannels);

			for(int i = 0; i < others.size(); i++)
			{
				others[i].prepareAndProcessBlock(numSamples, blockID);
			}
			
			output_.prepareAndProcessBlock(numSamples, blockID);
		}
		else
		{
			for(int i = 0; i < others.size(); i++)
			{
				others[i].prepareAndProcessBlock(numSamples, blockID);
			}
		}
		
	}
	
	/**
	 Get ready to play.
	 
	 The UGen++ system is provided with a sample rate and buffer size at this point and the constructGraph() 
	 pure virtual function is called.
	 */
	void audioDeviceAboutToStart (AudioIODevice* device)
	{
		//const ScopedLock sl(lock);	
		UGen::prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
		output_.setSource(constructGraph(input_), true, 0.0);
	}
	
	/**  */
	void audioDeviceStopped() 
	{ 
		UGen::shutdown();
	}
	
	/** Get a reference to the AudioDeviceManager() */
	AudioDeviceManager& getAudioDeviceManager() throw() { return audioDeviceManager;	}
	
	int getNumInputs() const throw()					{ return numInputs_;			}
	int getNumOutputs() const throw()					{ return numOutputs_;			}
	
	/** 
	 Get a reference to the input UGen.
	 
	 Be careful not to store this reference.
	 
	 @return a reference to the input UGen
	 */
	UGen& getInput() throw()							{ return input_;				}
	
	/** 
	 Get a reference to the output UGen.
	 
	 Be careful not to store this reference. This could be useful if the output UGen is
	 a Plug, in which case its source could be changed.
	 
	 @return a reference to the output UGen
	 */
	UGen& getOutput() throw()							{ return output_;				}
	
	/**
	 Set the input UGen.
	 
	 Replace the input UGen with another, in practice this is unlikely to be needed as the 
	 number input channels is fixed at construction time and the only UGen which makes sense
	 is an AudioIn.
	 
	 @param ugen	the new UGen to use as the host input.
	 */
	void setInput(UGen const& ugen) throw() 
	{ 
		const ScopedLock sl(lock);
		input_ = ugen;
	}
	
	/**
	 Set the output UGen.
	 
	 Replace the current output UGen with another. In practice it will usually be better to use a Plug 
	 and change its source (which can optionally include a crossfade to the new souce) or some other method
	 which will avoid discontinuitites in the signal.
	 
	 @param ugen	the new UGen to use as the host output.
	 */
	void setOutput(UGen const& ugen) throw() 
	{ 
		const ScopedLock sl(lock);
		output_ = ugen;
	}
	
	/**
	 Add a UGen to the dependency of this host.
	 
	 This is mainly used for Scope and/or DiskOut.
	 */
	void addOther(UGen const& ugen) throw()
	{
		const ScopedLock sl(lock);
		others <<= ugen;
	}
	
	/**
	 Construct a UGen graph.
	 
	 You must implement this in your subclass. You should return a UGen which will be the UGen graph which is 
	 performed and rendered to the host. The input parameter may be ignored if only signal generation is required 
	 or may be used if a processing algorithm is being implemented (e.g., filtering incoming audio data).
	 
	 @param input	the input UGen which will contain audio data from the host
	 @return		the UGen graph which will be performed
	 */
	virtual UGen constructGraph(UGen const& input) = 0;
	
protected:
	CriticalSection lock;
	AudioDeviceManager audioDeviceManager;
	
private:
	const int numInputs_, numOutputs_;
	int bufferSize;
	UGen input_;
	UGen output_;
	UGenArray others;
	JuceTimerDeleter* juceDeleter;
};



#endif // _UGEN_ugen_JuceIOHost_H_
