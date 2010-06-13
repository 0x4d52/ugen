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

#ifndef _UGEN_ugen_WrapFold_H_
#define _UGEN_ugen_WrapFold_H_

#include "../core/ugen_UGen.h"

#define WrapFold_InputsWithTypesAndDefaults	UGen const& input, UGen const& lower, UGen const& upper = 0.f
#define WrapFold_InputsWithTypesOnly		UGen const& input, UGen const& lower, UGen const& upper
#define WrapFold_InputsNoTypes				input, lower, upper
#define WrapFold_InputsEnum					Input, Lower, Upper

class WrapFoldBaseUGenInternal : public UGenInternal
{
public:
	WrapFoldBaseUGenInternal(WrapFold_InputsWithTypesOnly) throw();
	
	enum Inputs { WrapFold_InputsEnum, NumInputs };
};

class WrapUGenInternal : public WrapFoldBaseUGenInternal
{
public:
	WrapUGenInternal(WrapFold_InputsWithTypesOnly) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class FoldUGenInternal : public WrapFoldBaseUGenInternal
{
public:
	FoldUGenInternal(WrapFold_InputsWithTypesOnly) throw();
	UGenInternal* getChannel(const int channel) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


UGenSublcassDeclarationNoDefault(Wrap, (WrapFold_InputsNoTypes), (WrapFold_InputsWithTypesAndDefaults), COMMON_UGEN_DOCS);
UGenSublcassDeclarationNoDefault(Fold, (WrapFold_InputsNoTypes), (WrapFold_InputsWithTypesAndDefaults), COMMON_UGEN_DOCS);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define WrapFold WrapFold()
#endif


#endif // _UGEN_ugen_WrapFold_H_