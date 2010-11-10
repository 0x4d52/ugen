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

#ifndef _UGEN_ugen_Buffer_H_
#define _UGEN_ugen_Buffer_H_

#include "../core/ugen_SmartPointer.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../envelopes/ugen_EnvCurve.h"
#include "../core/ugen_Arrays.h"
#include "../core/ugen_Text.h"

class BufferChannelInternal : public SmartPointer
{
public:
	BufferChannelInternal(const unsigned int size, bool zeroData = false) throw();
	BufferChannelInternal(const unsigned int size, const unsigned int sourceDataSize, const float* sourceData) throw();
	BufferChannelInternal(const unsigned int size, const double start, const double end) throw();
	~BufferChannelInternal() throw();
	
	inline float getSampleUnchecked(const int index) const throw() { return data[index]; }
	inline float getSampleUnchecked(const double index) const throw() { return getSampleUnchecked((float)index); }
	inline float getSampleUnchecked(const float index) const throw() 
	{ 
		const int iIndex0 = (int)index;
		unsigned int iIndex1 = iIndex0+1;
		if(iIndex1 >= size_)
			iIndex1 = 0;
		const float frac1 = (float)index - (float)iIndex0;
		const float frac0 = 1.f - frac1;
		
		return data[iIndex0] * frac0 + data[iIndex1] * frac1; // need to optimise this at some point		
	}
	
	friend class Buffer;
	friend class PartBuffer;
	friend class ComplexBuffer;
	friend class TapInUGenInternal;
	friend class TapOutUGenInternal;
	
private:
	float* data;
	unsigned int size_;
	unsigned int allocatedSize;
	unsigned int currentWriteBlockID;
	int circularHead; // -1 means it is not a crcular buffer
	int previousCircularHead;
	
	BufferChannelInternal (const BufferChannelInternal&);
    const BufferChannelInternal& operator= (const BufferChannelInternal&);
};

class BufferSpec
{
public:
	BufferSpec(const int size = 1, const int numChannels = 1, bool zeroData = false)
	:	numChannels_(numChannels),
		size_(size), 
		zeroData_(zeroData)
	{
		ugen_assert(size > 0);
		ugen_assert(numChannels > 0);
	}
	
	friend class Buffer;
	
private:
	int numChannels_;
	int size_;
	bool zeroData_;
};

class Value;
class ValueArray;
class UGen;

/**
 Buffer stores one or more arrays of floats.
 
 This will be used to store memory resisdent sample data for playback. It can also be used
 for wavetables, paramaters for things like Env and arrays of constants in other places (e.g., 
 the frequency input of FSinOsc since its frequency can't be modulated). The B macro is short for Buffer. Buffers with
 one channel upto length 32 can be initialised by enclosing double or float values in B(...). E.g. 
 
 @code B(0.0, 0.5, 1.0, 0.5); @endcode
 
 Creates a Buffer with one channel, and size 4 with data 0.0, 0.5, 1.0 & 0.5 at indices 0, 1, 2 & 3
 respectively.
 
 (possibly for UGenOutput? later) 
  
 @see PlayBuf, TableOsc, HarmonicOsc
 
 */
