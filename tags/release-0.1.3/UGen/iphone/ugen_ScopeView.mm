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

#include "../core/ugen_StandardHeader.h"

#if defined(UGEN_IPHONE) && !defined(UGEN_JUCE)

BEGIN_UGEN_NAMESPACE

#include "ugen_ScopeView.h"

END_UGEN_NAMESPACE

#ifdef UGEN_NAMESPACE
using namespace UGEN_NAMESPACE;
#endif

// obj c code

@implementation UIScopeViewBase

@synthesize peer;

- (id)init
{	
	if (self = [super init])
	{
		nsLock = [[NSLock alloc] init];
		[self initPeer];
	}
	
	return self;
}

- (id)initWithFrame:(CGRect)frame;
{	
	if (self = [super initWithFrame:frame])
	{
		nsLock = [[NSLock alloc] init];
		[self initPeer];
	}
	
	return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{	
	if (self = [super initWithCoder: decoder])  
	{
		nsLock = [[NSLock alloc] init];
		[self initPeer];
	}
	
	return self;
}

- (id)initPeer
{
	peer = 0;
	return self;
}

-(void) dealloc
{
	delete peer;
	[nsLock release];
	[super dealloc];
}

- (void)lock
{
	[nsLock lock];
}

- (void)unlock
{
	[nsLock unlock];
}

- (BOOL)tryLock
{
	return [nsLock tryLock];
}

- (void)setPolarity:(BOOL)isBipolar
{
	if(!peer) return;
	
	peer->setPolarity(isBipolar);
}

- (BOOL)getPolarity
{
	if(!peer) { ugen_assertfalse; return false; }
	
	return peer->getPolarity();
}

- (void)setYMaximum:(float)maximum
{
	if(!peer) return;
	
	peer->setYMaximum(maximum);
}

- (float)getYMaximum
{
	if(!peer) { ugen_assertfalse; return 0.f; }
	
	return peer->getYMaximum();
}

- (void)setLowerMargin:(float)margin
{
	if(!peer) return;
	
	peer->setLowerMargin(margin);
}

- (float)getLowerMargin
{
	if(!peer) { ugen_assertfalse; return 0.f; }
	
	return peer->getLowerMargin();
}

- (void)setScaleX:(ScopeGUI::Scales)scale
{
	if(!peer) return;
	
	peer->setScaleX(scale);
}

- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size
{
	if(!peer) return;
	
	peer->setScaleX(scale, size);
}

- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num
{
	if(!peer) return;
	
	peer->setScaleX(scale, size, num);
}

- (void)setScaleX:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num labellingFirst:(BOOL)flag
{
	if(!peer) return;
	
	peer->setScaleX(scale, size, num, flag);
}

- (ScopeGUI::Scales)getScaleX
{
	if(!peer) { ugen_assertfalse; return ScopeGUI::LabelXNone; }
	
	return peer->getScaleX();
}


- (void)setScaleY:(ScopeGUI::Scales)scale
{
	if(!peer) return;
	
	peer->setScaleY(scale);
}

- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size
{
	if(!peer) return;
	
	peer->setScaleY(scale, size);
}

- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num
{
	if(!peer) return;
	
	peer->setScaleY(scale, size, num);
}

- (void)setScaleY:(ScopeGUI::Scales)scale withMarkSpacing:(double)size usingLabelHop:(int)num toDecimalPlaces:(int)places
{
	if(!peer) return;
	
	peer->setScaleY(scale, size, num, places);
}

- (ScopeGUI::Scales)getScaleY
{
	if(!peer) { ugen_assertfalse; return ScopeGUI::LabelYNone; }
	
	return peer->getScaleY();
}


- (void) setAudioBuffer:(Buffer)buffer
{
	if(!peer) return;
	
	peer->setAudioBuffer(buffer);
}

- (void) setAudioBuffer:(Buffer)buffer withOffset:(double)offset
{
	if(!peer) return;
	
	peer->setAudioBuffer(buffer, offset);
}

- (void) setAudioBuffer:(Buffer)buffer withOffset:(double)offset withFFT:(int)size
{
	if(!peer) return;
	
	peer->setAudioBuffer(buffer, offset, size);
}

- (Buffer) getAudioBuffer
{
	if(!peer) { ugen_assertfalse; return Buffer(); }
	
	return peer->getAudioBuffer();
}

- (void) setWrap:(double)amount
{
	if(!peer) return;
	
	peer->setWrap(amount);
}

- (double) getWrap
{
	if(!peer) { ugen_assertfalse; return 0.0; }
	
	return peer->getWrap();
}


- (void)setSmoothing:(Buffer)smooth
{
	if(!peer) return;
	
	peer->setSmoothing(smooth);
}

- (Buffer)getSmoothing
{
	if(!peer) return Buffer();
	
	return peer->getSmoothing();
}

- (void)setScopeColour:(ScopeGUI::ScopeColours)which:(RGBAColour)colour
{
	if(!peer) return;
	
	peer->setScopeColour(which, colour);
}

- (RGBAColour)getScopeColour:(ScopeGUI::ScopeColours)which
{
	if(!peer) { ugen_assertfalse; return RGBAColour(); }
	
	return peer->getScopeColour(which);
}

- (void)setMarkXHeight:(int)size
{
	if(!peer) return;
	
	peer->setMarkXHeight(size);
}

- (void)setMarkYWidth:(int)size
{
	if(!peer) return;
	
	peer->setMarkYWidth(size);
}

- (void)setMarkSizes:(int)size
{
	if(!peer) return;
	
	peer->setMarkSizes(size);
}

- (int)getMarkXHeight
{
	if(!peer) { ugen_assertfalse; return 0; }
	
	return peer->getMarkXHeight();
}

- (int)getMarkYWidth
{
	if(!peer) { ugen_assertfalse; return 0; }
	
	return peer->getMarkYWidth();
}


@end



@implementation UIScopeView

- (id)initPeer
{
	if(peer) delete peer;
	peer = new ScopeView(self, ScopeGUI::Lines);
	return self;
}

- (id)initPeerWithStyle:(ScopeGUI::ScopeStyles)style
{
	if(peer) delete peer;
	peer = new ScopeView(self, style);
	return self;
}

- (ScopeView*)peer
{
	return (ScopeView*)peer;
}

-(void) dealloc
{
	[super dealloc];
}

- (void) drawRect: (CGRect) aRect
{	
	if(peer) peer->paint(aRect);
}

@end



@implementation UIRadialScopeView

- (id)initPeer
{
	if(peer) delete peer;
	peer = new RadialScopeView(self, ScopeGUI::Lines);
	return self;
}

- (id)initPeerWithStyle:(ScopeGUI::ScopeStyles)style
{
	if(peer) delete peer;
	peer = new RadialScopeView(self, style);
	return self;
}

- (RadialScopeView*)peer
{
	return (RadialScopeView*)peer;
}

-(void) dealloc
{
	[super dealloc];
}

- (void) drawRect: (CGRect) aRect
{	
	if(peer) peer->paint(aRect);
}

@end


BEGIN_UGEN_NAMESPACE

// c++ code in here

ScopeViewBase::ScopeViewBase(UIScopeViewBase* peerToUse, ScopeStyles style) throw()
:	ScopeGUI(style),
	peer(peerToUse)
{
	[peer retain];
}

ScopeViewBase::~ScopeViewBase()
{
	[peer release];
}

void ScopeViewBase::updateGUI() throw()
{
	[peer performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
}

bool ScopeViewBase::isValid() throw()
{
	return peer != 0; // not sure how to check this for cocoa views
}

void ScopeViewBase::lock() throw()
{
	[peer lock];
}

void ScopeViewBase::unlock() throw()
{
	[peer unlock];
}

bool ScopeViewBase::tryLock() throw()
{
	return [peer tryLock];
}

int ScopeViewBase::getWidth() const throw()
{
	CGRect const& bounds = [peer bounds];
	return bounds.size.width;
}

int ScopeViewBase::getHeight() const throw()
{
	CGRect const& bounds = [peer bounds];
	return bounds.size.height;
}

static const CGAffineTransform flipTransform = CGAffineTransformMake(1.0,0.0,0.0,-1.0, 0.0,0.0);

static inline void floatToString(char* string, const int size, const float value, const int decimalPlaces)
{
	snprintf(string, size, "%.*f", decimalPlaces, value);
}

static inline void timeToTimecodeString (char* string, const int size, const double seconds)
{
    const double absSecs = fabs (seconds);
    const char* const sign = (seconds < 0) ? "-" : "";
	
    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;
	const int ms =  ((int)(absSecs * 1000 + 0.5)) % 1000;
	
	if(hours == 0)
	{
		if(mins == 0)
		{
			snprintf(string, size, "%s%01d.%03d", sign, secs, ms);
		}
		else
		{
			snprintf(string, size, "%s%01d:%02d.%03d", sign, mins, secs, ms);
		}
	}
	else
	{
		snprintf(string, size, "%s%01d:%02d:%02d.%03d", sign, hours, mins, secs, ms);
	}
}

static inline void frequencyToString (char* string, const int size, const double frequency)
{
	if(frequency >= 1000.0)
	{
		snprintf(string, size, "%.2fkHz", frequency * 0.001);
	}
	else
	{
		snprintf(string, size, "%dHz", (int)(frequency + 0.5));
	}
}


ScopeView::ScopeView(UIScopeViewBase* peer, ScopeStyles style) throw()
:	ScopeViewBase(peer, style),
	previousWidth(0)
{
}

ScopeView::~ScopeView()
{
}

void ScopeView::paint(CGRect const& rect) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextClearRect(context, rect);
	
	CGContextSetFillColor(context, colours[Background].getFloatArray());
	CGContextFillRect(context, rect);
	CGContextStrokePath(context);
			
	const int width = getWidth();
	if(width != previousWidth)
	{
		lock();
		resizedGUI();
		unlock();
		
		previousWidth = width;
	}
	else
		calculateBuffers();
	
	lock();
	
	if(minDrawBuffer.size() > 0 && maxDrawBuffer.size() > 0)
	{		
		if(isBipolar)
			paintBipolar(rect);
		else
			paintUnipolar(rect);
	}
	
	unlock();
}

void ScopeView::paintBipolar(CGRect const& rect) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	CGContextSetLineWidth(context, 1.0f);
		
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		const int channelHeight = getHeight() / numChannels;
		const int halfChannelHeight = channelHeight / 2;
		
		const float halfChannelHeightOverYMaximum = halfChannelHeight / yMaximum;
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float top = channelHeight * channel - 0.5f;
			const float middle = top + halfChannelHeight;
			const float bottom = top + channelHeight;
			
			// top line
			if(channel > 0)
			{
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, 0.5, top + 1.f);
				CGContextAddLineToPoint(context, getWidth()-1, top + 1.f);
				CGContextSetStrokeColor(context, colours[TopLine].getFloatArray());
				CGContextStrokePath(context);
			}
			
			float oldMinimum = clip2(minDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;
					
			// first scope line
			CGContextBeginPath(context);
			CGContextSetStrokeColor(context, colours[Trace].getFloatArray());
			
			CGContextMoveToPoint(context, 0.5f, middle - oldMaximum);
			CGContextAddLineToPoint(context, 0.5f, middle - oldMinimum);
			
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
				
				// the scope lines
				CGContextMoveToPoint(context, i + 0.5f, lineTop);
				CGContextAddLineToPoint(context, i + 0.5f, lineBottom);
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
			
			CGContextStrokePath(context);
			
			paintXScale(rect, middle);
			paintYScale(rect, middle, top);
			paintYScale(rect, middle, bottom);
			paintChannelLabel(rect, channelLabels.wrapAt(channel), channel, top);
		}		
	}
}

