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

#ifndef _UGEN_ugen_ScopeView_H_
#define _UGEN_ugen_ScopeView_H_

#include "../gui/ugen_Scope.h"

class ScopeView;
class RadialScopeView;
class ScopeViewBase;

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif


@interface UIScopeViewBase : UIView
{
	NSLock* nsLock;
	ScopeViewBase* peer;
}
@property (nonatomic,readonly) ScopeViewBase* peer;
- (id)initPeer;

- (void)lock;
- (void)unlock;
- (BOOL)tryLock;

// Obj-C wrappers for ScopeView & ScopeGUI functions...

- (void)setPolarity:(BOOL)isBipolar;
- (BOOL)getPolarity;
- (void)setYMaximum:(float)maximum;
- (float)getYMaximum;
- (void)setLowerMargin:(float)margin;
- (float)getLowerMargin;

- (void)setScaleX:(ScopeGUI::Scales)scale;
- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size;
- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num;
- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num labellingFirst:(BOOL)flag;
- (ScopeGUI::Scales)getScaleX;

- (void)setScaleY:(ScopeGUI::Scales)scale;
- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size;
- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num;
- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num toDecimalPlaces:(int)places;
- (ScopeGUI::Scales)getScaleY;

- (void)setAudioBuffer:(Buffer)buffer;
- (void)setAudioBuffer:(Buffer)buffer withOffset:(double)offset;
- (void)setAudioBuffer:(Buffer)buffer withOffset:(double)offset withFFT:(int)size;
- (Buffer)getAudioBuffer;

- (void)setWrap:(double)amount;
- (double)getWrap;

- (void)setSmoothing:(Buffer)smooth;
- (Buffer)getSmoothing;
- (void)setScopeColour:(ScopeGUI::ScopeColours)which:(RGBAColour)colour;
- (RGBAColour)getScopeColour:(ScopeGUI::ScopeColours)which;
- (void)setMarkXHeight:(int)size;
- (void)setMarkYWidth:(int)size;
- (void)setMarkSizes:(int)size;
- (int)getMarkXHeight;
- (int)getMarkYWidth;

@end

@interface UIScopeView : UIScopeViewBase
{
}
- (id)initPeerWithStyle:(ScopeGUI::ScopeStyles)style;
- (ScopeView*)peer;
@end

@interface UIRadialScopeView : UIScopeViewBase
{
}
- (id)initPeerWithStyle:(ScopeGUI::ScopeStyles)style;
- (RadialScopeView*)peer;
@end

//typedef struct UIScopeViewBaseStructPtr
//{
//	@defs(UIScopeViewBase);
//} *UIScopeViewBaseStruct;

BEGIN_UGEN_NAMESPACE


class ScopeViewBase : public ScopeGUI 
{
public:
	ScopeViewBase(UIScopeViewBase* peer, ScopeStyles style = Lines) throw();
	~ScopeViewBase();
	
	void updateGUI() throw();
	bool isValid() throw();
	void lock() throw();
	void unlock() throw();
	bool tryLock() throw();
	int getWidth() const throw();
	int getHeight() const throw();
	
	virtual void paint(CGRect const& rect) throw() = 0;
	virtual void paintBipolar(CGRect const& rect) throw() = 0;
	virtual void paintUnipolar(CGRect const& rect) throw() { } 
	virtual void paintXScale(CGRect const& rect, const int y) throw() { }
	virtual void paintYScale(CGRect const& rect, const int zero, const int maximum) throw() { }
	virtual void paintChannelLabel(CGRect const& rect, Text const& label, const int index, const int top) throw() { }
	
	UIScopeViewBase* getPeer() throw() { return peer; }
	
protected:
	UIScopeViewBase* peer;
};

class ScopeView : public ScopeViewBase
{
public:
	ScopeView(UIScopeViewBase* peer, ScopeStyles style = Lines) throw();
	~ScopeView();
	
	int getDisplayBufferSize() const { return getWidth(); }
	
	void paint(CGRect const& rect) throw();
	void paintBipolar(CGRect const& rect) throw();
	void paintUnipolar(CGRect const& rect) throw();
	void paintXScale(CGRect const& rect, const int y) throw();
	void paintYScale(CGRect const& rect, const int zero, const int maximum) throw();
	void paintChannelLabel(CGRect const& rect, Text const& label, const int index, const int top) throw();
	
private:
	int previousWidth;
};


class RadialScopeView : public ScopeViewBase
{
public:
	RadialScopeView(UIScopeViewBase* peer, ScopeStyles style = Lines) throw();
	~RadialScopeView();	
	
	void paint(CGRect const& rect) throw();
	void paintBipolar(CGRect const& rect) throw();
//	void paintUnipolar(CGRect const& rect) throw();
	void paintXScale(CGRect const& rect, const int y) throw();
//	void paintYScale(CGRect const& rect, const int zero, const int maximum) throw();
//	void paintChannelLabel(CGRect const& rect, Text const& label, const int index, const int top) throw();
	
	double getDelta() const throw();
	double getDeltaAngle() const throw();
	
	int getDisplayBufferSize() const throw();
	int getRadius() const throw();
	int getCentreX() const throw();
	int getCentreY() const throw();
	inline int getDiameter() const throw() { return getRadius() * 2; }
	
private:
	int previousRadius;
	Buffer sine, cosine;
};



#endif // _UGEN_ugen_ScopeView_H_
