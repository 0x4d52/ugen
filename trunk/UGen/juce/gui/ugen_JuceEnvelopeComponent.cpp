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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef UGEN_NOEXTGPL

#include "../../core/ugen_StandardHeader.h"

#ifdef JUCE_VERSION

BEGIN_UGEN_NAMESPACE

#include "ugen_JuceEnvelopeComponent.h"


EnvelopeHandleComponentConstrainer::EnvelopeHandleComponentConstrainer(EnvelopeHandleComponent* handleOwner)
	:	leftLimit(0),
		rightLimit(0xffffff),
		handle(handleOwner)
{
}

void EnvelopeHandleComponentConstrainer::checkBounds (Rectangle<int>& bounds,
					  const Rectangle<int>& old, const Rectangle<int>& limits,
					  bool isStretchingTop, bool isStretchingLeft,
					  bool isStretchingBottom, bool isStretchingRight)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponentConstrainer::checkBounds\n");
#endif
	
	ComponentBoundsConstrainer::checkBounds(bounds,
											old,limits,
											isStretchingTop,isStretchingLeft,
											isStretchingBottom,isStretchingRight);
	
	// prevent this handle moving before the previous point
	// or after the next point	
	bounds.setPosition(jlimit(leftLimit, rightLimit, bounds.getX()), bounds.getY());
	
	
	// then use the handle to access the envelope to then quantise x+y..
	
//	EnvelopeComponent* env = handle->getParentComponent();
//	
//	if(env)
//	{
//		double domain = env->convertPixelsToDomain(bounds.getX());
//		double value = env->convertPixelsToValue(bounds.getY());
//		
//		domain = env->quantiseDomain(domain);
//		value = env->quantiseValue(value);
//		
//		bounds.setPosition(env->convertDomainToPixels(domain), 
//						   env->convertValueToPixels(value));
//	}
}


void EnvelopeHandleComponentConstrainer::setAdjacentHandleLimits(int setLeftLimit, int setRightLimit)
{
	leftLimit = setLeftLimit;
	rightLimit = setRightLimit;
}

EnvelopeHandleComponent::EnvelopeHandleComponent()
	:	dontUpdateTimeAndValue(false),
		lastX(-1),
		lastY(-1),
		resizeLimits(this),
		ignoreDrag(false)
		
{
	setMouseCursor(MouseCursor::CrosshairCursor);
	resetOffsets();
}

EnvelopeComponent* EnvelopeHandleComponent::getParentComponent() const
{
	return (EnvelopeComponent*)Component::getParentComponent();
}

void EnvelopeHandleComponent::updateTimeAndValue()
{
	time = getParentComponent()->convertPixelsToDomain(getX());
	value = getParentComponent()->convertPixelsToValue(getY());
	
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::updateTimeAndValue(%f, %f)\n", time, value);
#endif
}

void EnvelopeHandleComponent::updateLegend()
{
	String text;
	
	int width = getParentWidth();
	int places;
	
	if(width >= 165) {
		EnvelopeComponent *env = getParentComponent();
		
		if(env && env->isLoopNode(this))
			text << T("(Loop) ");
		else if(env && env->isReleaseNode(this))
			text << T("(Release) ");
		else
			text << T("Point ");
		
		places = 3;
	}
	else if(width >= 140) {
		text << T("Point ");
		places = 3;
	} else if(width >= 115) {
		text << T("Pt ");
		places = 3;
	} else if(width >= 100) {
		text << T("Pt ");
		places = 2;
	} else if(width >= 85) {
		text << T("Pt ");
		places = 1;
	} else if(width >= 65) {
		text << T("P ");
		places = 1;
	} else {
		places = 1;
	}
	
	text << (getHandleIndex())
		 << T(": ")
		 << String(time, places) 
		 << T(", ") 
		 << String(value, places);
	
	getParentComponent()->setLegendText(text);
}

void EnvelopeHandleComponent::paint(Graphics& g)
{
	EnvelopeComponent *env = getParentComponent();
	RGBAColour handleColour;
	
	if(env == 0)
	{
		handleColour = 0xFF69B4FF;
	}
	else if(env->isReleaseNode(this))
	{
		handleColour = env->getEnvColour(EnvelopeComponent::ReleaseNode);
	}
	else if(env->isLoopNode(this))
	{
		handleColour = env->getEnvColour(EnvelopeComponent::LoopNode);
	}
	else
	{
		handleColour = env->getEnvColour(EnvelopeComponent::Node);
	}
	
	g.setColour(handleColour);
	g.fillRect(1, 1, getWidth()-2, getHeight()-2);
}

void EnvelopeHandleComponent::moved()
{
	if(dontUpdateTimeAndValue == false)
		updateTimeAndValue();
}

void EnvelopeHandleComponent::mouseMove(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::mouseMove\n");
#endif
}

void EnvelopeHandleComponent::mouseEnter(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::mouseEnter\n");
#endif
	
	setMouseCursor(MouseCursor::CrosshairCursor);	
	updateLegend();
}

void EnvelopeHandleComponent::mouseExit(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::mouseExit\n");
#endif
	
	getParentComponent()->setLegendTextToDefault();
}

void EnvelopeHandleComponent::mouseDown(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::mouseDown (%d, %d)\n", e.x, e.y);
#endif
	
	setMouseCursor(MouseCursor::NoCursor);
	
	if(e.mods.isShiftDown()) {
		
		getParentComponent()->setLegendTextToDefault();
		removeThisHandle();
		
		
	} 
	else if(e.mods.isCtrlDown())
	{
		if(getParentComponent()->getAllowNodeEditing())
		{
			ignoreDrag = true;
			
			if(PopupComponent::getActivePopups() < 1)
			{
				EnvelopeNodePopup::create(this, getScreenX()+e.x, getScreenY()+e.y);
			}
		}
	}
	else 
	{
		
		offsetX = e.x;
		offsetY = e.y;
		
		resizeLimits.setMinimumOnscreenAmounts(HANDLESIZE,HANDLESIZE,HANDLESIZE,HANDLESIZE);
		
		EnvelopeHandleComponent* previousHandle = getPreviousHandle();
		EnvelopeHandleComponent* nextHandle = getNextHandle();
		
		int leftLimit = previousHandle == 0 ? 0 : previousHandle->getX()+2;
		int rightLimit = nextHandle == 0 ? getParentWidth()-HANDLESIZE : nextHandle->getX()-2;
//		int leftLimit = previousHandle == 0 ? 0 : previousHandle->getX();
//		int rightLimit = nextHandle == 0 ? getParentWidth()-HANDLESIZE : nextHandle->getX();

		
		resizeLimits.setAdjacentHandleLimits(leftLimit, rightLimit);

		dragger.startDraggingComponent(this, e);//&resizeLimits);
	
	}
}