void ScopeView::paintUnipolar(CGRect const& rect) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
		
	CGContextSetLineWidth(context, 1.0f);
	
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		const int channelHeight = getHeight() / numChannels;
		const float channelHeightOverYMaximum = channelHeight / yMaximum;
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float top = channelHeight * channel - 0.5f;
			const float bottom = top + channelHeight;
			
			// top line
			if(channel > 0)
			{
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, 0.5, top + 1.f);
				CGContextAddLineToPoint(context, getWidth()-1, top + 1.f);
				CGContextSetStrokeColor(context, colours[TopLine].getFloatArray());
				CGContextStrokePath(context);
			}
			
			// middle line
			CGContextBeginPath(context);
			CGContextMoveToPoint(context, 0.5, bottom);
			CGContextAddLineToPoint(context, getWidth()-1, bottom);
			CGContextSetStrokeColor(context, colours[ZeroLine].getFloatArray());
			CGContextStrokePath(context);
			
			float oldMinimum = clip(minDrawBuffer.getSampleUnchecked(channel, 0), 0.f, yMaximum) * channelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip(maxDrawBuffer.getSampleUnchecked(channel, 0), 0.f, yMaximum) * channelHeightOverYMaximum + 0.5f;
			
			// first scope line
			CGContextBeginPath(context);
			CGContextSetStrokeColor(context, colours[Trace].getFloatArray());
			
			CGContextMoveToPoint(context, 0.5f, bottom - oldMaximum);
			CGContextAddLineToPoint(context, 0.5f, bottom - oldMinimum);
			
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
				
				// the scope lines
				CGContextMoveToPoint(context, i + 0.5f, lineTop);
				CGContextAddLineToPoint(context, i + 0.5f, lineBottom);
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
			
			CGContextStrokePath(context);
			
			paintXScale(rect, bottom);
			paintYScale(rect, bottom, top);
			paintChannelLabel(rect, channelLabels.wrapAt(channel), channel, top);
		}	
	}
}