class Buffer
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	Buffer() throw();
	Buffer(BufferSpec const& spec) throw();
	Buffer(int i00) throw();
	Buffer(int i00, int i01) throw();
	Buffer(int i00, int i01, int i02) throw();
	
	Buffer(double i00) throw();
	Buffer(double i00,
		   double i01, 
		   double i02 = INFINITY, 
		   double i03 = INFINITY, 
		   double i04 = INFINITY, 
		   double i05 = INFINITY, 
		   double i06 = INFINITY, 
		   double i07 = INFINITY, 
		   double i08 = INFINITY, 
		   double i09 = INFINITY, 
		   double i10 = INFINITY, 
		   double i11 = INFINITY, 
		   double i12 = INFINITY, 
		   double i13 = INFINITY, 
		   double i14 = INFINITY, 
		   double i15 = INFINITY, 
		   double i16 = INFINITY, 
		   double i17 = INFINITY, 
		   double i18 = INFINITY, 
		   double i19 = INFINITY, 
		   double i20 = INFINITY, 
		   double i21 = INFINITY, 
		   double i22 = INFINITY, 
		   double i23 = INFINITY, 
		   double i24 = INFINITY, 
		   double i25 = INFINITY, 
		   double i26 = INFINITY, 
		   double i27 = INFINITY, 
		   double i28 = INFINITY, 
		   double i29 = INFINITY, 
		   double i30 = INFINITY, 
		   double i31 = INFINITY) throw();
	
	Buffer(ValueArray const& array) throw();
	
	template<class NumericalType>
	Buffer(NumericalArray<NumericalType> const& array) throw()
	:	numChannels_(1),
		size_(array.size())
	{
		channels = new BufferChannelInternal*[numChannels_];
		channels[0] = new BufferChannelInternal(size_, false);
		for(int i = 0; i < size_; i++)
		{
			channels[0]->data[i] = (float)array[i];
		}
	}		
		
	template<class NumericalType>
	Buffer(NumericalArray2D<NumericalType> const& array) throw()
	:	numChannels_(array.size()),
		size_(array.numColumns())
	{
		channels = new BufferChannelInternal*[numChannels_];
		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			channels[channel] = new BufferChannelInternal(size_, false);
			const NumericalArray<NumericalType>& singleArray = array[channel];
			float* channelData = channels[channel]->data;
			for(int sample = 0; sample < size_; sample++)
			{
				channelData[sample] = (float)singleArray[sample];
			}
		}
	}		
	
	/** Create a single channel Buffer ramping from start to end. */
	static Buffer line(const int size, const double start, const double end) throw();
	
	/** Create a single channel Buffer containing an arithmetic series. */
	static Buffer series(const int size, const double start, const double grow) throw();
	
	/** Create a single channel Buffer containing a geometric series. */
	static Buffer geom(const int size, const double start, const double grow) throw();
	
	static Buffer newClear(const int size = 1, const int numChannels = 1) throw();
	static Buffer withSize(const int size = 1, const int numChannels = 1, bool zeroData = false) throw();

	static Buffer rand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	static Buffer rand2(const int size, const double positive, const int numChannels = 1) throw();
	static Buffer exprand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	static Buffer linrand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	
	static Buffer sineTable(const int size, const float repeats = 1.f) throw();
	static Buffer cosineTable(const int size, const float repeats = 1.f) throw();
	static Buffer cosineWindow(const int size, const float repeats = 1.f) throw();
	static Buffer triangleWindow(const int size) throw();
	static Buffer bartlettWindow(const int size) throw();
	static Buffer hannWindow(const int size) throw();
	static Buffer hammingWindow(const int size) throw();
	static Buffer blackmanWindow(const int size, const float alpha = 0.16f) throw();
	//static Buffer kaiserWindow(const int size, const float alpha = 3.0f) throw();
	
	/** Synthesise a Buffer using a UGen graph. 
	 Be very careful that none of the UGens in the graph 
	 are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.
	 @param size	The number of samples to generate.
	 @param graph	The audio graph to process to synthesise the audio Buffer.	*/
	static Buffer synth(const int size, UGen const& graph) throw();
	
	/** Synthesise into an exisiting Buffer using a UGen graph. 
	 Be very careful that none of the UGens in the graph 
	 are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.
	 @param graph	The audio graph to process to synthesise the audio Buffer.	*/	
	void synthInPlace(UGen const& graph) throw();
	
	Buffer(const int size, const float* sourceData) throw();
	Buffer(const int size, const int numChannels, const float** sourceDataArray) throw();
	Buffer(BufferChannelInternal *internalToUse) throw();
	
	Buffer(const char *audioFilePath) throw();
	Buffer(Text const& audioFilePath) throw();
	Buffer(Text const& audioFilePath, double& sampleRate) throw();
	bool write(Text const& audioFilePath, bool overwriteExisitingFile = false, int bitDepth = 24) throw();
	
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	Buffer(AudioSampleBuffer& audioSampleBuffer) throw();
	Buffer(String const& audioFilePath) throw();
	Buffer(String const& audioFilePath, double& sampleRate) throw();
	Buffer(const File& audioFile) throw();
	Buffer(const File& audioFile, double& sampleRate) throw();
	
	bool write(const File& audioFile, bool overwriteExisitingFile = false, int bitDepth = 24) throw();
	bool write(const File::SpecialLocationType directory, bool overwriteExisitingFile = false, int bitDepth = 24) throw();
	
