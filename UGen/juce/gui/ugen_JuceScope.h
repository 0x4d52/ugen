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
#include "ugen_Collections.h"


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
	void handleCommandMessage (const int commandId);
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
	ScopeCuePointLabel(ScopeCuePointComponent *owner);
	
	void showPopupMenu();
	
	TextEditor* createEditorComponent();
	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	
	void editorShown (TextEditor* editorComponent);
    void editorAboutToBeHidden (TextEditor* editorComponent);
	
	int getCuePosition();
	void checkPosition();
	bool doesPreferToAttachOnLeft() const;
private:
	Component::SafePointer<ScopeCuePointComponent> owner;
	int oldWidth, oldHeight;
};

struct CuePointData
{
	RGBAColour lineColour, textColour;
	CuePoint cuePoint;
	bool labelPrefersToAttachOnLeft;
};

class ScopeCuePointComponent :	public Component,
								public Label::Listener
{
public:
	ScopeCuePointComponent(ScopeControlComponent* owner, 
						   ScopeRegionComponent* region, 
						   CuePoint const& cuePoint,
						   const bool createdFromMouseClick = false,
						   const bool labelPrefersToAttachOnLeft = true);
	~ScopeCuePointComponent();
	
	void choosePopupMenu(const int offset);
	virtual void showPopupMenu(const int offset);
	void doCommand(const int commandID);

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
	
	void setLabelPosition(const bool checkLabel = false);
	const Text& getLabel() const;
	void setLabel(Text const& text);
	void editLabel();
	void labelTextChanged (Label* labelThatHasChanged);
	inline bool doesLabelPreferToAttachOnLeft() const { return cueData.labelPrefersToAttachOnLeft; }

	void setColours(RGBAColour const& lineColour, RGBAColour const& textColour);
	
	static void swapCuePoints(Component::SafePointer<ScopeCuePointComponent> &cue1, 
							  Component::SafePointer<ScopeCuePointComponent> &cue2);
		
protected:
	Component::SafePointer<ScopeControlComponent> owner;
	Component::SafePointer<ScopeRegionComponent> region;
	Component::SafePointer<ScopeCuePointLabel> label;

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
	void showPopupMenu(const int offset);
	void doCommand(const int commandID);
//	void mouseDown (const MouseEvent& e);
};


class ScopeRegionComponent : public Component
{
public:
	ScopeRegionComponent(ScopeControlComponent* owner, 
						 CuePoint const& startPoint = CuePoint(), 
						 CuePoint const& endPoint = CuePoint(),
						 const bool createdFromMouseClick = false);
	
	ScopeRegionComponent(ScopeControlComponent* owner, 
						 Region const& region,
						 const bool createdFromMouseClick = false);	
	
	~ScopeRegionComponent();
	
	bool hitTest (int x, int y);
	void mouseDown (const MouseEvent& e);	
	
	void choosePopupMenu(const int offset);
	virtual void showPopupMenu(const int offset);
	void doCommand(const int commandID, const int offset);

	ScopeCuePointComponent* getStartPoint() { return startPoint; }
	ScopeCuePointComponent* getEndPoint() { return endPoint; }
	
	void getRegionPosition(int& start, int& end);
	void setRegionOffsets(const int start, const int end);
	void getRegionOffsets(int& start, int& end);
	Region& getRegion() { return region; }
	
	
	void checkPosition();
	
	void setHeight(const int height);
	void paint(Graphics& g);
	
	void setColours(RGBAColour const& startColour, 
					RGBAColour const& endColour, 
					RGBAColour const& textColour, 
					RGBAColour const& fillColour);
	
private:
	void init(CuePoint const& startCue, 
			  CuePoint const& endCue, 
			  const bool createdFromMouseClick);
	
protected:
	Component::SafePointer<ScopeControlComponent> owner;
	Component::SafePointer<ScopeCuePointComponent> startPoint;
	Component::SafePointer<ScopeCuePointComponent> endPoint;
	RGBAColour fillColour; 
	bool changingBoth;
	Region region;
};

//typedef ScopeRegionComponent ScopeSelectionComponent;

class ScopeSelectionComponent : public ScopeRegionComponent
{
public:
	ScopeSelectionComponent(ScopeControlComponent* owner,
							const int initialStart = 0, 
							const int initialEnd = 0);
	
	void showPopupMenu(const int offset);
	void doCommand(const int commandID, const int offset);

	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	
};

class ScopeLoopComponent : public ScopeRegionComponent
{
public:
	ScopeLoopComponent(ScopeControlComponent* owner,
					   LoopPoint const& loopPoint,
					   const bool createdFromMouseClick = false);
	~ScopeLoopComponent();

	void showPopupMenu(const int offset);
	void doCommand(const int commandID, const int offset);

	LoopPoint& getLoopPoint() { return loopPoint; }
	
private:
	LoopPoint loopPoint;
};


