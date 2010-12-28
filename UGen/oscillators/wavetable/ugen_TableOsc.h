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

#ifndef _UGEN_ugen_TableOsc_H_
#define _UGEN_ugen_TableOsc_H_

#include "../../core/ugen_UGen.h"
#include "../../basics/ugen_MulAdd.h"

/** @ingroup UGenInternals */
class TableOscUGenInternal :	public UGenInternal
{
public:
	TableOscUGenInternal(UGen const& freq, const float initialPhase, Buffer const& table) throw();
	UGenInternal* getChannel(const int channel) throw();									
	UGenInternal* getKr() throw();															
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	double getDuration() const throw();
	double getPosition() const throw();
	bool setPosition(const double newPosition) throw();	
	
	enum Inputs { Freq, NumInputs };

protected:	
//	inline float lookupIndex(const float fIndex) throw()
//	{
//		const float* const wavetable = this->wavetable;
//		const int iIndex0 = (int)fIndex;
//		const float frac = fIndex - iIndex0;
//		const float* pValue0 = wavetable + iIndex0;
//		const float value0 = *pValue0;
//		const float value1 = *(pValue0+1);
//		return value0 + frac * (value1 - value0);
//	}
	
//	inline float lookupIndex(float fIndex) throw()
//	{
//		float* wavetable = this->wavetable;
//		int iIndex0 = (int)fIndex;
//		float frac = fIndex - iIndex0;
//		float* pValue0 = wavetable + iIndex0;
//		float value0 = *pValue0;
//		float value1 = *(pValue0+1);
//		return value0 + frac * (value1 - value0);
//	}
	
	inline 
	float lookupIndex(const float fIndex) throw()
	{
		const int iIndex0 = (int)fIndex;
		const float* pValue0 = wavetable + iIndex0;
		const float value0 = *pValue0;
		return value0 + (fIndex - iIndex0) * (*(pValue0+1) - value0);
	}
	
	Buffer table_;
	const float wavetableSize;
	float *wavetable;
	float currentPhase;
};

/** @ingroup UGenInternals */
class TableOscUGenInternalK : public TableOscUGenInternal 
{ 
public: 
	TableOscUGenInternalK (UGen const& freq, const float initialPhase, Buffer const& table) throw();	
	UGenInternal* getKr() throw() { incrementRefCount(); return this; } 
	float getValue(const int /*channel*/) const throw() { return value; } 
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw(); 
private: 
	float value; 
};


#define Osc_Docs	@param	freq			The fundamental frequency of the wave.							\
					@param	initialPhase	The phase (0-1) at which the wave should start on creation.		\
											This may be specified with multiple values using a single		\
											channel Buffer as an array.

#define TableOsc_Docs		@param	table				The wavetable to use. This should be a single channel Buffer			\
														containing a single cycle of the desired waveform. The size of			\
														Buffer is not important (athough larger sizes will generallly			\
														result in higher quality) but the Buffer should have an					\
														interpolation sample after the last value. For example, an 8192			\
														point Buffer (as used with SinOsc) actually has 8193 values but			\
														this is shrunk (using Buffer::shrinkSize()) to a size of 8192 and		\
														the value at sample [8192] and sample [0] should be the same.

#define HarminicOsc_Docs	@param	harmonicWeights		This creates an internal wavetable using a series of sine wave			\
														harmonics. The amplitude of these sines is govened by this single		\
														channel Buffer. e.g.,													\
														@code Buffer(1.0, 0.5, 0.333, 0.25, 0.2, 0.167, 0.143, 0.125) @endcode	\
														.. would make a pseudo-sawtooth wave.

/**
 A general purpose wavetable-based oscillator.
 @ingroup AllUGens OscUGens
 @see HarmonicOsc, SinOsc
 */
DirectMulAddUGenDeclaration(TableOsc, 
							(table, freq, initialPhase), 
							(table, freq, initialPhase, MulAdd_ArgsCall), 
							(Buffer const& table, UGen const& freq = 440.f, Buffer const& initialPhase = 0.f), 
							(Buffer const& table, UGen const& freq = 440.f, Buffer const& initialPhase = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS TableOsc_Docs Osc_Docs MulAddArgs_Docs);


/**
 A wavetable-based oscillator initialised with an array of the harmonic amplitudes.
 @ingroup AllUGens OscUGens
 @see TableOsc, SinOsc
 */
DirectMulAddUGenDeclaration(HarmonicOsc, 
							(harmonicWeights, freq, initialPhase), 
							(harmonicWeights, freq, initialPhase, MulAdd_ArgsCall), 
							(Buffer const& harmonicWeights, UGen const& freq = 440.f, Buffer const& initialPhase = 0.f), 
							(Buffer const& harmonicWeights, UGen const& freq = 440.f, Buffer const& initialPhase = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS HarminicOsc_Docs Osc_Docs MulAddArgs_Docs);


/**
 A wavetable-based oscillator using an internal 8192-point sine table.
 @ingroup AllUGens OscUGens
 @see FSinOsc, TableOsc, HarmonicOsc
 */
DirectMulAddUGenDeclaration(SinOsc, 
							(freq, initialPhase), 
							(freq, initialPhase, MulAdd_ArgsCall), 
							(UGen const& freq = 440.f, Buffer const& initialPhase = 0.f), 
							(UGen const& freq = 440.f, Buffer const& initialPhase = 0.f, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS Osc_Docs MulAddArgs_Docs);



#endif // _UGEN_ugen_TableOsc_H_