protected:
	double initFromJuceFile(const File& audioFile) throw();
	bool initFromJuceFile(const File& audioFile, bool overwriteExisitingFile, int bitDepth) throw();
public:
#endif
#if defined(UGEN_IPHONE) || defined(DOXYGEN)
protected:
	double initFromAudioFile(const char* audioFilePath) throw();
	bool initFromAudioFileWav16(const char* audioFilePath, bool overwriteExisitingFile) throw();
	bool initFromAudioFileAiff16(const char* audioFilePath, bool overwriteExisitingFile) throw();
	bool initFromAudioFileWav24(const char* audioFilePath, bool overwriteExisitingFile) throw();
	bool initFromAudioFileAiff24(const char* audioFilePath, bool overwriteExisitingFile) throw();
	bool initFromAudioFileWav32(const char* audioFilePath, bool overwriteExisitingFile) throw();
	bool initFromAudioFileAiff32(const char* audioFilePath, bool overwriteExisitingFile) throw();


public:
#endif
	Buffer(Buffer const& channels0, Buffer const& channels1) throw();
	~Buffer() throw();
	
	Buffer(Buffer const& copy) throw();
	Buffer copy() const throw();
	Buffer& operator= (Buffer const& other) throw();

	bool operator== (Buffer const& other) const throw();
	bool operator!= (Buffer const& other) const throw();
	
	void incrementInternals() throw();	
	void decrementInternals() throw();	
	
	/// @} <!-- end Construction and destruction ------------------------------------------------------ -->

	
	/// @name Data access and manipulation
	/// @{
	
	Buffer interleave() throw();
	Buffer deinterleave(const int numChannels) throw();
	
	/** Convert a buffer to a 2D array of a specified type. */
	template<class NumericalType>
	inline NumericalArray2D<NumericalType> toArray() const throw()
	{
		NumericalArray2D<NumericalType> array2d;
		
		if(channels != 0)
		{
			for(int i = 0; i < numChannels_; i++)
			{
				NumericalArray<float> array = NumericalArray<float>::withArray(size_, channels[i]->data, false);
				array2d << NumericalArray<NumericalType>(array);
			}
		}
		
		return array2d;
	}
	
	template<class NumericalType>
	inline operator NumericalArray<NumericalType> () const throw()
	{
		return toArray<NumericalType>(0);
	}
	
	/** Convert one channel of the buffer to an array of a specified type */
	template<class NumericalType>
	inline NumericalArray<NumericalType> toArray(const int channel) const throw()
	{
		NumericalArray<NumericalType> array;
		
		if(channels != 0)
		{
			int channelToUse = channel;
			while(channelToUse < 0)
				channelToUse += numChannels_;
			
			NumericalArray<float> floatArray = NumericalArray<float>::withArray(size_, 
																				channels[channel % numChannels_]->data, 
																				false);
			array = floatArray;
		}
		
		return array;
	}
	
	inline float* getData(const int channel = 0) throw()								{ ugen_assert(channel >= 0); return channels[channel % numChannels_]->data; }
	inline const float* getData(const int channel = 0) const throw()					{ ugen_assert(channel >= 0); return channels[channel % numChannels_]->data; }
	inline float* getDataUnchecked(const int channel = 0) throw()						{ ugen_assert(channel >= 0); return channels[channel]->data; }
	inline const float* getDataUnchecked(const int channel = 0) const throw()			{ ugen_assert(channel >= 0); return channels[channel]->data; }
	
	inline int getNumChannels() const throw()											{ return numChannels_; }
	inline int size() const throw()														{ return size_; }
	double duration() const throw();
	inline int allocatedSize() const throw()											{ return channels[0]->allocatedSize; }
	inline void clear() throw()
	{
		if(channels != 0)
		{
			for(int i = 0; i < numChannels_; i++)
			{
				memset(getData(i), 0, size_ * sizeof(float));
			}
		}	
	}	
	
	inline void setSample(const int channel, const int sampleIndex, const float value) const throw()	
	{ 
		if(sampleIndex >= 0 && sampleIndex < size_) 
			channels[channel]->data[sampleIndex] = value;
	}
	
	inline float getSample(const int channel, const int sampleIndex) const throw()	
	{ 
		if(sampleIndex < 0 || sampleIndex >= size_) 
			return 0.f;
		else
			return channels[channel]->data[sampleIndex];
	}
	
	inline float getSample(const int channel, const double sampleIndex) const throw()	
	{ 
		if(sampleIndex < 0.0 || (int)sampleIndex >= size_) 
			return 0.f;
		else
			return channels[channel]->getSampleUnchecked(sampleIndex);
	}
	
	inline float getSample(const int channel, const float sampleIndex) const throw()	
	{ 
		if(sampleIndex < 0.0 || (int)sampleIndex >= size_) 
			return 0.f;
		else
			return channels[channel]->getSampleUnchecked(sampleIndex);
	}	
	
	inline void setSampleUnchecked(const int sampleIndex, const float value) const throw()	
	{ 
		ugen_assert(sampleIndex >= 0 && sampleIndex < size_);
		
		channels[0]->data[sampleIndex] = value;
	}
	
	inline void setSampleUnchecked(const int channel, const int sampleIndex, const float value) const throw()	
	{ 
		ugen_assert(channel >= 0 && channel < numChannels_);
		ugen_assert(sampleIndex >= 0 && sampleIndex < size_);
		
		channels[channel]->data[sampleIndex] = value;
	}
	
	inline float getSampleUnchecked(const int channel, const int sampleIndex) const throw()	
	{ 
		ugen_assert(channel >= 0 && channel < numChannels_);
		ugen_assert(sampleIndex >= 0 && sampleIndex < size_);

		return channels[channel]->data[sampleIndex];
	}
	
	inline float getSampleUnchecked(const int sampleIndex) const throw()	
	{ 
		ugen_assert(sampleIndex >= 0 && sampleIndex < size_);

		return channels[0]->data[sampleIndex];
	}
	
	inline float getSampleUnchecked(const int channel, const double index) const throw()
	{ 
		ugen_assert(channel >= 0 && channel < numChannels_);
		ugen_assert(index >= 0 && index < size_);

		return channels[channel]->getSampleUnchecked(index);	
	}
	
	inline float getSampleUnchecked(const int channel, const float index) const throw()
	{ 
		ugen_assert(channel >= 0 && channel < numChannels_);
		ugen_assert(index >= 0 && index < size_);
		
		return channels[channel]->getSampleUnchecked(index);	
	}	
	
	inline float lookup(const int channel, const double phase) const throw()
	{ 
		if(channel < 0 || channel >= numChannels_) return 0.f;
	
		return channels[channel]->getSampleUnchecked(ugen::clip(phase, 0.0, 1.0) * (size_-1));	
	}
	
	inline float lookup(const double phase) const throw()
	{ 
		return channels[0]->getSampleUnchecked(ugen::clip(phase, 0.0, 1.0) * (size_-1));	
	}
	
	float operator[] (const int sampleIndex) const throw()
	{
		if(sampleIndex < 0 || sampleIndex >= size_) 
		{
			//ugen_assertfalse;
			return 0.f;
		}
		else
			return channels[0]->data[sampleIndex];
	}
	
	Buffer operator[] (IntArray const& indices) const throw()
	{
		Buffer newBuffer(BufferSpec(indices.size(), numChannels_, false));
		
		const int numIndices = indices.size();
		for(int channel = 0; channel < numChannels_; channel++)
		{
			for(int i = 0; i < numIndices; i++)
			{
				const int index = indices.atUnchecked(i);
				newBuffer.setSample(channel, i, getSample(channel, index));
			}
		}
		
		return newBuffer;
	}
	
	float at(const int sampleIndex) const throw()
	{
		if(sampleIndex < 0 || sampleIndex >= size_) 
			return 0.f;
		else
			return channels[0]->data[sampleIndex];		
	}
	
	float at(const int channel, const int sampleIndex) const throw()
	{
		ugen_assert(channel >= 0 && channel < numChannels_);
		
		if(sampleIndex < 0 || sampleIndex >= size_) 
			return 0.f;
		else
			return channels[channel]->data[sampleIndex];		
	}
	
	Buffer at(IntArray const& indices) const throw()
	{
		Buffer newBuffer(BufferSpec(indices.size(), numChannels_, false));
		
		const int numIndices = indices.size();
		for(int channel = 0; channel < numChannels_; channel++)
		{
			for(int i = 0; i < numIndices; i++)
			{
				const int index = indices.atUnchecked(i);
				newBuffer.setSample(channel, i, getSample(channel, index));
			}
		}
		
		return newBuffer;
	}
	
	float wrapAt(int sampleIndex) const throw()
	{
		while(sampleIndex < 0) 
			sampleIndex += size_;
		
		return channels[0]->data[sampleIndex % size_];		
	}
	
	float wrapAt(const int channel, int sampleIndex) const throw()
	{		
		while(sampleIndex < 0) 
			sampleIndex += size_;
		
		return channels[channel]->data[sampleIndex % size_];		
	}
	
	inline unsigned int getCurrentWriteBlockID(const int channel) const throw()
	{
		ugen_assert((channel >= 0) && (channel < numChannels_)); 
		return channels[channel]->currentWriteBlockID;
	}
	
	inline void setCircularHead(const unsigned int blockID, const int channel, const int position) throw() 
	{ 
		ugen_assert((channel >= 0) && (channel < numChannels_)); 
		BufferChannelInternal* internal = channels[channel];
		internal->previousCircularHead = internal->circularHead;
		internal->circularHead = position; 
		internal->currentWriteBlockID = blockID;
	}
	
	inline int getCircularHead(const unsigned int blockID, const int channel) const throw() 
	{ 
		ugen_assert(channel >= 0); 
		BufferChannelInternal* internal = channels[channel % numChannels_];
		if(blockID == internal->currentWriteBlockID)
		{
			return internal->previousCircularHead;
		}
		else
		{
			return internal->circularHead;
		}
	}
	
	Buffer operator<< (Buffer const& other) const throw();
	Buffer operator, (Buffer const& other) const throw();
	Buffer append(Buffer const& other) const throw();
	
	//Buffer addChannels(Buffer const& newChannels) const throw();
	Buffer getChannel(const int channel) const throw();
	Buffer getRegion(const int startSample, const int endSample = -1, const int startChannel = -1, const int endChannel = -1) const throw();
	
	/** Reduce the apparent size of the Buffer without deallocating the extra memory.
	 This might seem like an unusual function but its main purpose is for
	 creating interpolatable wavetables for use with TableOsc. For example, one
	 way to may a sine table for use with TableOsc would be to do:
	 @code
		Buffer wt = Buffer::line(513, 0.0, 2.0 * pi); // create a Buffer with a ramp from 0-2pi over samples 0-512 (inclusive)
		wt = wt.sin(); // unary sine operation on the value in the Buffer, sample [0] and sample [512] should be the same
		wt.shrinkSize(1);  // shrink the apparent Buffer size by 1 so it appears to have only 512 samples (but
		                   // really it still has 513 allocated to it until desctruction)
		UGen osc = TableOsc::AR(wt, 440, 0.0, 0.3); // use the wavetable
	 @endcode
	 @see TableOsc */
	Buffer& shrinkSize(const int amount = 1) throw();
	float sum(const int channel) const throw();
	float sum() const throw();
	Buffer normalise() const throw();
	Buffer normaliseEach() const throw();	
	float findMaximum(const int channel = -1) const throw();
	float findMinimum(const int channel = -1) const throw();
	float findPeak(const int channel = -1) const throw();
	
	/** Blends this buffer with another.
	 If @c fraction is between 0 and 1 the two buffers are interpolated. (Where 0 is all 
	 ths buffer and 1 is all the other buffer.) If @c fraction is between 0 and -1 then
	 the buffers are blended using a "peak-hold" algorithm. That is if new values are greater
	 than the original values they are just assigned, if the new value is lower then the 
	 @c fraction is used to interpolate (here the absolute of the @c fraction argument is
	 used to put this into the range 0 and 1.)
	 
	 @see ScopeGUI */
	Buffer blend(Buffer const& other, double fraction) const throw();
	
	/** Create a new Buffer that has the start and end crossfaded for looping.
	 This is useful to use when loading data into a Buffer from an audio file for looping
	 using PlayBuf. The new buffer will be shorter that the original according to the fade 
	 time since this is used to make the cross fade. Curently the fade shape is linear.
	 @param fadeTime		The length of the crossfade.
	 @param fadeInShape		The shape for the fade in of the crossfade 
							(currently only EnvCurve::Linear is supported).
	 @param fadeOutShape	The shape for the fade in of the crossfade 
							(currently only EnvCurve::Linear is supported).
	 @return				The crossfaded Buffer. */
	Buffer loopFade(const float fadeTime = 0.1f,
					EnvCurve const& fadeInShape  = EnvCurve::Linear, 
					EnvCurve const& fadeOutShape = EnvCurve::Linear) const throw();
	
	Buffer resample(const int newSize) const throw();
	Buffer changeSampleRate(const double oldSampleRate, const double newSampleRate = 0.0) const throw();
	
	Buffer reverse() const throw();
	Buffer mix() const throw();
		
	/** Process this Buffer to a new Buffer through a UGen graph.
	 Be very careful that none of the UGens in either the input or graph 
	 parameters are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.
	 @param input	An AudioIn UGen which is at the top of the graph.
	 @param graph	The audio graph to process the audio with input at the top. */
	Buffer process(UGen const& input, UGen const& graph) const throw();
	
	/** Process this Buffer in-place through a UGen graph.
	 Be very careful that none of the UGens in either the input or graph 
	 parameters are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.	 
	 @param input	An AudioIn UGen which is at the top of the graph.
	 @param graph	The audio graph to process the audio with input at the top. */
	void processInPlace(UGen const& input, UGen const& graph) throw();
	
	/// @} <!-- end Data access and manipulation ------------------------------ -->
	
	/** @name Unary Ops.
	 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except mix() and unary operator- 
	 
	 These return a new Buffer with the unary operator applied to all channels.
	 */
	/// @{
	
	Buffer operator- () const throw();
	UnaryOpMethodsDeclare(Buffer);
	Buffer reciprocalExceptZero() const throw();
	
	/// @} <!-- end Unary Ops ------------------------------ -->
	
	/** @name Binary Ops. 
	 
	 The definition of these (except the assignment versions) is done by macros in ugen_BinaryOpUGens.cpp 
	 
	 These return a new Buffer with the binary operator applied to all channels. The resulting Buffer will
	 be the size of the largest of the two input Buffers, the smaller Buffer will be effectively padded with
	 zeroes. The resulting Buffer will have a number of channels equal to the highest number of channels in
	 the two input Buffers. When applying the binary operator to two Buffers which have a different number
	 of channels the smaller Buffer's channels will be accessed cyclically (i.e., using wrapped indices).
	 */
	/// @{
	BinaryOpMethodsDeclare(Buffer);
	
	Buffer& operator+= (Buffer const& other) throw();
	Buffer& operator-= (Buffer const& other) throw();
	Buffer& operator*= (Buffer const& other) throw();
	Buffer& operator/= (Buffer const& other) throw();
	Buffer& operator<<= (Buffer const& other) throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
	/// @name Miscellaneous
	/// @{
	
	bool isNull() const throw() { return size_ == 0 || numChannels_ == 0; }
	
