// $Id: ugen_JuceScope.h 984 2010-01-31 12:28:49Z mgrobins $
// $HeadURL: http://164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/juce/gui/ugen_JuceScope.h $

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

#ifndef _UGEN_ugen_JuceScope_H_
#define _UGEN_ugen_JuceScope_H_

#include "../../gui/ugen_Scope.h"


class ScopeComponentBase :	public ScopeGUI, // swapped these round (ScopeGUI first) as a hack for the scope<->gui connection
							public Component
							
{
public:
	ScopeComponentBase(ScopeStyles style = Lines);
	
	void resized();
	
	void updateGUI() throw();
	bool isValid() throw();
	void lock() throw();
	void unlock() throw();
	bool tryLock() throw();
	
private:
	CriticalSection juceLock;
	
	enum CommandIDs { Repaint, NumCommandIDs };
	void handleCommandMessage (int commandId);
};



/** @ingroup GUITools */
class ScopeComponent : public ScopeComponentBase
{
public:
	ScopeComponent(ScopeStyles style = Lines);
	
	void paint(Graphics& g);
	void paintBipolar(Graphics& g);
	void paintUnipolar(Graphics& g);
	void paintXScale(Graphics& g, const int y);
	void paintYScale(Graphics& g, const int zero, const int maximum);
	void paintChannelLabel(Graphics& g, Text const& label, const int index, const int top);
	
	
	inline int getDisplayBufferSize() const throw() { return Component::getWidth(); }
	//inline int getHeight() const throw() { return Component::getHeight(); }
		
private:
};



class RadialScopeComponent : public ScopeComponentBase
{
public:
	RadialScopeComponent(ScopeStyles style = Lines);
		
	void paint(Graphics& g);
	void paintBipolar(Graphics& g);
	//void paintUnipolar(Graphics& g);
	void paintXScale(Graphics& g, const int y);
	void paintYScale(Graphics& g, const int zero, const int maximum);
	void paintChannelLabel(Graphics& g, Text const& label, const int index, const int top);
	
	double getDelta() const throw();
	double getDeltaAngle() const throw();
	
	int getDisplayBufferSize() const throw();
	int getRadius() const throw();
	int getCentreX() const throw();
	int getCentreY() const throw();
	inline int getDiameter() const throw() { return getRadius() * 2; }
	
private:
	Buffer sine, cosine;
};



#endif // _UGEN_ugen_JuceScope_H_
