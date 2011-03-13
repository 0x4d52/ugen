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

ScopeCuePointLabel::ScopeCuePointLabel(ScopeCuePointComponent *o)
:	Label("ScopeControlLabel"),
	owner(o)
{
	setMouseCursor(MouseCursor::PointingHandCursor);
	setFont(11);
	setEditable(false, true, true);
	setBorderSize(2, 2);	
}

void ScopeCuePointLabel::showPopupMenu()
{
	//?
}

TextEditor* ScopeCuePointLabel::createEditorComponent()
{
	TextEditor* editorComponent = Label::createEditorComponent();
	editorComponent->setColour(TextEditor::textColourId, findColour(Label::textColourId));
	editorComponent->setColour(TextEditor::highlightedTextColourId, findColour(Label::textColourId));
	editorComponent->setSize(jmin(getWidth(), 100), getHeight() + 2);
	return editorComponent;
}

void ScopeCuePointLabel::mouseDown(const MouseEvent& e)
{
//	if(e.mods.isPopupMenu())
//	{
//		//
//	}
//	else 
	if(owner != 0)
	{
		owner->mouseEnter(e.getEventRelativeTo(owner));
		owner->mouseDown(e.getEventRelativeTo(owner));
	}
}

void ScopeCuePointLabel::mouseDrag(const MouseEvent& e)
{
	if(owner != 0)
	{
		owner->mouseDrag(e.getEventRelativeTo(owner));
	}	
}

void ScopeCuePointLabel::mouseUp(const MouseEvent& e)
{
	if(owner != 0)
	{
		owner->mouseUp(e.getEventRelativeTo(owner));
		owner->mouseExit(e.getEventRelativeTo(owner));
	}	
}

void ScopeCuePointLabel::editorShown (TextEditor* editorComponent)
{
	oldWidth = getWidth();
	oldHeight = getHeight();
	setSize(jmin(oldWidth, 100), oldHeight + 2);
	editorComponent->setSize(jmin(oldWidth, 100), oldHeight + 2);
}

void ScopeCuePointLabel::editorAboutToBeHidden (TextEditor* editorComponent)
{
	setSize(getWidth(), oldHeight);
}

int ScopeCuePointLabel::getCuePosition()
{
	if(owner != 0)
	{
		return owner->getCuePosition();
	}	
	else
	{
		return -1;
	}
}

void ScopeCuePointLabel::checkPosition()
{
	const int moveCloser = 0;
	int labelWidth = getWidth();
	int position = getCuePosition();		
	int width = getParentWidth();
	
	if(doesPreferToAttachOnLeft())
	{
		if((position + labelWidth - 2) > width)
		{
			setTopRightPosition(position+moveCloser, 0);
		}
		else
		{
			setTopLeftPosition(position-moveCloser, 0);
		}
	}
	else
	{
		if((position - labelWidth + 2) >= 0)
		{
			setTopRightPosition(position+moveCloser, 0);
		}
		else
		{
			setTopLeftPosition(position-moveCloser, 0);
		}			
	}	
}

bool ScopeCuePointLabel::doesPreferToAttachOnLeft() const
{
	if(owner != 0)
	{
		return owner->doesLabelPreferToAttachOnLeft();
	}
	
	return true;
}

ScopeCuePointComponent::ScopeCuePointComponent(ScopeControlComponent* o, 
											   ScopeRegionComponent* r,
											   CuePoint const& cuePointToUse,
											   const bool createdFromMouseClick,
											   const bool labelPrefersToAttachOnLeft)
:	owner(o),
	region(r),
	beingDragged(createdFromMouseClick)
{
	setMouseCursor(MouseCursor::LeftRightResizeCursor);
	
	cueData.cuePoint = cuePointToUse;
	cueData.labelPrefersToAttachOnLeft = labelPrefersToAttachOnLeft;
	cueData.textColour = owner->getColour(ScopeControlComponent::CuePointTextColour);
	cueData.lineColour = owner->getColour(ScopeControlComponent::CuePointColour);
	
	owner->addPointLabel(label = new ScopeCuePointLabel(this));
	setLabel(cueData.cuePoint.getLabel());
	
	label->setColour(Label::textColourId, Colour(cueData.textColour.get32bitColour()));
	label->setColour(Label::backgroundColourId, Colour(cueData.textColour.get32bitColour()).withAlpha(0.2f));

	label->addListener(this);
}

ScopeCuePointComponent::~ScopeCuePointComponent()
{
	if(owner != 0)
	{
		owner->removeCuePoint(cueData.cuePoint);
		owner->removePointLabel(label);
	}
		
	label.deleteAndZero();
}

void ScopeCuePointComponent::choosePopupMenu(const int offset)
{
	if(region == 0)
	{
		showPopupMenu(offset);
	}
	else 
	{
		region->showPopupMenu(offset);
	}	
}

void ScopeCuePointComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	Text label = cueData.cuePoint.getLabel();
	Text heading = "Cue Point: ";
	if(label.length() > 0)
	{
		heading += "\"";
		heading += label;
		heading += "\"";
	}
	
	m.addItem(-1, (const char*)heading, false);	
		
	m.addItem (1, "Edit Cue Point Label");
	m.addItem (2, "Delete Cue Point");
	m.addItem (3, "Set to Zero");
	m.addItem (4, "Set to End");
