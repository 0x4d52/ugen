// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
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

#include "../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceUtility.h"
#include "../core/ugen_UGen.h"

PopupComponent::PopupComponent(const int max) 
:	maxCount(max) 
{
	activePopups++;
	startTimer(100);
}

PopupComponent::~PopupComponent()
{
	activePopups--;
}

void PopupComponent::paint(Graphics &g)
{
	g.setColour(Colour::greyLevel(0.5).withAlpha(0.75f));
	g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 5);
}	

void PopupComponent::mouseDown(MouseEvent const& e)
{
	(void)e;
	resetCounter();
}	

void PopupComponent::resetCounter()
{
	counter = maxCount;
}

void PopupComponent::timerCallback()
{
	if(Component::getNumCurrentlyModalComponents() > 0 && counter >= 0)
	{
		resetCounter();
	}
	else
	{
		counter--;		
		if(counter < 0)
		{
			stopTimer();
			delete this;
		}
	}
}

void PopupComponent::expire()
{
    expired();
	counter = -1;
}

int PopupComponent::activePopups = 0;



BufferProcess::BufferProcess() throw()
:	Thread("BufferProcess")
{
	startThread();
}

BufferProcess::~BufferProcess()
{
    signalThreadShouldExit();
    event.signal();

	stopThread(4000);
}

void BufferProcess::add(Buffer const& buffer, UGen const& input, UGen const& graph, const int bufferID) throw()
{
	ugen_assert(buffer.size() > 0);
	ugen_assert(buffer.getNumChannels() > 0);
	ugen_assert(input.getNumChannels() > 0);
	ugen_assert(graph.getNumChannels() > 0);
	
	const ScopedLock sl(lock);
	
	buffers.add(buffer);
	inputs.add(input);
	graphs.add(graph);
	ids.add(bufferID);
    
    event.signal();
}

void BufferProcess::add(const int size, UGen const& graph, const int bufferID) throw()
{
	ugen_assert(size > 0);
	ugen_assert(graph.getNumChannels() > 0);
	
	const ScopedLock sl(lock);
	
	buffers.add(Buffer::withSize(size, graph.getNumChannels(), false));
	inputs.add(UGen::getNull());
	graphs.add(graph);
	ids.add(bufferID);
    
    event.signal();
}


void BufferProcess::run() throw()
{
	while(threadShouldExit() == false)
	{
		event.wait();
		
		const ScopedLock sl(lock);
		
		int count = buffers.length();
		bool needClear = false;
		
		for(int cueIndex = 0; cueIndex < count; cueIndex++)
		{
			if(threadShouldExit() == true) break;
			
			needClear = true;
			
			Buffer buffer = buffers[cueIndex];
			
			if(inputs[cueIndex].isNull())
			{
				buffer.synthInPlace(graphs[cueIndex]);
				sendBuffer(buffer, 0.0, ids[cueIndex]);
			}
			else
			{
				Buffer result = buffer.process(inputs[cueIndex], graphs[cueIndex]);
				sendBuffer(result, 0.0, ids[cueIndex]);
			}
		}
		
		if(needClear)
		{
			buffers = ObjectArray<Buffer>();
			inputs = ObjectArray<UGen>();
			graphs = ObjectArray<UGen>();
			ids = IntArray();
		}
	}
}

int64 AudioIOHelper::getChunkPosition(AudioFormatReader* reader, const char* name)
{
	String format = reader->getFormatName();
	
	if(format == "WAV file")
	{
		return getWavChunkPosition(reader->input, name);
	}
	else if(format == "AIFF file")
	{
		return getAiffChunkPosition(reader->input, name);
	}
	
	return -1;
}

CuePointArray AudioIOHelper::getCuePoints(AudioFormatReader* reader)
{
	String format = reader->getFormatName();
	
	if(format == "WAV file")
	{
		return getWavCuePoints(reader->input);
	}
	else if(format == "AIFF file")
	{
		return getAiffCuePoints(reader->input);
	}	
	
	return CuePointArray();
}


void AudioIOHelper::writeCuePoints(AudioFormatWriter* &writer, FileOutputStream* output, CuePointArray const& cues)
{
	String format = writer->getFormatName();
	
	if(format == "WAV file")
	{
		writeWavCuePoints(writer, output, cues);
		
	}
	else if(format == "AIFF file")
	{
		writeAiffCuePoints(writer, output, cues);
	}		
}