void EnvelopeHandleComponent::mouseDrag(const MouseEvent& e)
{
	if(ignoreDrag == true) return;
	
	if(e.mods.isAltDown()) {

		int moveX = e.x-offsetX;
		int moveY = offsetY-e.y;
		
		offsetTimeAndValue(moveX * FINETUNE, moveY * FINETUNE, FINETUNE);
		ignoreDrag = true;
		setMousePositionToThisHandle();
		ignoreDrag = false;
		
	} else {
				
		dragger.dragComponent(this, e, &resizeLimits);
	}
	
	updateLegend();
	getParentComponent()->repaint();
	getParentComponent()->sendChangeMessage();
	
	if(lastX == getX() && lastY == getY()) {	
		setMousePositionToThisHandle();
#ifdef MYDEBUG
		int x, y;
		Desktop::getMousePosition(x, y);	
		printf("screen pos (and mouse pos): %d (%d) %d (%d)\n", getScreenX(), x, getScreenY(), y);
#endif	
	}

	lastX = getX();
	lastY = getY();
	
#ifdef MYDEBUG
	double domain = getParentComponent()->convertPixelsToDomain(getX());
	double value = getParentComponent()->convertPixelsToValue(getY());
	printf("MyEnvelopeHandleComponent::mouseDrag(%d, %d) [%f, %f] (%d, %d)\n", e.x, e.y, 
		   domain,
		   value,
		   getParentComponent()->convertDomainToPixels(domain),
		   getParentComponent()->convertValueToPixels(value));
#endif
	
}


void EnvelopeHandleComponent::mouseUp(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::mouseUp\n");
#endif
	
	if(ignoreDrag == true)
	{
		ignoreDrag = false;
		return;
	}
		
//	if(e.mods.isCtrlDown() == false)
//	{
		EnvelopeComponent *env = getParentComponent();
		env->quantiseHandle(this);
//	}
	
	setMouseCursor(MouseCursor::CrosshairCursor);
	setMousePositionToThisHandle();
	
	offsetX = 0;
	offsetY = 0;
}


EnvelopeHandleComponent* EnvelopeHandleComponent::getPreviousHandle() const
{
	return getParentComponent()->getPreviousHandle(this);
}

EnvelopeHandleComponent* EnvelopeHandleComponent::getNextHandle() const
{
	return getParentComponent()->getNextHandle(this);
}

void EnvelopeHandleComponent::removeThisHandle()
{
	getParentComponent()->removeHandle(this);
}

void EnvelopeHandleComponent::setMousePositionToThisHandle()
{
	Desktop::setMousePosition(Point<int>(getScreenX()+offsetX, getScreenY()+offsetY));
}

int EnvelopeHandleComponent::getHandleIndex() const
{
	return getParentComponent()->getHandleIndex(const_cast<EnvelopeHandleComponent*>(this));
}

void EnvelopeHandleComponent::setTime(double timeToSet)
{
	bool oldDontUpdateTimeAndValue = dontUpdateTimeAndValue;
	dontUpdateTimeAndValue = true;
	
	time = constrainDomain(timeToSet);
	
	setTopLeftPosition(getParentComponent()->convertDomainToPixels(time), 
					   getY());
	
	dontUpdateTimeAndValue = oldDontUpdateTimeAndValue;
	
	getParentComponent()->repaint();
	((EnvelopeComponent*)getParentComponent())->sendChangeMessage();
}

void EnvelopeHandleComponent::setValue(double valueToSet)
{
	bool oldDontUpdateTimeAndValue = dontUpdateTimeAndValue;
	dontUpdateTimeAndValue = true;
	
	value = constrainValue(valueToSet);
	
	setTopLeftPosition(getX(), 
					   getParentComponent()->convertValueToPixels(value));
	
	dontUpdateTimeAndValue = oldDontUpdateTimeAndValue;
	
	getParentComponent()->repaint();
	((EnvelopeComponent*)getParentComponent())->sendChangeMessage();
}

void EnvelopeHandleComponent::setCurve(EnvCurve curveToSet)
{
	curve = curveToSet;
	getParentComponent()->repaint();
	((EnvelopeComponent*)getParentComponent())->sendChangeMessage();
}

void EnvelopeHandleComponent::setTimeAndValue(double timeToSet, double valueToSet, double quantise)
{
	bool oldDontUpdateTimeAndValue = dontUpdateTimeAndValue;
	dontUpdateTimeAndValue = true;

#ifdef MYDEBUG
	printf("MyEnvelopeHandleComponent::setTimeAndValue original (%f, %f)\n", timeToSet, valueToSet);
#endif
	
	if(quantise > 0.0) {
		int steps;

		steps		= valueToSet / quantise;
		valueToSet	= steps * quantise;
		steps		= timeToSet  / quantise;
		timeToSet	= steps * quantise;
		
#ifdef MYDEBUG
		printf("MyEnvelopeHandleComponent::setTimeAndValue quantised (%f, %f)\n", timeToSet, valueToSet);
#endif
	}
	
//	valueToSet = getParentComponent()->quantiseValue(valueToSet);
//	timeToSet = getParentComponent()->quantiseDomain(timeToSet);
	
	value = constrainValue(valueToSet);
	time = constrainDomain(timeToSet);
	
	setTopLeftPosition(getParentComponent()->convertDomainToPixels(time), 
					   getParentComponent()->convertValueToPixels(value));
	
	dontUpdateTimeAndValue = oldDontUpdateTimeAndValue;
	
	getParentComponent()->repaint();
	getParentComponent()->sendChangeMessage();
}