//	m.addItem (5, "Move to zero crossing");
	
	const int result = m.show();
	
	if(result == 1)
	{
		editLabel();
	}
	else if(result == 2)
	{
		owner->removeCuePoint(this);
	}
	else if(result == 3)
	{
		setSampleOffset(0);
	}
	else if(result == 4)
	{
		setSampleOffset(owner->getMaxSize());
	}
}

void ScopeCuePointComponent::setHeight(const int height)
{
	setSize(3, height);
	checkPosition();
	setLabelPosition();
}

void ScopeCuePointComponent::checkPosition()
{	
	int x = owner->samplesToPixels(cueData.cuePoint.getSampleOffset());
	setTopLeftPosition(x-1, 0);
}

void ScopeCuePointComponent::paint(Graphics& g)
{	
	g.setColour(Colour(cueData.lineColour.get32bitColour())); 
	g.drawVerticalLine(1, 0, getHeight());
}

void ScopeCuePointComponent::setSampleOffset(const int newOffsetSamples)
{
	cueData.cuePoint.getSampleOffset() = newOffsetSamples;
	checkPosition();
}

const int& ScopeCuePointComponent::getSampleOffset()
{
	return cueData.cuePoint.getSampleOffset();
}

void ScopeCuePointComponent::mouseDown (const MouseEvent& e)
{
	// temp - need to do this manually probably
	
	int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
	
	if(!beingDragged && e.mods.isPopupMenu())
	{
		choosePopupMenu(offset);
	}
	else if(!beingDragged && e.mods.isAltDown())
	{
		if(region == 0)
		{
			owner->removeCuePoint(this);
		}
		else 
		{
			ScopeLoopComponent* loop = dynamic_cast<ScopeLoopComponent*> (region.getComponent());
			
			if(loop != 0)
			{
				owner->removeLoopPoint(loop);
			}
			else
			{
				owner->removeRegion(region);
			}
		}

	}
	else
	{
		dragger.startDraggingComponent (this, e);
		beingDragged = true;
	}
}

void ScopeCuePointComponent::mouseDrag (const MouseEvent& e)
{
	// temp - need to do this manually probably
	
	if(beingDragged)
	{
		constrain.setMinimumOnscreenAmounts(0xffffff, 0, 0xffffff, 0);
		dragger.dragComponent (this, e, &constrain);
	}
}

void ScopeCuePointComponent::mouseUp (const MouseEvent& e)
{
	beingDragged = false;
}

void ScopeCuePointComponent::moved()
{
	if(beingDragged)
	{
		int x = getX()+1;		
		
		int maxSize = owner->getMaxSize() - 1;
		if(maxSize <= 0) maxSize = 0x7fffffff;
		
		setSampleOffset(jlimit(0, maxSize, owner->pixelsToSamples(x)));
	}
	
	if(region != 0)
	{
		// perhaps do thi with a listener instead ??
		region->checkPosition();
	}
	
	setLabelPosition();
	owner->avoidPointLabelCollisions();
}

void ScopeCuePointComponent::setLabelPosition(const bool checkLabel)
{
	if(label != 0 && owner != 0)
	{
		label->checkPosition();
		if(checkLabel) label->setText(getLabel(), false);
	}	
}

const Text& ScopeCuePointComponent::getLabel() const
{
	return cueData.cuePoint.getLabel();
}

void ScopeCuePointComponent::setLabel(Text const& text)
{
	if(label != 0)
	{
		const Font& font = label->getFont();
		
		int w = font.getStringWidth((const char*)text);
		if(w > 0) w += 5;
		
		int h = font.getHeight();
		
		label->setSize(w,h);
		label->setText((const char*)text, false);
	}
	
	cueData.cuePoint.getLabel() = text;
}

void ScopeCuePointComponent::editLabel()
{
	const Text& text = cueData.cuePoint.getLabel();
	
	if(text.length() < 1)
	{
		setLabel(" ");
	}
	
	label->showEditor();	
}

void ScopeCuePointComponent::labelTextChanged (Label* labelThatHasChanged)
{
	if(labelThatHasChanged == label)
	{
		setLabel(label->getText().trim());
	}
}

void ScopeCuePointComponent::setColours(RGBAColour const& lineColour, RGBAColour const& textColour)
{
	cueData.lineColour = lineColour;
	cueData.textColour = textColour;
	
	if(label != 0)
	{
		label->setColour(Label::textColourId, Colour(cueData.textColour.get32bitColour()));
		label->setColour(Label::backgroundColourId, Colour(cueData.textColour.get32bitColour()).withAlpha(0.2f));
	}

	repaint();
}

void ScopeCuePointComponent::swapCuePoints(Component::SafePointer<ScopeCuePointComponent> &cue1, 
										   Component::SafePointer<ScopeCuePointComponent> &cue2)
{	
	// swap data
	swapVariables(cue1, cue2);
	swapVariables(cue1->cueData, cue2->cueData);

	// check label position and update text
	cue1->setLabelPosition(true);
	cue2->setLabelPosition(true);
		
	// check cue colour is correct
	cue1->repaint();
	cue2->repaint();
}

