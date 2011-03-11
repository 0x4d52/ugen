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




#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceScope.h"


static inline const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);
    const char* const sign = (seconds < 0) ? "-" : "";
	
    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;
	const int ms = roundDoubleToInt (absSecs * 1000) % 1000;
	
	const int size = 128;
	char buf[size];
	
	if(hours == 0)
	{
		if(mins == 0)
		{
			snprintf(buf, size, "%s%01d.%03d", sign, secs, ms);			
		}
		else
		{
			snprintf(buf, size, "%s%01d:%02d.%03d", sign, mins, secs, ms);			
		}
	}
	else
	{
		snprintf(buf, size, "%s%01d:%02d:%02d.%03d", sign, hours, mins, secs, ms);
	}
	
	return String(buf);
}


ScopeComponentBase::ScopeComponentBase(ScopeGUI::ScopeStyles style)
:	ScopeGUI(style)
{
}

void ScopeComponentBase::resized()
{
	resizedGUI();
}

void ScopeComponentBase::handleCommandMessage (int commandId)
{
	switch(commandId)
	{
		case Repaint: {
			repaint();
		} break;
	}
}

void ScopeComponentBase::updateGUI() throw()
{
	hasDisplayed = false;
	postCommandMessage(Repaint);
}

bool ScopeComponentBase::isValid() throw()
{
	return true;//isValidComponent();
}

void ScopeComponentBase::lock() throw()
{
	juceLock.enter();
}

void ScopeComponentBase::unlock() throw()
{
	juceLock.exit();
}

bool ScopeComponentBase::tryLock() throw()
{
	return juceLock.tryEnter();
}

ScopeComponent::ScopeComponent(ScopeStyles style)
:	ScopeComponentBase(style)
{
}

void ScopeComponent::paint(Graphics& g)
{	
	g.setColour(Colour(colours[Background].get32bitColour()));
	g.fillRect(0, 0, getWidth(), getHeight());

	calculateBuffers();
	
	lock();
	
	if(minDrawBuffer.size() > 0 && maxDrawBuffer.size() > 0)
	{		
		if(isBipolar)
			paintBipolar(g);
		else
			paintUnipolar(g);
	}
	
	unlock();
	
	hasDisplayed = true;
}

void ScopeComponent::paintBipolar(Graphics& g)
{
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		
		const int channelHeight = getHeight() / numChannels;
		const int halfChannelHeight = channelHeight / 2;
		
		const float halfChannelHeightOverYMaximum = halfChannelHeight / yMaximum;
			
		for(int channel = 0; channel < numChannels; channel++)
		{
			const int top = channelHeight * channel;
			const int middle = top + halfChannelHeight;
			const int bottom = top + channelHeight;
			
			if(channel > 0)
			{
				g.setColour(Colour(colours[TopLine].get32bitColour()));
				//g.drawHorizontalLine(top, 0, getWidth());
				g.drawLine(0, top - 0.5f, getWidth(), top - 0.5f, 1.f);
			}
			
			paintXScale(g, middle);
			paintYScale(g, middle, top);
			paintYScale(g, middle, bottom);
			
			float oldMinimum = clip2(minDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;
			
			g.setColour(Colour(colours[Trace].get32bitColour()));
			g.drawVerticalLine(0, middle - oldMaximum, middle - oldMinimum);
			
			oldMinimum += 1.f;
			oldMaximum -= 1.f;
									
			for(int i = 1; i < bufferSizeMinus1; i++)
			{				
				const float minimum = clip2(minDrawBuffer.getSampleUnchecked(channel, i), yMaximum) * halfChannelHeightOverYMaximum;
				const float maximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, i), yMaximum) * halfChannelHeightOverYMaximum;
				
				float lineTop = max(maximum, oldMinimum);
				float lineBottom = min(minimum, oldMaximum);
				
				if(style_ == ScopeGUI::Bars)
				{
					if(lineTop < 0.f && lineBottom < 0.f)
					{
						lineTop = 0.f;
						lineBottom = min(lineTop, lineBottom);
					}
					else if(lineTop > 0.f && lineBottom > 0.f)
					{
						lineTop = max(lineTop, lineBottom);
						lineBottom = 0.f;
					}
				}
				
				lineTop = middle - lineTop;
				lineBottom = middle - lineBottom;
								
				g.drawVerticalLine(i, lineTop, lineBottom);				
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
			
			paintChannelLabel(g, channelLabels.wrapAt(channel), channel, top);
		}		
	}
}

