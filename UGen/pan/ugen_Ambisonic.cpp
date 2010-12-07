// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-9 by Martin Robinson www.miajo.co.uk
 
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
 
 Based Dave Malham's Bpan
 http://www.dmalham.freeserve.co.uk/bpan_help.html
 
 ==============================================================================
 */

#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Ambisonic.h"

PanBUGenInternal::PanBUGenInternal(UGen const& input, UGen const& azimuth, UGen const& elevation, UGen const& distance) throw()
:	ProxyOwnerUGenInternal(NumInputs, 3),
	distanceFactor(-4.5),
	wLevel((float)sqrt2OverTwo),
	xyzLevel(1.f),
	centreSize(1.f)  // should be no less than 0.001
{
	inputs[Input] = input;
	inputs[Azimuth] = azimuth;
	inputs[Elevation] = elevation;		
	inputs[Distance] = distance;
}

static const float dBFactor = (-6.0/20.0);

void PanBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamplesW = proxies[W]->getSampleData();
	float* outputSamplesX = proxies[X]->getSampleData();
	float* outputSamplesY = proxies[Y]->getSampleData();
	float* outputSamplesZ = proxies[Z]->getSampleData();
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	float azimuth = *(inputs[Azimuth].processBlock(shouldDelete, blockID, channel));
	float elevation = *(inputs[Elevation].processBlock(shouldDelete, blockID, channel));
	float distance = *(inputs[Distance].processBlock(shouldDelete, blockID, channel));	
	
	float x = cos(azimuth) * cos(elevation);
	float y = sin(azimuth) * cos(elevation);
	float z = sin(elevation);
	
	float wDistance, xyzDistance;
	
	if (distance >= centreSize) 
	{
 		float factor = pow(10, (distanceFactor * dBFactor * log(distance / centreSize)) / logTwo); // ?? * oneOverLog2
 		wDistance = wLevel * factor;
 		xyzDistance =  xyzLevel * factor;
	} 
	else 
	{
 		wDistance =  wLevel * (2 - (distance / centreSize));
 		xyzDistance =  xyzLevel * (distance / centreSize) ;
	}
	
	float w = wDistance;
	x *= xyzDistance;
	y *= xyzDistance;
	z *= xyzDistance;
		
	while(numSamplesToProcess--)
	{
		float input = *inputSamples++;
		
		*outputSamplesW++ = input * w;
		*outputSamplesX++ = input * x;
		*outputSamplesY++ = input * y;
		*outputSamplesZ++ = input * z;
	}
}


END_UGEN_NAMESPACE