ScopeInsertComponent::ScopeInsertComponent(ScopeControlComponent* owner, 
										   ScopeRegionComponent* region)
:	ScopeCuePointComponent(owner, region, CuePoint())
{
	RGBAColour textColour = owner->getColour(ScopeControlComponent::InsertPointTextColour);
	RGBAColour lineColour = owner->getColour(ScopeControlComponent::InsertPointColour);

	setColours(lineColour, textColour);
}

void ScopeInsertComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	m.addItem(-1, "Insert/play Point:", false);	
	
	m.addItem (3, "Set to Zero");
	m.addItem (4, "Set to End");
	//	m.addItem (5, "Move to zero crossing");
	
	const int result = m.show();
	
	if(result == 3)
	{
		setSampleOffset(0);
	}
	else if(result == 4)
	{
		setSampleOffset(owner->getMaxSize());
	}
}

ScopeRegionComponent::ScopeRegionComponent(ScopeControlComponent* o, 
										   CuePoint const& startCue, 
										   CuePoint const& endCue,
										   const bool createdFromMouseClick)
:	owner(o),
	fillColour(owner->getColour(ScopeControlComponent::RegionFillColour)),
	changingBoth(false)
{
	init(startCue, endCue, createdFromMouseClick);
}

ScopeRegionComponent::ScopeRegionComponent(ScopeControlComponent* o, 
										   Region const& regionToUse,
										   const bool createdFromMouseClick)
:	owner(o),
	fillColour(owner->getColour(ScopeControlComponent::RegionFillColour)),
	changingBoth(false),
	region(regionToUse)
{
	init(region.getStartPoint(), region.getEndPoint(), createdFromMouseClick);
}

void ScopeRegionComponent::init(CuePoint const& startCue, 
								CuePoint const& endCue,
								const bool createdFromMouseClick)
{
	setInterceptsMouseClicks(true, false);
	
	RGBAColour startColour = owner->getColour(ScopeControlComponent::RegionStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::RegionEndColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::RegionTextColour);
	
	// set cues to 0 if they are -1?
	
	owner->addAndMakeVisible(startPoint = new ScopeCuePointComponent(owner, this, startCue, false, true));
	startPoint->setColours(startColour, textColour);
	
	owner->addAndMakeVisible(endPoint = new ScopeCuePointComponent(owner, this, endCue, createdFromMouseClick, false));
	endPoint->setColours(endColour, textColour);	
}

ScopeRegionComponent::~ScopeRegionComponent()
{
	if(owner != 0) 
	{
		owner->removeRegion(region);
		owner->removeChildComponent(startPoint);
		owner->removeChildComponent(endPoint);
	}
	
	startPoint.deleteAndZero();
	endPoint.deleteAndZero();
}

bool ScopeRegionComponent::hitTest (int x, int y)
{
	bool allowsClicksOnThisComponent;
	bool allowsClicksOnChildComponents;
	
	getInterceptsMouseClicks(allowsClicksOnThisComponent,
							 allowsClicksOnChildComponents);
	
	return allowsClicksOnThisComponent;
}

void ScopeRegionComponent::mouseDown (const MouseEvent& e)
{
	int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
	
	if(e.mods.isPopupMenu())
	{
		choosePopupMenu(offset);
	}
}

void ScopeRegionComponent::choosePopupMenu(const int offset)
{
	showPopupMenu(offset);	
}

void ScopeRegionComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	Text heading = "Region: ";
	Text labelStart = region.getStartPoint().getLabel();
	Text labelEnd = region.getEndPoint().getLabel();

	if(labelStart.length() > 0 && labelEnd.length())
	{
		heading += "\"";
		heading += labelStart;
		heading += "...";
		heading += labelEnd;
		heading += "\"";
	}
	else if(labelStart.length())
	{
		heading += "\"";
		heading += labelStart;
		heading += "\"";		
	}
	else if(labelEnd.length())
	{
		heading += "\"";
		heading += labelEnd;
		heading += "\"";		
	}
	
	m.addItem(-1, (const char*)heading, false);	
	
	m.addItem (1, "Edit Start Label");
	m.addItem (2, "Edit End Label");
	m.addItem (3, "Delete Region");
	m.addItem (4, "Select Region");	
	
	if(offset >= 0)
	{
		m.addSeparator();
		m.addItem (6, "Add Cue Point");
	}
	
	const int result = m.show();
	
	if(result == 1)
	{
		startPoint->editLabel();
	}
	else if(result == 2)
	{
		endPoint->editLabel();
	}
	else if(result == 3)
	{
		owner->removeRegion(this);
	}
	else if(result == 4)
	{
		int start, end;
		getRegionOffsets(start, end);
		owner->setSelection(start, end);
	}
	else if(result == 6)
	{
		owner->addNextCuePointAt(offset, true, false);
	}
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

