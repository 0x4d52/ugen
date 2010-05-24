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
 
 This Convolution code is thanks to Alex Harker.
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_Convolution_H_
#define _UGEN_ugen_Convolution_H_


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MixUGen.h"
#include "../fft/ugen_FFTEngineInternal.h"

//#define BUFFERSIZES_DEFAULT (44100 * 30)

/** Stores a "partitioned" FFT buffer. */
class PartBuffer : public Buffer
{
public:
	/** Construct a partitioned buffer from a real (audio) buffer 
	 @param original	The audio buffer to FTT and partition.
	 @param startPoint	The sample number at which to start reading within the original.
	 @param endPoint	The sample number at which to stop reading within the origina.
						If this is 0 all the remain samples in the original will be read.
	 @param fftSizeLog2	This will control the FFT size of the FFT operations on the partitioning.
						The FFT size will be @f$ 2^{fftSizeLog2} @f$ if fftSizeLog2 is less than 4 then an
						FFT size of @f$ 2^{12} = 4096 @f$ is used. */
	PartBuffer(Buffer const& original, long startPoint = 0, long endPoint = 0, long fftSizeLog2 = 0) throw();
			   //, long MaxFFTSizelog2 = 0) throw();
	PartBuffer(BufferChannelInternal *internalToUse, int Partitions, long StartPoint, long EndPoint, long FFTSizelog2) throw();
	
	/** Destructor. */
	~PartBuffer();
	
	PartBuffer getChannel(const int channel) const throw();
	
	//PartBuffer(PartBuffer const& copy) throw();
	//PartBuffer& operator= (PartBuffer const& other) throw();
	
	/** Get a pointer to the real samples.
	 @param channel The channel whose real samples will be accessed. */
	inline float* getDataReal(const int channel) throw()	{ return channels[channel % numChannels_]->data; }
	
	/** Get a pointer to the imaginary samples.
	 @param channel The channel whose imaginary samples will be accessed. */
	inline float* getDataImag(const int channel) throw()	{ return channels[channel % numChannels_]->data + bufferSize; }
	
	/** Get the number of partitioned that were generated.
	 @return The number of partitions. */
	inline int getNumPartitions() const { return numPartitions; }
	
	friend class PartConvolveUGenInternal;
	
private:	
	
	static inline long fftSizeLog2FromArg(long fftSizeLog2) throw() { return fftSizeLog2 < 4 ? 12 : fftSizeLog2;	}
	static inline long fftSizeFromArg(long fftSizeLog2) throw()		{ return 1 << fftSizeLog2FromArg(fftSizeLog2);	}
	
	long bufferSize;
	int numPartitions;
	long startPoint, endPoint, length;
	int fftSizeLog2, fftSize;
	int fftSizeOver2, fftSizeOver4;
	
	FFTEngineInternal *fftEngine;	
	Buffer partTempBuffer;
	
	void partitionImpulse(Buffer const& original);
	void partitionImpulseChannel(Buffer const& original, const int channel);
};

/** A UGenInternal that performs real time partioned convolution.
 @ingroup UGenInternals
 @see PartConvolve, ZeroLatencyConvolve */
class PartConvolveUGenInternal : public UGenInternal
{
public:
	PartConvolveUGenInternal(UGen const& input, 
							 PartBuffer const& partImpulse) throw(); 
	~PartConvolveUGenInternal() throw();
	UGenInternal* getChannel(const int channel) throw();									// necessary if there are input ugens which may have more than one channel
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
		
	static inline vFloat vecSplat (float splatval) throw()
	{
		vFloat ReturnVec;
		float ReturnFloats[4] UGEN_ALIGN = {splatval, splatval, splatval, splatval};
		memcpy(&ReturnVec, ReturnFloats, 4 * sizeof(float));
		return ReturnVec;
	}	
	
	
protected:
	PartBuffer partImpulse_;
	
	int bufPosition;
	long startPoint, endPoint, length;
	
	// Scheduling Stuff
	int partitionsDone, scheduleCounter, lastPart, validPart;
	
	// FFT Stuff
	FFTEngineInternal *fftEngine;
	vFloat *fftBuffers[4];
	int tillNextFFT, rwPointer1, rwPointer2, fftOffset, fftSize, fftSizeLog2;
	int fftSizeOver4;
	
