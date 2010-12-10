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
 
 Based Dave Malham`s code
 http://www.dmalham.freeserve.co.uk
 
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
	
	const float cosElevation = cos(elevation);
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
	const float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	const float azimuth = *(inputs[Azimuth].processBlock(shouldDelete, blockID, channel));
	const float elevation = *(inputs[Elevation].processBlock(shouldDelete, blockID, channel));
	const float distance = *(inputs[Distance].processBlock(shouldDelete, blockID, channel));	
	
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

		const float slopeFactor = 1.f / (float)numSamplesToProcess;
		const float slopeW = (newW - w) * slopeFactor;
		const float slopeX = (newX - x) * slopeFactor;
		const float slopeY = (newY - y) * slopeFactor;
		const float slopeZ = (newZ - z) * slopeFactor;
		
		while(numSamplesToProcess--)
		{
			const float input = *inputSamples++;
			
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
	const float* wSamples = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	const float* xSamples = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	const float* ySamples = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	const float* zSamples = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	float* outputSamples = uGenOutput.getSampleData();
	
	const float wLevel = sqrt2OverTwo;
	const float xLevel = 0.5f * cosAzimuth; // why 0.5?
	const float yLevel = 0.5f * sinAzimuth;
	const float zLevel = 0.5f * sinElevation;
	
	while(numSamplesToProcess--)
	{
		*outputSamples++ =	*wSamples++ * wLevel + 
					*xSamples++ * xLevel + 
					*ySamples++ * yLevel + 
					*zSamples++ * zLevel;
	}
}

DecodeB::DecodeB(UGen const& bFormat, FloatArray const& azimuth, FloatArray const& elevation) throw()
{	
	const int numChannels = ugen::max(azimuth.length(), elevation.length());
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

ManipulateBUGenInternal::ManipulateBUGenInternal(UGen const& bFormat, 
												 UGen const& parameter) throw()
:	ProxyOwnerUGenInternal(NumInputs, 3),
	currentParam(parameter.getValue(0)),
	sinParam(sin(currentParam)),
	cosParam(cos(currentParam))
{
	inputs[BFormat] = bFormat; // checked to be four channels
	inputs[Parameter] = parameter; // checked to be one channel
}

RotateBUGenInternal::RotateBUGenInternal(UGen const& bFormat, UGen const& param) throw()
:	ManipulateBUGenInternal(bFormat, param)
{
}

void RotateBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	const float* inputSamplesW = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	const float* inputSamplesX = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	const float* inputSamplesY = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	const float* inputSamplesZ = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	const float rotate = *(inputs[Parameter].processBlock(shouldDelete, blockID, 0));
	float* outputSamplesW = proxies[W]->getSampleData();
	float* outputSamplesX = proxies[X]->getSampleData();
	float* outputSamplesY = proxies[Y]->getSampleData();
	float* outputSamplesZ = proxies[Z]->getSampleData();
	
	float sinParam = this->sinParam;
	float cosParam = this->cosParam;
		
	if(rotate == currentParam)
	{
		while(numSamplesToProcess--)
		{
			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX * cosParam - *inputSamplesY * sinParam;
			*outputSamplesY++ = *inputSamplesX * sinParam + *inputSamplesY * cosParam;
			*outputSamplesZ++ = *inputSamplesZ++;
			
			inputSamplesX++;
			inputSamplesY++;
		}		
	}
	else
	{
		const float newSinParam = sin(rotate);
		const float newCosParam = cos(rotate);
		const float slope = 1.f/(float)numSamplesToProcess;
		const float slopeSin = (newSinParam - sinParam) * slope;
		const float slopeCos = (newCosParam - cosParam) * slope;
		
		while(numSamplesToProcess--)
		{
			sinParam += slopeSin;
			cosParam += slopeCos;

			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX * cosParam - *inputSamplesY * sinParam;
			*outputSamplesY++ = *inputSamplesX * sinParam + *inputSamplesY * cosParam;
			*outputSamplesZ++ = *inputSamplesZ++;
			
			inputSamplesX++;
			inputSamplesY++;
		}
		
		this->sinParam = newSinParam;
		this->cosParam = newCosParam;
		this->currentParam = rotate;
	}	
}

TiltBUGenInternal::TiltBUGenInternal(UGen const& bFormat, UGen const& param) throw()
:	ManipulateBUGenInternal(bFormat, param)
{
}

void TiltBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	const float* inputSamplesW = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	const float* inputSamplesX = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	const float* inputSamplesY = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	const float* inputSamplesZ = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	const float tilt = *(inputs[Parameter].processBlock(shouldDelete, blockID, 0));
	float* outputSamplesW = proxies[W]->getSampleData();
	float* outputSamplesX = proxies[X]->getSampleData();
	float* outputSamplesY = proxies[Y]->getSampleData();
	float* outputSamplesZ = proxies[Z]->getSampleData();
	
	float sinParam = this->sinParam;
	float cosParam = this->cosParam;
	
	if(tilt == currentParam)
	{		
		while(numSamplesToProcess--)
		{
			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX++; 
			*outputSamplesY++ = *inputSamplesY * cosParam - *inputSamplesZ * sinParam;
			*outputSamplesZ++ = *inputSamplesY * sinParam + *inputSamplesZ * cosParam;
			
			inputSamplesY++;
			inputSamplesZ++;
		}		
	}
	else
	{
		const float newSinParam = sin(tilt);
		const float newCosParam = cos(tilt);
		const float slope = 1.f/(float)numSamplesToProcess;
		const float slopeSin = (newSinParam - sinParam) * slope;
		const float slopeCos = (newCosParam - cosParam) * slope;
		
		while(numSamplesToProcess--)
		{
			sinParam += slopeSin;
			cosParam += slopeCos;
			
			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX++; 
			*outputSamplesY++ = *inputSamplesY * cosParam - *inputSamplesZ * sinParam;
			*outputSamplesZ++ = *inputSamplesY * sinParam + *inputSamplesZ * cosParam;
			
			inputSamplesY++;
			inputSamplesZ++;
		}
		
		this->sinParam = newSinParam;
		this->cosParam = newCosParam;
		this->currentParam = tilt;
	}		
}

TumbleBUGenInternal::TumbleBUGenInternal(UGen const& bFormat, UGen const& param) throw()
:	ManipulateBUGenInternal(bFormat, param)
{
}

void TumbleBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	const float* inputSamplesW = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	const float* inputSamplesX = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	const float* inputSamplesY = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	const float* inputSamplesZ = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	const float tumble = *(inputs[Parameter].processBlock(shouldDelete, blockID, 0));
	float* outputSamplesW = proxies[W]->getSampleData();
	float* outputSamplesX = proxies[X]->getSampleData();
	float* outputSamplesY = proxies[Y]->getSampleData();
	float* outputSamplesZ = proxies[Z]->getSampleData();
	
	float sinParam = this->sinParam;
	float cosParam = this->cosParam;
	
	if(tumble == currentParam)
	{		
		while(numSamplesToProcess--)
		{
			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX * cosParam - *inputSamplesZ * sinParam; 
			*outputSamplesY++ = *inputSamplesY++;
			*outputSamplesZ++ = *inputSamplesX * sinParam + *inputSamplesZ * cosParam;
			
			inputSamplesX++;
			inputSamplesZ++;
		}		
	}
	else
	{
		const float newSinParam = sin(tumble);
		const float newCosParam = cos(tumble);
		const float slope = 1.f/(float)numSamplesToProcess;
		const float slopeSin = (newSinParam - sinParam) * slope;
		const float slopeCos = (newCosParam - cosParam) * slope;
		
		while(numSamplesToProcess--)
		{
			sinParam += slopeSin;
			cosParam += slopeCos;
			
			*outputSamplesW++ = *inputSamplesW++;
			*outputSamplesX++ = *inputSamplesX * cosParam - *inputSamplesZ * sinParam; 
			*outputSamplesY++ = *inputSamplesY++;
			*outputSamplesZ++ = *inputSamplesX * sinParam + *inputSamplesZ * cosParam;
			
			inputSamplesX++;
			inputSamplesZ++;
		}
		
		this->sinParam = newSinParam;
		this->cosParam = newCosParam;
		this->currentParam = tumble;
	}		
}
	
RotateB::RotateB(UGen const& bFormat, UGen const& rotate) throw()
{	
	initInternal(4);
	
	UGen bFormatChecked = bFormat;
	
	while(bFormatChecked.getNumChannels() < 4)
	{
		bFormatChecked = UGen(bFormatChecked, UGen::getNull());
	}
	
	RotateBUGenInternal* internal = new RotateBUGenInternal(bFormatChecked, rotate.mix());
	generateFromProxyOwner(internal);
}

TiltB::TiltB(UGen const& bFormat, UGen const& tilt) throw()
{	
	initInternal(4);
	
	UGen bFormatChecked = bFormat;
	
	while(bFormatChecked.getNumChannels() < 4)
	{
		bFormatChecked = UGen(bFormatChecked, UGen::getNull());
	}
	
	TiltBUGenInternal* internal = new TiltBUGenInternal(bFormatChecked, tilt.mix());
	generateFromProxyOwner(internal);
}

TumbleB::TumbleB(UGen const& bFormat, UGen const& tumble) throw()
{	
	initInternal(4);
	
	UGen bFormatChecked = bFormat;
	
	while(bFormatChecked.getNumChannels() < 4)
	{
		bFormatChecked = UGen(bFormatChecked, UGen::getNull());
	}
	
	TumbleBUGenInternal* internal = new TumbleBUGenInternal(bFormatChecked, tumble.mix());
	generateFromProxyOwner(internal);
}

ZoomBUGenInternal::ZoomBUGenInternal(UGen const& bFormat, UGen const& azimuth, UGen const& elevation, UGen const& zoom) throw()
:	ProxyOwnerUGenInternal(NumInputs, 3),
	currentAzimuth(azimuth.getValue(0)),
	currentElevation(elevation.getValue(0)),
	currentZoom(ugen::clip2(zoom.getValue(0), 1.f))
{
	calculateSinCos(currentAzimuth, currentElevation, 
		cosAzimuthPre, sinAzimuthPre, cosElevationPre, sinElevationPre,
		cosAzimuthPost, sinAzimuthPost, cosElevationPost, sinElevationPost);
	
	calculateZoom(currentZoom, zoomWW, zoomXW, zoomWX, zoomYY, zoomZZ);
	
	inputs[BFormat] = bFormat;
	inputs[Azimuth] = azimuth;
	inputs[Elevation] = elevation;
	inputs[Zoom] = zoom;
}

void ZoomBUGenInternal::calculateSinCos(const float azimuth, const float elevation, 
	float& cosAzimuthPre, float& sinAzimuthPre, float& cosElevationPre, float& sinElevationPre,
	float& cosAzimuthPost, float& sinAzimuthPost, float& cosElevationPost, float& sinElevationPost) throw()
{
	cosAzimuthPre = cos(azimuth);
	sinAzimuthPre = sin(azimuth);
	cosElevationPre = cos(elevation); 
	sinElevationPre = sin(elevation);
	cosAzimuthPost = cos(-azimuth);
	sinAzimuthPost = sin(-azimuth);
	cosElevationPost = cos(-elevation); 
	sinElevationPost = sin(-elevation);
}

void ZoomBUGenInternal::calculateZoom(const float zoom, 
	float& zoomWW, float& zoomXW, float& zoomWX, float& zoomYY, float& zoomZZ) throw()
{
	zoomWW = (float)sqrt2Minus1 * fabs(zoom) + 1.f;	
	
	zoomXW = zoom * 0.5f;
	zoomWX = zoom;
	zoomYY = sqrt(1.f - zoom*zoom);
	zoomZZ = zoomYY;
}

void ZoomBUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{	
	int numSamplesToProcess = uGenOutput.getBlockSize();
	const float* inputSamplesW = inputs[BFormat].processBlock(shouldDelete, blockID, W);
	const float* inputSamplesX = inputs[BFormat].processBlock(shouldDelete, blockID, X);
	const float* inputSamplesY = inputs[BFormat].processBlock(shouldDelete, blockID, Y);
	const float* inputSamplesZ = inputs[BFormat].processBlock(shouldDelete, blockID, Z);
	const float azimuth = *(inputs[Azimuth].processBlock(shouldDelete, blockID, 0));
	const float elevation = *(inputs[Elevation].processBlock(shouldDelete, blockID, 0));
	const float zoom = clip2(*inputs[Zoom].processBlock(shouldDelete, blockID, 0), 1.f);
	float* outputSamplesW = proxies[W]->getSampleData();
	float* outputSamplesX = proxies[X]->getSampleData();
	float* outputSamplesY = proxies[Y]->getSampleData();
	float* outputSamplesZ = proxies[Z]->getSampleData();
	
	float cosAzimuthPre		= this->cosAzimuthPre; 
	float sinAzimuthPre		= this->sinAzimuthPre; 
	float cosElevationPre		= this->cosElevationPre; 
	float sinElevationPre		= this->sinElevationPre;
	float cosAzimuthPost		= this->cosAzimuthPost; 
	float sinAzimuthPost		= this->sinAzimuthPost; 
	float cosElevationPost	= this->cosElevationPost; 
	float sinElevationPost	= this->sinElevationPost;
	float zoomWW			= this->zoomWW; 
	float zoomXW			= this->zoomXW; 
	float zoomWX			= this->zoomWX; 
	float zoomYY			= this->zoomYY; 
	float zoomZZ			= this->zoomZZ;
	
	if((azimuth == currentAzimuth) && (elevation == currentElevation) && (zoom == currentZoom))
	{
		while(numSamplesToProcess--)
		{
			float w = *inputSamplesW++;
			float x = *inputSamplesX++;
			float y = *inputSamplesY++;
			float z = *inputSamplesZ++;
			
			float dw = w;
			float dy = (x * sinAzimuthPre) + (y * cosAzimuthPre);
			float dx = (x * cosAzimuthPre) - (y * sinAzimuthPre);
			
			x = (dx * cosElevationPre) - (z * sinElevationPre);
			y = dy;
			z = (dx * sinElevationPre) + (z * cosElevationPre);
			
			dx = x;
			
			w = (zoomWW * dw) + (zoomXW * dx);
			x = dx + (zoomWX * dw);
			y *= zoomYY;	
			z *= zoomZZ;	
			
			dx = (x * cosElevationPost) - (z * sinElevationPost);
			z = (x * sinElevationPost) + (z * cosElevationPost);
			
			dy = y;
			y = (dx * sinAzimuthPost) + (dy * cosAzimuthPost);
			x = (dx * cosAzimuthPost) - (dy * sinAzimuthPost);
			
			*outputSamplesW++ = w;
			*outputSamplesX++ = x;
			*outputSamplesY++ = y;
			*outputSamplesZ++ = z;
		}
	}
	else 
	{
		const float slope = 1.f/(float)numSamplesToProcess;

		float newCosAzimuthPre, newSinAzimuthPre, newCosElevationPre, newSinElevationPre;
		float newCosAzimuthPost, newSinAzimuthPost, newCosElevationPost, newSinElevationPost;
		calculateSinCos(azimuth, elevation, 
						newCosAzimuthPre, newSinAzimuthPre, newCosElevationPre, newSinElevationPre,
						newCosAzimuthPost, newSinAzimuthPost, newCosElevationPost, newSinElevationPost);
		
		float slopeCosAzimuthPre = (newCosAzimuthPre - cosAzimuthPre) * slope;
		float slopeSinAzimuthPre = (newSinAzimuthPre - sinAzimuthPre) * slope;
		float slopeCosElevationPre = (newCosElevationPre - cosElevationPre) * slope;
		float slopeSinElevationPre = (newSinElevationPre - sinElevationPre) * slope;
		float slopeCosAzimuthPost = (newCosAzimuthPost - cosAzimuthPost) * slope;
		float slopeSinAzimuthPost = (newSinAzimuthPost - sinAzimuthPost) * slope;
		float slopeCosElevationPost = (newCosElevationPost - cosElevationPost) * slope;
		float slopeSinElevationPost = (newSinElevationPost - sinElevationPost) * slope;
		
		if(zoom == currentZoom)
		{
			while(numSamplesToProcess--)
			{
				cosAzimuthPre += slopeCosAzimuthPre;
				sinAzimuthPre	+= slopeSinAzimuthPre;
				cosElevationPre += slopeCosElevationPre;	
				sinElevationPre += slopeSinElevationPre;
				cosAzimuthPost += slopeCosAzimuthPost;
				sinAzimuthPost += slopeSinAzimuthPost;
				cosElevationPost += slopeCosElevationPost;
				sinElevationPost += slopeSinElevationPost;
				
				float w = *inputSamplesW++;
				float x = *inputSamplesX++;
				float y = *inputSamplesY++;
				float z = *inputSamplesZ++;
				
				float dw = w;
				float dy = (x * sinAzimuthPre) + (y * cosAzimuthPre);
				float dx = (x * cosAzimuthPre) - (y * sinAzimuthPre);
				
				x = (dx * cosElevationPre) - (z * sinElevationPre);
				y = dy;
				z = (dx * sinElevationPre) + (z * cosElevationPre);
				
				dx = x;
				
				w = (zoomWW * dw) + (zoomXW * dx);
				x = dx + (zoomWX * dw);
				y *= zoomYY;	
				z *= zoomZZ;	
				
				dx = (x * cosElevationPost) - (z * sinElevationPost);
				z = (x * sinElevationPost) + (z * cosElevationPost);
				
				dy = y;
				y = (dx * sinAzimuthPost) + (dy * cosAzimuthPost);
				x = (dx * cosAzimuthPost) - (dy * sinAzimuthPost);
				
				*outputSamplesW++ = w;
				*outputSamplesX++ = x;
				*outputSamplesY++ = y;
				*outputSamplesZ++ = z;
			}
		}
		else
		{						
			float newZoomWW, newZoomXW, newZoomWX, newZoomYY, newZoomZZ;
			calculateZoom(zoom, newZoomWW, newZoomXW, newZoomWX, newZoomYY, newZoomZZ);
			
			float slopeZoomWW = (newZoomWW - zoomWW) * slope;
			float slopeZoomXW = (newZoomXW - zoomXW) * slope;
			float slopeZoomWX = (newZoomWX - zoomWX) * slope;
			float slopeZoomYY = (newZoomYY - zoomYY) * slope;
			float slopeZoomZZ 
			= (newZoomZZ - zoomZZ) * slope;
			
			while(numSamplesToProcess--)
			{
				cosAzimuthPre += slopeCosAzimuthPre;
				sinAzimuthPre	+= slopeSinAzimuthPre;
				cosElevationPre += slopeCosElevationPre;	
				sinElevationPre += slopeSinElevationPre;
				cosAzimuthPost += slopeCosAzimuthPost;
				sinAzimuthPost += slopeSinAzimuthPost;
				cosElevationPost += slopeCosElevationPost;
				sinElevationPost += slopeSinElevationPost;
				zoomWW += slopeZoomWW;
				zoomXW += slopeZoomXW;
				zoomWX += slopeZoomWX;
				zoomYY += slopeZoomYY;
				zoomZZ += slopeZoomZZ;
				
				float w = *inputSamplesW++;
				float x = *inputSamplesX++;
				float y = *inputSamplesY++;
				float z = *inputSamplesZ++;
				
				float dw = w;
				float dy = (x * sinAzimuthPre) + (y * cosAzimuthPre);
				float dx = (x * cosAzimuthPre) - (y * sinAzimuthPre);
				
				x = (dx * cosElevationPre) - (z * sinElevationPre);
				y = dy;
				z = (dx * sinElevationPre) + (z * cosElevationPre);
				
				dx = x;
				
				w = (zoomWW * dw) + (zoomXW * dx);
				x = dx + (zoomWX * dw);
				y *= zoomYY;	
				z *= zoomZZ;	
				
				dx = (x * cosElevationPost) - (z * sinElevationPost);
				z = (x * sinElevationPost) + (z * cosElevationPost);
				
				dy = y;
				y = (dx * sinAzimuthPost) + (dy * cosAzimuthPost);
				x = (dx * cosAzimuthPost) - (dy * sinAzimuthPost);
				
				*outputSamplesW++ = w;
				*outputSamplesX++ = x;
				*outputSamplesY++ = y;
				*outputSamplesZ++ = z;
			}
			
			this->zoomWW = newZoomWW; 
			this->zoomXW = newZoomXW; 
			this->zoomWX = newZoomWX; 
			this->zoomYY = newZoomYY; 
			this->zoomZZ = newZoomZZ;
		}
		
		this->cosAzimuthPre = newCosAzimuthPre; 
		this->sinAzimuthPre = newSinAzimuthPre; 
		this->cosElevationPre = newCosElevationPre; 
		this->sinElevationPre = newSinElevationPre;
		this->cosAzimuthPost = newCosAzimuthPost; 
		this->sinAzimuthPost = newSinAzimuthPost; 
		this->cosElevationPost = newCosElevationPost; 
		this->sinElevationPost = newSinElevationPost;
		this->currentAzimuth = azimuth;
		this->currentElevation = elevation;
		this->currentZoom = zoom;		
	}
}

ZoomB::ZoomB(UGen const& bFormat, UGen const& azimuth, UGen const& elevation, UGen const& zoom) throw()
{
	initInternal(4);
	
	UGen bFormatChecked = bFormat;
	
	while(bFormatChecked.getNumChannels() < 4)
	{
		bFormatChecked = UGen(bFormatChecked, UGen::getNull());
	}
	
	ZoomBUGenInternal* internal = new ZoomBUGenInternal(bFormatChecked, 
								  azimuth.mix(), 
								  elevation.mix(), 
								  zoom.mix());
	generateFromProxyOwner(internal);
	
}

END_UGEN_NAMESPACE