void ScopeComponent::paintUnipolar(Graphics& g)
{
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		
		const int channelHeight = getHeight() / numChannels;
		const float channelHeightOverYMaximum = channelHeight / yMaximum;
			
		for(int channel = 0; channel < numChannels; channel++)
		{
			const int top = channelHeight * channel;
			const int bottom = top + channelHeight;
			
			if(channel > 0)
			{
				g.setColour(Colour(colours[TopLine].get32bitColour()));
				//g.drawHorizontalLine(top, 0, getWidth());
				g.drawLine(0, top - 0.5f, getWidth(), top - 0.5f, 1.f);
			}
			
			paintXScale(g, bottom);
			paintYScale(g, bottom, top);
			
			float oldMinimum = clip(minDrawBuffer.getSampleUnchecked(channel, 0), 0.f, yMaximum) * channelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip(maxDrawBuffer.getSampleUnchecked(channel, 0), 0.f, yMaximum) * channelHeightOverYMaximum + 0.5f;
			
			g.setColour(Colour(colours[Trace].get32bitColour()));
			g.drawVerticalLine(0, bottom - oldMaximum, bottom - oldMinimum);
			
			oldMinimum += 1.f;
			oldMaximum -= 1.f;
			
			for(int i = 1; i < bufferSizeMinus1; i++)
			{				
				const float minimum = clip(minDrawBuffer.getSampleUnchecked(channel, i), 0.f, yMaximum) * channelHeightOverYMaximum;
				const float maximum = clip(maxDrawBuffer.getSampleUnchecked(channel, i), 0.f, yMaximum) * channelHeightOverYMaximum;
				
				float lineTop = max(maximum, oldMinimum);
				float lineBottom = min(minimum, oldMaximum);
							
				if(style_ == ScopeGUI::Bars)
				{
					if(lineTop < 0.f && lineBottom < 0.f)
					{
						lineTop = 0.f;
						lineBottom = min(lineTop, lineBottom);
					}
					else if(lineTop > 0.f && lineBottom > 0.f)
					{
						lineTop = max(lineTop, lineBottom);
						lineBottom = 0.f;
					}
				}
				
				lineTop = bottom - lineTop;
				lineBottom = bottom - lineBottom;
				
				g.drawVerticalLine(i, lineTop, lineBottom);				
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
					
			paintChannelLabel(g, channelLabels.wrapAt(channel), channel, top);
		}		
	}
}

void ScopeComponent::paintXScale(Graphics& g, const int y)
{
	g.setColour(Colour(colours[ZeroLine].get32bitColour()));
	//g.drawHorizontalLine(y, 0, getWidth());
	g.drawLine(0, y - 0.5f, getWidth(), y - 0.5f, 1.f);
	
	if(scaleX != ScopeGUI::LabelXNone)
	{		
		const double audioBufferSize = audioBuffer.size();
		const double samplesPerPixel = (double)getDisplayBufferSize() / audioBufferSize;
		
		const int markSpacing = markSpacingX;
		const unsigned int labelHop = labelHopX;

		unsigned int mark = 0;
		const int labelOffsetX = 3;
		const int labelOffsetY = markXHeight > 5 ? 4 : markXHeight + 1;
		
		g.setFont(textSizeX);
		g.setColour(Colour(colours[LabelMarks].get32bitColour()));
		
		for(int audioIndex = 0; audioIndex < audioBufferSize; audioIndex += markSpacing)
		{			
			const int x = audioIndex * samplesPerPixel + 0.5;
			
			if(((labelHop <= 1) || ((mark % labelHop) == 0)))
			{
				g.drawVerticalLine(x, y - markXHeight * 1.5, y);
				
				
				if(scaleX != ScopeGUI::LabelXMarks && 
				   (((mark == 0) && (labelFirstX == false)) == false)
				) {
					String label;
					if(scaleX == ScopeGUI::LabelXTime)
					{
						label = timeToTimecodeString((audioIndex + offsetSamples) / UGen::getSampleRate());
					}
					else if((scaleX == ScopeGUI::LabelXFrequency) && (fftSize > 0))
					{
						const double fftFreq = UGen::getSampleRate() / fftSize;
						const double binFreq = (audioIndex + offsetSamples) * fftFreq;
						
						if(binFreq >= 1000.0)
							label = String(binFreq * 0.001, 2) + T("kHz");
						else
							label = String((int)(binFreq + 0.5)) + T("Hz");
					}
					else
					{
						label = String((int)(audioIndex + offsetSamples + 0.5));
					}
					
					g.setColour(Colour(colours[TextX].get32bitColour()));
					g.drawSingleLineText(label, x + labelOffsetX, y - labelOffsetY);
					g.setColour(Colour(colours[LabelMarks].get32bitColour()));
				}
			}
			else
			{
				g.drawVerticalLine(x, y-markXHeight, y);
			}
			
			mark++;
		}
		
	}
}

