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
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_BEQ_H_
#define _UGEN_ugen_BEQ_H_


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"

#define BEQ_COEFF_TYPE float
#define BEQ_CALC_TYPE float

/**
 Base class for the internal BEQ classes.
 
 Inherit from this using the control input for whatever use is appropriate for the base class (e.g., 
 reciprocal of Q, S, or bandwidth.
 @ingroup UGenInternals */
class BEQBaseUGenInternal : public UGenInternal
{
public:
	BEQBaseUGenInternal(UGen const& input, UGen const& freq, UGen const& control, UGen const& gain) throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Freq, Control, Gain, NumInputs }; // subclass should define their own interpretation of "Control" whether rQ, rS or bw
	
	virtual void calculateCoeffs(const float freq, const float control, const float gain) = 0;
		
	void initValue(const float value) throw();
	
protected:
	BEQ_COEFF_TYPE y1, y2, a0, a1, a2, b1, b2;
	float currentFreq, currentControl, currentGain;	
};

/**
 Low pass filter internal. @ingroup UGenInternals
 */
class BLowPassUGenInternal : public BEQBaseUGenInternal
{
public:
	BLowPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw();
	UGenInternal* getChannel(const int channel) throw();
	
	enum ControlInput { ReciprocalQ = Control };
	
	void calculateCoeffs(const float freq, const float control, const float gain);
};

#define BHiAndLowPass_Docs	@param input	The input source to filter.								\
							@param freq		The cut-off frequency.									\
							@param rq		The reciprocal of Q (e.g., for a Q of 10 use 0.1)

/**
 Low pass filter.
 
 12dB/oct rolloff - 2nd order resonant Low Pass Filter.
 
 @ingroup AllUGens FilterUGens
 @see LPF, RLPF, SOS, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BLowPass, (input, freq, rq), 
									  (input, freq, rq, MulAdd_ArgsCall), 
									  (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f),
									  (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BHiAndLowPass_Docs MulAddArgs_Docs);

/**
 Low pass filter.
 
 24dB/oct rolloff - 4th order resonant Low Pass Filter.
 
 @ingroup AllUGens FilterUGens
 @see LPF, RLPF, SOS, BLowPass, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BLowPass4, (input, freq, rq), 
									   (input, freq, rq, MulAdd_ArgsCall), 
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f),
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BHiAndLowPass_Docs MulAddArgs_Docs);

/**
 High pass filter internal. @ingroup UGenInternals
 */
class BHiPassUGenInternal : public BEQBaseUGenInternal
{
public:
	BHiPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw();
	UGenInternal* getChannel(const int channel) throw();
	
	enum ControlInput { ReciprocalQ = Control };
	
	void calculateCoeffs(const float freq, const float control, const float gain);
};

/**
 High pass filter.
 
 12dB/oct rolloff - 2nd order resonant Low Pass Filter.
 
 @ingroup AllUGens FilterUGens
 @see HPF, RHPF, SOS, BLowPass, BLowPass4, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BHiPass, (input, freq, rq), 
									 (input, freq, rq, MulAdd_ArgsCall), 
									 (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f),
									 (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BHiAndLowPass_Docs MulAddArgs_Docs);

/**
 High pass filter.
 
 24dB/oct rolloff - 4th order resonant Low Pass Filter.
 
 @ingroup AllUGens FilterUGens
 @see HPF, RHPF, SOS, BLowPass, BLowPass4, BHiPass, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BHiPass4, (input, freq, rq), 
									  (input, freq, rq, MulAdd_ArgsCall), 
									  (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f),
									  (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BHiAndLowPass_Docs MulAddArgs_Docs);

/**
 Band pass filter internal. @ingroup UGenInternals
 */
class BBandPassUGenInternal : public BEQBaseUGenInternal
{
public:
	BBandPassUGenInternal(UGen const& input, UGen const& freq, UGen const& bw) throw();
	UGenInternal* getChannel(const int channel) throw();
	
	enum ControlInput { BW = Control };
	
	void calculateCoeffs(const float freq, const float control, const float gain);
};

#define BBandPassAndStop_Docs	@param input	The input source to filter.								\
								@param freq		The centre frequency.									\
								@param bw		The bandwidth in octaves between -3dB frequencies.


/**
 Band pass filter.

 @ingroup AllUGens FilterUGens
 @see SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandStop, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BBandPass, (input, freq, bw), 
									   (input, freq, bw, MulAdd_ArgsCall), 
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& bw = 1.f),
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& bw = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BBandPassAndStop_Docs MulAddArgs_Docs);

/**
 Band stop filter internal. @ingroup UGenInternals
 */
class BBandStopUGenInternal : public BEQBaseUGenInternal
{
public:
	BBandStopUGenInternal(UGen const& input, UGen const& freq, UGen const& bw) throw();
	UGenInternal* getChannel(const int channel) throw();
	
	enum ControlInput { BW = Control };
	
	void calculateCoeffs(const float freq, const float control, const float gain);
};