void ScopeRegionComponent::setRegionOffsets(const int start, const int end)
{
	changingBoth = true;
	startPoint->setSampleOffset(jmin(start, end));
	endPoint->setSampleOffset(jmax(start, end));
	changingBoth = false;
	checkPosition();
}

void ScopeRegionComponent::getRegionOffsets(int& start, int& end)
{
	start = startPoint->getSampleOffset();
	end = endPoint->getSampleOffset();
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

ScopeSelectionComponent::ScopeSelectionComponent(ScopeControlComponent* owner,
												 const int initialStart, 
												 const int initialEnd)
:	ScopeRegionComponent(owner, CuePoint(initialStart), CuePoint(initialEnd))
{
	//setInterceptsMouseClicks(false, false);

	RGBAColour startColour = owner->getColour(ScopeControlComponent::SelectionStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::SelectionEndColour);
	RGBAColour fillColour = owner->getColour(ScopeControlComponent::SelectionFillColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::SelectionTextColour);

	setColours(startColour, endColour, textColour, fillColour);
}

void ScopeSelectionComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	m.addItem(-1, "Selection:", false);	
	
	m.addItem (3, "Set to Zero");
	m.addItem (4, "Set to End");
	//	m.addItem (5, "Move to zero crossing");
	m.addItem (6, "Select All");
	m.addItem (7, "Create Loop from Selection");
	m.addItem (8, "Create Region from Selection");
	
	m.addSeparator();
	
	if(offset >= 0)
	{
		m.addItem (9, "Add Cue Point");
	}
	
	m.addItem (10, "Delete Cue Points in Selection");
	m.addItem (11, "Delete Loop Points in Selection");
	m.addItem (12, "Delete Regions in Selection");
	m.addItem (13, "Delete Cue/Loop Points and Regions in Selection");
	
	const int result = m.show();
	
	if(result == 3)
	{
		setRegionOffsets(0, 0);
	}
	else if(result == 4)
	{
		setRegionOffsets(owner->getMaxSize(), owner->getMaxSize());
	}
	else if(result == 6)
	{
		setRegionOffsets(0, owner->getMaxSize());
	}
	else if(result == 7)
	{
		owner->addNextLoopPointAt(startPoint->getSampleOffset(), 
								  endPoint->getSampleOffset(), 
								  true, false);
//		owner->setSelection(0,0);
	}
	else if(result == 8)
	{
		owner->addNextRegionAt(startPoint->getSampleOffset(), 
							   endPoint->getSampleOffset(),
							   true, false);
//		owner->setSelection(0,0);
	}
	else if(result == 9)
	{
		owner->addNextCuePointAt(offset, true, false);
	}
	else if(result == 10)
	{
		int start, end;
		owner->getSelection(start, end);
		owner->clearCuePointsBetween(start, end);
	}
	else if(result == 11)
	{
		int start, end;
		owner->getSelection(start, end);
		owner->clearLoopPointsBetween(start, end);
	}
	else if(result == 12)
	{
		int start, end;
		owner->getSelection(start, end);
		owner->clearRegionsBetween(start, end);
	}
	else if(result == 13)
	{
		int start, end;
		owner->getSelection(start, end);
		owner->clearCuePointsBetween(start, end);
		owner->clearLoopPointsBetween(start, end);
		owner->clearRegionsBetween(start, end);
	}
}


ScopeLoopComponent::ScopeLoopComponent(ScopeControlComponent* owner,
									   LoopPoint const& loopPointToUse,
									   const bool createdFromMouseClick)
:	ScopeRegionComponent(owner, 
						 loopPointToUse.getStartPoint(), 
						 loopPointToUse.getEndPoint(), 
						 createdFromMouseClick),
	loopPoint(loopPointToUse)
{
	RGBAColour startColour = owner->getColour(ScopeControlComponent::LoopPointStartColour);
	RGBAColour endColour = owner->getColour(ScopeControlComponent::LoopPointEndColour);
	RGBAColour fillColour = owner->getColour(ScopeControlComponent::LoopFillColour);
	RGBAColour textColour = owner->getColour(ScopeControlComponent::LoopTextColour);
	
	setColours(startColour, endColour, textColour, fillColour);
}

ScopeLoopComponent::~ScopeLoopComponent()
{
	if(owner != 0) 
	{
		owner->removeLoopPoint(loopPoint);
	}	
}

void ScopeLoopComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	Text heading = "Loop: ";
	Text labelStart = loopPoint.getStartPoint().getLabel();
	Text labelEnd = loopPoint.getEndPoint().getLabel();
	
	if(labelStart.length() > 0 && labelEnd.length())
	{
		heading += "\"";
		heading += labelStart;
		heading += "...";
		heading += labelEnd;
		heading += "\"";
	}
	else if(labelStart.length())
	{
		heading += "\"";
		heading += labelStart;
		heading += "\"";		
	}
	else if(labelEnd.length())
	{
		heading += "\"";
		heading += labelEnd;
		heading += "\"";		
	}
	
	m.addItem(-1, (const char*)heading, false);	
	
	m.addItem (1, "Edit Start Label");
	m.addItem (2, "Edit End Label");
	m.addItem (3, "Delete Loop");
	m.addItem (4, "Select Loop");
		
	int loopType = loopPoint.getType();
	
	PopupMenu sub;
	sub.setLookAndFeel(owner);
	sub.addItem(101, "No Loop", true, loopType == LoopPoint::NoLoop);
	sub.addItem(102, "Forward", true, loopType == LoopPoint::Forward);
	sub.addItem(103, "Ping-Pong", true, loopType == LoopPoint::PingPong);
	sub.addItem(104, "Reverse", true, loopType == LoopPoint::Reverse);
	m.addSubMenu("Loop Type", sub);
	
	if(offset >= 0)
	{
		m.addSeparator();
		m.addItem (6, "Add Cue Point");
	}
	
	const int result = m.show();
	
	if (result == 1)
	{
		startPoint->editLabel();
	}
	else if (result == 2)
	{
		endPoint->editLabel();
	}
	else if (result == 3)
	{
		owner->removeLoopPoint(this);
	}
	else if(result == 4)
	{
		int start, end;
		getRegionOffsets(start, end);
		owner->setSelection(start, end);
	}	
	else if(result == 6)
	{
		owner->addNextCuePointAt(offset, true, false);
	}
}

ScopeControlComponent::ScopeControlComponent(CriticalSection& criticalSection, ScopeStyles style, DisplayOptions optionsToUse)
:	ScopeComponent(style),
	metaDataLock(criticalSection),
	options(optionsToUse),
	scopeInsert(0),
	scopeSelection(0),
	draggingCuePoint(0),
	maxSize(0),
	defaultCueLabel("Marker"),
	defaultCueLabelNumber(1),
	defaultLoopLabel("Loop"),
	defaultLoopLabelNumber(1),
	defaultRegionLabel("Region"),
	defaultRegionLabelNumber(1)
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
	controlColours[InsertPointTextColour]	= RGBAColour(1.0, 1.0, 1.0, 1.0);
	controlColours[SelectionStartColour]	= controlColours[LoopPointStartColour];
	controlColours[SelectionEndColour]		= controlColours[LoopPointEndColour];
	controlColours[SelectionFillColour]		= RGBAColour(1.0, 1.0, 1.0, 0.5);
	controlColours[SelectionTextColour]		= RGBAColour(1.0, 1.0, 1.0, 1.0);
	
	if(options & Insert)
	{
		addAndMakeVisible(scopeInsert = new ScopeInsertComponent(this, 0));
		scopeInsert->setLabel("play");
	}
	
	if(options & Selection)
	{
		addAndMakeVisible(scopeSelection = new ScopeSelectionComponent(this));
	}
}

ScopeControlComponent::~ScopeControlComponent()
{
	clearCuePoints();
	clearLoopPoints();
	clearRegions();
	deleteAllChildren();
}

const Font ScopeControlComponent::getPopupMenuFont()
{
	return 12;
}

void ScopeControlComponent::choosePopupMenu(const int offset)
{
	showPopupMenu(offset);
}

void ScopeControlComponent::showPopupMenu(const int offset)
{
	PopupMenu m;
	m.setLookAndFeel(this);
	
	m.addItem (1, "Add Cue Point");
	
	const int result = m.show();

	if(result == 1)
	{
		addNextCuePointAt(offset, true, false);
	}
}

void ScopeControlComponent::setAudioBuffer(Buffer const& audioBufferToUse, const double offset, const int fftSizeOfSource)
{
	ScopeComponent::setAudioBuffer(audioBufferToUse, offset, fftSizeOfSource);
	resized();
}

RGBAColour& ScopeControlComponent::getColour(ControlColours colour)
{
	return controlColours[colour];
}

void ScopeControlComponent::setMetaData(MetaData const& newMetaData)
{
	// clear current meta data
	clearCuePoints();
	
	// now iterate through all the metadata adding the part we display
	
	const CuePointArray& cuePoints = newMetaData.getCuePoints();
	
	for(int i = 0; i < cuePoints.length(); i++)
	{
		const CuePoint& cuePoint = cuePoints[i];
		
		addCuePoint(cuePoint, false);
	}
	
	// then take a reference of the source meta data
	metaData = newMetaData;
	
	// this should then stay in sync with the displayed data
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
	
	for(int i = 0; i < scopeCuePoints.size(); i++)
	{
		scopeCuePoints[i]->setHeight(height);
	}
	
	for(int i = 0; i < scopeRegions.size(); i++)
	{
		scopeRegions[i]->setHeight(height);
	}
	
	for(int i = 0; i < scopeLoops.size(); i++)
	{
		scopeLoops[i]->setHeight(height);
	}
	
	avoidPointLabelCollisions();
}