void ScopeComponent::paintYScale(Graphics& g, const int zero, const int maximum)
{	
	if(scaleY != ScopeGUI::LabelYNone)
	{		
		const double markSpacing = markSpacingY;
		const int decimalPlaces = decimalPlacesY;
		const unsigned int labelHop = labelHopY;
		
		unsigned int mark = 0;
		const int labelOffsetX = markYWidth * 1.5 + 2;
		const int labelOffsetY = textSizeY / 2 + 2;
		
		const int height = zero-maximum;
		const double pixelFactor = (double)height / yMaximum;
		
		const double sign = height > 0 ? 1.0 : -1.0;
		
		g.setFont(textSizeY);
		g.setColour(Colour(colours[LabelMarks].get32bitColour()));
		
		for(double level = 0.0; level <= yMaximum; level += markSpacing)
		{
			const int y = zero - level * pixelFactor;
						
			if((((labelHop <= 1) || ((mark % labelHop) == 0)) && 
			   (mark > 0) && 
			   ((level+markSpacing) <= yMaximum))
			) {
				//g.drawHorizontalLine(y, 0, markYWidth * 1.5);
				g.drawLine(0, y - 0.5f, markYWidth * 1.5, y - 0.5f, 1.f);
				if(scaleY != ScopeGUI::LabelYMarks)
				{
					g.setColour(Colour(colours[TextY].get32bitColour()));
					g.drawSingleLineText(String(sign * level, decimalPlaces), labelOffsetX, y + labelOffsetY);
					g.setColour(Colour(colours[LabelMarks].get32bitColour()));
				}
			}
			else
			{
				//g.drawHorizontalLine(y, 0, markYWidth);
				g.drawLine(0, y - 0.5f, markYWidth, y - 0.5f, 1.f);
			}
			
			mark++;
		}
	}
}

void ScopeComponent::paintChannelLabel(Graphics& g, Text const& label, const int index, const int top)
{
	if(labelChannels)
	{
		const int size = 256;
		char labelToPaint[size];
		
		if(label.length() == 0)
		{
			snprintf(labelToPaint, size, "Ch%d", index + channelLabelOffset);
		}
		else
		{
			snprintf(labelToPaint, size, (const char*)label, index + channelLabelOffset);
		}
		
		g.setFont(textSizeChannel);
		g.setColour(Colour(colours[TextChannel].get32bitColour()));
		
		const int left = scaleY == ScopeGUI::LabelYNone ? 3 : markYWidth * 1.5 + 3;
		
		g.drawText(labelToPaint, 
				   left, 
				   top + 3, 
				   getWidth() - left - 3, 
				   textSizeChannel, 
				   Justification::right, 
				   false);
	}
}

ScopeCuePointComponent::ScopeCuePointComponent(ScopeControlComponent* o, ScopeRegionComponent* r)
:	owner(o),
	region(r),
	offsetSamples(0.0),
	beingDragged(false)
{
	setMouseCursor(MouseCursor::LeftRightResizeCursor);
	
	cueData.textColour = owner->getColour(ScopeControlComponent::CuePointTextColour);
	cueData.lineColour = owner->getColour(ScopeControlComponent::CuePointColour);
	
	owner->addAndMakeVisible(cueData.label = new Label("CueLabel",""));
	cueData.label->setFont(10);
	
	cueData.label->setColour(Label::textColourId, Colour(cueData.textColour.get32bitColour()));
}

