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

#if defined(UGEN_CONVOLUTION) && UGEN_CONVOLUTION

#if !defined(WIN32) && !defined(UGEN_IPHONE)
	#include <Accelerate/Accelerate.h>
	#include <CoreServices/CoreServices.h>
#endif
#include "../core/ugen_StandardHeader.h"


BEGIN_UGEN_NAMESPACE
#include "ugen_Convolution.h"
#include "../fft/ugen_FFTEngineInternal.h"


PartBuffer::PartBuffer() throw()
:	bufferSize(0),
	numPartitions(0),
	startPoint(0),
	endPoint(0),
	length(0),
	fftSizeLog2(0),
	fftSize(0),
	fftSizeOver2(0),
	fftSizeOver4(0)
{				
}

PartBuffer::PartBuffer(Buffer const& original, 
					   long startPointToUse,
					   long endPointToUse, 
					   long fftSizelog2ToUse) throw()
:	Buffer(BufferSpec((endPointToUse ? endPointToUse - startPointToUse : quantiseUp(original.size(), fftSizeFromArg(fftSizelog2ToUse))) * 2,
		   original.getNumChannels(), 
		   false)),
	bufferSize(size_ / 2),
	numPartitions(0),
	startPoint(startPointToUse),
	endPoint(endPointToUse),
	length(0),
	fftSizeLog2(fftSizeLog2FromArg(fftSizelog2ToUse)),
	fftSize(fftSizeFromArg(fftSizelog2ToUse)),
	fftSizeOver2(fftSize >> 1),
	fftSizeOver4(fftSizeOver2 >> 1),
	partTempBuffer(BufferSpec(fftSize + 2, 1, false)),
	fftEngine(fftSize)
{				
	partitionImpulse(original);
}

PartBuffer::PartBuffer(BufferChannelInternal *internalToUse,
					   int partitionsInBuffer,
					   long startPointToUse,
					   long endPointToUse, 
					   long fftSizelog2ToUse) throw()
:	Buffer(internalToUse),
	bufferSize(size_ / 2),
	numPartitions(partitionsInBuffer),
	startPoint(startPointToUse),
	endPoint(endPointToUse),
	length(0),
	fftSizeLog2(fftSizeLog2FromArg(fftSizelog2ToUse)),
	fftSize(fftSizeFromArg(fftSizelog2ToUse)),
	fftSizeOver2(fftSize >> 1),
	fftSizeOver4(fftSizeOver2 >> 1),
	partTempBuffer(BufferSpec(fftSize + 2, 1, false)),
	fftEngine(fftSize)
{
}

PartBuffer::~PartBuffer()
{
}

PartBuffer PartBuffer::getChannel(const int channel) const throw()
{
	if(channel < 0 || channel >= numChannels_) 
	{
		ugen_assertfalse;
		return Buffer();
	}
	
	if(numChannels_ == 1) return *this;
	
	return PartBuffer(channels[channel], startPoint, endPoint, fftSizeLog2);
}

void PartBuffer::partitionImpulse(Buffer const& original)
{
	for(int i = 0; i < original.getNumChannels(); i++)
	{
		partitionImpulseChannel(original, i);
	}
}