//	static Buffer tableSine512;
//	static Buffer tableSine8192;
//	static Buffer tableCosine512;
//	static Buffer tableCosine8192;
//	static Buffer tableConstantPan512;
	
	static const Buffer& getTableSine512() throw();
	static const Buffer& getTableSine8192() throw();
	static const Buffer& getTableCosine512() throw();
	static const Buffer& getTableCosine8192() throw();
	static const Buffer& getTableConstantPan512() throw();
	
	/// @} <!-- end Miscellaneous ------------------------------------------------------------ -->

	
protected:
	int numChannels_;
	int size_;
	BufferChannelInternal** channels;
};

/** A buffer for storing complex value.
 
 This isn't finsihed and isn't currently used for anything in the library
 (it was designed when implementing the FFT and convolution stuff). */
class ComplexBuffer : public Buffer
{
public:
	
	/// @name Construction and Destruction. 
	/// @{
	
	ComplexBuffer(const int size, bool zeroData = false) throw();
	ComplexBuffer(const int size, const float* realSamples, const float* imagSamples) throw();
	ComplexBuffer(Buffer const& realBuffer) throw();
	~ComplexBuffer();
	
	/// @} <!-- end Construction and destruction ------------------------------------------------------ -->
	
	/// @name Data access and manipulation
	/// @{
	
