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


#ifndef _UGEN_ugen_XFadePlayBuf_H_
#define _UGEN_ugen_XFadePlayBuf_H_

#ifndef UGEN_NOEXTGPL

#include "../core/ugen_UGen.h"
#include "../spawn/ugen_Textures.h"

class XFadeLoopSpecInternal : public SmartPointer
{
public:
	XFadeLoopSpecInternal(Buffer const& sound, const int startLoop, const int endLoop, const float fadeTime = 0.05f);
	
	friend class XFadeLoopPlayBufUGenInternal;
	friend class XFadeLoopSpec;
	
private:
	const int numChannels;
	const float fadeTime_;
	const int startLoop_;
	const int endLoop_;
	Buffer soundOnset;
	Buffer soundLoop;	
};

class XFadeLoopSpec : public SmartPointerContainer<XFadeLoopSpecInternal>
{
public:
	XFadeLoopSpec(Buffer const& sound, const int startLoop, const int endLoop, const float fadeTime = 0.05f);
	
	inline int getNumChannels() const			{ return getInternal()->numChannels;	}
	inline float getFadeTime() const			{ return getInternal()->fadeTime_;		}
	inline int getStartLoop() const				{ return getInternal()->startLoop_;		}
	inline int getEndLoop() const				{ return getInternal()->endLoop_;		}
	inline const Buffer& getSoundOnset() const	{ return getInternal()->soundOnset;		}
	inline const Buffer& getSoundLoop() const	{ return getInternal()->soundLoop;		}	
	
private:
	XFadeLoopSpec();
};

class XFadeLoopPlayBufUGenInternal : public ProxyOwnerUGenInternal
{	
public:
	XFadeLoopPlayBufUGenInternal(XFadeLoopSpec const& spec, UGen const& rate = 1.f);	
	~XFadeLoopPlayBufUGenInternal();
		
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw();
	
	enum Inputs { Rate, NumInputs};
	
	friend class OnsetLoopXFade;
	
private:
	const float fadeTime;
	const int startLoop;
	const int endLoop;
	Buffer soundOnset;
	Buffer soundLoop;
	
	float** const bufferData;
	
	UGen graph;
	
	class OnsetLoopXFade : public TrigXFadeEventBase<XFadeLoopPlayBufUGenInternal>
	{
	public:
		OnsetLoopXFade(XFadeLoopPlayBufUGenInternal *owner);		
		UGen spawnEvent(TrigXFadeUGenInternal& spawn, const int eventCount, void* extraArgs);
	};
	
	XFadeLoopPlayBufUGenInternal();
};


class XFadeLoopPlayBuf : public UGen 
{ 
public: 	
	XFadeLoopPlayBuf (XFadeLoopSpec const& spec, UGen const& rate = 1.f) throw();
	
	static inline UGen AR (Buffer const& sound, const int startLoop, const int endLoop, const float fadeTime = 0.05f, UGen const& rate = 1.f) throw() 
	{ 
		return XFadeLoopPlayBuf (XFadeLoopSpec(sound, startLoop, endLoop, fadeTime), rate); 
	} 	
	
	static inline UGen AR (XFadeLoopSpec const& spec, UGen const& rate = 1.f) throw() 
	{ 
		return XFadeLoopPlayBuf (spec, rate); 
	} 		
};

#endif // gpl

#endif // _UGEN_ugen_XFadePlayBuf_H_