void PartBuffer::partitionImpulseChannel(Buffer const& original, const int channel)
{
	const float *bufferSamples;
	long impulseLength;

	//bufferSamples = original.getDataReadOnly(channel);
	bufferSamples = original.getData(channel);
	
	impulseLength = original.size();
		
	long fftSize = this->fftSize;
	long fftSizeHalved = fftSize / 2;
		
	long startPoint = this->startPoint;
	long endPoint = this->endPoint;
	long length = this->length;
	long numSamples;
	
	if (length && length < impulseLength)
		impulseLength = length;	
	if (endPoint && endPoint < impulseLength)
		impulseLength = endPoint;
	impulseLength -= startPoint;
	if (impulseLength < 0) impulseLength = 0;
	
	int numPartitions = impulseLength / fftSizeHalved;
	if (impulseLength % fftSizeHalved) numPartitions++;
	
	if (numPartitions * fftSizeHalved > this->bufferSize) 
		numPartitions = this->bufferSize / fftSizeHalved;
	
	float *bufferTemp1 = partTempBuffer.getData(); // this->Part_Temp.realp;
	float *impulseReal = getDataReal(channel);
	float *impulseImag = getDataImag(channel);
		
	DSPSplitComplex bufferTemp2;
	
	for (int i = 0; i < numPartitions; i++)
	{
		int bufferPos = i * fftSizeHalved;
		bufferTemp2.realp = impulseReal + (bufferPos);
		bufferTemp2.imagp = impulseImag + (bufferPos);
		
		// For proper ampwarping etc. - get two partitions - take FFTs of both - clip - ifft and overlap with hannging window
		
		if (impulseLength >= (bufferPos + fftSizeHalved)) numSamples = fftSizeHalved;
		else numSamples = impulseLength - bufferPos;
		
#ifdef UGEN_VDSP
//	#ifdef UGEN_IPHONE
//		cblas_ccopy(numSamples, (float*)bufferSamples + startPoint + bufferPos, 1, (float*)bufferTemp1, 1);
//	#else
//		vScopy(numSamples, bufferSamples + startPoint + bufferPos, bufferTemp1);
//	#endif	
		memcpy(bufferTemp1, bufferSamples + startPoint + bufferPos, sizeof(float) * numSamples);
		vDSP_vclr(bufferTemp1+numSamples, 1, partTempBuffer.size()-numSamples);
#else
		// copy real time domain samples to Buffer_Temp1
		memcpy(bufferTemp1, bufferSamples + startPoint + bufferPos, sizeof(float) * numSamples);
		
		// zero pad the rest of Buffer_Temp1
		for (int j = numSamples; j < partTempBuffer.size(); j++)
			bufferTemp1[j] = 0;
#endif
		
		// Do FFT Straight Into Position...
		
		fftEngine.getInternal()->fft(bufferTemp2, bufferTemp1);
	}
		
	this->numPartitions = numPartitions;
}

PartConvolveUGenInternal::PartConvolveUGenInternal(UGen const& input, 
												   PartBuffer const& partImpulse) throw()
:	UGenInternal(NumInputs),
	partImpulse_(partImpulse),
	startPoint(partImpulse.startPoint),
	endPoint(partImpulse.endPoint),
	length(partImpulse.length),
	fftSize(partImpulse.fftSize),
	fftSizeOver4(partImpulse.fftSizeOver4),
	fftSizeLog2(partImpulse.fftSizeLog2),
	bufferSize(partImpulse.bufferSize),
	resetAll(1),
	fftEngine(fftSize),
	scaleMultD(1.0 / (double) (fftSize * 4)),	
	scaleMult(vecSplat((float) (scaleMultD))),
	inputBuffer(BufferSpec((int)(bufferSize * 2), 1, false)),
	inputBufferSamples(inputBuffer.getData()),
	fftBuffersMemory(BufferSpec(fftSize * 4, 1, false)),
	fftBuffersMemorySamples(fftBuffersMemory.getData()),
	fftTempBuffer(BufferSpec(fftSize + 2, 1, false)),
	fftTempBufferSamples(fftTempBuffer.getData())
{		
	inputs[Input] = input;
	
	fftBuffers[0] = (vFloat*)fftBuffersMemorySamples;
	fftBuffers[1] = fftBuffers[0] + fftSizeOver4;											
	fftBuffers[2] = fftBuffers[1] + fftSizeOver4;											
	fftBuffers[3] = fftBuffers[2] + fftSizeOver4;	
		
//	fftEngine = new FFTEngineInternal(fftSize);
}

PartConvolveUGenInternal::~PartConvolveUGenInternal() throw()
{
}

UGenInternal* PartConvolveUGenInternal::getChannel(const int channel) throw()
{
	int numInternalChannels = ugen::max(inputs[Input].getNumChannels(), 
										partImpulse_.getNumChannels()); 
		
	if(channel < 0 || numInternalChannels < 2)
	{
		incrementRefCount();
		return this;																		
	}
	else
	{
		UGenInternal* inputInternal	= inputs[Input].getInternalUGen(channel % inputs[Input].getNumChannels());
		PartBuffer partImpulseChannel = partImpulse_.getChannel(channel % partImpulse_.getNumChannels());
		
		return new PartConvolveUGenInternal(UGen(inputInternal, channel), partImpulseChannel);
	}
}


void PartConvolveUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int numSamples = uGenOutput.getBlockSize();
    
    vFloat *outputSamples = (vFloat*)uGenOutput.getSampleData();		
	vFloat *inputSamples = (vFloat*)inputs[Input].processBlock(shouldDelete, blockID, channel);
    	
	DSPSplitComplex impulseTemp, bufferTemp;
	DSPSplitComplex impulse;
		
	impulse.realp = partImpulse_.getDataReal(channel);
	impulse.imagp = partImpulse_.getDataImag(channel);
	
	//LOCAL_DECLARE(float* const, fftBuffersMemorySamples);
	LOCAL_DECLARE(float* const, fftTempBufferSamples);
	LOCAL_DECLARE(float* const, inputBufferSamples);
	
	LOCAL_DECLARE(int, tillNextFFT);
	LOCAL_DECLARE(int, rwPointer1);
	LOCAL_DECLARE(int, rwPointer2);
	LOCAL_DECLARE(int, fftOffset);
		
	LOCAL_DECLARE(const int, fftSize);
	const int fftSizeHalved = fftSize >> 1 ;
	const int fftSizeHalvedOver4 = fftSizeHalved >> 2;
	LOCAL_DECLARE(const int, fftSizeLog2);
	const int fftSizeHalvedLog2 = fftSizeLog2-1;
	LOCAL_DECLARE(const int, bufferSize);
	
#ifndef UGEN_VDSP
	LOCAL_DECLARE(const int, fftSizeOver4);
#endif
	
	DSPSplitComplex fftTemp;
	fftTemp.realp = fftTempBufferSamples;
	fftTemp.imagp = fftTemp.realp + fftSizeHalved;
	
	const int Partitions = partImpulse_.numPartitions;
	LOCAL_DECLARE(int, bufPosition);
	
	// Schedule Stuff
	LOCAL_DECLARE(int, partitionsDone);
	LOCAL_DECLARE(int, scheduleCounter);
	LOCAL_DECLARE(int, lastPart);
	LOCAL_DECLARE(int, validPart);
	
	// FFT Stuff	
