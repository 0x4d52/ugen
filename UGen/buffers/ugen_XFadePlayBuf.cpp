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


#ifndef UGEN_NOEXTGPL

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_XFadePlayBuf.h"
#include "ugen_PlayBuf.h"
#include "../oscillators/simple/ugen_Impulse.h"

XFadeLoopSpecInternal::XFadeLoopSpecInternal(Buffer const& sound, 
											 const int startLoop, 
											 const int endLoop, 
											 const float fadeTime)
:	numChannels(sound.getNumChannels()),
	fadeTime_(ugen::max(fadeTime, 0.f)),
	startLoop_(ugen::clip(startLoop, 0, sound.size())),
	endLoop_(ugen::clip(endLoop, startLoop_, sound.size())),
	soundOnset(sound),
	soundLoop(sound.getRegion(startLoop_, endLoop_ + fadeTime_ * UGen::getSampleRate() - 1).loopFade(fadeTime_))	
{
}

XFadeLoopSpec::XFadeLoopSpec(Buffer const& sound, 
							 const int startLoop, 
							 const int endLoop, 
							 const float fadeTime)
:	SmartPointerContainer<XFadeLoopSpecInternal>(new XFadeLoopSpecInternal(sound, 
																		   startLoop, 
																		   endLoop, 
																		   fadeTime))
{
}

XFadeLoopPlayBufUGenInternal::OnsetLoopXFade::OnsetLoopXFade(XFadeLoopPlayBufUGenInternal *owner)
:	TrigXFadeEventBase<XFadeLoopPlayBufUGenInternal>(owner)
{
}

UGen XFadeLoopPlayBufUGenInternal::OnsetLoopXFade::spawnEvent(TrigXFadeUGenInternal& /*spawn*/, 
															  const int eventCount, 
															  void* /*extraArgs*/)
{
	if(eventCount == 0)
		return PlayBuf::AR(getOwner()->soundOnset, getOwner()->inputs[XFadeLoopPlayBufUGenInternal::Rate], 0, 0, 0);
	else if(eventCount == 1)
		return PlayBuf::AR(getOwner()->soundLoop, getOwner()->inputs[XFadeLoopPlayBufUGenInternal::Rate], 0, 0, 1);
	else
		return 0;
}

XFadeLoopPlayBufUGenInternal::XFadeLoopPlayBufUGenInternal(XFadeLoopSpec const& spec, UGen const& rate)
:	ProxyOwnerUGenInternal(NumInputs, spec.getNumChannels() - 1),
	fadeTime(spec.getFadeTime()),
	startLoop(spec.getStartLoop()),
	endLoop(spec.getEndLoop()),
	soundOnset(spec.getSoundOnset()),
	soundLoop(spec.getSoundLoop()),
	bufferData(new float*[spec.getNumChannels()])
{
	inputs[Rate] = rate;
	
	float startLoopRate = UGen::getSampleRate() / startLoop;
	UGen trig = Impulse::AR(startLoopRate * rate);
	
	const int maxRepeats = 2;
	graph = TrigXFade<OnsetLoopXFade, XFadeLoopPlayBufUGenInternal>::AR(spec.getNumChannels(), 
																		trig, 
																		ugen::max(fadeTime, 0.05f), 
																		maxRepeats,
																		this);
}

XFadeLoopPlayBufUGenInternal::~XFadeLoopPlayBufUGenInternal()
{
	delete [] bufferData;
}

void XFadeLoopPlayBufUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int /*channel*/) throw()
{	
	const int numSamplesToProcess = uGenOutput.getBlockSize();	
	const int numChannels = getNumChannels();
	for(int channel = 0; channel < numChannels; channel++)
	{
		bufferData[channel] = proxies[channel]->getSampleData();
	}
	
	graph.prepareForBlock(numSamplesToProcess, blockID, -1);
	graph.setOutputs(bufferData, numSamplesToProcess, numChannels);
	graph.processBlock(shouldDelete, blockID, -1);
}


XFadeLoopPlayBuf::XFadeLoopPlayBuf (XFadeLoopSpec const& spec, UGen const& rate) throw()
{
	initInternal(spec.getNumChannels());
	generateFromProxyOwner(new XFadeLoopPlayBufUGenInternal(spec, rate));	
}



END_UGEN_NAMESPACE

#endif