ScopeCuePointComponent::~ScopeCuePointComponent()
{
	if(owner != 0)
	{
		owner->removeChildComponent(cueData.label);
	}
		
	cueData.label.deleteAndZero();
}

void ScopeCuePointComponent::setHeight(const int height)
{
	setSize(3, height);
	checkPosition();
	setLabelPosition();
}

void ScopeCuePointComponent::checkPosition()
{	
	double bufferOffsetSamples = owner->getSampleOffset();
	const double audioBufferSize = owner->getAudioBuffer().size();
	const double pixelsPerSample = owner->getDisplayBufferSize() / audioBufferSize;

	int x = (offsetSamples - bufferOffsetSamples) * pixelsPerSample + 0.5; 
	
	setTopLeftPosition(x-1, 0);
}

void ScopeCuePointComponent::paint(Graphics& g)
{	
	g.setColour(Colour(cueData.lineColour.get32bitColour())); // could use a virtual function to get this...
	g.drawVerticalLine(1, 0, getHeight());
}

void ScopeCuePointComponent::setSampleOffset(const double newOffsetSamples)
{
	offsetSamples = newOffsetSamples;
	checkPosition();
}

void ScopeCuePointComponent::mouseDown (const MouseEvent& e)
{
	// temp - need to do this manually probably
	dragger.startDraggingComponent (this, e);
	beingDragged = true;
}

void ScopeCuePointComponent::mouseDrag (const MouseEvent& e)
{
	// temp - need to do this manually probably
	constrain.setMinimumOnscreenAmounts(0xffffff, 0, 0xffffff, 0);
	dragger.dragComponent (this, e, &constrain);
}

void ScopeCuePointComponent::mouseUp (const MouseEvent& e)
{
	beingDragged = false;
}

void ScopeCuePointComponent::moved()
{
	// check if this is begin dragged and if so..
	if(beingDragged)
	{
		double bufferOffsetSamples = owner->getSampleOffset();
		const double audioBufferSize = owner->getAudioBuffer().size();
		const double samplesPerPixel = audioBufferSize / owner->getDisplayBufferSize();
		int x = getX()+1;
		
		offsetSamples = x * samplesPerPixel + bufferOffsetSamples; // rounding?
	}
	
	
	if(region != 0)
	{
		// perhaps do thi with a listener instead ??
		region->checkPosition();
	}
	
	setLabelPosition();
}

void ScopeCuePointComponent::setLabelPosition()
{
	if(cueData.label != 0)
	{
		cueData.label->setTopLeftPosition(getX(), getY());
	}	
}

Text ScopeCuePointComponent::getLabel() const
{
	return cueData.label->getText(false);
}

void ScopeCuePointComponent::setLabel(Text const& text)
{
	if(cueData.label != 0)
	{
		cueData.label->setSize(text.length() * 10,  // need to measure really... and do it in a listener if we can edit the labels
							   cueData.label->getFont().getHeight());
		cueData.label->setText((const char*)text, true);
	}
}

void ScopeCuePointComponent::setColours(RGBAColour const& lineColour, RGBAColour const& textColour)
{
	cueData.lineColour = lineColour;
	cueData.textColour = textColour;
	
	if(cueData.label != 0)
	{
		cueData.label->setColour(Label::textColourId, Colour(cueData.textColour.get32bitColour()));
	}

	repaint();
}

void ScopeCuePointComponent::swapCuePoints(Component::SafePointer<ScopeCuePointComponent> &cue1, 
										   Component::SafePointer<ScopeCuePointComponent> &cue2)
{	
	// swap data
	swapVariables(cue1->cueData, cue2->cueData);
	swapVariables(cue1, cue2);
	
	// check labels
	cue1->setLabelPosition();
	cue2->setLabelPosition();
	
	// check cue colour is correct
	cue1->repaint();
	cue2->repaint();
}