//	LOCAL_DECLARE(FFTEngineInternal*, fftEngine);
	
	LOCAL_DECLARE(vFloat **, fftBuffers);
	vFloat *tempvPointer1, *tempvPointer2;
	
	int numVectorsRemaining = numSamples >> 2;
	
	LOCAL_DECLARE(vFloat, scaleMult);
	vFloat zero = {0.,0.,0.,0.};
	
	if (resetAll) {
		// FFT Related
		int N = 1 << (fftSizeLog2 - 3);
		int randomStart;
		while (N <= (randomStart = std::rand() / (RAND_MAX / N))); // perhaps use Ran088 here?
		
		tillNextFFT = randomStart;
		rwPointer1 = (fftSizeHalved >> 2) - tillNextFFT;
		rwPointer2 = rwPointer1 + tillNextFFT;
		fftOffset = 0;

#ifdef UGEN_VDSP
		vDSP_vclr(fftTempBufferSamples, 1, fftSize);
		vDSP_vclr(fftBuffersMemorySamples, 1, fftSize<<2);
#else
		tempvPointer1 = (vFloat*)fftTempBufferSamples;
		for (int i = 0; i < fftSizeOver4; i++)	tempvPointer1[i] = zero;
		
		tempvPointer1 = (vFloat*)fftBuffersMemorySamples;
		for (int i = 0; i < fftSize; i++)		tempvPointer1[i] = zero;
#endif			
		// Scheduling Related
		bufPosition = 0;
		scheduleCounter = 0;
		partitionsDone = 0;
		lastPart = 0;
		
		// How Many Partitions To Do...
		validPart = 1;
		
		this->resetAll = 0;
	}
	
	while (numVectorsRemaining > 0) {
		// Check whether there will be another FFT this vector
		int loop;
		int fftTest = numVectorsRemaining - tillNextFFT;
		if (fftTest > 0) {
			loop = tillNextFFT;
			numVectorsRemaining -= tillNextFFT;
			tillNextFFT = 0;
		} else {
			loop = numVectorsRemaining;
			tillNextFFT -= numVectorsRemaining;
			numVectorsRemaining = 0;
		}
		
		// Load input into buffer (twice) and output from the output buffer
		// Put it straight into the right place - using overlap save it would seem....
		
		
//#if 0 //defined(UGEN_VDSP) // not sure why this doesn't work
//		const int loopX4 = loop<<2;
//	#ifdef UGEN_IPHONE
//		cblas_ccopy(loopX4, (float*)inputSamples, 1, (float*)(fftBuffers[0] + rwPointer1), 1);
//		cblas_ccopy(loopX4, (float*)inputSamples, 1, (float*)(fftBuffers[1] + rwPointer2), 1);
//		cblas_ccopy(loopX4, (float*)(fftBuffers[3] + rwPointer1), 1, (float*)outputSamples, 1);
//	#else
//		vScopy(loopX4, inputSamples, fftBuffers[0] + rwPointer1);
//		vScopy(loopX4, inputSamples, fftBuffers[1] + rwPointer2);
//		vScopy(loopX4, fftBuffers[3] + rwPointer1, outputSamples);
//	#endif	
//		rwPointer1 += loop;
//		rwPointer2 += loop;
//		inputSamples += loop;
//		outputSamples += loop;
#if 1 // let's try this...
		const int loopX4 = loop<<2;
		memcpy(fftBuffers[0] + rwPointer1, inputSamples, loopX4*sizeof(float));
		memcpy(fftBuffers[1] + rwPointer2, inputSamples, loopX4*sizeof(float));
		memcpy(outputSamples, fftBuffers[3] + rwPointer1, loopX4*sizeof(float));
		rwPointer1 += loop;
		rwPointer2 += loop;
		inputSamples += loop;
		outputSamples += loop;		
#else
		for (int i = 0; i < loop; i++)
		{
			*(fftBuffers[0] + rwPointer1) = *inputSamples; 
			*(fftBuffers[1] + rwPointer2) = *inputSamples;
			
			*outputSamples++ = *(fftBuffers[3] + rwPointer1);	
			
			rwPointer1++;
			rwPointer2++;
			inputSamples++;
		}
#endif
		
		// Work Loop
		
		if (partitionsDone >= validPart - 1) loop = 0;		// Check To See If there's more processing scheduled
		else loop = 1;
		scheduleCounter++;
		
		while (loop)
		{
			// How Many Partitions To Do....
			int numPartitionsToDo = (int)((float) ((scheduleCounter * (validPart - 1)) / (float) ((fftSizeHalved / numSamples) - 1)) - partitionsDone);
			if (scheduleCounter >= (fftSizeHalved / numSamples) - 1) 
				numPartitionsToDo = (validPart - partitionsDone) - 1;
			
			// Calculate Buffer Wraparound
			int nextPart = lastPart;
			if (nextPart >= Partitions) nextPart = 0;			// Flip Round Buffer Immediately
			lastPart = nextPart + numPartitionsToDo;
			if (lastPart > Partitions) {						// Flip Round In Middle
				lastPart = Partitions;
				numPartitionsToDo = lastPart - nextPart;
			}
			else loop = 0;										// No Flip To Do
			
			// Do Processing
			int offset = (partitionsDone + 1) << fftSizeHalvedLog2; // * fftSizeHalved;
			impulseTemp.realp = impulse.realp + offset;
			impulseTemp.imagp = impulse.imagp + offset;
			offset = nextPart << fftSizeHalvedLog2; // * fftSizeHalved;
			bufferTemp.realp = inputBufferSamples + offset; // InputBuffer.realp + Offset;
			bufferTemp.imagp = bufferTemp.realp + bufferSize; // InputBuffer.imagp + Offset;
			
			partitionsDone += numPartitionsToDo;
			
			for (int i = 0; i < numPartitionsToDo; i++) {	
				MultAndAdd(bufferTemp, impulseTemp, fftTemp, fftSizeHalvedOver4);
				impulseTemp.realp += fftSizeHalved;
				impulseTemp.imagp += fftSizeHalved;
				bufferTemp.realp += fftSizeHalved;
				bufferTemp.imagp += fftSizeHalved;
			}
		}
		
		// Check that there is a new FFTs worth of buffer 
		
		if (tillNextFFT == 0)
		{			
			if (++validPart > Partitions) validPart = Partitions;
			
			// Calculate the position to put the FFT output into
			impulseTemp = impulse;
			const int offset = bufPosition << fftSizeHalvedLog2; // * fftSizeHalved;
			bufferTemp.realp = inputBufferSamples + offset; // InputBuffer.realp + Offset;
			bufferTemp.imagp = bufferTemp.realp + bufferSize; // InputBuffer.imagp + Offset;
			
			// Do the FFT, Process and Convert back with scaling
			tempvPointer1 = fftBuffers[fftOffset];
			fftEngine.getInternal()->fft(bufferTemp, (float*)tempvPointer1);
						
			// Process First Partition Here (we need it now!)
			
			MultAndAdd(bufferTemp, impulseTemp, fftTemp, fftSizeHalvedOver4);//fftSizeHalved >> 2);
			
			if (--bufPosition < 0)
				bufPosition = Partitions - 1;
			
			// Processing Done - Do Inverse FFT
			fftEngine.getInternal()->ifft((float*)fftBuffers[2], fftTemp);
						
			// Overlap Save the Result
			loop = fftSize >> 3;
			int test = fftOffset;
			
			tempvPointer1 = fftBuffers[3] + (loop * (1 - test));
			tempvPointer2 = fftBuffers[3] + (loop * test);
			
			// post ifft scaling required by vDSP fft processes
			
#ifdef UGEN_VDSP
			vDSP_vsmsa((float*)fftBuffers[2], 1, (float*)&scaleMult, (float*)&zero, (float*)tempvPointer1, 1, loop<<2);
			vDSP_vsma((float*)(fftBuffers[2]+loop), 1, (float*)&scaleMult, (float*)tempvPointer2, 1, (float*)tempvPointer2, 1, loop<<2);
#else
			for (int i = 0; i < loop; i++)
				*(tempvPointer1++) = VEC_MUL_OP(*(fftBuffers[2] + i), scaleMult ZEROARG);

			for (int i = loop; i < fftSize >> 2; i++) {
				*(tempvPointer2) = VEC_ADD_OP(*(tempvPointer2), VEC_MUL_OP(*(fftBuffers[2] + i), scaleMult ZEROARG)); 
				tempvPointer2++;
			}			
#endif		

			// Clear Buffer
#ifdef UGEN_VDSP
			vDSP_vclr(fftTemp.realp, 1, fftSizeHalved);
			vDSP_vclr(fftTemp.imagp, 1, fftSizeHalved);
#else
			for (int i = 0; i < fftSizeHalved; i++) fftTemp.realp[i] = 0.f;
			for (int i = 0; i < fftSizeHalved; i++) fftTemp.imagp[i] = 0.f;
#endif					
			// Set wait till next data to hop time
			tillNextFFT = loop;
			
			// Reset RW_Pointers
			if (fftOffset) rwPointer1 = 0;
			else rwPointer2 = 0;
			fftOffset = 1 - fftOffset; // swaps between 0 and 1
			
			// Reset Schedule Stuff
			lastPart = bufPosition + 1;
			scheduleCounter = 0;
			partitionsDone = 0;
		}
	}
	
	LOCAL_COPY(bufPosition);
	LOCAL_COPY(fftOffset);
	LOCAL_COPY(tillNextFFT);
	LOCAL_COPY(rwPointer1);
	LOCAL_COPY(rwPointer2);
	
	LOCAL_COPY(partitionsDone);
	LOCAL_COPY(scheduleCounter);
	LOCAL_COPY(lastPart);
	LOCAL_COPY(validPart);
}


