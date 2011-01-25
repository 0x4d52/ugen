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

#ifndef _UGEN_ugen_FFTEngineInternal_H_
#define _UGEN_ugen_FFTEngineInternal_H_

#include "../buffers/ugen_Buffer.h"
#include "../core/ugen_SmartPointer.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_BinaryOpUGens.h"
#include "../core/ugen_Bits.h"


//#warning REMOVE THESE DEFINES AFTER TESTING!!!!!
//#define UGEN_FFTREAL 1	// best for other platforms
//#define UGEN_FFTW 1		// best for windows
							// otherwise use vDSP on the Mac (fastest of all)

#if (defined (_WIN32) || defined (_WIN64) || defined(UGEN_IPHONE) || defined(UGEN_ANDROID)) // make this so it is "not mac" rather than "is windows"...
	#ifndef UGEN_NEON
		typedef struct _vFloat {
			float f[4];
		} vFloat;
	#endif

	#ifndef __VDSP__
		typedef struct _DSPSplitComplex {
			float *             realp;
			float *             imagp;
		} DSPSplitComplex;
	#endif

	#if defined(UGEN_VFP)
		#include "../iphone/armasm/ugen_vfp_FFTUtilities.h"
	#elif defined(UGEN_NEON)
		#include "../iphone/armasm/ugen_neon_Utilities.h"
		#define VEC_MUL_OP vmulq_f32
		#define VEC_ADD_OP vaddq_f32
		#define VEC_SUB_OP vsubq_f32
//	#elif defined(UGEN_VDSP) // these are slow...
//		inline vFloat VEC_MUL_OP(vFloat const& leftVec, vFloat const& rightVec)
//		{
//			vFloat returnVec;
//			vDSP_vmul((float*)&leftVec, 1, (float*)&rightVec, 1, (float*)&returnVec, 1, 4);
//			return returnVec;
//		}
//
//		inline vFloat VEC_ADD_OP(vFloat const& leftVec, vFloat const& rightVec)
//		{
//			vFloat returnVec;
//			vDSP_vadd((float*)&leftVec, 1, (float*)&rightVec, 1, (float*)&returnVec, 1, 4);			
//			return returnVec;
//		}
//
//		inline vFloat VEC_SUB_OP(vFloat const& leftVec, vFloat const& rightVec)
//		{
//			vFloat returnVec;
//			vDSP_vsub((float*)&leftVec, 1, (float*)&rightVec, 1, (float*)&returnVec, 1, 4);
//			return returnVec;
//		}
	#else // other platform windows or non vfp/neon iphone for example....
		inline vFloat VEC_MUL_OP(vFloat const& leftVec, vFloat const& rightVec)
		{
			vFloat returnVec;
			float *returnFloat = (float *) &returnVec;
			float *leftFloat = (float *) &leftVec;
			float *rightFloat = (float *) &rightVec;
			
			returnFloat[0] = leftFloat[0] * rightFloat[0];
			returnFloat[1] = leftFloat[1] * rightFloat[1];
			returnFloat[2] = leftFloat[2] * rightFloat[2];
			returnFloat[3] = leftFloat[3] * rightFloat[3];
			
			return returnVec;
		}

		inline vFloat VEC_ADD_OP(vFloat const& leftVec, vFloat const& rightVec)
		{
			vFloat returnVec;
			float *returnFloat = (float *) &returnVec;
			float *leftFloat = (float *) &leftVec;
			float *rightFloat = (float *) &rightVec;
			
			returnFloat[0] = leftFloat[0] + rightFloat[0];
			returnFloat[1] = leftFloat[1] + rightFloat[1];
			returnFloat[2] = leftFloat[2] + rightFloat[2];
			returnFloat[3] = leftFloat[3] + rightFloat[3];
			
			return returnVec;
		}

		inline vFloat VEC_SUB_OP(vFloat const& leftVec, vFloat const& rightVec)
		{
			vFloat returnVec;
			float *returnFloat = (float *) &returnVec;
			float *leftFloat = (float *) &leftVec;
			float *rightFloat = (float *) &rightVec;
			
			returnFloat[0] = leftFloat[0] - rightFloat[0];
			returnFloat[1] = leftFloat[1] - rightFloat[1];
			returnFloat[2] = leftFloat[2] - rightFloat[2];
			returnFloat[3] = leftFloat[3] - rightFloat[3];
			
			return returnVec;
		}
	#endif

	#define ZEROARG
	#define ZEROINIT
	#if !defined(UGEN_FFTW) && !defined(UGEN_VDSP)
		#define UGEN_FFTREAL 1
	#endif