/**
 Band stop filter.
 
 @ingroup AllUGens FilterUGens
 @see SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BHiShelf, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BBandStop, (input, freq, bw), 
									   (input, freq, bw, MulAdd_ArgsCall), 
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& bw = 1.f),
									   (UGen const& input, UGen const& freq = 1200.f, UGen const& bw = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BBandPassAndStop_Docs MulAddArgs_Docs);

/**
 Peaking filter internal. @ingroup UGenInternals
 */
class BPeakEQUGenInternal : public BEQBaseUGenInternal
{
public:
	BPeakEQUGenInternal(UGen const& input, UGen const& freq, UGen const& rq, UGen const& gain) throw();
	UGenInternal* getChannel(const int channel) throw();
	
	enum ControlInput { ReciprocalQ = Control };
	
	void calculateCoeffs(const float freq, const float control, const float gain);
};

#define BPeapEQ_Docs	@param input	The input source to filter.								\
						@param freq		The centre frequency.									\
						@param rq		The reciprocal of Q (e.g., for a Q of 10 use 0.1).		\
						@param gain		Boost/cut the center frequency (in dBs).

/**
 Peaking filter, parametric EQ.
 
 @ingroup AllUGens FilterUGens
 @see SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf
*/
DirectMulAddUGenDeclaration(BPeakEQ, (input, freq, rq, gain), 
									 (input, freq, rq, gain, MulAdd_ArgsCall), 
									 (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, UGen const& gain = 0.f),
									 (UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, UGen const& gain = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BPeapEQ_Docs MulAddArgs_Docs);

/**
 Low shelving filter internal. @ingroup UGenInternals
 */
class BLowShelfUGenInternal : public BEQBaseUGenInternal
	{
	public:
		BLowShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw();
		UGenInternal* getChannel(const int channel) throw();
		
		enum ControlInput { ReciprocalS = Control };
		
		void calculateCoeffs(const float freq, const float control, const float gain);
	};

#define BShelf_Docs		@param input	The input source to filter.								\
						@param freq		The centre frequency.									\
						@param rs		The reciprocal of S.  Shell boost/cut slope.			\
										When S = 1, the shelf slope is as steep as it can be	\
										and remain monotonically increasing or decreasing gain	\
										with frequency.											\
						@param gain		Boost/cut beyond the center frequency (in dBs).

/**
 Low shelving filter.
 
 @ingroup AllUGens FilterUGens
 @see SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BLowShelf, 
							(input, freq, rs, gain), 
							(input, freq, rs, gain, MulAdd_ArgsCall), 
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rs = 1.f, UGen const& gain = 0.f),
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rs = 1.f, UGen const& gain = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BShelf_Docs MulAddArgs_Docs);

/**
 High shelving filter internal. @ingroup UGenInternals
 */
class BHiShelfUGenInternal : public BEQBaseUGenInternal
	{
	public:
		BHiShelfUGenInternal(UGen const& input, UGen const& freq, UGen const& rs, UGen const& gain) throw();
		UGenInternal* getChannel(const int channel) throw();
		
		enum ControlInput { ReciprocalS = Control };
		
		void calculateCoeffs(const float freq, const float control, const float gain);
	};

/**
 High shelving filter.
 
 @ingroup AllUGens FilterUGens
 @see SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BLowShelf, BPeakEQ
 */
DirectMulAddUGenDeclaration(BHiShelf, 
							(input, freq, rs, gain), 
							(input, freq, rs, gain, MulAdd_ArgsCall), 
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rs = 1.f, UGen const& gain = 0.f),
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rs = 1.f, UGen const& gain = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BShelf_Docs MulAddArgs_Docs);

/**
 All pass filter internal. @ingroup UGenInternals
 */
class BAllPassUGenInternal : public BEQBaseUGenInternal
	{
	public:
		BAllPassUGenInternal(UGen const& input, UGen const& freq, UGen const& rq) throw();
		UGenInternal* getChannel(const int channel) throw();
		
		enum ControlInput { ReciprocalQ = Control };
		
		void calculateCoeffs(const float freq, const float control, const float gain);
	};

#define BAllPass_Docs	@param input	The input source to filter.								\
						@param freq		The centre frequency.									\
						@param rq		The reciprocal of Q (e.g., for a Q of 10 use 0.1).


/**
 All pass filter.
 
 @ingroup AllUGens FilterUGens
 @see AllpassL, AllpassN, SOS, BLowPass, BLowPass4, BHiPass, BHiPass4, BBandPass, BBandStop, BHiShelf, BLowShelf, BPeakEQ
  */
DirectMulAddUGenDeclaration(BAllPass, 
							(input, freq, rq), 
							(input, freq, rq, MulAdd_ArgsCall), 
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f),
							(UGen const& input, UGen const& freq = 1200.f, UGen const& rq = 1.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS BAllPass_Docs MulAddArgs_Docs);

/** A macro as a shorthand for BLowPass. 
 Resonant low pass filter. */ 
#define RLPF BLowPass

/** A macro as a shorthand for BHiPass.
 Resonant high pass filter. */ 
#define RHPF BHiPass



#endif // _UGEN_ugen_BEQ_H_
