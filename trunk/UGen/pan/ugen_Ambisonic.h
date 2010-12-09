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
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_Ambisonic_H_
#define _UGEN_ugen_Ambisonic_H_

#include "../core/ugen_UGen.h"

class PanBUGenInternal : public ProxyOwnerUGenInternal
{
public:
	PanBUGenInternal(UGen const& input, UGen const& azimuth, UGen const& elevation, UGen const& distance) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	void calculate(const float azimuth, const float elevation, const float distance, 
				   float& w, float& x, float& y, float& z) throw();
	
	enum Inputs { Input, Azimuth, Elevation, Distance, NumInputs };
	enum Outputs { W, X, Y, Z };
	
protected:
	float distanceFactor;
	float wLevel;
	float xyzLevel;
	float centreSize;
	
	float w, x, y, z;
	float currentAzimuth, currentElevation, currentDistance;
};

//could have PanB2 with only w,x,y if elevation is 0

UGenSublcassDeclaration(PanB, (input, azimuth, elevation, distance),
					    (UGen const& input, UGen const& azimuth, UGen const& elevation = 0.f, UGen const& distance = 1.f), 
						COMMON_UGEN_DOCS);

class DecodeBUGenInternal : public UGenInternal
{
public:
	DecodeBUGenInternal(UGen const& bFormat, const float azimuth, const float elevation) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { BFormat, NumInputs };
	enum Outputs { W, X, Y, Z };

protected:
	float cosAzimuth, sinAzimuth, sinElevation;
};

UGenSublcassDeclaration(DecodeB, (bFormat, azimuth, elevation),
					    (UGen const& bFormat, FloatArray const& azimuth, FloatArray const& elevation = 0.f), 
						COMMON_UGEN_DOCS);


class ManipulateBUGenInternal : public ProxyOwnerUGenInternal
{
public:
	ManipulateBUGenInternal(UGen const& bFormat, UGen const& parameter) throw();
		
	enum Inputs { BFormat, Parameter, NumInputs };
	enum Outputs { W, X, Y, Z };

protected:
	float currentParam, sinParam, cosParam;
};

class RotateBUGenInternal : public ManipulateBUGenInternal
{
public:
	RotateBUGenInternal(UGen const& bFormat, UGen const& rotate) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class TiltBUGenInternal : public ManipulateBUGenInternal
{
public:
	TiltBUGenInternal(UGen const& bFormat, UGen const& tilt) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

class TumbleBUGenInternal : public ManipulateBUGenInternal
{
public:
	TumbleBUGenInternal(UGen const& bFormat, UGen const& tumble) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

UGenSublcassDeclaration(RotateB, (bFormat, rotate),
					    (UGen const& bFormat, UGen const& rotate), 
						COMMON_UGEN_DOCS);
UGenSublcassDeclaration(TiltB, (bFormat, tilt),
					    (UGen const& bFormat, UGen const& tilt), 
						COMMON_UGEN_DOCS);
UGenSublcassDeclaration(TumbleB, (bFormat, tumble),
					    (UGen const& bFormat, UGen const& tumble), 
						COMMON_UGEN_DOCS);

class ZoomBUGenInternal : public ProxyOwnerUGenInternal
{
public:
	ZoomBUGenInternal(UGen const& bFormat, UGen const& azimuth, UGen const& elevation, UGen const& zoom) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	void calculateSinCos(const float azimuth, const float elevation, 
						 float& cosAzimuthPre, float& sinAzimuthPre, float& cosElevationPre, float& sinElevationPre,
						 float& cosAzimuthPost, float& sinAzimuthPost, float& cosElevationPost, float& sinElevationPost) throw();
	
	void calculateZoom(const float zoom, float& zoomWW, float& zoomXW, float& zoomWX, float& zoomYY, float& zoomZZ) throw();
	
	enum Inputs { BFormat, Azimuth, Elevation, Zoom, NumInputs };
	enum Outputs { W, X, Y, Z };
	
protected:
	float currentAzimuth, currentElevation, currentZoom;
	float cosAzimuthPre, sinAzimuthPre, cosElevationPre, sinElevationPre;
	float cosAzimuthPost, sinAzimuthPost, cosElevationPost, sinElevationPost;
	float zoomWW, zoomXW, zoomWX, zoomYY, zoomZZ;
};

UGenSublcassDeclaration(ZoomB, (bFormat, azimuth, elevation, zoom),
					    (UGen const& bFormat, UGen const& azimuth, UGen const& elevation, UGen const& zoom), 
						COMMON_UGEN_DOCS);


#endif // _UGEN_ugen_Ambisonic_H_