	long bufferSize;
	int resetAll;
	
	// FFTSize
	double scaleMultD;		
	vFloat scaleMult UGEN_ALIGN;
	
	// Buffers
	Buffer inputBuffer;
	float * const inputBufferSamples;
	Buffer fftBuffersMemory;
	float * const fftBuffersMemorySamples;
	Buffer fftTempBuffer;
	float * const fftTempBufferSamples;
};




		
/** Real time partitioned convolution UGen. 
 @ingroup FFTUGens */
UGenSublcassDeclaration(PartConvolve, (input, impulse, startPoint, endPoint, fftSizeLog2),
						(UGen const& input, Buffer const& impulse, 
						 long startPoint = 0, 
						 long endPoint = 0, 
						 long fftSizeLog2 = 0), COMMON_UGEN_DOCS);


#if defined(UGEN_FFTW) || defined(UGEN_FFTREAL)
DefineCustomUGen(ZeroLatencyConvolve,
				 (Mix(UGenArray(PartConvolve(input, impulse, 8192, 0, 14)) <<
					 UGenArray(PartConvolve(input, impulse, 2048, 8192, 12)) <<
					 UGenArray(PartConvolve(input, impulse, 512, 2048, 10)) <<
					 UGenArray(PartConvolve(input, impulse, 128, 512, 8)) <<
					 UGenArray(PartConvolve(input, impulse, 0, 128, 6)))),			// No TimeConvolve yet on Windows
				 (UGen const& input, Buffer const& impulse), COMMON_UGEN_DOCS);

#else // assume we have the Mac vDSP interfaces

/** A UGenInternal which performs time domain convolution.
 @ingroup UGenInternals */
class TimeConvolveUGenInternal : public UGenInternal
{
public:
	TimeConvolveUGenInternal(UGen const& input, 
							 Buffer const& impulse, 
							 long startPoint = 0, long endPoint = 0, long fftSizeLog2 = 0) throw();
	~TimeConvolveUGenInternal() throw();
	UGenInternal* getChannel(const int channel) throw();									// necessary if there are input ugens which may have more than one channel
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();	
	
	enum Inputs { Input, NumInputs };
	
protected:
	Buffer filters;
	Buffer ioBuffers;
	
	long position;
	
	enum io
	{
		InputBuffer = 0,
		OutputBuffer = 1
	};
};

/** Real time time domain convolution. 
 @ingroup FFTUGens */
UGenSublcassDeclaration(TimeConvolve, (input, impulse, startPoint, endPoint, fftSizelog2),
						(UGen const& input, Buffer const& impulse, 
						 long startPoint = 0, 
						 long endPoint = 0, 
						 long fftSizelog2 = 0), COMMON_UGEN_DOCS);

/** Real time zero latency convolution. 
 @ingroup FFTUGens */
DefineCustomUGen(ZeroLatencyConvolve,
				 (Mix(UGenArray(PartConvolve(input, impulse, 8192, 0, 14)) <<
					 UGenArray(PartConvolve(input, impulse, 2048, 8192, 12)) <<
					 UGenArray(PartConvolve(input, impulse, 512, 2048, 10)) <<
					 UGenArray(PartConvolve(input, impulse, 128, 512, 8)) <<
					 UGenArray(TimeConvolve(input, impulse, 0, 128)))),
				 (UGen const& input, Buffer const& impulse),
				 COMMON_UGEN_DOCS);

#endif // assumed we have the Mac vDSP interfaces

/** True stereo, real time, zero latency convolution ! 
 @ingroup FFTUGens */
DefineCustomUGen(TrueStereoConvolve,
				 (ZeroLatencyConvolve(input, impulseLeft)+ZeroLatencyConvolve(input, impulseRight)),
				 (UGen const& input, Buffer const& impulseLeft, Buffer const& impulseRight),
				 COMMON_UGEN_DOCS);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define PartConvolve PartConvolve()
#define TimeConvolve TimeConvolve()
#define ZeroLatencyConvolve ZeroLatencyConvolve()
#define TrueStereoConvolve TrueStereoConvolve()
#endif


#endif // _UGEN_ugen_Convolution_H_