void EnvelopeHandleComponent::offsetTimeAndValue(double offsetTime, double offsetValue, double quantise)
{
	setTimeAndValue(time+offsetTime, value+offsetValue, quantise);
}


double EnvelopeHandleComponent::constrainDomain(double domainToConstrain) const
{ 
	EnvelopeHandleComponent* previousHandle = getPreviousHandle();
	EnvelopeHandleComponent* nextHandle = getNextHandle();

	int leftLimit = previousHandle == 0 ? 0 : previousHandle->getX();
	int rightLimit = nextHandle == 0 ? getParentWidth()-HANDLESIZE : nextHandle->getX();
	
	double left = getParentComponent()->convertPixelsToDomain(leftLimit);
	double right = getParentComponent()->convertPixelsToDomain(rightLimit);
	
	if(previousHandle != 0) left += FINETUNE;
	if(nextHandle != 0) right -= FINETUNE;
		
	return jlimit(left, right, domainToConstrain); 
}

double EnvelopeHandleComponent::constrainValue(double valueToConstrain) const
{
	return getParentComponent()->constrainValue(valueToConstrain);
}

void EnvelopeHandleComponent::recalculatePosition()
{
	bool oldDontUpdateTimeAndValue = dontUpdateTimeAndValue;
	dontUpdateTimeAndValue = true;
	setTopLeftPosition(getParentComponent()->convertDomainToPixels(time), 
					   getParentComponent()->convertValueToPixels(value));
	dontUpdateTimeAndValue = oldDontUpdateTimeAndValue;
	getParentComponent()->repaint();
}


EnvelopeComponent::EnvelopeComponent()
:	minNumHandles(0),
	maxNumHandles(0xffffff),
	domainMin(0.0),
	domainMax(1.0),
	valueMin(0.0),
	valueMax(1.0),
	valueGrid((valueMax-valueMin) / 10),
	domainGrid((domainMax-domainMin) / 16),
	gridDisplayMode(GridNone),
	gridQuantiseMode(GridNone),
	draggingHandle(0),
	curvePoints(64),
	releaseNode(-1),
	loopNode(-1),
	allowCurveEditing(true),
	allowNodeEditing(true)
{
	setMouseCursor(MouseCursor::NormalCursor);
	setBounds(0, 0, 200, 200); // non-zero size to start with
		
	colours[Node]				= 0xFF69B4FF;
	colours[ReleaseNode]		= 0xB469FFFF;
	colours[LoopNode]			= 0x69FFB4FF;
	colours[Line]				= 0xFFFFFFFF;
	colours[LoopLine]			= 0xB469FFFF;
	colours[Background]			= 0x555555FF;
	colours[GridLine]			= 0x888888FF;
	colours[LegendText]			= 0x000000FF;
	colours[LegendBackground]	= 0x696969FF;
}

EnvelopeComponent::~EnvelopeComponent()
{
	deleteAllChildren();
}

void EnvelopeComponent::setDomainRange(const double min, const double max)
{
	bool changed = false;
	
	if(domainMin != min)
	{
		changed = true;
		domainMin = min;
	}
	
	if(domainMax != max)
	{
		changed = true;
		domainMax = max;
	}
	
	if(changed == true)
	{
		recalculateHandles();
	}
}

void EnvelopeComponent::getDomainRange(double& min, double& max) const
{
	min = domainMin;
	max = domainMax;
}

void EnvelopeComponent::setValueRange(const double min, const double max)
{
	bool changed = false;
	
	if(valueMin != min)
	{
		changed = true;
		valueMin = min;
	}
	
	if(valueMax != max)
	{
		changed = true;
		valueMax = max;
	}
	
	if(changed == true)
	{
		recalculateHandles();
	}	
}

void EnvelopeComponent::getValueRange(double& min, double& max) const
{
	min = valueMin;
	max = valueMax;
}

void EnvelopeComponent::recalculateHandles()
{
	for(int i = 0; i < handles.size(); i++) 
	{
		handles.getUnchecked(i)->recalculatePosition();
	}
}

void EnvelopeComponent::setGrid(const GridMode display, const GridMode quantise, const double domainQ, const double valueQ)
{
	if(quantise != GridLeaveUnchanged)
		gridQuantiseMode = quantise;
	
	if((display != GridLeaveUnchanged) && (display != gridDisplayMode))
	{
		gridDisplayMode = display;
		repaint();
	}
	
	if((domainQ > 0.0) && (domainQ != domainGrid))
	{
		domainGrid = domainQ;
		repaint();
	}
	
	if((valueQ > 0.0) && (valueQ != valueGrid))
	{
		valueGrid = valueQ;
		repaint();
	}
}

void EnvelopeComponent::getGrid(GridMode& display, GridMode& quantise, double& domainQ, double& valueQ) const
{
	display = gridDisplayMode;
	quantise = gridQuantiseMode;
	domainQ = domainGrid;
	valueQ = valueGrid;
}