#else
	#if (TARGET_RT_LITTLE_ENDIAN)
		#define VEC_MUL_OP _mm_mul_ps
		#define VEC_ADD_OP _mm_add_ps
		#define VEC_SUB_OP _mm_sub_ps
		#define ZEROARG
		#define ZEROINIT
	#else
		#define VEC_MUL_OP vec_madd
		#define VEC_ADD_OP vec_add
		#define VEC_SUB_OP vec_sub
		#define ZEROARG , zero
		#define ZEROINIT vFloat zero = {0.,0.,0.,0.};
	#endif
#endif

#ifdef UGEN_FFTW
	END_UGEN_NAMESPACE
		#include <fftw3.h>
	BEGIN_UGEN_NAMESPACE
#elif !defined(UGEN_VDSP)
	#define UGEN_FFTREAL 1
	END_UGEN_NAMESPACE
		#include "../fftreal/FFTReal.h"
	BEGIN_UGEN_NAMESPACE
	// e.g. fftsize=8
	//            | Positive FFT   | Negative FFT
	//Bin         | Real part      | imaginary part  | imaginary part
	//------------+----------------+-----------------+----------------0 
	//0  dc       | f [0]          | 0               | 0
	//1           | f [1]          | f [5]           | -f [5]
	//2           | f [2],         | f [6]           | -f [6]
	//3           | f [3]          | f [7]           | -f [7]
	//4  nyquist  | f [4]          | 0               | 0
	//
	//... mirror:
	//
	//5           | f [3]          | -f [7]          | f [7]
	//6           | f [2]          | -f [6]          | f [6]
	//7           | f [1]          | -f [5]          | f [5]
#endif

#if defined(UGEN_NOEXTGPL) && (defined(UGEN_FFTW) || defined(UGEN_FFTREAL))
#warning UGEN_NOEXTGPL: FFT Real and FFTW are GPL!
#endif


/**
 Provides real to complex FFT and complex to real IFFT processes using a selection of underlying libraries.
 
 The fastest on the Mac is vDSP but you need to define UGEN_VDSP equalt to 1 as a preprocessor macro. FFTReal (which is 
 in the source code tree and doesn't need to be downloaded separately) is used by default. You can use FFTW if you have it
 installed and define UGEN_FFTW equal to 1 before this file (e.g., doing this in preprocessor macros should ensure this).
 
 FFTReal is the slowest but at least it's available. FFTW on Windows compares well with vDSP on the Mac.
 */
class FFTEngineInternal : public SmartPointer
{
public:
	FFTEngineInternal(const int fftSize) throw();
	~FFTEngineInternal();
	
	void dispose();
	
	inline int size() const throw() { return fftSize; }
	inline Buffer& getFFTWindow() throw() { return fftWindow; }
	inline Buffer& getIFFTWindow() throw() { return ifftWindow; }
	inline void setFFTWindow(Buffer const& window) throw() 
	{ 
		fftWindow = window.resample(fftSize); 
		fftWindowSamples = fftWindow.getData();
		fftWindowFactor = fftWindow.sum(0) / fftSize;
	}
	
	inline void setIFFTWindow(Buffer const& window) throw() 
	{ 
		ifftWindow = window.resample(fftSize); 
		ifftWindowSamples = ifftWindow.getData();
	}
	
	void fft(Buffer const& outputBuffer, 
			 Buffer const& inputBuffer, 
			 const bool applyWindow = false,
			 const int outputChannel = 0,
			 const int inputChannel = 0) throw();
	
	void fft(Buffer const& outputBuffer, 
			 Buffer const& inputBuffer, 
			 const bool applyWindow,
			 const int outputChannel,
			 const int inputChannel,
			 const int outputOffset,
			 const int inputOffset) throw();
	