void ScopeView::paintXScale(CGRect const& rect, const int y) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	// zero line
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, 0.5, y + 0.5);
	CGContextAddLineToPoint(context, getWidth()-1, y + 0.5);
	CGContextSetStrokeColor(context, colours[ZeroLine].getFloatArray());
	CGContextStrokePath(context);
	
	if(scaleX != ScopeGUI::LabelXNone)
	{		
		const double audioBufferSize = audioBuffer.size();
		const double samplesPerPixel = (double)getDisplayBufferSize() / audioBufferSize;
		
		const int markSpacing = markSpacingX;
		const unsigned int labelHop = labelHopX;
		
		unsigned int mark = 0;
		const int labelOffsetX = 3;
		const int labelOffsetY = markXHeight > 5 ? 4 : markXHeight + 1;
		
		CGContextSelectFont (context, "Arial", textSizeX, kCGEncodingMacRoman);
		CGContextSetRGBFillColor (context, 0, 1, 0, 1);
		CGContextSetTextMatrix(context, flipTransform);
		
		CGContextSetFillColor(context, colours[TextX].getFloatArray());
		CGContextSetStrokeColor(context, colours[LabelMarks].getFloatArray());
		
		CGContextBeginPath(context);
		for(int audioIndex = 0; audioIndex < audioBufferSize; audioIndex += markSpacing)
		{			
			const int x = audioIndex * samplesPerPixel + 0.5;
			
			if(((labelHop <= 1) || ((mark % labelHop) == 0)))
			{
				CGContextMoveToPoint(context, x + 0.5, y + 0.5);
				CGContextAddLineToPoint(context, x + 0.5, y - markXHeight * 1.5 + 0.5);
				
				if(scaleX != ScopeGUI::LabelXMarks && 
				   (((mark == 0) && (labelFirstX == false)) == false)
				) {
					char label[256];
					if(scaleX == ScopeGUI::LabelXTime)
					{
						timeToTimecodeString(label, sizeof(label), 
											 (audioIndex + offsetSamples) / UGen::getSampleRate());
					}
					else if((scaleX == ScopeGUI::LabelXFrequency) && (fftSize > 0))
					{
						const double fftFreq = UGen::getSampleRate() / fftSize;
						const double binFreq = (audioIndex + offsetSamples) * fftFreq;
						
						frequencyToString(label, sizeof(label), binFreq);
					}
					else
					{
						snprintf(label, sizeof(label), "%d", (int)(audioIndex + offsetSamples + 0.5));
					}
					
					CGContextShowTextAtPoint(context, 
											 x + labelOffsetX + 0.5f, 
											 y - labelOffsetY + 0.5f, 
											 label, strlen(label));	
				}
			}
			else
			{
				CGContextMoveToPoint(context, x + 0.5, y + 0.5);
				CGContextAddLineToPoint(context, x + 0.5, y - markXHeight + 0.5);
			}
			
			mark++;
		}	
		CGContextStrokePath(context);
	}
}

