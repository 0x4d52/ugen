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




#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

#ifndef UGEN_ANDROID // fixme

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceScope.h"
#include "ugen_JuceScopeProperties.h"


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

class PrefsDialogWindow : public DialogWindow
{
public:
    PrefsDialogWindow (const String& title, const Colour& colour, const bool escapeCloses)
	: DialogWindow (title, colour, escapeCloses, true)
    {
		setTitleBarHeight(22);
        if (! JUCEApplication::isStandaloneApp())
            setAlwaysOnTop (true);
    }
	
    void closeButtonPressed()
    {
        setVisible (false);
    }
};

static int showModalPrefs (const String& dialogTitle,
						   Component* contentComponent,
						   Component* componentToCentreAround,
						   const Colour& colour,
						   const bool escapeKeyTriggersCloseButton,
						   const bool shouldBeResizable,
						   const bool useBottomRightCornerResizer)
{
    PrefsDialogWindow dw (dialogTitle, colour, escapeKeyTriggersCloseButton);
	
	// JUCE_VERSION >= 79169
	
#if JUCE_VERSION >= 79169
	dw.setContentNonOwned (contentComponent, true);
#else
    dw.setContentComponent (contentComponent, true, true);
#endif
	
    dw.centreAroundComponent (componentToCentreAround, dw.getWidth(), dw.getHeight());
    dw.setResizable (shouldBeResizable, useBottomRightCornerResizer);
    const int result = dw.runModalLoop();
	
#if JUCE_VERSION < 79169
    dw.setContentComponent (0, false);
#endif
	
	return result;
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
			
//			paintXScale(g, middle);
//			paintYScale(g, middle, top);
//			paintYScale(g, middle, bottom);
			
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
			
			paintChannelLabel(g, channelLabels.wrapAt(channel), channel, bottom-textSizeChannel-6);
			paintXScale(g, middle);
			paintYScale(g, middle, top);
			paintYScale(g, middle, bottom);			
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
			
//			paintXScale(g, bottom);
//			paintYScale(g, bottom, top);
			
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
					
			paintChannelLabel(g, channelLabels.wrapAt(channel), channel, bottom-textSizeChannel-6);
			paintXScale(g, bottom);
			paintYScale(g, bottom, top);
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

		unsigned int mark = offsetSamples / markSpacing; // was 0
		const int labelOffsetX = 3;
		const int labelOffsetY = markXHeight > 5 ? 4 : markXHeight + 1;
		bool isFirstMark = true;
		
		g.setFont(textSizeX);
		g.setColour(Colour(colours[LabelMarks].get32bitColour()));
		
		const int inititalIndex = markSpacing - int(offsetSamples+0.5) % markSpacing; // was 0
		
		for(int audioIndex = inititalIndex; audioIndex < audioBufferSize; audioIndex += markSpacing)
		{			
			const int x = audioIndex * samplesPerPixel + 0.5;
			
			if(((labelHop <= 1) || ((mark % labelHop) == 0)))
			{
				g.drawVerticalLine(x, y - markXHeight * 1.5, y);
				
				if(scaleX != ScopeGUI::LabelXMarks && 
				   (((isFirstMark == true) && (labelFirstX == false)) == false)
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
							label = String(binFreq * 0.001, 2) + "kHz";
						else
							label = String((int)(binFreq + 0.5)) + "Hz";
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
			
			isFirstMark = false;
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
										
					if(scaleY == ScopeGUI::LabelYAmplitude)
					{
						g.drawSingleLineText(String(sign * level, decimalPlaces), labelOffsetX, y + labelOffsetY);
					}
					if(scaleY == ScopeGUI::LabelYPercent)
					{
						g.drawSingleLineText(String(sign * level * 100.0, decimalPlaces)+"%", labelOffsetX, y + labelOffsetY);
					}
					else if(scaleY == ScopeGUI::LabelYDecibels)
					{
						g.drawSingleLineText(String(sign * ampdb(level), decimalPlaces)+"dB", labelOffsetX, y + labelOffsetY);
					}
					
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

static void addCommandToMenu(PopupMenu& m, 
							 ScopeControlComponent::Command command, 
							 bool isActive = true, 
							 bool isTicked = false)
{
	m.addItem(command, ScopeControlComponent::getCommand(command), isActive, isTicked);
}

static void addPrefsCommandToMenu(PopupMenu& m)
{
	m.addSeparator();
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
}

ScopeCuePointLabel::ScopeCuePointLabel(ScopeCuePointComponent *o)
:	Label("ScopeControlLabel"),
	owner(o)
{
	setMouseCursor(MouseCursor::PointingHandCursor);
#if JUCE_IOS
	setFont(18);
#else
	setFont(11);
#endif
	
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
	//if(e.source.getIndex() > 0) return;
		
	if(owner != 0)
	{
		owner->mouseEnter(e.getEventRelativeTo(owner));
		owner->mouseDown(e.getEventRelativeTo(owner));
	}
}

void ScopeCuePointLabel::mouseDrag(const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	if(owner != 0)
	{
		owner->mouseDrag(e.getEventRelativeTo(owner));
	}	
}

void ScopeCuePointLabel::mouseUp(const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
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

void ScopeCuePointLabel::editorAboutToBeHidden (TextEditor* /*editorComponent*/)
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
	const int offset = 0;//owner->getClickMargin() - 1;

	int labelWidth = getWidth();
	int position = getCuePosition();		
	int width = getParentWidth();
	
	if(doesPreferToAttachOnLeft())
	{
		if((position + labelWidth - 2) > width)
		{
			setTopRightPosition(position+moveCloser+offset, 0);
		}
		else
		{
			setTopLeftPosition(position-moveCloser+offset, 0);
		}
	}
	else
	{
		if((position - labelWidth + 2) >= 0)
		{
			setTopRightPosition(position+moveCloser+offset, 0);
		}
		else
		{
			setTopLeftPosition(position-moveCloser+offset, 0);
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
:	
#if JUCE_IOS
	clickMargin(7),
	displayClickMargin(true),
#else
	clickMargin(1),
	displayClickMargin(false),
#endif
	owner(o),
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

void ScopeCuePointComponent::showPopupMenu(const int /*offset*/)
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
	m.addSeparator();	
	
	addCommandToMenu(m, ScopeControlComponent::EditCuePointLabel);
	addCommandToMenu(m, ScopeControlComponent::DeleteCuePoint);
	addCommandToMenu(m, ScopeControlComponent::SetToZero);
	addCommandToMenu(m, ScopeControlComponent::SetToEnd);
//	addCommandToMenu(m, ScopeControlComponent::MoveToZeroCrossings);
	m.addSeparator();	
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
	
	const int result = m.show();
	if(result != 0) doCommand(result);
}

void ScopeCuePointComponent::doCommand(int commandID)
{
	switch (commandID)
	{
		case ScopeControlComponent::EditCuePointLabel:	editLabel();							break;
		case ScopeControlComponent::DeleteCuePoint:		owner->removeCuePoint(this);			break;
		case ScopeControlComponent::SetToZero:			setSampleOffset(0);						break;
		case ScopeControlComponent::SetToEnd:			setSampleOffset(owner->getMaxSize());	break;
		case ScopeControlComponent::OpenProperties:		openProperties();						break;
	}	
}

void ScopeCuePointComponent::setHeight(const int height)
{
	setSize(clickMargin + clickMargin + 1, height);
	checkPosition();
	setLabelPosition();
}

void ScopeCuePointComponent::checkPosition()
{	
	int x = owner->samplesToPixels(cueData.cuePoint.getSampleOffset());
	setTopLeftPosition(x - clickMargin, 0);
}

void ScopeCuePointComponent::paint(Graphics& g)
{	
	if(displayClickMargin)
	{
		g.fillAll(Colour(cueData.lineColour.get32bitColour()).withAlpha(0.15f));
		g.setColour(Colour(cueData.lineColour.get32bitColour()).withAlpha(0.3f)); 
		g.drawRect(0, 0, getWidth(), getHeight(), 1);
	}
	
	g.setColour(Colour(cueData.lineColour.get32bitColour())); 
	g.drawVerticalLine(clickMargin, 0, getHeight());
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
	//if(e.source.getIndex() > 0) return;
	
	int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
	
	if(!beingDragged && e.mods.isPopupMenu())
	{
		choosePopupMenu(offset);
	}
	else if(!beingDragged && e.mods.isAltDown())
	{
		if(region == 0)
		{
			ScopeInsertComponent* insert = dynamic_cast<ScopeInsertComponent*> (this);
			if(insert == 0) owner->removeCuePoint(this);
		}
		else 
		{
			ScopeLoopComponent* loop = dynamic_cast<ScopeLoopComponent*> (region.getComponent());
			ScopeSelectionComponent* select = dynamic_cast<ScopeSelectionComponent*> (region.getComponent());
			
			if(loop != 0)
			{
				owner->removeLoopPoint(loop);
			}
			else if(select == 0)
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
	//if(e.source.getIndex() > 0) return;
	
	if(beingDragged)
	{
		constrain.setMinimumOnscreenAmounts(0xffffff, 0, 0xffffff, 0);
		dragger.dragComponent (this, e, &constrain);
		
		if(owner != 0)
		{
			int x = e.getEventRelativeTo(owner).x;
			double samplesPerPixel = (double)owner->getAudioBuffer().size() / (double)owner->getDisplayBufferSize();

			if(x < 0)
			{
				beginDragAutoRepeat(50);
				owner->offsetBy(x*samplesPerPixel*0.5+0.5);
			}
			else 
			{
				int w = owner->getWidth();
				
				if(x > w)
				{
					beginDragAutoRepeat(50);
					owner->offsetBy((x-w)*samplesPerPixel*0.5+0.5);
				}
				else 
				{
					beginDragAutoRepeat(0);
				}
			}
			
		}
	}
}

void ScopeCuePointComponent::mouseUp (const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	beingDragged = false;
	
	if(e.source.isTouch())
	{
		if(e.getLengthOfMousePress() < 250)
		{
			int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
			choosePopupMenu(offset);
		}
	}
}

void ScopeCuePointComponent::moved()
{
	if(beingDragged)
	{
		int x = getX() + clickMargin;
		int maxSize = owner->getMaxSize() - 1;
		if(maxSize <= 0) maxSize = 0x7fffffff;
		
		setSampleOffset(jlimit(0, maxSize, owner->pixelsToSamples(x)));
	}
	
	if(region != 0)
	{
		region->checkPosition();
	}
	
	setLabelPosition();
	owner->avoidPointLabelCollisions();
}

void ScopeCuePointComponent::visibilityChanged()
{
	bool visbility = isVisible();
	
	if(label != 0) label->setVisible(visbility);	
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

const Text& ScopeCuePointComponent::getComment() const
{
	return cueData.cuePoint.getComment();
}

void ScopeCuePointComponent::setComment(Text const& text)
{
	cueData.cuePoint.getComment() = text;
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

String ScopeCuePointComponent::getPropertiesName()
{
	return "Cue Point";
}

void ScopeCuePointComponent::openProperties()
{
	ScopeCuePointProperties props(this, getPropertiesName());
	
	props.setSize(600, 242);
	
	showModalPrefs(getPropertiesName()+ " Properties", &props, this,
				   Colour::greyLevel(0.9f).withAlpha(0.9f), 
				   true, true, false);		
}

ScopeInsertComponent::ScopeInsertComponent(ScopeControlComponent* owner, 
										   ScopeRegionComponent* region)
:	ScopeCuePointComponent(owner, region, CuePoint())
{
	RGBAColour textColour = owner->getColour(ScopeControlComponent::InsertPointTextColour);
	RGBAColour lineColour = owner->getColour(ScopeControlComponent::InsertPointColour);

	setColours(lineColour, textColour);
}

void ScopeInsertComponent::showPopupMenu(const int /*offset*/)
{
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	m.addItem(-1, "Insert/play Point:", false);	
	m.addSeparator();
	
	addCommandToMenu(m, ScopeControlComponent::SetToZero);
	addCommandToMenu(m, ScopeControlComponent::SetToEnd);
	
	m.addSeparator();	
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
	
	const int result = m.show();
	if(result != 0) doCommand(result);		
}

void ScopeInsertComponent::doCommand(const int commandID)
{
	switch (commandID)
	{
		case ScopeControlComponent::SetToZero:			setSampleOffset(0);						break;
		case ScopeControlComponent::SetToEnd:			setSampleOffset(owner->getMaxSize());	break;
		case ScopeControlComponent::OpenProperties:		openProperties();						break;
	}	
}

//void ScopeInsertComponent::mouseDown (const MouseEvent& e)
//{
//	int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
//	
//	if(!beingDragged && e.mods.isPopupMenu())
//	{
//		choosePopupMenu(offset);
//	}	
//}
//
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
	
	owner->addChildComponent(endPoint = new ScopeCuePointComponent(owner, this, endCue, createdFromMouseClick, false));
	endPoint->setColours(endColour, textColour);
	
	owner->addChildComponent(startPoint = new ScopeCuePointComponent(owner, this, startCue, false, true));
	startPoint->setColours(startColour, textColour);
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
	(void)x;
	(void)y;
	bool allowsClicksOnThisComponent;
	bool allowsClicksOnChildComponents;
	
	getInterceptsMouseClicks(allowsClicksOnThisComponent,
							 allowsClicksOnChildComponents);
	
	return allowsClicksOnThisComponent;
}

void ScopeRegionComponent::mouseDown (const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	int offset = owner ? owner->pixelsToSamples(e.getEventRelativeTo(owner).x) : -1;
	
	if(e.mods.isPopupMenu())
	{
		choosePopupMenu(offset);
	}
}

void ScopeRegionComponent::visibilityChanged()
{
	bool visbility = isVisible();
	
	if(startPoint != 0) startPoint->setVisible(visbility);	
	if(endPoint != 0)	endPoint->setVisible(visbility);	
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
	m.addSeparator();
	
	addCommandToMenu(m, ScopeControlComponent::EditStartLabel);
	addCommandToMenu(m, ScopeControlComponent::EditEndLabel);
	addCommandToMenu(m, ScopeControlComponent::DeleteRegion);
	addCommandToMenu(m, ScopeControlComponent::SelectRegion);
	
	if(offset >= 0)
	{
		m.addSeparator();
		addCommandToMenu(m, ScopeControlComponent::AddCuePoint);
	}
	
	m.addSeparator();	
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
	
	const int result = m.show();
	if(result != 0) doCommand(result, offset);
}

void ScopeRegionComponent::doCommand(const int commandID, int offset)
{
	switch (commandID)
	{
		case ScopeControlComponent::EditStartLabel:		startPoint->editLabel();						break;
		case ScopeControlComponent::EditEndLabel:		endPoint->editLabel();							break;
		case ScopeControlComponent::DeleteRegion:		owner->removeRegion(this);						break;
		case ScopeControlComponent::AddCuePoint:		owner->addNextCuePointAt(offset, true, false);	break;
		case ScopeControlComponent::SelectRegion: {
			int start, end;
			getRegionOffsets(start, end);
			owner->setSelection(start, end);			
		} break;
			
		case ScopeControlComponent::OpenProperties:		openProperties();								break;
	}	
}

void ScopeRegionComponent::getRegionPosition(int& start, int& end)
{ 
	if(startPoint != 0 && endPoint != 0)
	{
		start = startPoint->getCuePosition();
		end = endPoint->getCuePosition();
		
		if(start > end)
		{
			ScopeCuePointComponent::swapCuePoints(startPoint, endPoint);
			
			endPoint->toBehind(startPoint);
			
			start = startPoint->getCuePosition();
			end = endPoint->getCuePosition();
		}
	}
}

void ScopeRegionComponent::setRegionOffsets(const int start, const int end)
{
	changingBoth = true;
	if(startPoint != 0 && endPoint != 0)
	{
		startPoint->setSampleOffset(jmin(start, end));
		endPoint->setSampleOffset(jmax(start, end));
	}
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
		setBounds(start, 0, end - start, owner->getHeight());
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
	g.fillRect(1, 0, getWidth()-1, getHeight());
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

String ScopeRegionComponent::getPropertiesName()
{
	return "Region";
}

void ScopeRegionComponent::openProperties()
{
	ScopeRegionProperties props(this, getPropertiesName());
	
	props.setSize(600, 230);
	
	showModalPrefs(getPropertiesName()+ " Properties", &props, this,
				   Colour::greyLevel(0.9f).withAlpha(0.9f), 
				   true, true, false);	
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
	int start, end;
	owner->getSelection(start, end);
	bool nonZeroSelectionLength = end > start;
	
	PopupMenu m;
	m.setLookAndFeel(owner);
	
	m.addItem(-1, "Selection:", false);	
	m.addSeparator();
	
	addCommandToMenu(m, ScopeControlComponent::SetToZero);
	addCommandToMenu(m, ScopeControlComponent::SetToEnd);
//	addCommandToMenu(m, ScopeControlComponent::MoveToZeroCrossings);

	addCommandToMenu(m, ScopeControlComponent::SelectAll);
	addCommandToMenu(m, ScopeControlComponent::CreateLoopFromSelection, nonZeroSelectionLength);
	addCommandToMenu(m, ScopeControlComponent::CreateRegionFromSelection, nonZeroSelectionLength);
		
	m.addSeparator();
	
	if(offset >= 0)
	{
		addCommandToMenu(m, ScopeControlComponent::AddCuePoint);
	}
	
	addCommandToMenu(m, ScopeControlComponent::DeleteCuePointsInSelection, nonZeroSelectionLength);
	addCommandToMenu(m, ScopeControlComponent::DeleteLoopPointsInSelection, nonZeroSelectionLength);
	addCommandToMenu(m, ScopeControlComponent::DeleteRegionsInSelection, nonZeroSelectionLength);
	addCommandToMenu(m, ScopeControlComponent::DeleteCuesLoopsRegionsInSelection, nonZeroSelectionLength);

	m.addSeparator();
	
	addCommandToMenu(m, ScopeControlComponent::ZoomIn);
	addCommandToMenu(m, ScopeControlComponent::ZoomOut);
	addCommandToMenu(m, ScopeControlComponent::ZoomToSelection, nonZeroSelectionLength);
	addCommandToMenu(m, ScopeControlComponent::ZoomToWindow);
	
	m.addSeparator();	
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
	
	const int result = m.show();
	if(result != 0) doCommand(result, offset);
}

void ScopeSelectionComponent::doCommand(const int commandID, const int offset)
{
	const float zoomFactor = 16.f;
	
	int start, end;
	owner->getSelection(start, end);

	switch (commandID)
	{
		case ScopeControlComponent::SetToZero:							setRegionOffsets(0, 0);											break;
		case ScopeControlComponent::SetToEnd:							setRegionOffsets(owner->getMaxSize(), owner->getMaxSize());		break;
		case ScopeControlComponent::SelectAll:							setRegionOffsets(0, owner->getMaxSize());						break;
		case ScopeControlComponent::CreateLoopFromSelection:			owner->addNextLoopPointAt(startPoint->getSampleOffset(), 
																								  endPoint->getSampleOffset(), 
																								  true, false);							break;
		case ScopeControlComponent::CreateRegionFromSelection:			owner->addNextRegionAt(startPoint->getSampleOffset(), 
																							   endPoint->getSampleOffset(),
																							   true, false);							break;
		case ScopeControlComponent::AddCuePoint:						owner->addNextCuePointAt(offset, true, false);					break;
		case ScopeControlComponent::DeleteCuePointsInSelection:			owner->clearCuePointsBetween(start, end);						break;
		case ScopeControlComponent::DeleteLoopPointsInSelection:		owner->clearLoopPointsBetween(start, end);						break;
		case ScopeControlComponent::DeleteRegionsInSelection:			owner->clearRegionsBetween(start, end);							break;
		case ScopeControlComponent::DeleteCuesLoopsRegionsInSelection:	owner->clearCuePointsBetween(start, end);
			owner->clearLoopPointsBetween(start, end);
			owner->clearRegionsBetween(start, end);							break;
		case ScopeControlComponent::ZoomToSelection:					owner->zoomToOffsets(start, end);								break;
		case ScopeControlComponent::ZoomToWindow:						owner->zoomOutFully();											break;
			
		case ScopeControlComponent::ZoomIn:	{
			owner->zoomAround(offset < 0 ? owner->getMaxSize()/2 : offset, -1.f/zoomFactor);
		} break;
			
		case ScopeControlComponent::ZoomOut: {
			owner->zoomAround(offset < 0 ? owner->getMaxSize()/2 : offset, 1.f/zoomFactor);
		} break;
			
		case ScopeControlComponent::OpenProperties:		openProperties();	break;
	}				
}

void ScopeSelectionComponent::mouseDown (const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	if(e.mods.isPopupMenu())
	{
		ScopeRegionComponent::mouseDown(e);
	}
	else if(owner != 0)
	{
		owner->mouseEnter(e.getEventRelativeTo(owner));
		owner->mouseDown(e.getEventRelativeTo(owner));
	}
}

void ScopeSelectionComponent::mouseDrag (const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	if(owner != 0)
	{
		owner->mouseDrag(e.getEventRelativeTo(owner));
	}
}

void ScopeSelectionComponent::mouseUp (const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	if(owner != 0)
	{
		owner->mouseUp(e.getEventRelativeTo(owner));
		owner->mouseExit(e.getEventRelativeTo(owner));
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
	m.addSeparator();
	
	addCommandToMenu(m, ScopeControlComponent::EditStartLabel);
	addCommandToMenu(m, ScopeControlComponent::EditEndLabel);
	addCommandToMenu(m, ScopeControlComponent::DeleteLoop);
	addCommandToMenu(m, ScopeControlComponent::SelectLoop);
			
	int loopType = loopPoint.getType();
	
	PopupMenu sub;
	sub.setLookAndFeel(owner);
	
	addCommandToMenu(sub, ScopeControlComponent::LoopTypeNoLoop, true, loopType == LoopPoint::NoLoop);
	addCommandToMenu(sub, ScopeControlComponent::LoopTypeForward, true, loopType == LoopPoint::Forward);
	addCommandToMenu(sub, ScopeControlComponent::LoopTypePingPong, true, loopType == LoopPoint::PingPong);
	addCommandToMenu(sub, ScopeControlComponent::LoopTypeReverse, true, loopType == LoopPoint::Reverse);
	
	m.addSubMenu("Loop Type", sub);
	
	if(offset >= 0)
	{
		m.addSeparator();
		addCommandToMenu(m, ScopeControlComponent::AddCuePoint);
	}
	
	m.addSeparator();	
	addCommandToMenu(m, ScopeControlComponent::OpenProperties);
	
	const int result = m.show();
	if(result != 0) doCommand(result, offset);
}

void ScopeLoopComponent::doCommand(const int commandID, const int offset)
{
	switch (commandID)
	{
		case ScopeControlComponent::EditStartLabel:		startPoint->editLabel();							break;
		case ScopeControlComponent::EditEndLabel:		endPoint->editLabel();								break;
		case ScopeControlComponent::DeleteLoop:			owner->removeLoopPoint(this);						break;
		case ScopeControlComponent::AddCuePoint:		owner->addNextCuePointAt(offset, true, false);		break;	
		case ScopeControlComponent::SelectLoop:	{
			int start, end;
			getRegionOffsets(start, end);
			owner->setSelection(start, end);			
		} break;
			
		case ScopeControlComponent::LoopTypeNoLoop:		loopPoint.getType() = LoopPoint::NoLoop;			break;
		case ScopeControlComponent::LoopTypeForward:	loopPoint.getType() = LoopPoint::Forward;			break;
		case ScopeControlComponent::LoopTypePingPong:	loopPoint.getType() = LoopPoint::PingPong;			break;
		case ScopeControlComponent::LoopTypeReverse:	loopPoint.getType() = LoopPoint::Reverse;			break;
			
		case ScopeControlComponent::OpenProperties:		openProperties();									break;
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
	defaultRegionLabelNumber(1),
	dragScroll(false),
	dragZoomX(false),
	dragZoomY(false)
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
	
	addChildComponent(scopeInsert = new ScopeInsertComponent(this, 0));
	scopeInsert->setVisible((options & DisplayInsert) != 0);
	scopeInsert->setLabel("play");

	addChildComponent(scopeSelection = new ScopeSelectionComponent(this), 0);
	scopeSelection->setVisible((options & DisplaySelection) != 0);
	scopeSelection->getStartPoint()->setLabel("<");
	scopeSelection->getEndPoint()->setLabel(">");
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
	
	addCommandToMenu(m, ScopeControlComponent::AddCuePoint);
	m.addSeparator();
	addCommandToMenu(m, ScopeControlComponent::ZoomIn);
	addCommandToMenu(m, ScopeControlComponent::ZoomOut);
	addCommandToMenu(m, ScopeControlComponent::ZoomToWindow);
	
	addPrefsCommandToMenu(m);
		
	const int result = m.show();
	
	if(result != 0) doCommand(result, offset);
}

void ScopeControlComponent::doCommand(const int commandID, const int offset)
{
	const float zoomFactor = 16;

	switch (commandID)
	{
		case ScopeControlComponent::AddCuePoint:		addNextCuePointAt(offset, true, false);		break;
		case ScopeControlComponent::ZoomToWindow:		zoomOutFully();								break;
			
		case ScopeControlComponent::ZoomIn:	{
			zoomAround(offset < 0 ? maxSize/2 : offset, -1.f/zoomFactor);
		} break;
			
		case ScopeControlComponent::ZoomOut: {
			zoomAround(offset < 0 ? maxSize/2 : offset, 1.f/zoomFactor);
		} break;
			
		case ScopeControlComponent::OpenProperties:	openProperties();							break;
	}				
}

void ScopeControlComponent::setAudioBuffer(Buffer const& buffer, const double offset, const int fftSizeOfSource)
{
	ugen_assert(fftSizeOfSource <= 0); // just to check for now...
	
	ScopeComponent::setAudioBuffer(buffer, offset, fftSizeOfSource);
	originalBuffer = buffer;
	originalBufferOffset = offset;
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
	//if(e.source.getIndex() > 0) return;
	
	yMaxOrig = yMaximum;
	getCurrentLimits(origStart, origEnd);

	int offset = pixelsToSamples(e.x);
	origMouseDownSamples = offset;
	origMouseDownProportion = (double)e.x / (double)getWidth();
	
	if(e.mods.isCommandDown())
	{		
		if(e.mods.isAltDown())
		{
			dragScroll = false;
			dragZoomX = dragZoomY = true; // we decide which in mouseDrag
		}
		else 
		{
			dragScroll = true;
			dragZoomX = dragZoomY = false;
			lastDragX = e.x;
		}
	}
	else if(e.mods.isPopupMenu() && e.mods.isAltDown())
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
		if(e.mods.isPopupMenu())
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
	else if(e.mods.isLeftButtonDown())
	{
		if(options & DisplayInsert)		setInsertOffset(offset);
		if(options & DisplaySelection)	setSelection(offset, offset);
		
		if((options & DisplaySelection) && (scopeSelection != 0))
		{
			draggingCuePoint = scopeSelection->getEndPoint();
		}
		else if((options & DisplayInsert) && (scopeInsert != 0))
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
	//if(e.source.getIndex() > 0) return;
	
	if(e.mods.isCommandDown())
	{		
		if(dragScroll)
		{
			int amount = lastDragX - e.x;
			
			double samplesPerPixel = (double)getAudioBuffer().size() / (double)getDisplayBufferSize();
			int width = getWidth();
			
			if(e.x < 0)
			{
				beginDragAutoRepeat(50);
				amount = -e.x * samplesPerPixel + 0.5;
			}
			else if(e.x >= width)
			{
				beginDragAutoRepeat(50);
				amount = -(e.x - width) * samplesPerPixel + 0.5;
			}
			else
			{
				beginDragAutoRepeat(0);
				amount = amount * samplesPerPixel + 0.5;
			}

			if(amount != 0)
			{
				offsetBy(amount);
				lastDragX = e.x;
			}			
		}
		else if(dragZoomX && dragZoomY)
		{
			if(!e.mouseWasClicked())
			{
				int dx = e.getDistanceFromDragStartX();
				int dy = e.getDistanceFromDragStartY();
				
				if(abs(dx) > abs(dy))
				{
					dragZoomY = false;
				}
				else 
				{
					dragZoomX = false;
				}
			}
		}
		else if(dragZoomX)
		{
			double length = origEnd-origStart;			
			double diff = e.getMouseDownX()-e.x;
			double newLength = powf(2.f, diff/50.f) * length;
						
			int newStart = origMouseDownSamples - newLength * origMouseDownProportion + 0.5;
			int newEnd = origMouseDownSamples + newLength * (1.0-origMouseDownProportion) + 0.5;
			
			if(newStart < newEnd)
			{
				zoomToOffsets(newStart, newEnd);
			}
			else
			{
				zoomToOffsets(origMouseDownSamples-0.5, 
							  origMouseDownSamples+0.5);
			}
		}
		else if(dragZoomY)
		{
			float diff = e.y - e.getMouseDownY();
			float factor = powf(2.f, diff/50.f) * yMaxOrig;
						
			setYMaximum(jmax(0.00000001f, factor));
		}
	}
	else
	{
		// finsihed scrolling if we were since the command key is now up
		//..
		
		if(draggingCuePoint != 0)
		{
			draggingCuePoint->mouseDrag(e.getEventRelativeTo(draggingCuePoint));
		}
	}
}

void ScopeControlComponent::mouseUp(const MouseEvent& e)
{
	//if(e.source.getIndex() > 0) return;
	
	if(e.mods.isCommandDown())
	{
		//..
	}
	else if(draggingCuePoint != 0)
	{
		draggingCuePoint->mouseUp(e.getEventRelativeTo(draggingCuePoint));
		draggingCuePoint->mouseExit(e.getEventRelativeTo(draggingCuePoint));
		draggingCuePoint = 0;
	}	
	
	dragScroll = dragZoomX = dragZoomY = false;
}

void ScopeControlComponent::setDisplayOptions(int newOptions)
{
	if((options & DisplayCuePoints) != (newOptions & DisplayCuePoints))
	{
		for(int i = 0; i < scopeCuePoints.size(); i++)
		{
			scopeCuePoints[i]->setVisible(newOptions & DisplayCuePoints);
		}
	}
	
	if((options & DisplayLoopPoints) != (newOptions & DisplayLoopPoints))
	{
		for(int i = 0; i < scopeLoops.size(); i++)
		{
			scopeLoops[i]->setVisible((newOptions & DisplayLoopPoints) != 0);
		}
	}
	
	if((options & DisplayRegions) != (newOptions & DisplayRegions))
	{
		for(int i = 0; i < scopeRegions.size(); i++)
		{
			scopeRegions[i]->setVisible((newOptions & DisplayRegions) != 0);
		}
	}
	
	if((options & DisplayInsert) != (newOptions & DisplayInsert))
	{
		scopeInsert->setVisible((newOptions & DisplayInsert) != 0);
	}
	
	if((options & DisplaySelection) != (newOptions & DisplaySelection))
	{
		scopeSelection->setVisible((newOptions & DisplaySelection) != 0);
	}
	
	options = newOptions;
	avoidPointLabelCollisions();
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

void ScopeControlComponent::getCurrentLimits(int& start, int& end)
{	
	start = getSampleOffset();
	end = start + getAudioBuffer().size();
}

void ScopeControlComponent::offsetBy(const int offset)
{
	int start, end;
	getCurrentLimits(start, end);
	zoomToOffsets(start + offset, end + offset);
}

void ScopeControlComponent::zoomToOffsets(int start, int end)
{
	if(start >= end)
	{
		ugen_assertfalse;
		return;
	}
			
	int originalBufferSize = originalBuffer.size();
	int originalBufferEnd = originalBufferOffset + originalBuffer.size();
	
	if((start < originalBufferOffset) && (end > originalBufferEnd))
	{
		ScopeComponent::setAudioBuffer(originalBuffer, originalBufferOffset, fftSize);
		resized();
	}
	else
	{
		int newSize = end - start;
		
		if(newSize > originalBufferSize) 
			newSize = originalBufferSize;

		if(start < originalBufferOffset)
		{
			start = originalBufferOffset;
			end  = start + newSize;
		}
		else if(end > originalBufferEnd)
		{
			end = originalBufferEnd;
			start = originalBufferEnd - newSize;
		}
		
		Buffer zoomedBuffer = Buffer::withSize(1, originalBuffer.getNumChannels());
		zoomedBuffer.referTo(originalBuffer, start - originalBufferOffset, newSize);
		
		ScopeComponent::setAudioBuffer(zoomedBuffer, start, fftSize);
		resized();
	}
}

void ScopeControlComponent::zoomAround(const int offset, const float amount)
{
	const float factor = amount + 1.f;
	
	const int newSize = getAudioBuffer().size() * factor + 0.5f;
	const int halfSize = newSize / 2;
	
	int start = offset - halfSize;
	int end = offset + halfSize;
	
	int lastOriginal = originalBufferOffset + originalBuffer.size();
	
	if(end > lastOriginal)
	{
		int diff = end - lastOriginal;
		start -= diff;
		end -= diff;
	}
	else if(start < originalBufferOffset)
	{
		int diff = originalBufferOffset - start;
		start += diff;
		end += diff;		
	}
			
	zoomToOffsets(start, end);
}

void ScopeControlComponent::zoomOutFully()
{
	ScopeComponent::setAudioBuffer(originalBuffer, originalBufferOffset, fftSize);
	resized();
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
	addChildComponent(label);
}

void ScopeControlComponent::removePointLabel(ScopeCuePointLabel* label)
{
	pointLabels.removeFirstMatchingValue(label);
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
			if(!labeli->isVisible()) continue;
			if(labeli->getText(false).trim().length() < 1) continue;
			
			for(int j = 0; j < pointLabels.size(); j++)
			{
				ScopeCuePointLabel* labelj = pointLabels[j];
			
				if(labeli == labelj) continue;
				if(!labelj->isVisible()) continue;
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
						j = 0; //?
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
		start = -1;
		end = -1;
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
	addChildComponent(cuePointComponent = new ScopeCuePointComponent(this, 0, cuePoint, createdFromMousClick));
	cuePointComponent->setVisible(options & DisplayCuePoints);
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
		scopeCuePoints.removeFirstMatchingValue(cuePointComponent);
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
	addChildComponent(loopComponent = new ScopeLoopComponent(this, loopPoint, createdFromMousClick), 0);
	loopComponent->setVisible((options & DisplayLoopPoints) != 0);
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
	
	loopPoint.getType() = LoopPoint::Forward;
	
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
		scopeLoops.removeFirstMatchingValue(loopComponent);
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
	addChildComponent(regionComponent = new ScopeRegionComponent(this, region, createdFromMousClick), 0);
	regionComponent->setVisible((options & DisplayRegions) != 0);
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
		scopeRegions.removeFirstMatchingValue(regionComponent);
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

void ScopeControlComponent::openProperties()
{
	ScopeControlProperties props(this);
	
	props.setSize(400, 600);
	
	if(propertyOpenness != 0)
	{
		props.restoreOpennessState(*propertyOpenness);
	}
	
	showModalPrefs("Scope Properties", &props, this,
				   Colour::greyLevel(0.9f).withAlpha(0.9f), 
				   true, true, false);	
		
	propertyOpenness = props.getOpennessState();
}

const ScopeControlComponent::CommandDictonary& ScopeControlComponent::buildCommandDictionary()
{
	static ScopeControlComponent::CommandDictonary c;
		
	c.put(AddCuePoint,						"Add Cue Point");
	c.put(CreateLoopFromSelection,			"Create Loop from Selection");
	c.put(CreateRegionFromSelection,		"Create Region from Selection");
	
	c.put(EditCuePointLabel,				"Edit Cue Point Label");
	c.put(EditStartLabel,					"Edit Start Label");
	c.put(EditEndLabel,						"Edit End Label");

	c.put(DeleteCuePoint,					"Delete Cue Point");
	c.put(DeleteRegion,						"Delete Region");
	c.put(DeleteLoop,						"Delete Loop");
	
	c.put(DeleteCuePointsInSelection,		"Delete Cue Points in Selection");
	c.put(DeleteLoopPointsInSelection,		"Delete Loop Points in Selection");
	c.put(DeleteRegionsInSelection,			"Delete Regions in Selection");
	c.put(DeleteCuesLoopsRegionsInSelection,"Delete Cue/Loop Points and Regions in Selection");
	
	c.put(SelectAll,						"Select All");
	c.put(SelectRegion,						"Select Region");
	c.put(SelectLoop,						"Select Loop");
	
	c.put(SetToZero,						"Set to Zero");
	c.put(SetToEnd,							"Set to End");
	
	c.put(LoopTypeNoLoop,					"No Loop");
	c.put(LoopTypeForward,					"Forward");
	c.put(LoopTypePingPong,					"Ping-Pong");
	c.put(LoopTypeReverse,					"Reverse");
	
	c.put(MoveToZeroCrossings,				"Move to Zero Crossings");
	
	c.put(ZoomIn,							"Zoom In");
	c.put(ZoomOut,							"Zoom Out");
	c.put(ZoomToSelection,					"Zoom to Selection");
	c.put(ZoomToWindow,						"Zoom to Window");
	
	c.put(OpenProperties,					"Properties...");
	
	return c;
}

const char* ScopeControlComponent::getCommand(Command commandID)
{
	static const ScopeControlComponent::CommandDictonary& commands = buildCommandDictionary();
	return (const char*)commands[commandID];
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
							label = String(binFreq * 0.001, 2) + "kHz";
						else
							label = String((int)(binFreq + 0.5)) + "Hz";
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
	(void)g;
	(void)zero;
	(void)maximum;
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

#endif
