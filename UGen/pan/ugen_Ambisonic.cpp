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
	centreSize(1.f),  // should be no less than 0.001
	currentAzimuth(azimuth.getValue(0)),
	currentElevation(elevation.getValue(0)),
	currentDistance(distance.getValue(0))
{
	calculate(currentAzimuth, currentElevation, currentDistance, w, x, y, z);
	
	inputs[Input] = input;
	inputs[Azimuth] = azimuth;
	inputs[Elevation] = elevation;		
	inputs[Distance] = distance;
}


void PanBUGenInternal::calculate(const float azimuth, const float elevation, const float distance,  
								 float& w, float& x, float& y, float& z) throw()
{
	static const float dBFactor = (-6.0/20.0);
	
	float cosElevation = cos(elevation);
	x = cos(azimuth) * cosElevation;
	y = sin(azimuth) * cosElevation;
	z = sin(elevation);
	
	float wDistance, xyzDistance;
	
	if (distance >= centreSize) 
	{
 		float factor = pow(10, (distanceFactor * dBFactor * log(distance / centreSize)) * oneOverLog2);
 		wDistance = wLevel * factor;
 		xyzDistance =  xyzLevel * factor;
	} 
	else 
	{
 		wDistance =  wLevel * (2 - (distance / centreSize));
 		xyzDistance =  xyzLevel * (distance / centreSize) ;
	}
	
	w = wDistance;
	x *= xyzDistance;
	y *= xyzDistance;
	z *= xyzDistance;	
}

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
	
	float w = this->w;
	float x = this->x;
	float y = this->y;
	float z = this->z;
	
	if((azimuth == currentAzimuth) && (elevation == currentElevation) && (distance == currentDistance))
	{
		while(numSamplesToProcess--)
		{
			float input = *inputSamples++;
			
			*outputSamplesW++ = input * w;
			*outputSamplesX++ = input * x;
			*outputSamplesY++ = input * y;
			*outputSamplesZ++ = input * z;
		}
	}
	else
	{
		float newW, newX, newY, newZ;
		
		calculate(azimuth, elevation, distance, newW, newX, newY, newZ);

		float slopeFactor = 1.f / (float)numSamplesToProcess;
		float slopeW = (w - newW) * slopeFactor;
		float slopeX = (x - newX) * slopeFactor;
		float slopeY = (y - newY) * slopeFactor;
		float slopeZ = (z - newZ) * slopeFactor;
		
		while(numSamplesToProcess--)
		{
			float input = *inputSamples++;
			
			w += slopeW;
			x += slopeX;
			y += slopeY;
			z += slopeZ;
			
			*outputSamplesW++ = input * w;
			*outputSamplesX++ = input * x;
			*outputSamplesY++ = input * y;
			*outputSamplesZ++ = input * z;
		}
		
		this->w = newW;
		this->x = newX;
		this->y = newY;
		this->z = newZ;
		this->currentAzimuth = azimuth;
		this->currentElevation = elevation;
		this->currentDistance = distance;
		
	}
	
	calculate(azimuth, elevation, distance, w, x, y, z);
		
	
}


PanB::PanB(UGen const& input, UGen const& azimuth, UGen const& elevation, UGen const& distance) throw()
{	
	initInternal(4);
	
	UGen inputChecked = input.mix();
	UGen azimuthChecked = azimuth.mix();
	UGen elevationChecked = elevation.mix();	
	UGen distanceChecked = distance.mix();	

	PanBUGenInternal*  internal = new PanBUGenInternal(input, azimuth, elevation, distance);
	internalUGens[PanBUGenInternal::W] = internal;
	internalUGens[PanBUGenInternal::X] = internal->getProxy(PanBUGenInternal::X);
	internalUGens[PanBUGenInternal::Y] = internal->getProxy(PanBUGenInternal::Y);
	internalUGens[PanBUGenInternal::Z] = internal->getProxy(PanBUGenInternal::Z);
	
	// need to do initValue()...
}


DecodeBUGenInternal::DecodeBUGenInternal(UGen const& bFormat, 
					     const float azimuth, 
					     const float elevation) throw()
:	UGenInternal(NumInputs),
	cosAzimuth(cos(-azimuth)),
	sinAzimuth(sin(-azimuth)),
	sinElevation(sin(elevation))
{
	inputs[BFormat] = bFormat;
}

void DecodeBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* wSamples = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	float* xSamples = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	float* ySamples = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	float* zSamples = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	float* outputSamples = uGenOutput.getSampleData();
	
	float W_amp = sqrt2OverTwo;
	float X_amp = 0.5f * cosAzimuth;
	float Y_amp = 0.5f * sinAzimuth;
	float Z_amp = 0.5f * sinElevation;
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ =	*wSamples++ * W_amp + 
					*xSamples++ * X_amp + 
					*ySamples++ * Y_amp + 
					*zSamples++ * Z_amp;
	}
}

DecodeB::DecodeB(UGen const& bFormat, FloatArray const& azimuth, FloatArray const& elevation) throw()
{	
	int numChannels = ugen::max(azimuth.length(), elevation.length());
	initInternal(numChannels);
	
	UGen bFormatChecked = bFormat;
	
	while(bFormatChecked.getNumChannels() < 4)
	{
		bFormatChecked = UGen(bFormatChecked, UGen::getNull());
	}
	
	for(int i = 0; i < numChannels; i++)
	{
		internalUGens[i] = new DecodeBUGenInternal(bFormatChecked, azimuth.wrapAt(i), elevation.wrapAt(i));
	}
	
}

END_UGEN_NAMESPACE