void ScopeView::paintYScale(CGRect const& rect, const int zero, const int maximum) throw()
{
	if(scaleY != ScopeGUI::LabelYNone)
	{
		CGContextRef context = UIGraphicsGetCurrentContext();
		
		const double markSpacing = markSpacingY;
		const int decimalPlaces = decimalPlacesY;
		const unsigned int labelHop = labelHopY;
		
		unsigned int mark = 0;
		const int labelOffsetX = markYWidth * 1.5 + 2;
		const int labelOffsetY = textSizeY / 2 + 2;
		
		const int height = zero-maximum;
		const double pixelFactor = (double)height / yMaximum;
		
		const double sign = height > 0 ? 1.0 : -1.0;
		
		CGContextSelectFont (context, "Arial", textSizeY, kCGEncodingMacRoman);
		CGContextSetRGBFillColor (context, 0, 1, 0, 1);
		CGContextSetTextMatrix(context, flipTransform);

		CGContextSetFillColor(context, colours[TextY].getFloatArray());
		CGContextSetStrokeColor(context, colours[LabelMarks].getFloatArray());
		
		CGContextBeginPath(context);
		for(double level = 0.0; level <= yMaximum; level += markSpacing)
		{
			const int y = zero - level * pixelFactor;

			if((((labelHop <= 1) || ((mark % labelHop) == 0)) && 
			   (mark > 0) && 
			   ((level+markSpacing) <= yMaximum))
			) {
				CGContextMoveToPoint(context, 0.5, y + 0.5);
				CGContextAddLineToPoint(context, markYWidth * 1.5 + 0.5, y + 0.5);
				
				if(scaleY != ScopeGUI::LabelYMarks)
				{
					char label[256];
					floatToString(label, sizeof(label), sign * level, decimalPlaces);
					CGContextShowTextAtPoint(context, labelOffsetX + 0.5f, y + labelOffsetY + 0.5f, label, strlen(label));	
				}
			}
			else
			{
				CGContextMoveToPoint(context, 0.5, y + 0.5);
				CGContextAddLineToPoint(context, markYWidth + 0.5, y + 0.5);	
			}
			
			mark++;
		}
		CGContextStrokePath(context);
	}
}

