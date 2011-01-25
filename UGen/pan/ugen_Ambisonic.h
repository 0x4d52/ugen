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

#ifndef _UGEN_ugen_Ambisonic_H_
#define _UGEN_ugen_Ambisonic_H_

#include "../core/ugen_UGen.h"

/** @ingroup UGenInternals */
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


#define PanB_Doc	@param input		The mono input source. DOC_SINGLE								\
					@param azimuth		The angle in the horizontal plane in radians.					\
										0 is front, +ve is clockwise viewed from above.					\
										DOC_SINGLE														\
					@param elevation	The angle in the vertical plane in radians. 0 is ear-level,		\
										@f$\frac{\pi}{2}@f$ is directly above the head and				\
										@f$-\frac{\pi}{2}@f$ is directly below the head.				\
										DOC_SINGLE														\
					@param distance		Arbitrary scale distance parameter. 1 could be considered the	\
										distance from the centre to the loudspeaker on playback.		\
										Values less than 1 are closer, larger than 1 further away.		\
										DOC_SINGLE
	
/** Ambisonic panner.
 Pans a mono sound source into B format (W, X, Y, Z). 
 This always outputs the four channels in this order.
 @ingroup AllUGens ControlUGens
 @see Pan2, LinPan2, DecodeB, RotateB, TiltB, TumbleB, ZoomB */
UGenSublcassDeclaration(PanB, (input, azimuth, elevation, distance),
					    (UGen const& input, UGen const& azimuth, UGen const& elevation = 0.f, UGen const& distance = 1.f), 
						COMMON_UGEN_DOCS PanB_Doc);

/** @ingroup UGenInternals */
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

#define BFormat_Doc			The B-Format signal input source. This should be the four channels W, X, Y and Z. \
							Fewer than four channels will have silent channels added to make up four channels. \
							Where there are more than four channels only the the first four will be used.

#define DecodeB_Doc	@param bFormat		BFormat_Doc														\
					@param azimuths		The angle in the horizontal plane in radians.					\
										0 is front, +ve is clockwise viewed from above.					\
					@param elevations	The angle in the vertical plane in radians. 0 is ear-level,		\
										@f$\frac{\pi}{2}@f$ is directly above the head and				\
										@f$-\frac{\pi}{2}@f$ is directly below the head.

/** Ambisonic decoder.
 Decodes a B-Format (W, X, Y, Z) signal to one or more loudspeakers. FloatArray arrays
 can be passed for azimuth and elevation and where these two arrays are of a different
 length the smaller array will be wrapped (and the number of generated loudspeaker channels
 will be equal to the length of larger array). E.g.,
 @code
	UGen bf = PanB::AR(input, 0); // pan sound straight ahead
 
	// decode to ±30 degrees for stereo playback
	UGen stereo = DecodeB::AR(bf, FloatArray(deg2rad(-30), deg2rad(30)), 0); // 0 elevation used for both L+R
 @endcode
 @ingroup AllUGens ControlUGens
 @see PanB, RotateB, TiltB, TumbleB, ZoomB */
UGenSublcassDeclaration(DecodeB, (bFormat, azimuths, elevations),
					    (UGen const& bFormat, FloatArray const& azimuths, FloatArray const& elevations = 0.f), 
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

/** @ingroup UGenInternals */
class RotateBUGenInternal : public ManipulateBUGenInternal
{
public:
	RotateBUGenInternal(UGen const& bFormat, UGen const& rotate) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
class TiltBUGenInternal : public ManipulateBUGenInternal
{
public:
	TiltBUGenInternal(UGen const& bFormat, UGen const& tilt) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
class TumbleBUGenInternal : public ManipulateBUGenInternal
{
public:
	TumbleBUGenInternal(UGen const& bFormat, UGen const& tumble) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};


#define RotateB_Doc	@param bFormat		BFormat_Doc										\
					@param rotate		The rotation angle (on the horizontal plane) in radians. DOC_SINGLE
#define TiltB_Doc	@param bFormat		BFormat_Doc										\
					@param tilt			The tilt angle (leaning left/right when facing forwards) in radians. DOC_SINGLE
#define TumbleB_Doc	@param bFormat		BFormat_Doc										\
					@param tumble		The tumble (rolling forwards/backwards) angle in radians. DOC_SINGLE

/** Rotate an ambisonic soundfield.
 Rotates a B format signal (W, X, Y, Z). 
 This always outputs the four channels in this order.
 @ingroup AllUGens ControlUGens
 @see PanB, DecodeB, TiltB, TumbleB, ZoomB */
UGenSublcassDeclaration(RotateB, (bFormat, rotate),
					    (UGen const& bFormat, UGen const& rotate), 
						COMMON_UGEN_DOCS RotateB_Doc);

/** Tilt an ambisonic soundfield.
 Tilts a B format signal (W, X, Y, Z). 
 This always outputs the four channels in this order.
 @ingroup AllUGens ControlUGens
 @see PanB, DecodeB, RotateB, TumbleB, ZoomB */
UGenSublcassDeclaration(TiltB, (bFormat, tilt),
					    (UGen const& bFormat, UGen const& tilt), 
						COMMON_UGEN_DOCS TiltB_Doc);

/** Tumble an ambisonic soundfield.
 Tumbles a B format signal (W, X, Y, Z). 
 This always outputs the four channels in this order.
 @ingroup AllUGens ControlUGens
 @see PanB, DecodeB, TiltB, RotateB, ZoomB */
UGenSublcassDeclaration(TumbleB, (bFormat, tumble),
					    (UGen const& bFormat, UGen const& tumble), 
						COMMON_UGEN_DOCS TumbleB_Doc);

/** @ingroup UGenInternals */
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

#define ZoomB_Doc	@param bFormat		BFormat_Doc														\
					@param azimuth		The angle in the horizontal plane in radians.					\
										0 is front, +ve is clockwise viewed from above.					\
										DOC_SINGLE														\
					@param elevation	The angle in the vertical plane in radians. 0 is ear-level,		\
										@f$\frac{\pi}{2}@f$ is directly above the head and				\
										@f$-\frac{\pi}{2}@f$ is directly below the head.				\
										DOC_SINGLE														\
					@param zoom			Zoom factor. DOC_SINGLE

/** Zoom into a particular point in an ambisonic soundfield.
 Zooms into a partiuclar point at a given azimuth and elevation.
 This always outputs the four B format (W, X, Y, Z) channels in this order.
 @ingroup AllUGens ControlUGens
 @see PanB, DecodeB, TiltB, RotateB, TumbleB */
UGenSublcassDeclaration(ZoomB, (bFormat, azimuth, elevation, zoom),
					    (UGen const& bFormat, UGen const& azimuth, UGen const& elevation, UGen const& zoom), 
						COMMON_UGEN_DOCS);


#endif // _UGEN_ugen_Ambisonic_H_