	inline void fft(DSPSplitComplex& outputBuffer, const float* const inputBuffer) throw()
	{
		ugen_assert(inputBuffer != 0);
		
#ifdef UGEN_FFTW
		memcpy(transformBufferSamples, inputBuffer, fftSizeBytes);
		fftwf_execute(fftwPlan);

		float nyquist = transformBufferSamples[fftSize]; // remember nyquist val
		float *interleavedSamples = transformBufferSamples;
		// deinteleave
		for(int j = 0; j < fftSizeHalved; j++)
		{
			outputBuffer.realp[j] = *interleavedSamples++;
			outputBuffer.imagp[j] = *interleavedSamples++;
		}
		outputBuffer.imagp[0] = nyquist; // pack nyquist in
		
#elif defined(UGEN_FFTREAL)
		fftReal->do_fft(transformBufferSamples, inputBuffer);
		memcpy(outputBuffer.realp, transformBufferSplit.realp, fftSizeHalvedBytes);
		memcpy(outputBuffer.imagp, transformBufferSplit.imagp, fftSizeHalvedBytes);
		
#else
		static float scale = 0.5f;
		vDSP_vsmul(inputBuffer, 1, &scale, transformBufferSamples, 1, fftSize);
		vDSP_ctoz ((COMPLEX *) transformBufferSamples, 2, &outputBuffer, 1, fftSizeHalved);
		vDSP_fft_zrip (fftvDSP, &outputBuffer, 1, fftSizeLog2, FFT_FORWARD);
		
#endif
	}
	
	void fft(DSPSplitComplex& outputBuffer, const float* const inputBuffer, const bool applyWindow) throw();
	
	void ifft(Buffer const& outputBuffer, 
			  Buffer const& inputBuffer, 
			  const bool applyWindow = false, 
			  const bool applyScaling = false,
			  const int outputChannel = 0,
			  const int inputChannel = 0) throw();
	
	void ifft(Buffer const& outputBuffer, 
			  Buffer const& inputBuffer, 
			  const bool applyWindow, 
			  const bool applyScaling,
			  const int outputChannel,
			  const int inputChannel,
			  const int outputOffset,
			  const int inputOffset) throw();
	
	inline void ifft(float* const outputBuffer, DSPSplitComplex const& inputBuffer) throw()
	{
		ugen_assert(outputBuffer != 0);
		
#ifdef UGEN_FFTW	
		LOCAL_DECLARE(const int, fftSize);
		LOCAL_DECLARE(const int, fftSizeHalved);
		LOCAL_DECLARE(const int, fftSizeBytes);
		LOCAL_DECLARE(const int, fftSizeHalvedBytes);
		
		LOCAL_DECLARE(float * const, transformBufferSamples);
		LOCAL_DECLARE(DSPSplitComplex&, transformBufferSplit);

		float *interleavedSamples = transformBufferSamples;
		for(int j = 0; j < fftSizeHalved; j++)
		{
			*interleavedSamples++ = inputBuffer.realp[j];
			*interleavedSamples++ = inputBuffer.imagp[j];
		}
		transformBufferSamples[fftSize  ] = transformBufferSamples[1]; // nyquist
		transformBufferSamples[fftSize+1] = 0.f; // nyquist imag always zero
		transformBufferSamples[1        ] = 0.f; // DC imag always zero
		
		fftwf_execute(ifftwPlan);
		memcpy(outputBuffer, transformBufferSamples, fftSizeBytes);
#elif defined(UGEN_FFTREAL)
		memcpy(transformBufferSplit.realp, inputBuffer.realp, fftSizeBytes);
		fftReal->do_ifft(transformBufferSamples, outputBuffer);
#else
//	#ifdef UGEN_IPHONE
//		cblas_ccopy(fftSize, inputBuffer.realp, 1, transformBufferSplit.realp, 1);
//	#else
//		vScopy(fftSize, (const vFloat*)inputBuffer.realp, (vFloat*)transformBufferSplit.realp);
//	#endif			
		memcpy(transformBufferSplit.realp, inputBuffer.realp, fftSizeBytes);
		vDSP_fft_zrip (fftvDSP, &transformBufferSplit, 1, fftSizeLog2, FFT_INVERSE);
		vDSP_ztoc (&transformBufferSplit, 1, (COMPLEX *) outputBuffer, 2, fftSizeHalved);
#endif		
	}
	
	void ifft(float* const outputBuffer, DSPSplitComplex const& inputBuffer, const bool applyWindow, const bool applyScaling) throw();
	