void ScopeView::paintChannelLabel(CGRect const& rect, Text const& label, const int index, const int top) throw()
{
	if(labelChannels)
	{
		CGContextRef context = UIGraphicsGetCurrentContext();
		
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
		
		CGContextSelectFont (context, "Arial", textSizeChannel, kCGEncodingMacRoman);
		CGContextSetTextMatrix(context, flipTransform);
		CGContextSetFillColor(context, colours[TextChannel].getFloatArray());
				
		// measure the text
		CGContextSetTextDrawingMode(context, kCGTextInvisible);
		CGContextShowTextAtPoint(context, 0, 0, labelToPaint, strlen(labelToPaint));
		CGPoint pt = CGContextGetTextPosition(context);
		CGContextSetTextDrawingMode(context, kCGTextFill);
		
		// draw it right aligned
		CGContextShowTextAtPoint(context, 
								 getWidth() - pt.x - 3 + 0.5f,
								 top + textSizeChannel + 2 + 0.5f, 
								 labelToPaint, strlen(labelToPaint));	
	}	
}



RadialScopeView::RadialScopeView(UIScopeViewBase* peer, ScopeStyles style) throw()
:	ScopeViewBase(peer, style),
	previousRadius(0),
	sine(Buffer::getTableSine8192()),
	cosine(Buffer::getTableCosine8192())
{
	ugen_assert(sine.size() == cosine.size());
}

