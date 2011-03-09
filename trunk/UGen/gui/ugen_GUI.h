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

#ifndef _UGEN_ugen_GUI_H_
#define _UGEN_ugen_GUI_H_


/** A abstract interface for thread locking. 
 
 Derived classes will use an appropriate method e.g., a CriticalSection in Juce
 or an NSLock on iPhone.
 */
class Lock // This could be moved elsewhere in the source tree if more classes make use of it...
{
public:
	virtual ~Lock() { }
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual bool tryLock() = 0;	
};


/** A simple colour class. */
class RGBAColour
{
public:
	/** Create colour using 0-1 floats. */
	RGBAColour(float red, float green, float blue, float alpha = 1.f) throw() 
	:	r(red), g(green), b(blue), a(alpha) 
	{ 
		ugen_assert(r >= 0.f && r <= 1.f);
		ugen_assert(g >= 0.f && g <= 1.f);
		ugen_assert(b >= 0.f && b <= 1.f);
		ugen_assert(a >= 0.f && a <= 1.f);
	}
	
	/** Create colour using 0-1 doubles. */
	RGBAColour(double red, double green, double blue, double alpha = 1.0) throw() 
	:	r((float)red), g((float)green), b((float)blue), a((float)alpha) 
	{ 
		ugen_assert(r >= 0.f && r <= 1.f);
		ugen_assert(g >= 0.f && g <= 1.f);
		ugen_assert(b >= 0.f && b <= 1.f);
		ugen_assert(a >= 0.f && a <= 1.f);		
	}
	
	/** Create colour using 0-255 ints. */
	RGBAColour(int red, int green, int blue, int alpha = 255) throw() 
	:	r(red / 255.f), g(green / 255.f), b(blue / 255.f), a(alpha / 255.f) 
	{
		ugen_assert(r >= 0.f && r <= 1.f);
		ugen_assert(g >= 0.f && g <= 1.f);
		ugen_assert(b >= 0.f && b <= 1.f);
		ugen_assert(a >= 0.f && a <= 1.f);		
	}
	
	/** Create a colour using a 32-bit int, format:0xrrggbbaa. */
	RGBAColour(unsigned int rgba = 0x000000FF) throw()
	{
		a = (rgba & 0xFF) / 255.f;	rgba = rgba >> 8;
		b = (rgba & 0xFF) / 255.f;	rgba = rgba >> 8;
		g = (rgba & 0xFF) / 255.f;	rgba = rgba >> 8;
		r = (rgba & 0xFF) / 255.f;
		
		ugen_assert(r >= 0.f && r <= 1.f);
		ugen_assert(g >= 0.f && g <= 1.f);
		ugen_assert(b >= 0.f && b <= 1.f);
		ugen_assert(a >= 0.f && a <= 1.f);		
	}
	
#ifdef UGEN_JUCE
	/** Create a colout from a juce::Colour. */
	RGBAColour(juce::Colour const& juceColour) throw()
	:	r(0.f), g(0.f), b(0.f), a(0.f)
	{
		r = juceColour.getFloatRed();
		g = juceColour.getFloatGreen();
		b = juceColour.getFloatBlue();
		a = juceColour.getFloatAlpha();
	}
	
	/** Cast this colour to a juce::Colour. */
	operator juce::Colour () throw()
	{
		return juce::Colour(get32bitColour());
	}
#endif
	
#ifdef UGEN_IPHONE
	/* Create a colour from an Apple CGColorRef. */
	RGBAColour(CGColorRef colourRef)
	:	r(0.f), g(0.f), b(0.f), a(0.f)
	{
		memcpy(getFloatArray(), CGColorGetComponents(colourRef), 4 * sizeof(float));		
	}
	
//	RGBAColour(UIColor* col)
//	:	r(0.f), g(0.f), b(0.f), a(0.f)
//	{
//		memcpy(getFloatArray(), CGColorGetComponents(colourRef), 4 * sizeof(float));		
//	}	
#endif	
	
	inline float red() throw()		{ return r; }
	inline float green() throw()	{ return g; }
	inline float blue() throw()		{ return b; }
	inline float alpha() throw()	{ return a; }
	
	inline unsigned long redInt() throw()		{ unsigned long ret = (unsigned long)(r*255.f); return ret; }
	inline unsigned long greenInt() throw()		{ unsigned long ret = (unsigned long)(g*255.f); return ret; }
	inline unsigned long blueInt() throw()		{ unsigned long ret = (unsigned long)(b*255.f); return ret; }
	inline unsigned long alphaInt() throw()		{ unsigned long ret = (unsigned long)(a*255.f); return ret; }
	
	inline unsigned long get32bitColour() throw() 
	{ 
		unsigned long ret = ((alphaInt() << 24) | (redInt() << 16) | (greenInt() << 8) | blueInt()); 
		return ret;
	}
	
	inline float* getFloatArray() throw() { return &r; }
	
	static const RGBAColour& getNull() throw()
	{
		static RGBAColour null;
		return null;
	}
	
private:
	float r,g,b,a;
};

#endif // _UGEN_ugen_GUI_H_