int64 AudioIOHelper::getWavChunkPosition(InputStream* input, const char* name)
{		
	const int64 originalPosition = input->getPosition();
	int64 currentPosition = 0;
	input->setPosition(currentPosition);
	
	int64 chunkPosition = -1;
	
	if (input->readInt() == chunkName ("RIFF"))
	{
		if(chunkName(name) == chunkName ("RIFF"))
		{
			chunkPosition = currentPosition;
			goto exit;
		}
		
		const uint32 len = (uint32) input->readInt();
		const int64 end = input->getPosition() + len;
		
		currentPosition = input->getPosition();
		
		if (input->readInt() == chunkName ("WAVE"))
		{
			if(chunkName(name) == chunkName ("WAVE"))
			{
				chunkPosition = currentPosition;
				goto exit;
			}
			
			currentPosition = input->getPosition();
			
			while (currentPosition < end && !input->isExhausted())
			{
				const int chunkType = input->readInt();
				
				if (chunkType == chunkName(name))
				{
					chunkPosition = currentPosition;
					goto exit;
				}
				
				uint32 length = (uint32) input->readInt();
				
//				if(!length) continue;
				
				const int64 chunkEnd = input->getPosition() + length + (length & 1);

				input->setPosition (chunkEnd);
				currentPosition = chunkEnd;
			}
		}
	}
    
	
exit:
	input->setPosition(originalPosition);
	return chunkPosition;
}

