// $Id: ugen_JuceUtility.cpp 1000 2010-03-09 23:27:20Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/ugen_JuceUtility.cpp $

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

#include "ugen_JuceUtility.h"

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
	counter = -1;
}

int PopupComponent::activePopups = 0;

END_UGEN_NAMESPACE

#endif