ScopeRegionComponent::ScopeRegionComponent(ScopeControlComponent* o)
:	owner(o),
	fillColour(owner->getColour(ScopeControlComponent::RegionFillColour)),
	changingBoth(false)
{
	setInterceptsMouseClicks(false, false);
	
	RGBAColour startColour = owner->getColour(ScopeControlComponent::RegionStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::RegionEndColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::RegionTextColour);

	owner->addAndMakeVisible(startPoint = new ScopeCuePointComponent(owner, this));
	startPoint->setLabel("start");
	startPoint->setColours(startColour, textColour);
	
	owner->addAndMakeVisible(endPoint = new ScopeCuePointComponent(owner, this));
	endPoint->setLabel("end");
	endPoint->setColours(endColour, textColour);
}

ScopeRegionComponent::~ScopeRegionComponent()
{
	if(owner != 0) 
	{
		owner->removeChildComponent(startPoint);
		owner->removeChildComponent(endPoint);
	}
	
	startPoint.deleteAndZero();
	endPoint.deleteAndZero();
}

void ScopeRegionComponent::getRegionPosition(int& start, int& end)
{ 
	start = startPoint->getCuePosition();
	end = endPoint->getCuePosition();
	
	if(start > end)
	{
		ScopeCuePointComponent::swapCuePoints(startPoint, endPoint);
		start = startPoint->getCuePosition();
		end = endPoint->getCuePosition();
	}
}

void ScopeRegionComponent::setRegionOffsets(const double start, const double end)
{
	changingBoth = true;
	startPoint->setSampleOffset(jmin(start, end));
	endPoint->setSampleOffset(jmax(start, end));
	changingBoth = false;
	checkPosition();
}

void ScopeRegionComponent::checkPosition()
{
	if(!changingBoth)
	{
		int start, end;
		getRegionPosition(start, end);
		setBounds(start, 0, end-start+1, getParentHeight());
	}
}

void ScopeRegionComponent::setHeight(const int height)
{
	changingBoth = true;
	startPoint->setHeight(height);
	endPoint->setHeight(height);
	changingBoth = false;
	checkPosition();
}

void ScopeRegionComponent::paint(Graphics& g)
{		
	g.setColour(Colour(fillColour.get32bitColour()));
	g.fillRect(1, 0, getWidth()-2, getHeight());
}

void ScopeRegionComponent::setColours(RGBAColour const& startColour, 
									  RGBAColour const& endColour, 
									  RGBAColour const& textColour, 
									  RGBAColour const& newFillColour)
{
	startPoint->setColours(startColour, textColour);
	endPoint->setColours(endColour, textColour);
	fillColour = newFillColour;
	repaint();
}

ScopeSelectionComponent::ScopeSelectionComponent(ScopeControlComponent* owner)
:	ScopeRegionComponent(owner)
{
	RGBAColour startColour = owner->getColour(ScopeControlComponent::SelectionStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::SelectionEndColour);
	RGBAColour fillColour = owner->getColour(ScopeControlComponent::SelectionFillColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::SelectionTextColour);

	setColours(startColour, endColour, textColour, fillColour);
}

ScopeLoopComponent::ScopeLoopComponent(ScopeControlComponent* owner)
:	ScopeRegionComponent(owner)
{
	RGBAColour startColour = owner->getColour(ScopeControlComponent::LoopPointStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::LoopPointEndColour);
	RGBAColour fillColour = owner->getColour(ScopeControlComponent::LoopFillColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::LoopTextColour);
	
	setColours(startColour, endColour, textColour, fillColour);
}


