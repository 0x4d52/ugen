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

#include "../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceVoicer.h"
#include "../basics/ugen_MixUGen.h"

VoicerUGenInternal::VoicerUGenInternal(const int numChannels, 
									   const int midiChannel, 
									   const int numVoices, 
									   const bool forcedSteal,
									   const bool direct) throw()
:	VoicerBaseUGenInternal(numChannels, numVoices, forcedSteal),
	JuceMIDIInputListener(direct),
	midiChannel_(midiChannel),
	controllers(FloatArray::newClear(128)),
	keyPressure(FloatArray::newClear(128)),
	pitchWheel(0.f),
	channelPressure(0.f),
	program(0)
{
	controllers[7] = 1.f;
	controllers[10] = 0.5f;
	controllers[11] = 1.f;
}

//void VoicerUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
//{
//	const ScopedLock sl(lock);
//	
//	// render the current events
//	SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
//	
//	if(midiMessages.isEmpty() == false)
//	{
//		const int blockSize = uGenOutput.getBlockSize();
//		bool addedEvent = false;
//		MidiBuffer::Iterator iter(midiMessages);
//		MidiMessage message(0xf4, 0.0);
//		
//		int samplePos = 0;
//		while(iter.getNextEvent(message, samplePos) && (samplePos < blockSize))
//		{
//			if(samplePos < 0) samplePos = 0;
//				
//			const int midiChannel = midiChannel_;
//			
//			if(message.isNoteOnOrOff() && message.isForChannel(midiChannel))
//			{
//				const int midiNote = message.getNoteNumber();
//				const int velocity = message.getVelocity();
//				
//				const int userData = createUserData(midiChannel, midiNote);
//				
//				if(velocity > 0)
//				{
//					if(numVoices_ > 0)
//					{
//						const int voicesUsed = countNonstealingVoices();
//						if(voicesUsed >= numVoices_)
//						{
//							UGen& stealee = chooseStealee();
//							
//							if(stealee.isNotNull())
//							{
//								stealee.userData = stealingUserData;
//								stealee.steal(forcedSteal_);
//							}
//						}
//					}
//					
//					// stop double notes, AU lab was sending two ons but one off 
//					stealNote(midiChannel, midiNote, false, true); 
//					
//					addedEvent = true;
//					UGen newEvent = spawnEvent(*this, currentEventIndex++, midiChannel, midiNote, velocity);
//					newEvent.userData = userData;
//					
//					newEvent.prepareForBlock(blockSize, blockID); // prepare for full size (allocates the output buffers)
//					
//					const int currentBlockSampleID = blockID + samplePos;
//					const int numSamplesToProcess = blockSize - samplePos;
//						
//					UGen::setBlockSize(numSamplesToProcess);
//
//					if(samplePos > 0)
//						newEvent.prepareForBlock(numSamplesToProcess, currentBlockSampleID); // prepare for sub block
//					
//					for(int channel = 0; channel < numChannels; channel++)
//					{
//						bool shouleDeleteLocal = false;
//						const float *eventSamples = newEvent.processBlock(shouleDeleteLocal, currentBlockSampleID, channel);
//						accumulateSamples(bufferData[channel] + samplePos, eventSamples, numSamplesToProcess);
//					}
//					
//					events <<= newEvent;
//				}
//				else
//				{
//					UGen& releasee = chooseReleasee(midiChannel, midiNote);
//			
//					if(releasee.isNotNull()) releasee.release();
//				}
//			}
//		}
//		
//		if(addedEvent) 
//		{
//			UGen::setBlockSize(blockSize);
//			mixer = Mix(&events, false);
//		}
//		
//		midiMessages.clear();
//	}	
//}

void VoicerUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	if(shouldStopAllEvents() == true) initEvents();
	
	if(midiMessages.isEmpty() == true)
	{
		const ScopedLock sl(lock);
		SpawnBaseUGenInternal::processBlock(shouldDelete, blockID, -1);
	}
	else
	{
		const int blockSize = uGenOutput.getBlockSize();
		MidiBuffer::Iterator iter(midiMessages);
		MidiMessage message(0xf4, 0.0);
		
		int samplePos = 0;
		int startSample = 0;
		
		const int numChannels = getNumChannels();
		const int midiChannel = midiChannel_;
		
		while(iter.getNextEvent(message, samplePos) && (samplePos < blockSize))
		{
			const ScopedLock sl(lock);
				
			if(samplePos < 0) samplePos = 0;
			
			const int numSamples = samplePos - startSample;
		
			if(numSamples > 0)
			{				
				for(int channel = 0; channel < numChannels; channel++)
				{
					bufferData[channel] = proxies[channel]->getSampleData() + startSample;
				}
				
				mixer.prepareForBlock(numSamples, blockID + startSample, -1);
				mixer.setOutputs(bufferData, numSamples, numChannels);
				mixer.processBlock(shouldDelete, blockID + startSample, -1);				
			}
			
			startSample = samplePos;
			
			if(message.isForChannel(midiChannel))
			{
				if(message.isNoteOnOrOff())
				{
					const int midiNote = message.getNoteNumber();
					const int velocity = message.getVelocity();
										
					if(velocity > 0)
					{
						if(numVoices_ > 0)
						{
							const int voicesUsed = countNonstealingVoices();
							if(voicesUsed >= numVoices_)
							{
								UGen stealee = chooseStealee();
								
								if(stealee.isNotNull())
								{
									stealee.userData = stealingUserData;
									stealee.steal(forcedSteal_);
								}
							}
						}
						
						// stop double notes, AU lab was sending two ons but one off - seems fixed in Au Lab 2.2
						//stealNote(midiChannel, midiNote, false, true); 
						
						UGen newEvent = spawnEvent(*this, currentEventIndex++, midiChannel, midiNote, velocity);
						newEvent.userData = createUserData(midiChannel, midiNote);
						events.add(newEvent);
					}
					else
					{
						UGen releasee = chooseReleasee(midiChannel, midiNote);
						if(releasee.isNotNull()) releasee.release();
					}
				}
				else if(message.isController())
				{
					getController(message.getControllerNumber()) = message.getControllerValue() / 127.f;
				}
				else if(message.isPitchWheel())
				{
					getPitchWheel() = (jlimit(-8191, 8191, message.getPitchWheelValue() - 8192) / 8191.f);
				}
				else if(message.isChannelPressure())
				{
					getChannelPressure() = (message.getChannelPressureValue() / 127.f);
				}
				else if(message.isAftertouch())
				{
					getKeyPressure(message.getNoteNumber()) = (message.getAfterTouchValue() / 127.f);
				}
				else if(message.isProgramChange())
				{
					getProgram() = (message.getProgramChangeNumber());
				}
				else if(message.isAllNotesOff())
				{
					initEvents();
				}
			}
		}
		
		const ScopedLock sl(lock);
		
		const int numSamples = blockSize - startSample;
		
		if(numSamples > 0)
		{			
			for(int channel = 0; channel < numChannels; channel++)
			{
				bufferData[channel] = proxies[channel]->getSampleData() + startSample;
			}
			
			mixer.prepareForBlock(numSamples, blockID + startSample, -1);
			mixer.setOutputs(bufferData, numSamples, numChannels);
			mixer.processBlock(shouldDelete, blockID + startSample, -1);
		}
				
		midiMessages.clear();
		events.removeNulls();
	}	
}

void VoicerUGenInternal::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) throw()
{
	if(message.isForChannel(midiChannel_) == false)	return;
	
	if(message.isNoteOnOrOff())
	{
		const ScopedLock sl(lock);
		sendMidiNote(message.getChannel(), message.getNoteNumber(), message.getVelocity());
	}
	else if(message.isController())
	{
		getController(message.getControllerNumber()) = 
					 (message.getControllerValue() * (1.f / 127.f));
	}
	else if(message.isPitchWheel())
	{
		getPitchWheel() = (jlimit(-8191, 8191, message.getPitchWheelValue() - 8192) * (1.f / 8191.f));
	}
	else if(message.isChannelPressure())
	{
		getChannelPressure() = (message.getChannelPressureValue() * (1.f / 127.f));
	}
	else if(message.isAftertouch())
	{
		getKeyPressure(message.getNoteNumber()) = 
					  (message.getAfterTouchValue() * (1.f / 127.f));
	}
	else if(message.isProgramChange())
	{
		getProgram() = (message.getProgramChangeNumber());
	}
	else if(message.isAllNotesOff())
	{
		const ScopedLock sl(lock);
		initEvents();
	}
}

void VoicerUGenInternal::sendMidiBuffer(MidiBuffer const& midiMessagesToAdd) throw()
{	
	if(midiMessagesToAdd.isEmpty() == false)
	{
		const ScopedLock sl(lock);
		midiMessages.addEvents(midiMessagesToAdd, 0, -1, 0);
	}
}

const float& VoicerUGenInternal::getController(const int index) const throw()
{
	return controllers[index];
}

const float& VoicerUGenInternal::getKeyPressure(const int index) const throw()
{
	return keyPressure[index];
}

const float& VoicerUGenInternal::getPitchWheel() const throw()
{
	return pitchWheel;
}

const float& VoicerUGenInternal::getChannelPressure() const throw()
{
	return channelPressure;
}

const int& VoicerUGenInternal::getProgram() const throw()
{
	return program;
}

float& VoicerUGenInternal::getController(const int index) throw()
{
	return controllers[index];
}

float& VoicerUGenInternal::getKeyPressure(const int index) throw()
{
	return keyPressure[index];
}

float& VoicerUGenInternal::getPitchWheel() throw()
{
	return pitchWheel;
}

float& VoicerUGenInternal::getChannelPressure() throw()
{
	return channelPressure;
}

int& VoicerUGenInternal::getProgram() throw()
{
	return program;
}



END_UGEN_NAMESPACE

#endif