CuePointArray AudioIOHelper::getWavCuePoints(InputStream* input)
{
	CuePointArray cuePoints;
	int chunkType;

	int64 cueChunk = AudioIOHelper::getWavChunkPosition(input, "cue ");

	if(cueChunk >= 0)
	{		
		// get cue points
		
		input->setPosition(cueChunk);

		chunkType = input->readInt();
		/*const uint32 cueChunkLength = (uint32)*/ input->readInt();
		const int numCuePoints = input->readInt();
			
		for(int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{
			CuePoint cuePoint;
			
			cuePoint.getID() = input->readInt();
			input->readInt(); // position
			input->readInt(); // dataChunkID
			input->readInt(); // chunkStart
			input->readInt(); // blockStart
			cuePoint.getSampleOffset() = input->readInt();
					
			cuePoints.add(cuePoint);
		}
	}
	
	int64 listChunk = AudioIOHelper::getWavChunkPosition(input, "LIST");

	if(listChunk >= 0)
	{
		// get names for cue points if these are there
		
		input->setPosition(listChunk);
		
		chunkType = input->readInt();
		const uint32 listChunkLength = (uint32) input->readInt();
		
		if(listChunkLength > 0)
		{
			uint32 listChunkRemaining = listChunkLength;
					
			const int typeID = input->readInt(); 
			(void)typeID;
			listChunkRemaining -= 4;
			
			ugen_assert(typeID == chunkName("adtl"));
			
			while(listChunkRemaining > 0)
			{
				chunkType = input->readInt();
				listChunkRemaining -= 4;
				
				const uint32 chunkLength = (uint32) input->readInt();
				listChunkRemaining -= 4;
				
//				if(!chunkLength) continue;
				
				const uint32 paddedChunkLength = chunkLength + (chunkLength & 1);
				const uint32 chunkEnd = input->getPosition() + paddedChunkLength;
				
				Text text;
				int cueID = -1;
				
				// must rewrite this to skip other chunks e.g. 'ltxt'
				
				if ((chunkType == chunkName("labl")) || (chunkType == chunkName("note")))
				{
					cueID = input->readInt();

					char c[2];
					
					do
					{
						c[0] = input->readByte();
						c[1] = input->readByte();
						
						if(c[0]) 
						{
							text.add(c[0]);
							
							if(c[1])
							{
								text.add(c[1]);
							}
						}
					} while(c[0] && c[1]);
				}
				
				if (chunkType == chunkName("labl"))
				{
					cuePoints[cueID].getLabel() = text;
				}
				else if (chunkType == chunkName("note"))
				{
					cuePoints[cueID].getComment() = text;
				}
				
				listChunkRemaining -= paddedChunkLength;
				input->setPosition(chunkEnd);
			}
		}
	}
		
	return cuePoints;
}

int64 AudioIOHelper::getAiffChunkPosition(InputStream* input, const char* name)
{
	const int64 originalPosition = input->getPosition();
	int64 currentPosition = 0;
	input->setPosition(currentPosition);
	
	int64 chunkPosition = -1;
	
	if (input->readInt() == chunkName ("FORM"))
	{
		if(chunkName(name) == chunkName ("FORM"))
		{
			chunkPosition = currentPosition;
			goto exit;
		}
		
		const int len = input->readIntBigEndian();
		const int64 end = input->getPosition() + len;
		
		currentPosition = input->getPosition();

		const int nextType = input->readInt();
		if (nextType == chunkName ("AIFF") || nextType == chunkName ("AIFC"))
		{
			if(chunkName(name) == nextType)
			{
				chunkPosition = currentPosition;
				goto exit;
			}
			
			currentPosition = input->getPosition();
						
			while (currentPosition < end && !input->isExhausted())
			{
				const int type = input->readInt();
								
				if (type == chunkName(name))
				{
					chunkPosition = currentPosition;
					goto exit;
				}				
				
				const uint32 length = (uint32) input->readIntBigEndian();
				const int64 chunkEnd = input->getPosition() + length;
				
				input->setPosition (chunkEnd);
				currentPosition = chunkEnd;
			}
		}
	}

exit:
	input->setPosition(originalPosition);
	return chunkPosition;	
}

CuePointArray AudioIOHelper::getAiffCuePoints(InputStream* input)
{
	CuePointArray cuePoints;

	int64 markChunk = AudioIOHelper::getAiffChunkPosition(input, "MARK");
	
	if(markChunk >= 0)
	{	
		int chunkType;
		
		input->setPosition(markChunk);
		chunkType = input->readInt();
		/*const uint32 markChunkLength = (uint32)*/ input->readIntBigEndian();
		const int numCuePoints = (int)input->readShortBigEndian();
			
		for (int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{
			CuePoint cuePoint;
			
			cuePoint.getID() = (int)input->readShortBigEndian();
			cuePoint.getSampleOffset() = (int)input->readIntBigEndian();
			
			const char stringLen = input->readByte();
					
			for(int charIndex = 0; charIndex < stringLen; charIndex++)
			{
				char c = input->readByte();
				cuePoint.getLabel().add(c);
			}
			
			if((stringLen & 1) == 0) input->readByte();
			
			cuePoints.add(cuePoint);
		}
	}

	// could get comments here...
	int64 commentChunk = AudioIOHelper::getAiffChunkPosition(input, "COMT");
	
	if(commentChunk >= 0)
	{	
//		int chunkType;
//		
//		input->setPosition(commentChunk);
//		chunkType = input->readInt();
//		/*const uint32 commentChunkLength = (uint32)*/ input->readIntBigEndian();
//		const int numComments = (int)input->readShortBigEndian();
		
		//etc
	}
	
	
	
	return cuePoints;
}


void AudioIOHelper::writeWavCuePoints(AudioFormatWriter* &writer, FileOutputStream* output, CuePointArray const& cues)
{
	uint32 sizeWritten = 0;
	const int numCuePoints = cues.length();

	if(numCuePoints > 0)
	{		
		output->writeInt(chunkName("cue ")); 
		sizeWritten += 4;
		
		const uint32 cueChunkLength = numCuePoints * 24 + 4;
		output->writeInt(cueChunkLength);
		output->writeInt(numCuePoints);
				
		// measure how many bytes our LIST chunk needs
		uint32 listSize = 4; // account for the 'adtl' entry
		
		for(int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{
			CuePoint cuePoint = cues[cueIndex];

			// renumber IDs.. use cueIndex
			cuePoint.getID() = cueIndex;
			output->writeInt(cueIndex);
			output->writeInt(0);
			output->writeInt(chunkName("data"));
			output->writeInt(0);
			output->writeInt(0);
			output->writeInt(cuePoint.getSampleOffset());
			
			listSize += 12; // minimum size for each 'labl' chunk
			
			int labelSize = cuePoint.getLabel().size();
			
			if(labelSize & 1) labelSize++; // round to even
			
			listSize += labelSize;
			
			if(cuePoint.getComment().length() > 0)
			{
				listSize += 12; // minimum size for each 'note' chunk
				
				int commentSize = cuePoint.getComment().size();
				if(commentSize & 1) commentSize++; // round to even

				listSize += commentSize;
			}			
		}
		
		sizeWritten += cueChunkLength;
		
		output->writeInt(chunkName("LIST"));
		output->writeInt(listSize);
		
		sizeWritten += 8;
		
		output->writeInt(chunkName("adtl"));
		for(int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{
			CuePoint cuePoint = cues[cueIndex];

			output->writeInt(chunkName("labl"));
			int labelSize = cuePoint.getLabel().size() + 4;
			output->writeInt(labelSize);
			output->writeInt(cueIndex);
			
			Text label = cuePoint.getLabel();
			char* str = label.getArray();
			
			for(int i = 0; i < label.size(); i++)
			{
				output->writeByte(str[i]);
			}
			
			if(labelSize & 1) output->writeByte(0); // pad
			
			Text comment = cuePoint.getComment();
			
			if(comment.length() > 0)
			{
				output->writeInt(chunkName("note"));
				int commentSize = comment.size() + 4;
				output->writeInt(labelSize);
				output->writeInt(cueIndex);
				
				char* str = comment.getArray();
				
				for(int i = 0; i < comment.size(); i++)
				{
					output->writeByte(str[i]);
				}
				
				if(commentSize & 1) output->writeByte(0); // pad
			}
		}
		
		sizeWritten += listSize;
	}
	
	if(sizeWritten > 0)
	{
		File file = output->getFile();
		delete writer; // force the file to close and write the (incorrect) header
		writer = 0; // zero the caller's writer
		
		// find the current size it wrote to the header
		FileInputStream* input = file.createInputStream();
		input->setPosition(4);
		uint32 originalSize = input->readInt();
		delete input;
		
		output = file.createOutputStream();
		ugen_assert(output);
		
		// write the new header adding on our additional content
		output->setPosition(4);
		output->writeInt(originalSize + sizeWritten);
		
		delete output;
	}
}

void AudioIOHelper::writeAiffCuePoints(AudioFormatWriter* &writer, FileOutputStream* output, CuePointArray const& cues)
{
	uint32 sizeWritten = 0;
	const int numCuePoints = cues.length();
	
	if(numCuePoints > 0)
	{
		output->writeInt(chunkName("MARK"));
		sizeWritten += 4;
		
		// measure the data size
		uint32 listSize = 2; // num cue points
		for (int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{
			listSize += 2; // ID
			listSize += 4; // sample offset
			
			char stringLen = cues[cueIndex].getLabel().length();
			listSize++; // size char
			listSize += stringLen; // content chars
			
			if((stringLen & 1) == 0) listSize++; // pad
		}
		
		output->writeIntBigEndian((int)listSize);
		sizeWritten += 4;
		
		output->writeShortBigEndian((short)numCuePoints);
		
		for (int cueIndex = 0; cueIndex < numCuePoints; cueIndex++)
		{			
			CuePoint cuePoint = cues[cueIndex];

			// renumber IDs using cue index + 1 (0 is an invalid cue ID for AIFF)
			short cueID = (short)(cueIndex + 1);
			cuePoint.getID() = cueID;
			output->writeShortBigEndian(cueID);
			output->writeIntBigEndian((int)cuePoint.getSampleOffset());
			
			Text label = cuePoint.getLabel();
			char* str = label.getArray();

			char stringLen = label.length();
			output->writeByte(stringLen);
			
			for(int charIndex = 0; charIndex < stringLen; charIndex++)
			{
				output->writeByte(str[charIndex]);
			}
			
			if((stringLen & 1) == 0) output->writeByte(0); //pad
		}
		
		sizeWritten += listSize;
		
		// could add COMT comment chunks here ...
		//..
	}
	
	if(sizeWritten > 0)
	{
		File file = output->getFile();
		delete writer; // force the file to close and write the (incorrect) header
		writer = 0; // zero the caller's writer
		
		// find the current size it wrote to the header
		FileInputStream* input = file.createInputStream();
		input->setPosition(4);
		uint32 originalSize = input->readIntBigEndian();
		delete input;
		
		output = file.createOutputStream();
		ugen_assert(output);
		
		// write the new header adding on our additional content
		output->setPosition(4);
		output->writeIntBigEndian(originalSize + sizeWritten);
		
		delete output;
	}
}


ScopedIgnoreMouse::ScopedIgnoreMouse(Component *c) 
:	comp(c)
{
	comp->setInterceptsMouseClicks(false, false);
}

ScopedIgnoreMouse::~ScopedIgnoreMouse()
{
	comp->setInterceptsMouseClicks(true, true);
}


Interceptor::Interceptor(Component* _owner, const bool _constrain) 
:	owner(_owner),
	mouseIsOver(0),
	mouseIsDownOn(0),
	constrain(_constrain)
{ 
	setInterceptsMouseClicks(true, true);
}

void Interceptor::mouseEnter (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	mouseMove(e);
}

void Interceptor::mouseExit (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	
	if(mouseIsDownOn)
		mouseIsDownOn->mouseUp(e.getEventRelativeTo(mouseIsDownOn));
	
	mouseIsDownOn = 0;
	
	if(mouseIsOver)
		mouseIsOver->mouseExit(e.getEventRelativeTo(mouseIsOver));
	
	mouseIsOver = 0;
}

void Interceptor::mouseMove (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	
	Component *comp = owner->getComponentAt(e.x, e.y);
	
	if(comp == mouseIsOver)
	{
		if(mouseIsOver)
			mouseIsOver->mouseMove(e.getEventRelativeTo(mouseIsOver));
	}
	else
	{
		if(mouseIsOver)
			mouseIsOver->mouseExit(e.getEventRelativeTo(mouseIsOver));
		
		mouseIsOver = comp;
		
		if(mouseIsOver)
			mouseIsOver->mouseEnter(e.getEventRelativeTo(mouseIsOver));
	}
}

void Interceptor::mouseDown (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	
	if(mouseIsOver)
	{
		mouseIsDownOn = mouseIsOver;
		mouseIsDownOn->mouseDown(e.getEventRelativeTo(mouseIsDownOn));
		
		lastDragX = e.x;
		lastDragY = e.y;
	}
}

void Interceptor::mouseUp (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	
	if(mouseIsDownOn)
	{
		mouseIsDownOn->mouseUp(e.getEventRelativeTo(mouseIsDownOn));
		mouseIsDownOn = 0;
	}
}

void Interceptor::mouseDrag (const MouseEvent& e)
{
	ScopedIgnoreMouse m(this);
	
	Component *comp = owner->getComponentAt(e.x, e.y);
	
	if(comp == mouseIsDownOn)
	{
		if(mouseIsDownOn)
			mouseIsDownOn->mouseDrag(e.getEventRelativeTo(mouseIsDownOn));
	}
	else
	{
		int deltaX = e.x - lastDragX;
		int deltaY = e.y - lastDragY;
		double incX, incY;
		int steps;
		
		if(deltaX == 0)
		{
			incX = 0;
			incY = e.y > lastDragY ? 1.0 : -1.0;
			steps = std::abs(deltaY);
		}
		else if(deltaY == 0)
		{
			incX = e.x > lastDragX ? 1.0 : -1.0;
			incY = 0;
			steps = std::abs(deltaX);
		}
		else if(std::abs(deltaX) > std::abs(deltaY))
		{
			incX = e.x > lastDragX ? 1.0 : -1.0;;
			incY = (double)deltaY / std::abs(deltaX);
			steps = std::abs(deltaX);
		}
		else
		{
			incX = (double)deltaX / std::abs(deltaY);
			incY = e.y > lastDragY ? 1.0 : -1.0;
			steps = std::abs(deltaY);
		}
		
		double x = lastDragX;
		double y = lastDragY;
		
		while(steps-- > 0)
		{
			MouseEvent eventCopy = e.withNewPosition(Point<int>(constrain ? jlimit(1, getWidth()-2, (int)x) : (int)x, 
																constrain ? jlimit(1, getHeight()-2, (int)y) : (int)y));
			
			Component *comp = owner->getComponentAt(eventCopy.x, eventCopy.y);
			if(comp == mouseIsDownOn)
			{
				if(mouseIsDownOn)
					mouseIsDownOn->mouseDrag(eventCopy.getEventRelativeTo(mouseIsDownOn));
			}
			else
			{
				if(mouseIsDownOn)
				{
					mouseIsDownOn->mouseUp(eventCopy.getEventRelativeTo(mouseIsDownOn));
					mouseIsDownOn->mouseExit(eventCopy.getEventRelativeTo(mouseIsDownOn));
				}
				
				mouseIsDownOn = comp;
				
				if(mouseIsDownOn)
				{
					mouseIsDownOn->mouseEnter(eventCopy.getEventRelativeTo(mouseIsDownOn));
					mouseIsDownOn->mouseDown(eventCopy.getEventRelativeTo(mouseIsDownOn));
				}
			}
			
			x += incX;
			y += incY;
		}
	}
	
	lastDragX = e.x;
	lastDragY = e.y;
}	




END_UGEN_NAMESPACE

#endif