void EnvelopeComponent::paint(Graphics& g)
{
	paintBackground(g);
	
	if(handles.size() > 0)
	{
		Path path;
		Env env = getEnv();
		
		EnvelopeHandleComponent* handle = handles.getUnchecked(0);
		path.startNewSubPath((handle->getX() + handle->getRight()) * 0.5f,
							 (handle->getY() + handle->getBottom()) * 0.5f);
		
		const float firstTime = handle->getTime();
		float time = firstTime;
		
		for(int i = 1; i < handles.size(); i++) 
		{
			handle = handles.getUnchecked(i);
			float halfWidth = handle->getWidth()*0.5f;
			float halfHeight = handle->getHeight()*0.5f;
			
			float nextTime = handle->getTime();
			float handleTime = nextTime - time;
			float timeInc = handleTime / curvePoints;
			
			for(int j = 0; j < curvePoints; j++)
			{
				float value = env.lookup(time - firstTime);
				path.lineTo(convertDomainToPixels(time) + halfWidth, 
							convertValueToPixels(value) + halfHeight);
				time += timeInc;
			}
			
			path.lineTo((handle->getX() + handle->getRight()) * 0.5f,
						(handle->getY() + handle->getBottom()) * 0.5f);
			
			time = nextTime;
		}
		
		g.setColour(colours[Line]);
		g.strokePath (path, PathStrokeType(1.0f));
		
		if((loopNode >= 0) && (releaseNode >= 0) && (releaseNode > loopNode))
		{			
			EnvelopeHandleComponent* releaseHandle = handles[releaseNode];
			EnvelopeHandleComponent* loopHandle = handles[loopNode];
			
			if((releaseHandle != 0) && (loopHandle != 0))
			{
				// draw a horizontal line from release
				g.setColour(colours[LoopLine]);
				
				const float loopY = (loopHandle->getY() + loopHandle->getBottom()) * 0.5f;
				const float releaseY = (releaseHandle->getY() + releaseHandle->getBottom()) * 0.5f;
				const float loopX = (loopHandle->getX() + loopHandle->getRight()) * 0.5f;
				const float releaseX = (releaseHandle->getX() + releaseHandle->getRight()) * 0.5f;
				
				float dashes[] = { 5, 3 };
				g.drawDashedLine(loopX, releaseY, 
								 loopX, loopY, 
								 dashes, numElementsInArray(dashes), 0.5f);
				
				const int arrowLength = HANDLESIZE*2;
				
				g.drawLine(releaseX, releaseY, 
						   loopX + arrowLength, releaseY, 
						   0.5f);
				
				if(loopY == releaseY)
					g.setColour(colours[LoopNode]);
				
//				g.drawArrow(loopX + arrowLength, releaseY, 
//							loopX, releaseY, 
//							0.5f, HANDLESIZE, arrowLength);
				g.drawArrow(juce::Line<float>((float)(loopX + arrowLength), releaseY, loopX, releaseY), 
							0.5f, HANDLESIZE, arrowLength);
				
			}
		}
	}
}


void EnvelopeComponent::paintBackground(Graphics& g)
{
	g.setColour(colours[Background]);
	g.fillRect(0, 0, getWidth(), getHeight());
	
	g.setColour(colours[GridLine]);
	
	if((gridDisplayMode & GridValue) && (valueGrid > 0.0))
	{
		double value = valueMin;
		
		while(value <= valueMax)
		{
			//g.drawHorizontalLine(convertValueToPixels(value) + HANDLESIZE/2, 0, getWidth());
			float y = convertValueToPixels(value) + HANDLESIZE/2;
			y = round(y, 1.f) + 0.5f;
			g.drawLine(0, y, getWidth(), y, 1.0f);
			value += valueGrid;
		}
	}
	
	if((gridDisplayMode & GridDomain) && (domainGrid > 0.0))
	{
		double domain = domainMin;
		
		while(domain <= domainMax)
		{
			g.drawVerticalLine(convertDomainToPixels(domain) + HANDLESIZE/2, 0, getHeight());
			domain += domainGrid;
		}
	}
}

void EnvelopeComponent::resized()
{	
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::resized(%d, %d)\n", getWidth(), getHeight());
#endif
	if(handles.size() != 0) {
		for(int i = 0; i < handles.size(); i++) {
			EnvelopeHandleComponent* handle = handles.getUnchecked(i);
			
			bool oldDontUpdateTimeAndValue = handle->dontUpdateTimeAndValue;
			handle->dontUpdateTimeAndValue = true;
						
			handle->setTopLeftPosition(convertDomainToPixels(handle->getTime()),
									   convertValueToPixels(handle->getValue()));
			
			handle->dontUpdateTimeAndValue = oldDontUpdateTimeAndValue;
		}
	}	
}


void EnvelopeComponent::mouseEnter(const MouseEvent& e) {
	setMouseCursor(MouseCursor::NormalCursor);
}

void EnvelopeComponent::mouseMove(const MouseEvent& e) {
	setMouseCursor(MouseCursor::NormalCursor);
}

void EnvelopeComponent::mouseDown(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::mouseDown(%d, %d)\n", e.x, e.y);
#endif
	
	if(e.mods.isShiftDown()) 
	{
		
		// not needed ?
				
	}
	else if(e.mods.isCtrlDown())
	{
		if(getAllowCurveEditing())
		{
			float timeAtClick = convertPixelsToDomain(e.x);
			
			int i;
			EnvelopeHandleComponent* handle;
			
			for(i = 0; i < handles.size(); i++) {
				handle = handles.getUnchecked(i);
				if(handle->getTime() > timeAtClick)
					break;
			}
			
			if(PopupComponent::getActivePopups() < 1)
			{
				EnvelopeHandleComponent* prev = handle->getPreviousHandle();
				
				if(!prev)
				{
					EnvelopeCurvePopup::create(handle, getScreenX()+e.x, getScreenY()+e.y);
				}
				else
				{
					EnvelopeCurvePopup::create(handle, 
											   (handle->getScreenX() + prev->getScreenX())/2, 
											   jmax(handle->getScreenY(), prev->getScreenY())+10);
				}
			}
		}
	}
	else 
	{
		draggingHandle = addHandle(e.x,e.y, EnvCurve::Linear);
		
		if(draggingHandle != 0) {
			setMouseCursor(MouseCursor::NoCursor);
			draggingHandle->mouseDown(e.getEventRelativeTo(draggingHandle));
			draggingHandle->updateLegend();
		}
	}
}

void EnvelopeComponent::mouseDrag(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::mouseDrag(%d, %d)\n", e.x, e.y);
#endif
		
	if(draggingHandle != 0)
		draggingHandle->mouseDrag(e.getEventRelativeTo(draggingHandle));
}

void EnvelopeComponent::mouseUp(const MouseEvent& e)
{
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::mouseUp\n");
#endif
	
	if(draggingHandle != 0) 
	{
		if(e.mods.isCtrlDown() == false)
			quantiseHandle(draggingHandle);
		
		setMouseCursor(MouseCursor::CrosshairCursor);
		draggingHandle->setMousePositionToThisHandle();
		draggingHandle->resetOffsets();
		draggingHandle = 0;
	} else {
		setMouseCursor(MouseCursor::NormalCursor);
	}
}

void EnvelopeComponent::addListener (EnvelopeComponentListener* const listener)
{
	if (listener != 0)
        listeners.add (listener);
}

void EnvelopeComponent::removeListener (EnvelopeComponentListener* const listener)
{
	listeners.removeValue(listener);
}