RadialScopeView::~RadialScopeView()
{
}

void RadialScopeView::paint(CGRect const& rect) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextClearRect(context, rect);
	
	CGContextSetFillColor(context, colours[Background].getFloatArray());
	CGContextFillRect(context, rect);
	CGContextStrokePath(context);
		
	const int radius = getRadius();
	if(radius != previousRadius)
	{
		lock();
		resizedGUI();
		unlock();
		
		previousRadius = radius;
	}
	else
		calculateBuffers();
	
	lock();
	
	if(minDrawBuffer.size() > 0 && maxDrawBuffer.size() > 0)
	{		
		paintBipolar(rect);
	}
	
	unlock();
}

void RadialScopeView::paintBipolar(CGRect const& rect) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextSetLineWidth(context, 1.0f);
	
	const int numChannels = minDrawBuffer.getNumChannels();
	
	if(numChannels > 0)
	{
		const float centreX = getCentreX() + 0.5f;
		const float centreY = getCentreY() + 0.5f;
		const float innerMargin = min(getLowerMargin(), (float)getRadius());
		const int bufferSizeMinus1 = minDrawBuffer.size()-1;
		const int channelHeight = (getRadius() - innerMargin) / numChannels;
		const int halfChannelHeight = channelHeight / 2;
		const int wrapOffset = bufferWrap * minDrawBuffer.size();
		
		const float halfChannelHeightOverYMaximum = halfChannelHeight / yMaximum;
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const float minRadius = channelHeight * channel - 0.5f + innerMargin;
			const float zeroRadius = minRadius + halfChannelHeight;
			const float maxRadius = minRadius + channelHeight;
			
			// outer			
			CGContextSetStrokeColor(context, colours[TopLine].getFloatArray());
			CGContextStrokeEllipseInRect(context, CGRectMake(getCentreX()-maxRadius, getCentreY()-maxRadius, maxRadius*2, maxRadius*2));
			
			paintXScale(rect, zeroRadius);
//			paintYScale(rect, zeroRadius, minRadius);
//			paintYScale(rect, zeroRadius, maxRadius);
//			paintChannelLabel(rect, channelLabels.wrapAt(channel), channel, minRadius);
			
//			float oldMinimum = clip2(minDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
//			float oldMaximum = clip2(maxDrawBuffer.getSampleUnchecked(channel, 0), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;
			float oldMinimum = clip2(minDrawBuffer.wrapAt(channel, 0 + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum - 0.5f;
			float oldMaximum = clip2(maxDrawBuffer.wrapAt(channel, 0 + wrapOffset), yMaximum) * halfChannelHeightOverYMaximum + 0.5f;

			
			const double delta = getDelta() * sine.size();
			double position = delta;
			
			// first scope line
			CGContextBeginPath(context);
			CGContextSetStrokeColor(context, colours[Trace].getFloatArray());
			CGContextMoveToPoint(context, centreX, centreY - zeroRadius - oldMaximum);
			CGContextAddLineToPoint(context, centreX, centreY - zeroRadius - oldMinimum);
			
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
				
				// the scope lines
				const float sinAngle = sine.getSampleUnchecked(position);
				const float cosAngle = cosine.getSampleUnchecked(position);
				
				const float startX = sinAngle * lineBottom;
				const float startY = cosAngle * lineBottom;
				const float endX = sinAngle * lineTop;
				const float endY = cosAngle * lineTop;
				
				CGContextMoveToPoint(context, centreX + startX, centreY - startY);
				CGContextAddLineToPoint(context, centreX + endX, centreY - endY);
				
				oldMinimum = minimum + 0.5f;
				oldMaximum = maximum - 0.5f;
			}
			
			CGContextStrokePath(context);
			
			if(innerMargin > 0)
			{			
				// inner				
				CGContextSetStrokeColor(context, colours[TopLine].getFloatArray());
				CGContextStrokeEllipseInRect(context, CGRectMake(getCentreX()-innerMargin, 
																 getCentreY()-innerMargin, 
																 innerMargin*2, 
																 innerMargin*2));
			}
		}		
	}
}