ScopeControlComponent::ScopeControlComponent(ScopeStyles style, DisplayOptions optionsToUse)
:	ScopeComponent(style),
	options(optionsToUse),
	scopeInsert(0),
	scopeSelection(0)
{
	controlColours[CuePointColour]			= RGBAColour(1.0, 1.0, 0.0, 1.0);
	controlColours[CuePointTextColour]		= RGBAColour(1.0, 1.0, 0.0, 1.0);
	controlColours[LoopPointStartColour]	= RGBAColour(0.0, 1.0, 0.0, 1.0);
	controlColours[LoopPointEndColour]		= RGBAColour(1.0, 0.0, 0.0, 1.0);
	controlColours[LoopFillColour]			= RGBAColour(0.5, 0.5, 1.0, 0.5);
	controlColours[LoopTextColour]			= RGBAColour(0.5, 0.5, 1.0, 1.0);
	controlColours[RegionStartColour]		= controlColours[LoopPointStartColour];
	controlColours[RegionEndColour]			= controlColours[LoopPointEndColour];
	controlColours[RegionFillColour]		= RGBAColour(1.0, 1.0, 0.5, 0.5);
	controlColours[RegionTextColour]		= RGBAColour(1.0, 1.0, 0.5, 1.0);
	controlColours[InsertPointColour]		= RGBAColour(1.0, 1.0, 1.0, 1.0);
	controlColours[InsertTextColour]		= RGBAColour(1.0, 1.0, 1.0, 1.0);
	controlColours[SelectionStartColour]	= controlColours[LoopPointStartColour];
	controlColours[SelectionEndColour]		= controlColours[LoopPointEndColour];
	controlColours[SelectionFillColour]		= RGBAColour(1.0, 1.0, 1.0, 0.5);
	controlColours[SelectionTextColour]		= RGBAColour(1.0, 1.0, 1.0, 1.0);
	
	if(options & Insert)
	{
		addAndMakeVisible(scopeInsert = new ScopeInsertComponent(this, 0));
	}
	
	if(options & Selection)
	{
		addAndMakeVisible(scopeSelection = new ScopeSelectionComponent(this));
	}
}

ScopeControlComponent::~ScopeControlComponent()
{
	deleteAllChildren();
}

RGBAColour& ScopeControlComponent::getColour(ControlColours colour)
{
	return controlColours[colour];
}

void ScopeControlComponent::setMetaData(MetaData const& newMetaData)
{
//	lock();
//	metaData = newMetaData;
//	unlock();
//	
//	updateGUI();
}

void ScopeControlComponent::resized()
{
	resizedGUI();

	int height = getHeight();
	
	if(scopeInsert != 0) 
	{
		scopeInsert->setHeight(height);
	}
		
	if(scopeSelection != 0) 
	{
		scopeSelection->setHeight(height);
	}
}

void ScopeControlComponent::setInsertOffset(const double offset)
{
	if(scopeInsert != 0) 
	{
		scopeInsert->setSampleOffset(offset);
	}	
}

void ScopeControlComponent::setSelection(const double start, const double end)
{
	if(scopeSelection != 0)
	{
		scopeSelection->setRegionOffsets(start, end);
	}
}

RadialScopeComponent::RadialScopeComponent(ScopeStyles style)
:	ScopeComponentBase(style),
	sine(Buffer::getTableSine8192()),
	cosine(Buffer::getTableCosine8192())
{
	ugen_assert(sine.size() == cosine.size());
}

void RadialScopeComponent::paint(Graphics& g)
{
	g.setColour(Colour(colours[Background].get32bitColour()));
	g.fillRect(0, 0, getWidth(), getHeight());
	
	calculateBuffers();
	
	lock();
	
	if(minDrawBuffer.size() > 0 && maxDrawBuffer.size() > 0)
	{		
		paintBipolar(g);
	}
	
	unlock();
}