void EnvelopeComponent::sendChangeMessage()
{
	for (int i = listeners.size(); --i >= 0;)
    {
        ((EnvelopeComponentListener*) listeners.getUnchecked (i))->envelopeChanged (this);
        i = jmin (i, listeners.size());
    }
}

void EnvelopeComponent::setLegendText(Text const& legendText)
{
	EnvelopeContainerComponent* container = 
		dynamic_cast <EnvelopeContainerComponent*> (getParentComponent());
	
	if(container == 0) return;
	
	EnvelopeLegendComponent* legend = container->getLegendComponent();
	
	if(legend == 0) return;
	
	legend->setText(legendText);
}

void EnvelopeComponent::setLegendTextToDefault()
{
	EnvelopeContainerComponent* container = 
		dynamic_cast <EnvelopeContainerComponent*> (getParentComponent());
	
	if(container == 0) return;
	
	EnvelopeLegendComponent* legend = container->getLegendComponent();
	
	if(legend == 0) return;
	
	legend->setText();	
}

int EnvelopeComponent::getHandleIndex(EnvelopeHandleComponent* thisHandle) const
{
	return handles.indexOf(thisHandle);
}

EnvelopeHandleComponent* EnvelopeComponent::getHandle(const int index) const
{
	return handles[index];
}

EnvelopeHandleComponent* EnvelopeComponent::getPreviousHandle(const EnvelopeHandleComponent* thisHandle) const
{
	int thisHandleIndex = handles.indexOf(const_cast<EnvelopeHandleComponent*>(thisHandle));
	
	if(thisHandleIndex <= 0) 
		return 0;
	else 
		return handles.getUnchecked(thisHandleIndex-1);
}

EnvelopeHandleComponent* EnvelopeComponent::getNextHandle(const EnvelopeHandleComponent* thisHandle) const
{
	int thisHandleIndex = handles.indexOf(const_cast<EnvelopeHandleComponent*>(thisHandle));
	
	if(thisHandleIndex == -1 || thisHandleIndex >= handles.size()-1) 
		return 0;
	else
		return handles.getUnchecked(thisHandleIndex+1);
}


EnvelopeHandleComponent* EnvelopeComponent::addHandle(int newX, int newY, EnvCurve curve)
{
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::addHandle(%d, %d)\n", newX, newY);
#endif
	
	return addHandle(convertPixelsToDomain(newX), convertPixelsToValue(newY), curve);
}


EnvelopeHandleComponent* EnvelopeComponent::addHandle(double newDomain, double newValue, EnvCurve curve)
{
#ifdef MYDEBUG
	printf("MyEnvelopeComponent::addHandle(%f, %f)\n", (float)newDomain, (float)newValue);
#endif
	
//	newDomain = quantiseDomain(newDomain);
//	newValue = quantiseValue(newValue);
	
	if(handles.size() < maxNumHandles) {
		int i;
		for(i = 0; i < handles.size(); i++) {
			EnvelopeHandleComponent* handle = handles.getUnchecked(i);
			if(handle->getTime() > newDomain)
				break;
		}
		
		if(releaseNode >= i) releaseNode++;
		if(loopNode >= i) loopNode++;
		
		EnvelopeHandleComponent* handle;
		addAndMakeVisible(handle = new EnvelopeHandleComponent());
		handle->setSize(HANDLESIZE, HANDLESIZE);
		handle->setTimeAndValue(newDomain, newValue, 0.0);	
		handle->setCurve(curve);
		handles.insert(i, handle);
	//	sendChangeMessage();
		return handle;
	}
	else return 0;
}


void EnvelopeComponent::removeHandle(EnvelopeHandleComponent* thisHandle)
{
	if(handles.size() > minNumHandles) {
		int index = handles.indexOf(thisHandle);
		
		if(releaseNode >= 0)
		{
			if(releaseNode == index)
				releaseNode = -1;
			else if(releaseNode > index)
				releaseNode--;
		}
		
		if(loopNode >= 0)
		{
			if(loopNode == index)
				loopNode = -1;
			else if(loopNode > index)
				loopNode--;
		}
		
		handles.removeValue(thisHandle);
		removeChildComponent(thisHandle);
		delete thisHandle;
		sendChangeMessage();
		repaint();
	}
}

void EnvelopeComponent::quantiseHandle(EnvelopeHandleComponent* thisHandle)
{
	ugen_assert(thisHandle != 0);
	
	if((gridQuantiseMode & GridDomain) && (domainGrid > 0.0))
	{
		double domain = round(thisHandle->getTime(), domainGrid);
		thisHandle->setTime(domain);
		
	}
	
	if((gridQuantiseMode & GridValue) && (valueGrid > 0.0))
	{
		double value = round(thisHandle->getValue(), valueGrid);
		thisHandle->setValue(value);
	}
}

bool EnvelopeComponent::isReleaseNode(EnvelopeHandleComponent* thisHandle) const
{
	int index = handles.indexOf(thisHandle);
	
	if(index < 0) 
		return false;
	else
		return index == releaseNode;
}

bool EnvelopeComponent::isLoopNode(EnvelopeHandleComponent* thisHandle) const
{
	int index = handles.indexOf(thisHandle);
	
	if(index < 0) 
		return false;
	else
		return index == loopNode;	
}

void EnvelopeComponent::setReleaseNode(const int index)
{
	if((index >= -1) && index < handles.size())
	{
		releaseNode = index;
		repaint();
	}
}

int EnvelopeComponent::getReleaseNode() const
{
	return releaseNode;
}

void EnvelopeComponent::setLoopNode(const int index)
{
	if((index >= -1) && index < handles.size())
	{
		loopNode = index;
		repaint();
	}
}

int EnvelopeComponent::getLoopNode() const
{
	return loopNode;
}

void EnvelopeComponent::setAllowCurveEditing(const bool flag)
{
	allowCurveEditing = flag;
}

bool EnvelopeComponent::getAllowCurveEditing() const
{
	return allowCurveEditing;
}

void EnvelopeComponent::setAllowNodeEditing(const bool flag)
{
	allowNodeEditing = flag;
}

bool EnvelopeComponent::getAllowNodeEditing() const
{
	return allowNodeEditing;
}