void RadialScopeView::paintXScale(CGRect const& rect, const int radius) throw()
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextSetLineWidth(context, 1.0f);
	
	const int diameter = radius*2;
	const float centreX = getCentreX() + 0.5f;
	const float centreY = getCentreY() + 0.5f;
	
	// zero line	
	CGContextSetStrokeColor(context, colours[ZeroLine].getFloatArray());
	CGContextStrokeEllipseInRect(context, CGRectMake(centreX-radius, centreY-radius, diameter, diameter));
	
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
		
		CGContextSelectFont (context, "Arial", textSizeX, kCGEncodingMacRoman);
		CGContextSetRGBFillColor (context, 0, 1, 0, 1);
		
		CGContextSetFillColor(context, colours[TextX].getFloatArray());
		CGContextSetStrokeColor(context, colours[LabelMarks].getFloatArray());
		
		CGContextBeginPath(context);
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
				
				CGContextMoveToPoint(context, centreX+startX, centreY-startY);
				CGContextAddLineToPoint(context, centreX+endX, centreY-endY);
				
				if(scaleX != ScopeGUI::LabelXMarks && 
				   (((mark == 0) && (labelFirstX == false)) == false)
				   ) {
					char label[256];
					if(scaleX == ScopeGUI::LabelXTime)
					{
						timeToTimecodeString(label, sizeof(label), 
											 (audioIndex + offsetSamples) / UGen::getSampleRate());
					}
					else if((scaleX == ScopeGUI::LabelXFrequency) && (fftSize > 0))
					{
						const double fftFreq = UGen::getSampleRate() / fftSize;
						const double binFreq = (audioIndex + offsetSamples) * fftFreq;
						
						frequencyToString(label, sizeof(label), binFreq);
					}
					else
					{
						snprintf(label, sizeof(label), "%d", (int)(audioIndex + offsetSamples + 0.5));
					}
					
					const double rotation = piOverTwo-angle;
					//if(angle > pi) rotation += pi;					
					CGAffineTransform transform = CGAffineTransformRotate(flipTransform, rotation);
					CGContextSetTextMatrix(context, transform);
					CGContextShowTextAtPoint(context, 
											 centreX+endX*1.01, 
											 centreY-endY*1.01, 
											 label, strlen(label));	
				}
			}
			else
			{
				const float end = radius + markXHeight;
				const float endX = sinAngle * end;
				const float endY = cosAngle * end;
				
				CGContextMoveToPoint(context, centreX+startX, centreY-startY);
				CGContextAddLineToPoint(context, centreX+endX, centreY-endY);
			}
			
			mark++;
		}	
		CGContextStrokePath(context);
	}
	
}

double RadialScopeView::getDelta() const throw()
{
	return 1.0/getDisplayBufferSize();
}

double RadialScopeView::getDeltaAngle() const throw()
{
	return sin(1.0/getRadius());
}

int RadialScopeView::getDisplayBufferSize() const throw()
{
	const double angle = getDeltaAngle();
	return twoPi / angle + 0.5;
}

int RadialScopeView::getRadius() const throw()
{
	return ugen::min(getWidth(), getHeight()) / 2;
}

int RadialScopeView::getCentreX() const throw()
{
	return getWidth() / 2;
}

int RadialScopeView::getCentreY() const throw()
{
	return getHeight() / 2;
}


END_UGEN_NAMESPACE

#endif


/*
 static void testText()
 {
 CGContextRef context = UIGraphicsGetCurrentContext();
 
 const char* text = "10kHz -1.0";
 CGContextSelectFont (context, "Arial", 9.f, kCGEncodingMacRoman);
 CGContextSetRGBFillColor (context, 0, 1, 0, 1);
 CGContextSetTextMatrix(context, flipTransform);
 CGContextShowTextAtPoint(context, 20.f, 20.f, text, strlen(text));	
 }
 */