class ScopeControlComponent :	public ScopeComponent,
								public LookAndFeel
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
	
	ScopeControlComponent(CriticalSection& criticalSection, ScopeStyles style = Lines, DisplayOptions options = All);
	~ScopeControlComponent();
	
	const Font getPopupMenuFont();
	void choosePopupMenu(const int offset);
	void showPopupMenu(const int offset);
	void doCommand(const int commandID, const int offset);
	
	void setAudioBuffer(Buffer const& audioBufferToUse, const double offset = 0.0, const int fftSize = -1);
	
	RGBAColour& getColour(ControlColours colour);
	void setMetaData(MetaData const& metaData);
	void resized();
	
	void mouseDown (const MouseEvent& e);	
	void mouseDrag (const MouseEvent& e);
	void mouseUp (const MouseEvent& e);
	
	void setMaxSize(const int newSize);
	int getMaxSize();
	
	void getCurrentLimits(int& start, int& end);
	void offsetBy(const int offset);
	void zoomToOffsets(int start, int end);
	void zoomAround(const int offset, const float factor);
	void zoomOutFully();
	
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
	ScopeCuePointComponent* addCuePoint(CuePoint const& cuePoint, 
										const bool addToMetaData = true, 
										const bool createdFromMousClick = false);
	ScopeCuePointComponent* addNextCuePointAt(const int offset, 
											  const bool addToMetaData = true, 
											  const bool createdFromMousClick = false);
	void removeCuePoint(const int index);
	void removeCuePoint(CuePoint const& cuePoint);
	void removeCuePoint(ScopeCuePointComponent* cuePointComponent);
	void clearCuePoints();
	void clearCuePointsBetween(const int start, const int end);
	
	void setLoopPoint(const int index, const int start, const int end);
	ScopeCuePointComponent* addLoopPoint(LoopPoint const& loopPoint, 
										 const bool addToMetaData = true, 
										 const bool createdFromMousClick = false);
	ScopeCuePointComponent* addNextLoopPointAt(const int start, const int end, 
											   const bool addToMetaData = true, 
											   const bool createdFromMousClick = false);
	void removeLoopPoint(const int index);
	void removeLoopPoint(LoopPoint const& loopPoint);
	void removeLoopPoint(ScopeLoopComponent* loopComponent);
	void clearLoopPoints();
	void clearLoopPointsBetween(const int start, const int end);
	
	void setRegion(const int index, const int start, const int end);
	ScopeCuePointComponent* addRegion(Region const& region, 
									  const bool addToMetaData = true, 
									  const bool createdFromMousClick = false);
	ScopeCuePointComponent* addNextRegionAt(const int start, const int end, 
											const bool addToMetaData = true, 
											const bool createdFromMousClick = false);
	void removeRegion(const int index);
	void removeRegion(Region const& region);
	void removeRegion(ScopeRegionComponent* regionComponent);
	void clearRegions();
	void clearRegionsBetween(const int start, const int end);
	
	void openPreferences();
	
	enum Command
	{
		UserCancelled,
		AddCuePoint = 1000,
		CreateLoopFromSelection,			
		CreateRegionFromSelection,		
		EditCuePointLabel,				
		EditStartLabel,					
		EditEndLabel,						
		DeleteCuePoint,					
		DeleteRegion,						
		DeleteLoop,						
		DeleteCuePointsInSelection,		
		DeleteLoopPointsInSelection,
		DeleteRegionsInSelection,
		DeleteCuesLoopsRegionsInSelection,
		SelectAll,						
		SelectRegion,						
		SelectLoop,						
		SetToZero,						
		SetToEnd,							
		LoopTypeNoLoop,					
		LoopTypeForward,					
		LoopTypePingPong,					
		LoopTypeReverse,					
		MoveToZeroCrossings,				
		ZoomIn,							
		ZoomOut,							
		ZoomToSelection,					
		ZoomToWindow,	
		OpenPreferences
	};
	
	typedef Dictionary<Text,ScopeControlComponent::Command> CommandDictonary;
	static const char* getCommand(Command commandID); 	

	
private:
	static const CommandDictonary& buildCommandDictionary();

	CriticalSection& metaDataLock;
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
	Text defaultCueLabel;
	int defaultCueLabelNumber;
	Text defaultLoopLabel;
	int defaultLoopLabelNumber;
	Text defaultRegionLabel;
	int defaultRegionLabelNumber;
	Buffer originalBuffer;
	int originalBufferOffset;
	
	bool dragScroll:1, dragZoomX:1, dragZoomY:1;
	int lastDragX, lastDragY;
};


class ScopeControlPreferences :		public Component
{
public:
	ScopeControlPreferences(ScopeControlComponent* scope);
	~ScopeControlPreferences();
	
	int addScopeDisplayProperties();
	int addScopeControlProperties();
	
	void paint(Graphics& g);
		
private:
	ScopeControlComponent* scope;
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