void EnvelopeComponent::setReleaseNode(EnvelopeHandleComponent* thisHandle)
{
	setReleaseNode(handles.indexOf(thisHandle));
}

void EnvelopeComponent::setLoopNode(EnvelopeHandleComponent* thisHandle)
{
	setLoopNode(handles.indexOf(thisHandle));
}

double EnvelopeComponent::convertPixelsToDomain(int pixelsX, int pixelsXMax) const
{
	if(pixelsXMax < 0) 
		pixelsXMax = getWidth()-HANDLESIZE;
	
	return (double)pixelsX / pixelsXMax * (domainMax-domainMin) + domainMin;

}

double EnvelopeComponent::convertPixelsToValue(int pixelsY, int pixelsYMax) const
{
	if(pixelsYMax < 0)
		pixelsYMax = getHeight()-HANDLESIZE;
	
	return (double)(pixelsYMax - pixelsY) / pixelsYMax * (valueMax-valueMin) + valueMin;
}

double EnvelopeComponent::convertDomainToPixels(double domainValue) const
{
	return (domainValue - domainMin) / (domainMax - domainMin) * (getWidth() - HANDLESIZE);
}

double EnvelopeComponent::convertValueToPixels(double value) const
{	
	double pixelsYMax = getHeight()-HANDLESIZE;	
	return pixelsYMax-((value- valueMin) / (valueMax - valueMin) * pixelsYMax);
}

Env EnvelopeComponent::getEnv() const
{
	if(handles.size() < 1) return Env(B(0.0, 0.0), B(0.0));
	if(handles.size() < 2) return Env(B(0.0, 0.0), B(handles.getUnchecked(0)->getValue()));
	
	EnvelopeHandleComponent* startHandle = handles.getUnchecked(0);
	double currentLevel = startHandle->getValue();
	double currentTime = startHandle->getTime();
	
	Buffer levels = Buffer::newClear(handles.size());
	Buffer times = Buffer::newClear(handles.size()-1);
	EnvCurveList curves = EnvCurveList(EnvCurve::Empty, handles.size()-1);
	
	levels.setSampleUnchecked(0, currentLevel);
	
	for(int i = 1; i < handles.size(); i++) 
	{
		EnvelopeHandleComponent* handle = handles.getUnchecked(i);
		
		currentLevel = handle->getValue();
		double time = handle->getTime();
		
		levels.setSampleUnchecked(i, currentLevel);
		times.setSampleUnchecked(i-1, time-currentTime);
		curves[i-1] = handle->getCurve();
		
		currentTime = time;
	}
	
	return Env(levels, times, curves, releaseNode, loopNode);
}

void EnvelopeComponent::setEnv(Env const& env)
{
	double time = 0.0;
	
	const Buffer& levels = env.getLevels();
	const Buffer& times = env.getTimes();
	const EnvCurveList& curves = env.getCurves();
	
	ugen_assert(levels.size() == (times.size()+1));
	
	EnvelopeHandleComponent* handle = addHandle(time, (double)levels[0], EnvCurve::Linear);
	quantiseHandle(handle);

	for(int i = 0; i < times.size(); i++)
	{
		time += times[i];
		handle = addHandle(time, (double)levels[i+1], curves[i]);
		quantiseHandle(handle);
	}
	
	releaseNode = env.getReleaseNode();
	loopNode = env.getLoopNode();
}

float EnvelopeComponent::lookup(const float time) const
{
	if(handles.size() < 1)
	{
		return 0.f;
	}
	else 
	{
		EnvelopeHandleComponent *first = handles.getUnchecked(0);
		EnvelopeHandleComponent *last = handles.getUnchecked(handles.size()-1);
		
		if(time <= first->getTime())
		{
			return first->getValue();
		}
		else if(time >= last->getTime())
		{
			return last->getValue();
		}
		else
		{
			return getEnv().lookup(time - first->getTime());
		}
	}
}

void EnvelopeComponent::setMinMaxNumHandles(int min, int max)
{
	if(min <= max) {
		minNumHandles = min;
		maxNumHandles = max;
	} else {
		minNumHandles = max;
		maxNumHandles = min;
	}
	
	Random rand(Time::currentTimeMillis());
	
	if(handles.size() < minNumHandles) {
		int num = minNumHandles-handles.size();
		
		for(int i = 0; i < num; i++) {
			double randX = rand.nextDouble() * (domainMax-domainMin) + domainMin;
			double randY = rand.nextDouble() * (valueMax-valueMin) + valueMin;
						
			addHandle(randX, randY, EnvCurve::Linear);
		}
		
	} else if(handles.size() > maxNumHandles) {
		int num = handles.size()-maxNumHandles;
		
		for(int i = 0; i < num; i++) {
			removeHandle(handles.getLast());
		}
	}
	
}

double EnvelopeComponent::constrainDomain(double domainToConstrain) const
{
	return jlimit(domainMin, domainMax, domainToConstrain); 
}

double EnvelopeComponent::constrainValue(double valueToConstrain) const	
{ 
	return jlimit(valueMin, valueMax, valueToConstrain); 
}

//double EnvelopeComponent::quantiseDomain(double value)
//{
//	if((gridQuantiseMode & GridDomain) && (domainGrid > 0.0))
//		return round(value, domainGrid);
//	else
//		return value;
//}
//
//double EnvelopeComponent::quantiseValue(double value)
//{
//	if((gridQuantiseMode & GridValue) && (valueGrid > 0.0))
//		return round(value, valueGrid);
//	else
//		return value;
//}

void EnvelopeComponent::setEnvColour(const EnvColours which, RGBAColour const& colour) throw()
{
	if((which >= 0) && (which < NumEnvColours))
	{
		//lock();
		colours[which] = colour;
		//unlock();
		
		//updateGUI();
		getParentComponent()->repaint();
		repaint();
	}
}

const RGBAColour& EnvelopeComponent::getEnvColour(const EnvColours which) const throw()
{
	if((which < 0) || (which >= NumEnvColours)) 
		return RGBAColour::getNull();
	else
		return colours[which];	
}

EnvelopeLegendComponent::EnvelopeLegendComponent(Text const& _defaultText)
:	defaultText(_defaultText)
{
	addAndMakeVisible(text = new Label(T("legend"), defaultText));
	setBounds(0, 0, 100, 16); // the critical thing is the height which should stay constant
}