void ScopeControlComponent::mouseDown(const MouseEvent& e)
{
	int offset = pixelsToSamples(e.x);
	
	if(e.mods.isCtrlDown() && e.mods.isAltDown())
	{
		if(e.mods.isShiftDown())
		{
			draggingCuePoint = addNextRegionAt(offset, offset, true, true);
		}
		else
		{
			draggingCuePoint = addNextLoopPointAt(offset, offset, true, true);
		}
	}
	else if(e.mods.isShiftDown())
	{
		if(e.mods.isCtrlDown())
		{
			draggingCuePoint = addNextCuePointAt(offset, true, true);
		}
		else
		{
			if(scopeSelection != 0)
			{
				int startOffset, endOffset;
				getSelection(startOffset, endOffset);
				
				int startDistance = abs(startOffset - offset);
				int endDistance = abs(endOffset - offset);
				
				if(startDistance < endDistance)
				{
					setSelection(offset, endOffset);
					draggingCuePoint = scopeSelection->getStartPoint();
				}
				else
				{
					setSelection(startOffset, offset);
					draggingCuePoint = scopeSelection->getEndPoint();
				}
			}
			else if(scopeInsert != 0)
			{
				setInsertOffset(offset);
				draggingCuePoint = scopeInsert;
			}
		}
	}
	else if(e.mods.isPopupMenu())
	{
		choosePopupMenu(offset);
	}
	else
	{
		setInsertOffset(offset);
		setSelection(offset, offset);
		
		if(scopeSelection != 0)
		{
			draggingCuePoint = scopeSelection->getEndPoint();
		}
		else if(scopeInsert != 0)
		{
			draggingCuePoint = scopeInsert;
		}
	}
	
	if(draggingCuePoint != 0)
	{
		draggingCuePoint->mouseEnter(e.getEventRelativeTo(draggingCuePoint));
		draggingCuePoint->mouseDown(e.getEventRelativeTo(draggingCuePoint));
	}
}

void ScopeControlComponent::mouseDrag(const MouseEvent& e)
{
	if(draggingCuePoint != 0)
	{
		draggingCuePoint->mouseDrag(e.getEventRelativeTo(draggingCuePoint));
	}
}

void ScopeControlComponent::mouseUp(const MouseEvent& e)
{
	if(draggingCuePoint != 0)
	{
		draggingCuePoint->mouseUp(e.getEventRelativeTo(draggingCuePoint));
		draggingCuePoint->mouseExit(e.getEventRelativeTo(draggingCuePoint));
		draggingCuePoint = 0;
	}	
}

void ScopeControlComponent::setMaxSize(const int newSize)
{
	maxSize = newSize;
	
	/// check if any elements are outside this limit?
}

int ScopeControlComponent::getMaxSize()
{
	return maxSize;
}

int ScopeControlComponent::pixelsToSamples(const int pixels)
{
	double bufferOffsetSamples = getSampleOffset();
	const double audioBufferSize = getAudioBuffer().size();
	const double samplesPerPixel = audioBufferSize / getDisplayBufferSize();
	
	return pixels * samplesPerPixel + bufferOffsetSamples + 0.5;
}

int ScopeControlComponent::samplesToPixels(const int samples)
{
	double bufferOffsetSamples = getSampleOffset();
	const double audioBufferSize = getAudioBuffer().size();
	const double pixelsPerSample = getDisplayBufferSize() / audioBufferSize;
	
	return (samples - bufferOffsetSamples) * pixelsPerSample + 0.5; 
}

void ScopeControlComponent::addPointLabel(ScopeCuePointLabel* label)
{
	pointLabels.add(label);
	addAndMakeVisible(label);
}

void ScopeControlComponent::removePointLabel(ScopeCuePointLabel* label)
{
	pointLabels.removeValue(label);
	removeChildComponent(label);
}

void ScopeControlComponent::avoidPointLabelCollisions()
{
	// put them all at the top
	for(int i = 0; i < pointLabels.size(); i++)
	{
		ScopeCuePointLabel* label = pointLabels[i];
		label->checkPosition();
	}
	
	int maxY = getHeight();
	
	// then try to avoid them overlapping
	
	bool noCollisions = true;
	
	for(int n = 0; n < pointLabels.size(); n++)
	{
		for(int i = 0; i < pointLabels.size(); i++)
		{
			ScopeCuePointLabel* labeli = pointLabels[i];
			if(labeli->getText(false).trim().length() < 1) continue;
			
			for(int j = 0; j < pointLabels.size(); j++)
			{
				ScopeCuePointLabel* labelj = pointLabels[j];
			
				if(labeli == labelj) continue;
				if(labelj->getText(false).trim().length() < 1) continue;
				
				Rectangle<int> boundsi = labeli->getBounds();
				Rectangle<int> boundsj = labelj->getBounds();

				Rectangle<int> instersection = boundsi.getIntersection(boundsj);
				int height = instersection.getHeight();
				
				if(height > 0)
				{
					noCollisions = false;
					boundsi += Point<int>(0, height);
					
					if(boundsi.getBottom() < maxY)
					{
						labeli->setBounds(boundsi);
						j = 0;
					}
					else
					{
						break;
					}
				}
			}
		}
		
		if(noCollisions) break;
	}
}

void ScopeControlComponent::setInsertOffset(const int offset)
{
	if(scopeInsert != 0) 
	{
		scopeInsert->setSampleOffset(offset);
	}	
}