	inline float* getDataReal() throw()	{ return channels[0]->data; }
	inline float* getDataImag() throw()	{ return channels[1]->data; }
	inline const float* getDataRealReadOnly() const throw()	{ return channels[0]->data; }
	inline const float* getDataImagReadOnly() const throw()	{ return channels[1]->data; }
	
	/// @} <!-- end Data access and manipulation ------------------------------ -->
	
	/// @name Binary Ops. 
	/// @{
	
	ComplexBuffer operator+ (ComplexBuffer const& rightOperand) const throw();
	ComplexBuffer operator- (ComplexBuffer const& rightOperand) const throw();
	ComplexBuffer operator* (ComplexBuffer const& rightOperand) const throw();
	//ComplexBuffer operator/ (ComplexBuffer const& rightOperand) const throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
protected:
};

class BufferReceiver;
typedef ObjectArray<BufferReceiver*> BufferReceiverArray;

/** Subclasses of this collect samples into a Buffer and transmit it to BufferReceiver objects.
 This mechanism is used by the ScopeGUI class and the Sender UGen to display time-domain sample data.
 The FFTSender UGen collects buffers and sends FFT-transformed buffers (e.g., to a ScopeGUI) for
 dealing with spectral data. But they can be used for other purposes too. */
class BufferSender
{
public:
	BufferSender() throw();
	virtual ~BufferSender();
	
	void addBufferReceiver(BufferReceiver* const receiver) throw();
	void removeBufferReceiver(BufferReceiver* const receiver) throw();
	void sendBuffer(Buffer const& buffer, const double value1 = 0.0, const int value2 = 0) throw();
	
private:
	BufferReceiverArray receivers;
};

/** Subclasses of this receive Buffer objects from BufferSender objects. */
class BufferReceiver
{
public:
	
	BufferReceiver() throw() {}
	virtual ~BufferReceiver() {}
	
	/** This must be implmented.
	 The two values allow additional data to be sent along with the Buffer. This is commonly
	 used to send a time offset, FFT start bin and the FFT size if appropriate. */
	virtual void handleBuffer(Buffer const& buffer, const double value1, const int value2) = 0;
	
	/** This saves having to get the pointer to a BufferReceiver object, it will be casted automatically. */
	operator BufferReceiver*() throw() { return this; }
};


#endif // _UGEN_ugen_Buffer_H_