EnvelopeLegendComponent::~EnvelopeLegendComponent()
{
	deleteAllChildren();
}

EnvelopeComponent* EnvelopeLegendComponent::getEnvelopeComponent() const
{
	EnvelopeContainerComponent* parent = dynamic_cast<EnvelopeContainerComponent*> (getParentComponent());
	
	if(parent == 0) return 0;
	
	return parent->getEnvelopeComponent();
}

void EnvelopeLegendComponent::resized()
{
	text->setBounds(0, 0, getWidth(), getHeight());
}

void EnvelopeLegendComponent::paint(Graphics& g)
{
	EnvelopeComponent *env = getEnvelopeComponent();
	RGBAColour backColour = env ? env->getEnvColour(EnvelopeComponent::LegendBackground) : 0x696969FF;
	
	g.setColour(backColour);
	g.fillRect(0, 0, getWidth(), getHeight());
	
	RGBAColour textColour = env ? env->getEnvColour(EnvelopeComponent::LegendText) : 0x000000FF;
	text->setColour(Label::textColourId, textColour);
}

void EnvelopeLegendComponent::setText(Text const& legendText)
{
	text->setText(legendText, false);
	repaint();
}

void EnvelopeLegendComponent::setText()
{
	text->setText(defaultText, false);
	repaint();
}

EnvelopeContainerComponent::EnvelopeContainerComponent(Text const& defaultText)
{
	addAndMakeVisible(legend = new EnvelopeLegendComponent(defaultText));
	addAndMakeVisible(envelope = new EnvelopeComponent());
}

EnvelopeContainerComponent::~EnvelopeContainerComponent()
{
	deleteAllChildren();
}

void EnvelopeContainerComponent::resized()
{
	int legendHeight = legend->getHeight();
	
	envelope->setBounds(0, 
						0, 
						getWidth(), 
						getHeight()-legendHeight);
	
	legend->setBounds(0, 
					  getHeight()-legendHeight, 
					  getWidth(), 
					  legend->getHeight());
}

void EnvelopeContainerComponent::setLegendComponent(EnvelopeLegendComponent* newLegend)
{
	deleteAndZero(legend);
	addAndMakeVisible(legend = newLegend);
}

EnvelopeCurvePopup::EnvelopeCurvePopup(EnvelopeHandleComponent* handleToEdit)
:	handle(handleToEdit),
	initialised(false)
{
	resetCounter();
	
	EnvCurve curve = handle->getCurve();
	EnvCurve::CurveType type = curve.getType();
	float curveValue = curve.getCurve();
	
	addChildComponent(slider = new CurveSlider());
	slider->setSliderStyle(Slider::LinearBar);
	//slider->setTextBoxStyle(Slider::NoTextBox, false, 0,0);
	slider->setRange(-1, 1, 0.0);
	slider->setValue(curveValue, false);
	
	
	addAndMakeVisible(combo = new ComboBox(T("combo")));	
	combo->addItem(T("Empty"),			idOffset + (int)EnvCurve::Empty);
	combo->addItem(T("Numerical..."),	idOffset + (int)EnvCurve::Numerical);
	combo->addItem(T("Step"),			idOffset + (int)EnvCurve::Step);
	combo->addItem(T("Linear"),			idOffset + (int)EnvCurve::Linear);
	
	EnvelopeComponent *parent = handleToEdit->getParentComponent();
	double min, max;
	parent->getValueRange(min, max);
	
	if(((min > 0.0) && (max > 0.0)) || ((min < 0.0) && (min < 0.0)))
	{
		// exponential can't cross zero
		combo->addItem(T("Exponential"),	idOffset + (int)EnvCurve::Exponential);
	}
	
	combo->addItem(T("Sine"),			idOffset + (int)EnvCurve::Sine);
	combo->addItem(T("Welch"),			idOffset + (int)EnvCurve::Welch);
	
	combo->addListener(this);
	
	switch(type)
	{
		case EnvCurve::Empty:		combo->setSelectedId(idOffset + (int)EnvCurve::Empty,		false); break;
		case EnvCurve::Numerical:	combo->setSelectedId(idOffset + (int)EnvCurve::Numerical,	false); break;
		case EnvCurve::Step:		combo->setSelectedId(idOffset + (int)EnvCurve::Step,		false); break;
		case EnvCurve::Linear:		combo->setSelectedId(idOffset + (int)EnvCurve::Linear,		false); break;
		case EnvCurve::Exponential: combo->setSelectedId(idOffset + (int)EnvCurve::Exponential, false); break;
		case EnvCurve::Sine:		combo->setSelectedId(idOffset + (int)EnvCurve::Sine,		false); break;
		case EnvCurve::Welch:		combo->setSelectedId(idOffset + (int)EnvCurve::Welch,		false); break;
	}
	
	slider->addListener(this);
}

void EnvelopeCurvePopup::create(EnvelopeHandleComponent* handle, int x, int y)
{
	EnvelopeCurvePopup *popup = new EnvelopeCurvePopup(handle);
	popup->addToDesktop(ComponentPeer::windowIsTemporary);
	
	if(x > 20) x -= 20;
	//if(y > 20) y -= 20;
	
	const int w = 200;
	const int h = 50;
	const int r = x+w;
	const int b = y+h;
	
	Rectangle<int> monitorArea = popup->getParentMonitorArea();
	
	if(r > monitorArea.getRight())
		x = monitorArea.getRight() - w;
		
	if(b > monitorArea.getBottom())
		y = monitorArea.getBottom() - h;
		
	popup->setBounds(x, y, w, h);
	popup->setVisible(true);		
	popup->getPeer()->setAlwaysOnTop(true);	
}

EnvelopeCurvePopup::~EnvelopeCurvePopup()
{
	deleteAllChildren();
}

void EnvelopeCurvePopup::resized()
{
	combo->setBoundsRelative(0.05, 0.1, 0.9, 0.4);
	slider->setBoundsRelative(0.05, 0.50, 0.9, 0.4);
}

