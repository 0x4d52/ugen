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

#ifndef _UGEN_ugen_JuceScope_H_
#define _UGEN_ugen_JuceScope_H_

#include "../../gui/ugen_Scope.h"


class ScopeComponentBase :	public ScopeGUI,
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

class ScopeControlComponent;
class ScopeRegionComponent;
class ScopeCuePointComponent;

class ScopeCuePointLabel : public Label
{
public:
	ScopeCuePointLabel(ScopeCuePointComponent *owner,
					   String const& text = String::empty);
	TextEditor* createEditorComponent();
	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	int getCuePosition();
	void checkPosition();
	bool doesPreferToAttachOnLeft() const;
private:
	Component::SafePointer<ScopeCuePointComponent> owner;
};

struct CuePointData
{
	RGBAColour lineColour, textColour;
	Component::SafePointer<ScopeCuePointLabel> label;
	CuePoint cuePoint;
	bool labelPrefersToAttachOnLeft;
};

class ScopeCuePointComponent :	public Component,
								public Label::Listener
{
public:
	ScopeCuePointComponent(ScopeControlComponent* owner, 
						   ScopeRegionComponent* region, 
						   CuePoint const& cuePoint, //const double initialOffset = 0.0,
						   const bool createdFromMouseClick = false,
						   const bool labelPrefersToAttachOnLeft = true);
	~ScopeCuePointComponent();
	inline int getCuePosition() { return getX()+1; }
	void setHeight(const int height);
	void checkPosition();
	void paint(Graphics& g);
	
	const CuePoint& getCuePoint() const { return cueData.cuePoint; }
	void setSampleOffset(const int offsetSamples);
	const int& getSampleOffset();
	
	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);

	void moved();
	
	void setLabelPosition();
	const Text& getLabel() const;
	void setLabel(Text const& text);
	void labelTextChanged (Label* labelThatHasChanged);
	inline bool doesLabelPreferToAttachOnLeft() const { return cueData.labelPrefersToAttachOnLeft; }

	void setColours(RGBAColour const& lineColour, RGBAColour const& textColour);
	
	static void swapCuePoints(Component::SafePointer<ScopeCuePointComponent> &cue1, 
							  Component::SafePointer<ScopeCuePointComponent> &cue2);
		
private:
	Component::SafePointer<ScopeControlComponent> owner;
	Component::SafePointer<ScopeRegionComponent> region;
	//double offsetSamples;
	ComponentDragger dragger;
	ComponentBoundsConstrainer constrain;
	bool beingDragged;
	
	CuePointData cueData; // anything that needs to be transferred if two cues are swapped
};

//typedef ScopeCuePointComponent ScopeInsertComponent;

class ScopeInsertComponent : public ScopeCuePointComponent
{
public:
	ScopeInsertComponent(ScopeControlComponent* owner, ScopeRegionComponent* region);
};


class ScopeRegionComponent : public Component
{
public:
	ScopeRegionComponent(ScopeControlComponent* owner, const int initialStart = 0, const int initialEnd = 0);
	~ScopeRegionComponent();
	
	ScopeCuePointComponent* getStartPoint() { return startPoint; }
	ScopeCuePointComponent* getEndPoint() { return endPoint; }
	
	void getRegionPosition(int& start, int& end);
	void setRegionOffsets(const int start, const int end);
	void getRegionOffsets(int& start, int& end);
	
	void checkPosition();
	
	void setHeight(const int height);
	void paint(Graphics& g);
	
	void setColours(RGBAColour const& startColour, 
					RGBAColour const& endColour, 
					RGBAColour const& textColour, 
					RGBAColour const& fillColour);
	
private:
	Component::SafePointer<ScopeControlComponent> owner;
	Component::SafePointer<ScopeCuePointComponent> startPoint;
	Component::SafePointer<ScopeCuePointComponent> endPoint;
	RGBAColour fillColour; 
	bool changingBoth;
};

//typedef ScopeRegionComponent ScopeSelectionComponent;

class ScopeSelectionComponent : public ScopeRegionComponent
{
public:
	ScopeSelectionComponent(ScopeControlComponent* owner);
};

class ScopeLoopComponent : public ScopeRegionComponent
{
public:
	ScopeLoopComponent(ScopeControlComponent* owner);
private:
	// mode?
};

class ScopeControlComponent : public ScopeComponent
{
public:
	enum DisplayOptions
	{
		CuePoints	= 1,
		LoopPoints	= 2,
		Regions		= 4,
		Insert		= 8,
		Selection	= 16,
		All			= 0x7fff
	};
	
	enum ControlColours { 
		CuePointColour, CuePointTextColour,
		LoopPointStartColour, LoopPointEndColour, LoopFillColour, LoopTextColour,
		RegionStartColour, RegionEndColour, RegionFillColour, RegionTextColour,
		InsertPointColour, InsertPointTextColour,
		SelectionStartColour, SelectionEndColour, SelectionFillColour, SelectionTextColour,
		NumControlColours
	};
	
	ScopeControlComponent(ScopeStyles style = Lines, DisplayOptions options = All);
	~ScopeControlComponent();
	
	void setAudioBuffer(Buffer const& audioBufferToUse, const double offset = 0.0, const int fftSize = -1);
	
	RGBAColour& getColour(ControlColours colour);
	void setMetaData(MetaData const& metaData);
	void resized();
	
	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	
	void setMaxSize(const int newSize);
	int getMaxSize();
	
	int pixelsToSamples(const int pixels);
	int samplesToPixels(const int samples);
	
	void addPointLabel(ScopeCuePointLabel* label);
	void removePointLabel(ScopeCuePointLabel* label);
	void avoidPointLabelCollisions();
	
	void setInsertOffset(const int offset);
	const int& getInsertOffset();

	void setSelection(const int start, const int end);
	void getSelection(int& start, int& end);

	void setCuePoint(const int index, const int offset);
	ScopeCuePointComponent* addCuePoint(CuePoint const& cuePoint);//const double offset, Text const& label = Text::empty);
	void removeCuePoint(const int index);
	void removeCuePoint(CuePoint const& cuePoint);
	void removeCuePoint(ScopeCuePointComponent* cuePointComponent);
	void clearCuePoints();
	
private:
	DisplayOptions options;
	MetaData metaData;
	RGBAColour controlColours[NumControlColours];
	Array<ScopeCuePointLabel*> pointLabels;
	Array<ScopeCuePointComponent*> scopeCuePoints;
	Array<ScopeRegionComponent*> scopeRegions;
	Array<ScopeLoopComponent*> scopeLoops;
	ScopeInsertComponent* scopeInsert;
	ScopeSelectionComponent* scopeSelection;
	ScopeCuePointComponent* draggingCuePoint;
	int maxSize;
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


class MeterComponent :	public Component,
						public Timer
{
public:
	MeterComponent(String& name, float* value, const CriticalSection& lock);
	~MeterComponent();
	void paint(Graphics& g);
	void timerCallback();
	
private:
	float* value;
	float lastDisplayValue;
	const CriticalSection& lock;
};



#endif // _UGEN_ugen_JuceScope_H_