	Buffer rawToRealImagRawSplit(Buffer const& raw) throw();
	Buffer rawToRealImagUnpacked(Buffer const& raw) throw();
	Buffer rawToRealImagUnpacked(Buffer const& raw, const int firstBin, const int numBins) throw();
	Buffer rawToRealImagUnpackedSplit(Buffer const& raw, const int firstBin, const int numBins) throw();
	Buffer rawToMagnitudePhase(Buffer const& raw, 
							   const int firstBin, 
							   const int numBins, 
							   const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToMagnitudePhaseSplit(Buffer const& raw, 
									const int firstBin, 
									const int numBins, 
									const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToMagnitude(Buffer const& raw, 
						  const int firstBin, 
						  const int numBins,
						  const bool magnitudeAssumesWindowedFFT = true) throw();
	Buffer rawToPhase(Buffer const& raw, const int firstBin, const int numBins) throw();
	
private:
#ifdef UGEN_FFTW
	fftwf_plan fftwPlan, ifftwPlan;
#elif defined(UGEN_FFTREAL)
	FFTReal<float> *fftReal;
#else
    FFTSetup fftvDSP;
	int fftSizeLog2;
#endif	
	
	const int fftSize;
	const int fftSizeHalved;
	const int fftSizeBytes;
	const int fftSizeHalvedBytes;
	const float ifftScaling;
	
	Buffer transformBuffer;
	float * const transformBufferSamples;
	DSPSplitComplex transformBufferSplit;
	
	Buffer fftWindow, ifftWindow;
	float* fftWindowSamples;
	float* ifftWindowSamples;
	float fftWindowFactor;
	
	Buffer windowingBuffer;
	float * const windowingBufferSamples;
};

#if defined(UGEN_VDSP)

#define TEMPSIZE 65536
static float tempData[TEMPSIZE];

inline void MultAndAdd(DSPSplitComplex& In1, DSPSplitComplex& In2, DSPSplitComplex& Out, const int VecLength) throw()
{
	ugen_assert(VecLength > 0);
	ugen_assert(VecLength*4/2 <= TEMPSIZE);
		
	float Nyquist1, Nyquist2;
	Nyquist1 = In1.imagp[0];
	Nyquist2 = In2.imagp[0];
	
	In1.imagp[0] = 0.f;
	In2.imagp[0] = 0.f;
	
	Out.imagp[0] += Nyquist1 * Nyquist2;
	
	const int VecLengthX4 = VecLength<<2;
	
	DSPSplitComplex temp;
	temp.realp = tempData;
	temp.imagp = tempData + VecLengthX4;
	
	vDSP_zvmul(&In1, 1, &In2, 1, &temp, 1, VecLengthX4, 1);
	vDSP_zvadd(&temp, 1, &Out, 1, &Out, 1, VecLengthX4);
	
	In1.imagp[0] = Nyquist1;
	In2.imagp[0] = Nyquist2;
}
#elif !defined(UGEN_VFP) // && !defined(UGEN_NEON)
inline void MultAndAdd(DSPSplitComplex& In1, DSPSplitComplex& In2, DSPSplitComplex& Out, const int VecLength) throw()
{
	ugen_assert(VecLength > 0);
	
	vFloat *InReal1 = (vFloat *) In1.realp;
	vFloat *InImag1 = (vFloat *) In1.imagp;
	vFloat *InReal2 = (vFloat *) In2.realp;
	vFloat *InImag2 = (vFloat *) In2.imagp;
	vFloat *OutReal = (vFloat *) Out.realp;
	vFloat *OutImag = (vFloat *) Out.imagp;
	ZEROINIT
	
	float Nyquist1, Nyquist2;
	Nyquist1 = In1.imagp[0];
	Nyquist2 = In2.imagp[0];
	
	In1.imagp[0] = 0.f;
	In2.imagp[0] = 0.f;
	
	Out.imagp[0] += Nyquist1 * Nyquist2;
	
	for (int i = 0; i < VecLength; i++)
	{
		OutReal[i] = VEC_ADD_OP (OutReal[i], VEC_SUB_OP (VEC_MUL_OP(InReal1[i], InReal2[i] ZEROARG), VEC_MUL_OP(InImag1[i], InImag2[i] ZEROARG)));
		OutImag[i] = VEC_ADD_OP (OutImag[i], VEC_ADD_OP (VEC_MUL_OP(InReal1[i], InImag2[i] ZEROARG), VEC_MUL_OP(InImag1[i], InReal2[i] ZEROARG)));
	}
	
	In1.imagp[0] = Nyquist1;
	In2.imagp[0] = Nyquist2;
}
#endif



#endif // _UGEN_ugen_FFTEngineInternal_H_