void EnvelopeCurvePopup::sliderValueChanged(Slider* sliderThatChanged)
{
	resetCounter();
	
	if(sliderThatChanged == slider)
	{
		EnvCurve curve = handle->getCurve();
		double value = slider->getValue();
		value = cubed(value);
		value = linlin(value, -1.0, 1.0, -50.0, 50.0);
		curve.setCurve(value);
		handle->setCurve(curve);
	}
}

void EnvelopeCurvePopup::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
	resetCounter();
	
	if(comboBoxThatHasChanged == combo)
	{
		EnvCurve::CurveType type = (EnvCurve::CurveType)(combo->getSelectedId() - idOffset);
		bool showSlider = type == EnvCurve::Numerical;
		slider->setVisible(showSlider);
		
		EnvCurve curve = handle->getCurve();
		curve.setType(type);
		handle->setCurve(curve);
		
		if(!showSlider && initialised) expire();
		
		initialised = true;
	}
}

const int EnvelopeCurvePopup::idOffset = 1000;


EnvelopeNodePopup::EnvelopeNodePopup(EnvelopeHandleComponent* handleToEdit)
:	handle(handleToEdit),
	initialised(false)
{
	resetCounter();
	
	addChildComponent(setLoopButton = new TextButton(T("Set Y to Loop")));
	addChildComponent(setReleaseButton = new TextButton(T("Set Y to Release")));
	setLoopButton->addButtonListener(this);
	setReleaseButton->addButtonListener(this);
	
	addAndMakeVisible(combo = new ComboBox(T("combo")));	
	combo->addItem(T("Normal"),			idOffset + (int)Normal);
	combo->addItem(T("Release"),		idOffset + (int)Release);
	combo->addItem(T("Loop"),			idOffset + (int)Loop);
	combo->addItem(T("Release & Loop"),	idOffset + (int)ReleaseAndLoop);
	combo->addListener(this);
	
	EnvelopeComponent* env = handle->getParentComponent();
	
	if(env != 0)
	{
		if(env->isLoopNode(handle) && env->isReleaseNode(handle))
			combo->setSelectedId(idOffset + (int)ReleaseAndLoop, false);
		else if(env->isLoopNode(handle))
			combo->setSelectedId(idOffset + (int)Loop, false);
		else if(env->isReleaseNode(handle))
			combo->setSelectedId(idOffset + (int)Release, false);
		else
			combo->setSelectedId(idOffset + (int)Normal, false);
	}
	
}

void EnvelopeNodePopup::create(EnvelopeHandleComponent* handle, int x, int y)
{
	EnvelopeNodePopup *popup = new EnvelopeNodePopup(handle);
	popup->addToDesktop(ComponentPeer::windowIsTemporary);
	
	if(x > 20) x -= 20;
	if(y > 20) y -= 20;
	
	const int w = 200;
	const int h = 50;
	const int r = x+w;
	const int b = y+h;
	
	Rectangle<int> monitorArea = popup->getParentMonitorArea();
	
	if(r > monitorArea.getRight())
		x = monitorArea.getRight() - w;
	
	if(b > monitorArea.getBottom())
		y = monitorArea.getBottom() - h;
	
	popup->setBounds(x, y, w, h);
	popup->setVisible(true);		
	popup->getPeer()->setAlwaysOnTop(true);	
}

EnvelopeNodePopup::~EnvelopeNodePopup()
{
	deleteAllChildren();
}

void EnvelopeNodePopup::resized()
{
	combo->setBoundsRelative(0.05, 0.1, 0.9, 0.4);
	
	// the same pos but we should see only one at a time
	setLoopButton->setBoundsRelative(0.05, 0.50, 0.9, 0.4);
	setReleaseButton->setBoundsRelative(0.05, 0.50, 0.9, 0.4);
}

void EnvelopeNodePopup::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
	resetCounter();
	
	if(comboBoxThatHasChanged == combo)
	{
		EnvelopeComponent* env = handle->getParentComponent();
		
		if(env != 0)
		{
			NodeType type = (NodeType)(combo->getSelectedId() - idOffset);
			
			switch(type)
			{
				case Normal: {
					
					if(env->isLoopNode(handle)) 
						env->setLoopNode(-1);
					
					if(env->isReleaseNode(handle)) 
						env->setReleaseNode(-1);
					
					setLoopButton->setVisible(false);
					setReleaseButton->setVisible(false);
					
				} break;
					
				case Release: {
					
					if(env->isLoopNode(handle)) 
						env->setLoopNode(-1);
					
					env->setReleaseNode(handle);
					
					setReleaseButton->setVisible(false);
					
					if(env->getLoopNode() != -1)
						setLoopButton->setVisible(true);
					
				} break;
					
				case Loop: {
					
					env->setLoopNode(handle);
					
					if(env->isReleaseNode(handle)) 
						env->setReleaseNode(-1);
					
					if(env->getReleaseNode() != -1)
						setReleaseButton->setVisible(true);
					
					setLoopButton->setVisible(false);
					
				} break;
					
				case ReleaseAndLoop: {
					
					env->setLoopNode(handle);
					env->setReleaseNode(handle);
					
					setLoopButton->setVisible(false);
					setReleaseButton->setVisible(false);
					
				}
			}
		}
		
		if(initialised) expire();
		
		initialised = true;
	}
}

void EnvelopeNodePopup::buttonClicked(Button *button)
{
	EnvelopeComponent* env = handle->getParentComponent();
	
	if(env != 0)
	{
		if(button == setLoopButton)
		{
			int loopNode = env->getLoopNode();
			
			if(loopNode >= 0)
			{
				EnvelopeHandleComponent *loopHandle = env->getHandle(loopNode);
				
				if(loopHandle != 0)
				{
					float value = loopHandle->getValue();
					handle->setValue(value);
					expire();
				}
			}
		}
		else if(button == setReleaseButton)
		{
			int releaseNode = env->getReleaseNode();
			
			if(releaseNode >= 0)
			{
				EnvelopeHandleComponent *releaseHandle = env->getHandle(releaseNode);
				
				if(releaseHandle != 0)
				{
					float value = releaseHandle->getValue();
					handle->setValue(value);
					expire();
				}
			}
			
		}
	}
}

const int EnvelopeNodePopup::idOffset = 2000;


END_UGEN_NAMESPACE

#endif // juce

#endif // gpl