PartConvolve::PartConvolve(UGen const& input, 
						   Buffer const& impulse, 
						   long startPoint, long endPoint, long fftSizeLog2) throw()
{	
	int numChannels = ugen::max(input.getNumChannels(), impulse.getNumChannels());
	initInternal(numChannels);
	
	PartBuffer partImpulse(impulse, startPoint, endPoint, fftSizeLog2);
	
	for(int i = 0; i < numChannels; i++)
	{
		internalUGens[i] = new PartConvolveUGenInternal(input, partImpulse);
	}
}

PartConvolve::PartConvolve(UGen const& input, PartBuffer const& partImpulse) throw()
{
	int numChannels = ugen::max(input.getNumChannels(), partImpulse.getNumChannels());
	initInternal(numChannels);
		
	for(int i = 0; i < numChannels; i++)
	{
		internalUGens[i] = new PartConvolveUGenInternal(input, partImpulse);
	}
}

#if !defined(UGEN_FFTW) && !defined(UGEN_FFTREAL)
TimeConvolveUGenInternal::TimeConvolveUGenInternal(UGen const& input, 
												   Buffer const& impulse, 
												   long startPoint, 
												   long endPoint, 
												   long fftSizelog2) throw()
:	UGenInternal(NumInputs),
	ioBuffers(BufferSpec(8192, 2, true)),
	position(0)
{	
	inputs[Input] = input;
	
	if(endPoint == -1 && startPoint == -1)
	{
		// this is a straight copy...
		filters = impulse;
	}
	else
	{
		if(endPoint == 0)
			endPoint = impulse.size();
		
		if(endPoint-startPoint > 2044)
			endPoint = startPoint + 2044;
		
		filters = impulse.getRegion(endPoint-1, startPoint); // get samples in reverse
	}
}