const int& ScopeControlComponent::getInsertOffset()
{
	static int minusOne = -1;
	
	if(scopeInsert != 0) 
	{
		return scopeInsert->getSampleOffset();
	}		
	
	return minusOne;
}

void ScopeControlComponent::setSelection(const int start, const int end)
{
	if(scopeSelection != 0)
	{
		scopeSelection->setRegionOffsets(start, end);
	}
}

void ScopeControlComponent::getSelection(int& start, int& end)
{
	if(scopeSelection != 0)
	{
		scopeSelection->getRegionOffsets(start, end);
	}
	else
	{
		start = -1.0;
		end = -1.0;
	}
}

void ScopeControlComponent::setCuePoint(const int index, const int offset)
{
	ScopeCuePointComponent* cuePoint = scopeCuePoints[index];
	
	if(cuePoint != 0)
	{
		cuePoint->setSampleOffset(offset);
	}
}

ScopeCuePointComponent* ScopeControlComponent::addCuePoint(CuePoint const& cuePoint, 
														   const bool addToMetaData,
														   const bool createdFromMousClick)
{
	ScopeCuePointComponent* cuePointComponent;
	addAndMakeVisible(cuePointComponent = new ScopeCuePointComponent(this, 0, cuePoint, createdFromMousClick));
	scopeCuePoints.add(cuePointComponent);
	cuePointComponent->setHeight(getHeight());
	
	if(addToMetaData) 
	{
		const ScopedLock sl(metaDataLock);
		metaData.getCuePoints().add(cuePoint);
	}
	
	return cuePointComponent;
}

ScopeCuePointComponent* ScopeControlComponent::addNextCuePointAt(const int offset, 
																 const bool addToMetaData, 
																 const bool createdFromMousClick)
{
	CuePoint cuePoint;
	cuePoint.getSampleOffset() = offset;
	cuePoint.getLabel() = defaultCueLabel + " " + Text::fromValue(defaultCueLabelNumber++);
	return addCuePoint(cuePoint, addToMetaData, createdFromMousClick);			
}

void ScopeControlComponent::removeCuePoint(const int index)
{
	ScopeCuePointComponent* cuePointComponent = scopeCuePoints[index];
	removeCuePoint(cuePointComponent);
}

void ScopeControlComponent::removeCuePoint(CuePoint const& cuePoint)
{
	if(metaData.getCuePoints().contains(cuePoint))
	{	
		const ScopedLock sl(metaDataLock);
		metaData.getCuePoints().removeItem(cuePoint);
	}
	
	for(int i = 0; i < scopeCuePoints.size(); i++)
	{
		ScopeCuePointComponent* cuePointComponent = scopeCuePoints[i];
		
		if(cuePointComponent->getCuePoint() == cuePoint)
		{
			removeCuePoint(cuePointComponent);
		}
	}	
}

void ScopeControlComponent::removeCuePoint(ScopeCuePointComponent* cuePointComponent)
{
	if(cuePointComponent != 0)
	{
		scopeCuePoints.removeValue(cuePointComponent);
		removeChildComponent(cuePointComponent);
		deleteAndZero(cuePointComponent);
		avoidPointLabelCollisions();
	}
}

void ScopeControlComponent::clearCuePoints()
{
	int index = scopeCuePoints.size();
	
	while(index--)
	{
		removeCuePoint(scopeCuePoints[index]);
	}
}

static bool inRange(const int value, const int min, const int max)
{
	return (value >= min) && (value <= max);
}

void ScopeControlComponent::clearCuePointsBetween(const int start, const int end)
{
	int index = scopeCuePoints.size();
	
	while(index--)
	{
		ScopeCuePointComponent* cue = scopeCuePoints[index];
		
		int offset = cue->getSampleOffset();
		
		if(inRange(offset, start, end))
			removeCuePoint(cue);
	}		
}

void ScopeControlComponent::setLoopPoint(const int index, const int start, const int end)
{
	ScopeLoopComponent* loopPoint = scopeLoops[index];
	
	if(loopPoint != 0)
	{
		loopPoint->setRegionOffsets(start, end);
	}	
}

ScopeCuePointComponent* ScopeControlComponent::addLoopPoint(LoopPoint const& loopPoint, 
															const bool addToMetaData,
															const bool createdFromMousClick)
{
	ScopeLoopComponent* loopComponent;
	addAndMakeVisible(loopComponent = new ScopeLoopComponent(this, loopPoint, createdFromMousClick));
	scopeLoops.add(loopComponent);
	loopComponent->setHeight(getHeight());
	
	if(addToMetaData) 
	{
		const ScopedLock sl(metaDataLock);
		metaData.getLoopPoints().add(loopPoint);
	}
	
	return loopComponent->getEndPoint();
}

ScopeCuePointComponent* ScopeControlComponent::addNextLoopPointAt(const int start, const int end, 
																  const bool addToMetaData, 
																  const bool createdFromMousClick)
{
	LoopPoint loopPoint;
	loopPoint.getStartPoint().getSampleOffset() = start;
	loopPoint.getEndPoint().getSampleOffset() = end;
	
	Text prefix = defaultLoopLabel + " " + Text::fromValue(defaultLoopLabelNumber++);
	loopPoint.getStartPoint().getLabel() = prefix + " Start";
	loopPoint.getEndPoint().getLabel() = prefix + " End";
	
	return addLoopPoint(loopPoint, addToMetaData, createdFromMousClick);	
}