void RadialScopeComponent::paintBipolar(Graphics& g)
{
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const int centreX = getCentreX();
		const int centreY = getCentreY();
		const float innerMargin = min(getLowerMargin(), (float)getRadius());
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		const int channelHeight = (getRadius() - innerMargin) / numChannels;
		const int halfChannelHeight = channelHeight / 2;
		const int wrapOffset = bufferWrap * minDrawBuffer.size();
		
		const float halfChannelHeightOverYMaximum = halfChannelHeight / yMaximum;
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const int minRadius = channelHeight * channel + innerMargin;
			const int zeroRadius = minRadius + halfChannelHeight;
			const int maxRadius = minRadius + channelHeight;
			
			// outer
			g.setColour(Colour(colours[TopLine].get32bitColour()));
			g.drawEllipse(getCentreX()-maxRadius, getCentreY()-maxRadius, maxRadius*2, maxRadius*2, 1);
			
			paintXScale(g, zeroRadius);
			paintYScale(g, zeroRadius, maxRadius);
			paintYScale(g, zeroRadius, minRadius);
			
//			float oldMinimum = clip2(minDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
//			float oldMaximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;
			float oldMinimum = clip2(minDrawBuffer.wrapAt(channel, 0 + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip2(maxDrawBuffer.wrapAt(channel, 0 + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;
			
			const double delta = getDelta() * sine.size();
			double position = delta;
			
			// first scope line
			g.setColour(Colour(colours[Trace].get32bitColour()));
			g.drawVerticalLine(centreX, 
							   centreY - zeroRadius - oldMaximum, 
							   centreY - zeroRadius - oldMinimum);
			
			oldMinimum += 1.f;
			oldMaximum -= 1.f;
			
			for(int i = 1; i < bufferSizeMinus1; i++, position += delta)
			{				
//				const float minimum = clip2(minDrawBuffer.getSampleUnchecked(channel, i), yMaximum) * halfChannelHeightOverYMaximum;
//				const float maximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, i), yMaximum) * halfChannelHeightOverYMaximum;
				const float minimum = clip2(minDrawBuffer.wrapAt(channel, i + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum;
				const float maximum = clip2(maxDrawBuffer.wrapAt(channel, i + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum;
				
				float lineTop = max(maximum, oldMinimum);
				float lineBottom = min(minimum, oldMaximum);
				
				if(style_ == ScopeGUI::Bars)
				{
					if(lineTop < 0.f && lineBottom < 0.f)
					{
						lineTop = 0.f;
						lineBottom = min(lineTop, lineBottom);
					}
					else if(lineTop > 0.f && lineBottom > 0.f)
					{
						lineTop = max(lineTop, lineBottom);
						lineBottom = 0.f;
					}
				}
				
				lineTop = zeroRadius - lineTop;
				lineBottom = zeroRadius - lineBottom;
								
				const float sinAngle = sine.getSampleUnchecked(position);
				const float cosAngle = cosine.getSampleUnchecked(position);
				
				const float startX = sinAngle * lineBottom;
				const float startY = cosAngle * lineBottom;
				const float endX = sinAngle * lineTop;
				const float endY = cosAngle * lineTop;
				g.drawLine(centreX + startX, 
						   centreY - startY, 
						   centreX + endX, 
						   centreY - endY);
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
			
			//paintChannelLabel(g, channelLabels.wrapAt(channel), channel, top);
		}	
		
		
		if(innerMargin > 0)
		{			
			// inner
			g.setColour(Colour(colours[TopLine].get32bitColour()));
			g.drawEllipse(getCentreX()-innerMargin, getCentreY()-innerMargin, innerMargin*2, innerMargin*2, 1);
		}
	}
	
}

void RadialScopeComponent::paintXScale(Graphics& g, const int radius)
{
	g.setColour(Colour(colours[ZeroLine].get32bitColour()));
	
	const int diameter = radius*2;
	const int centreX = getCentreX();
	const int centreY = getCentreY();
	
	g.drawEllipse(centreX-radius, centreY-radius, diameter, diameter, 1);
	
	if(scaleX != ScopeGUI::LabelXNone)
	{		
//		ugen_assert(bufferWrap == 0.0); // need to upate this function if the waveform is rotated...
		
		const double audioBufferSize = audioBuffer.size();
		const double radiansPerSample = twoPi / audioBufferSize;
		const double radiansOffset = bufferWrap * twoPi;
		
		const int markSpacing = markSpacingX;
		const unsigned int labelHop = labelHopX;
		
		unsigned int mark = 0;
//		const int labelOffsetX = 3;
//		const int labelOffsetY = markXHeight > 5 ? 4 : markXHeight + 1;
				
		g.setFont(textSizeX);
		g.setColour(Colour(colours[LabelMarks].get32bitColour()));
		
		for(int audioIndex = 0; audioIndex < audioBufferSize; audioIndex += markSpacing)
		{			
			const float angle = audioIndex * radiansPerSample - radiansOffset;
			const float sinAngle = sin(angle);
			const float cosAngle = cos(angle);
			const float startX = sinAngle * radius;
			const float startY = cosAngle * radius;
			
			if(((labelHop <= 1) || ((mark % labelHop) == 0)))
			{				
				const float end = radius + markXHeight * 1.5;
				const float endX = sinAngle * end;
				const float endY = cosAngle * end;
				g.drawLine(centreX + startX, 
						   centreY - startY, 
						   centreX + endX, 
						   centreY - endY);
				
				if(scaleX != ScopeGUI::LabelXMarks && 
				   (((mark == 0) && (labelFirstX == false)) == false)
				   ) {
					String label;
					if(scaleX == ScopeGUI::LabelXTime)
					{
						label = timeToTimecodeString((audioIndex + offsetSamples) / UGen::getSampleRate());
					}
					else if((scaleX == ScopeGUI::LabelXFrequency) && (fftSize > 0))
					{
						const double fftFreq = UGen::getSampleRate() / fftSize;
						const double binFreq = (audioIndex + offsetSamples) * fftFreq;
						
						if(binFreq >= 1000.0)
							label = String(binFreq * 0.001, 2) + T("kHz");
						else
							label = String((int)(binFreq + 0.5)) + T("Hz");
					}
					else
					{
						label = String((int)(audioIndex + offsetSamples + 0.5));
					}
					
					g.setColour(Colour(colours[TextX].get32bitColour()));				
					const double rotation = -piOverTwo+angle;
					//if(angle > pi) rotation += pi;
					AffineTransform transform = AffineTransform::identity.rotated(rotation).translated(centreX+endX*1.01, centreY-endY*1.01);
					g.drawTextAsPath(label, transform);
					g.setColour(Colour(colours[LabelMarks].get32bitColour()));
				}
			}
			else
			{				
				const float end = radius + markXHeight;
				const float endX = sinAngle * end;
				const float endY = cosAngle * end;
				g.drawLine(centreX + startX, 
						   centreY - startY, 
						   centreX + endX, 
						   centreY - endY);
			}
			
			mark++;
		}
		
	}
	
}

void RadialScopeComponent::paintYScale(Graphics& g, const int zero, const int maximum)
{	

}

double RadialScopeComponent::getDelta() const throw()
{
	return 1.0/getDisplayBufferSize();
}

double RadialScopeComponent::getDeltaAngle() const throw()
{
	return sin(1.0/getRadius());
}

int RadialScopeComponent::getDisplayBufferSize() const throw()
{
	const double angle = getDeltaAngle();
	return twoPi / angle + 0.5;
}

int RadialScopeComponent::getRadius() const throw()
{
	return jmin(getWidth(), getHeight()) / 2;
}

int RadialScopeComponent::getCentreX() const throw()
{
	return getWidth() / 2;
}

int RadialScopeComponent::getCentreY() const throw()
{
	return getHeight() / 2;
}


MeterComponent::MeterComponent(String& name, float* valueToUse, const CriticalSection& lockToUse)
:	Component(name),
	value(valueToUse),
	lastDisplayValue(0.f),
	lock(lockToUse)
{
	startTimer((int)(0.020 * 1000));
}

MeterComponent::~MeterComponent()
{
	stopTimer();
}

void MeterComponent::paint(Graphics& g)
{
	lock.enter();
	float currentValue = jlimit(0.f, 1.f, zap(*value));
	lock.exit();
	
	g.fillAll(Colours::black);
	g.setColour(Colour(0xFF00FF00));
	
	if(getWidth() > getHeight())
	{
		// horizontal meter
		g.fillRect(0, 0, (int)(getWidth() * currentValue), getHeight());
	}
	else
	{
		// vertical meter
		g.fillRect(0, getHeight() - (int)(getHeight() * currentValue), 
				   getWidth(), (int)(getHeight() * currentValue));
	}
}

void MeterComponent::timerCallback()
{
	lock.enter();
	float currentValue = *value;
	lock.exit();
	
	if(lastDisplayValue != currentValue)
	{	
		lastDisplayValue = currentValue;
		repaint();
	}
}





END_UGEN_NAMESPACE

#endif