TimeConvolveUGenInternal::~TimeConvolveUGenInternal() throw()
{
}

UGenInternal* TimeConvolveUGenInternal::getChannel(const int channel) throw()
{
	int numInternalChannels = ugen::max(inputs[Input].getNumChannels(), 
										filters.getNumChannels()); 
	
	if(channel < 0 || numInternalChannels < 2)
	{
		incrementRefCount();
		return this;																		
	}
	else
	{
		UGenInternal* inputInternal	= inputs[Input].getInternalUGen(channel % inputs[Input].getNumChannels());
		Buffer filterChannel = filters.getChannel(channel % filters.getNumChannels());
		
		return new TimeConvolveUGenInternal(UGen(inputInternal, channel), filterChannel, -1, -1);
	}	
}

void TimeConvolveUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	float *inputSamples = (float *)inputs[Input].processBlock(shouldDelete, blockID, channel);
	float *outputSamples = (float *)uGenOutput.getSampleData();
	int numSamples = (int)uGenOutput.getBlockSize();
	
	float *filter = filters.getData(channel);
	float *inputBufferSamples = ioBuffers.getData(TimeConvolveUGenInternal::InputBuffer);
	long position = this->position;
	long filterlength = filters.size();
	
	// N.B. Size of input_buf is always multiple of vectsize
	// We must be able to read out input in one go 
	// Copy input twice
	
	memcpy(inputBufferSamples + position, inputSamples, sizeof(float) * numSamples);
	memcpy(inputBufferSamples + 4096 + position, inputSamples, sizeof(float) * numSamples);
	
	position += numSamples;
	if (position >= 4096) position = 0;
	
	// Convolve Here
	
	vDSP_conv (inputBufferSamples + 4096 + position - (filterlength + numSamples) + 1, 1, 
			   filter, 1, 
			   outputSamples, 1, 
			   numSamples, filterlength);
	
	this->position = position;	
}

TimeConvolve::TimeConvolve(UGen const& input, 
						   Buffer const& impulse, 
						   long startPoint, long endPoint, long fftSizeLog2) throw()
{	
	int numChannels = ugen::max(input.getNumChannels(), impulse.getNumChannels());
	initInternal(numChannels);
	
	for(int i = 0; i < numChannels; i++)
	{
		internalUGens[i] = new TimeConvolveUGenInternal(input, impulse, startPoint, endPoint, fftSizeLog2);
	}
}

#endif // #if !defined(UGEN_FFTW) && !defined(UGEN_FFTREAL)


END_UGEN_NAMESPACE

#endif