void ScopeControlComponent::removeLoopPoint(const int index)
{
	ScopeLoopComponent* loopComponent = scopeLoops[index];
	removeLoopPoint(loopComponent);
}

void ScopeControlComponent::removeLoopPoint(LoopPoint const& loopPoint)
{
	if(metaData.getLoopPoints().contains(loopPoint))
	{	
		const ScopedLock sl(metaDataLock);
		metaData.getLoopPoints().removeItem(loopPoint);
	}
	
	for(int i = 0; i < scopeLoops.size(); i++)
	{
		ScopeLoopComponent* loopComponent = scopeLoops[i];
		
		if(loopComponent->getLoopPoint() == loopPoint)
		{
			removeLoopPoint(loopComponent);
		}
	}		
}

void ScopeControlComponent::removeLoopPoint(ScopeLoopComponent* loopComponent)
{
	if(loopComponent != 0)
	{
		scopeLoops.removeValue(loopComponent);
		removeChildComponent(loopComponent);
		deleteAndZero(loopComponent);
		avoidPointLabelCollisions();
	}	
}

void ScopeControlComponent::clearLoopPoints()
{
	int index = scopeLoops.size();
	
	while(index--)
	{
		removeLoopPoint(scopeLoops[index]);
	}	
}

void ScopeControlComponent::clearLoopPointsBetween(const int start, const int end)
{
	int index = scopeLoops.size();
	
	while(index--)
	{
		ScopeLoopComponent* loop = scopeLoops[index];
		
		int loopStart, loopEnd;
		loop->getRegionOffsets(loopStart, loopEnd);
		
		if(inRange(loopStart, start, end) && inRange(loopEnd, start, end))
			removeLoopPoint(loop);
	}		
}

void ScopeControlComponent::setRegion(const int index, const int start, const int end)
{
	ScopeRegionComponent* region = scopeRegions[index];
	
	if(region != 0)
	{
		region->setRegionOffsets(start, end);
	}		
}

ScopeCuePointComponent* ScopeControlComponent::addRegion(Region const& region, 
														 const bool addToMetaData, 
														 const bool createdFromMousClick)
{
	ScopeRegionComponent* regionComponent;
	addAndMakeVisible(regionComponent = new ScopeRegionComponent(this, region, createdFromMousClick));
	scopeRegions.add(regionComponent);
	regionComponent->setHeight(getHeight());
	
	if(addToMetaData) 
	{
		const ScopedLock sl(metaDataLock);
		metaData.getRegions().add(region);
	}
	
	return regionComponent->getEndPoint();	
}

ScopeCuePointComponent* ScopeControlComponent::addNextRegionAt(const int start, const int end, 
															   const bool addToMetaData, 
															   const bool createdFromMousClick)
{
	Region region;
	region.getStartPoint().getSampleOffset() = start;
	region.getEndPoint().getSampleOffset() = end;
	
	Text prefix = defaultRegionLabel + " " + Text::fromValue(defaultRegionLabelNumber++);
	region.getStartPoint().getLabel() = prefix + " Start";
	region.getEndPoint().getLabel() = prefix + " End";
	
	return addRegion(region, addToMetaData, createdFromMousClick);	
}

void ScopeControlComponent::removeRegion(const int index)
{
	ScopeRegionComponent* regionComponent = scopeRegions[index];
	removeRegion(regionComponent);
}

void ScopeControlComponent::removeRegion(Region const& region)
{
	if(metaData.getRegions().contains(region))
	{	
		const ScopedLock sl(metaDataLock);
		metaData.getRegions().removeItem(region);
	}
	
	for(int i = 0; i < scopeRegions.size(); i++)
	{
		ScopeRegionComponent* regionComponent = scopeRegions[i];
		
		if(regionComponent->getRegion() == region)
		{
			removeRegion(regionComponent);
		}
	}			
}

void ScopeControlComponent::removeRegion(ScopeRegionComponent* regionComponent)
{
	if(regionComponent != 0)
	{
		scopeRegions.removeValue(regionComponent);
		removeChildComponent(regionComponent);
		deleteAndZero(regionComponent);
		avoidPointLabelCollisions();
	}		
}

void ScopeControlComponent::clearRegions()
{
	int index = scopeRegions.size();
	
	while(index--)
	{
		removeRegion(scopeRegions[index]);
	}		
}

void ScopeControlComponent::clearRegionsBetween(const int start, const int end)
{
	int index = scopeRegions.size();
	
	while(index--)
	{
		ScopeRegionComponent* region = scopeRegions[index];
		
		int regionStart, regionEnd;
		region->getRegionOffsets(regionStart, regionEnd);
		
		if(inRange(regionStart, start, end) && inRange(regionEnd, start, end))
			removeRegion(region);
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
