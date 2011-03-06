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

class CuePointInternal : public SmartPointer
{
public:
	CuePointInternal()
	:	cueID(-1), sampleOffset(-1)
	{
	}
	
	friend class CuePoint;
	
private:
	Text label;
	int cueID, sampleOffset;
};

/** Stores cue points or markers for audio files. */
class CuePoint : SmartPointerContainer<CuePointInternal>
{
public:
	CuePoint() 
	:	SmartPointerContainer<CuePointInternal> (new CuePointInternal())
	{
	}
	
	/** Human readable label for the marker. */
	Text& getLabel() { return getInternal()->label; }
	
	/** Human readable label for the marker. */
	const Text& getLabel() const { return getInternal()->label; }
	
	/** ID for the marker.
	 Can be zero for the WAV format but must be positive and non-zero for AIFF. */
	int& getID() { return getInternal()->cueID; }
	
	/** ID for the marker.
	 Can be zero for the WAV format but must be positive and non-zero for AIFF. */
	int getID() const { return getInternal()->cueID; }	
	
	/** The sample offset into the file of the marker. */
	int& getSampleOffset() { return getInternal()->sampleOffset; }
	
	/** The sample offset into the file of the marker. */
	int getSampleOffset() const { return getInternal()->sampleOffset; }
};

typedef ObjectArray<CuePoint> CuePointArray;

class MetaData
{
public:
	enum Type 
	{
		ReachedStart,
		ReachedEnd,
		CuePointInfo,
//		LoopPointInfo,
//		RegionInfo,
		
		Count
	};
	
	CuePointArray cuePoints;
};


class BufferChannelInternal : public SmartPointer
{
public:
	BufferChannelInternal(const unsigned int size, bool zeroData = false) throw();
	BufferChannelInternal(const unsigned int size, const unsigned int sourceDataSize, float* sourceData, const bool copyTheData) throw();
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
	
//	inline float getSampleUnchecked(const float fIndex) const throw()
//	{
//		const int iIndex0 = (int)fIndex;
//		const int iIndex1 = iIndex0+1;
//		const float* pValue0 = data + iIndex0;
//		const float* pValue1 = (iIndex1 >= (int)size_) ? data : pValue0+1;
//		const float value0 = *pValue0;
//		const float value1 = *pValue1;
//		return value0 + (fIndex - iIndex0) * (value1 - value0);
//	}
	
	
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
class BufferReceiver;

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
	
	/** Construct an empty Buffer. */
	Buffer() throw();
	
	/** Construct a Buffer using a BufferSpec. */
	Buffer(BufferSpec const& spec) throw();
	
	/** Constuct a single-channel Buffer containing one integer. */
	Buffer(int i00) throw();
	
	/** Constuct a single-channel Buffer containing two integers. */
	Buffer(int i00, int i01) throw();
	
	/** Constuct a single-channel Buffer containing three integers. */
	Buffer(int i00, int i01, int i02) throw();
	
	/** Constuct a single-channel Buffer containing one value. */
	Buffer(double i00) throw();
	
	/** Constuct a single-channel Buffer containing multiple values. 
	 For upto 32 values. */
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
	
	/** Constuct a single-channel Buffer from a ValueArray. */
	Buffer(ValueArray const& array) throw();
	
	/** Constuct a single-channel Buffer by copying the data from a NumericalArray (e.g., FloatArray). */
	template<class NumericalType>
	Buffer(NumericalArray<NumericalType> const& array) throw()
	:	numChannels_(1),
		size_(array.length())
	{
		channels = new BufferChannelInternal*[numChannels_];
		channels[0] = new BufferChannelInternal(size_, false);
		for(int i = 0; i < size_; i++)
		{
			channels[0]->data[i] = (float)array[i];
		}
	}	
	
	/** Constuct a single-channel Buffer by using the data from a NumericalArray<float> (i.e., FloatArray). 
	 Here there's an option to copy the data or just use the data from the original array directly. In the
	 latter case you must ensure that the original array exists for the same duration as this Buffer.*/	
	Buffer(NumericalArray<float>& array, const bool copyTheData = true) throw()
	:	numChannels_(1),
		size_(array.length())
	{
		channels = new BufferChannelInternal*[numChannels_];
		channels[0] = new BufferChannelInternal(size_, size_, array.getArray(), copyTheData);
	}		
		
	/** Constuct a multi-channel Buffer by copying the data from a NumericalArray2D (e.g., FloatArray2D). */
	template<class NumericalType>
	Buffer(NumericalArray2D<NumericalType> const& array) throw()
	:	numChannels_(array.length()),
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
	
	/** Constuct a multi-channel Buffer using the data from a NumericalArray2D<float> (i.e., FloatArray2D).
	 Here there's an option to copy the data or just use the data from the original array directly. In the
	 latter case you must ensure that the original array exists for the same duration as this Buffer. */
	Buffer(NumericalArray2D<float>& array, const bool copyTheData = true) throw()
	:	numChannels_(array.length()),
		size_(array.numColumns())
	{
		channels = new BufferChannelInternal*[numChannels_];
		
		for(int channel = 0; channel < numChannels_; channel++)
		{
			channels[channel] = new BufferChannelInternal(size_, size_, 
														  array[channel].getArray(), 
														  copyTheData);
		}
	}		
	
	
	/** Create a single channel Buffer ramping from start to end. */
	static Buffer line(const int size, const double start, const double end) throw();
	
	/** Create a single channel Buffer containing an arithmetic series. */
	static Buffer series(const int size, const double start, const double grow) throw();
	
	/** Create a single channel Buffer containing a geometric series. */
	static Buffer geom(const int size, const double start, const double grow) throw();
	
	/** Create a Buffer with a particular size and channel count defaulting to zero data. */
	static Buffer newClear(const int size = 1, const int numChannels = 1, bool zeroData = true) throw();
	
	/** Create a Buffer with a particular size and channel count defaulting to not zeroing the data. */
	static Buffer withSize(const int size = 1, const int numChannels = 1, bool zeroData = false) throw();

	/** Create a Buffer containing random numbers between lower and upper. */
	static Buffer rand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	
	/** Create a Buffer containing random numbers between -positive and positive. */
	static Buffer rand2(const int size, const double positive, const int numChannels = 1) throw();
	
	/** Create a Buffer containing random numbers exponentially distributed between lower and upper. */
	static Buffer exprand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	
	/** Create a Buffer containing random numbers linearly distributed between lower and upper. */
	static Buffer linrand(const int size, const double lower, const double upper, const int numChannels = 1) throw();
	
	/** Create a Buffer containing a certain number of cycles (repeats) of a sine wave (±1). */
	static Buffer sineTable(const int size, const float repeats = 1.f) throw();
	
	/** Create a Buffer containing a certain number of cycles (repeats) of a cosine wave (±1). */
	static Buffer cosineTable(const int size, const float repeats = 1.f) throw();
	
	/** Create a Buffer containing a certain number of cycles (repeats) of a cosine window (0 to 1). */
	static Buffer cosineWindow(const int size, const float repeats = 1.f) throw();
	
	/** Create a Buffer containing a triangle window. */
	static Buffer triangleWindow(const int size) throw();
	
	/** Create a Buffer containing a Bartlett window. */
	static Buffer bartlettWindow(const int size) throw();
	
	/** Create a Buffer containing a von Hann (Hanning) window. */
	static Buffer hannWindow(const int size) throw();
	
	/** Create a Buffer containing a Hamming window. */
	static Buffer hammingWindow(const int size) throw();
	
	/** Create a Buffer containing a Blackman window. */
	static Buffer blackmanWindow(const int size, const float alpha = 0.16f) throw();
	//static Buffer kaiserWindow(const int size, const float alpha = 3.0f) throw();
	
	/** Synthesise a Buffer using a UGen graph. 
	 Be very careful that none of the UGens in the graph 
	 are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.
	 @param size		The number of samples to generate.
	 @param graph		The audio graph to process to synthesise the audio Buffer. 
	 @param allAtOnce	If true the processing is done all in one go, if false it may yield the current thread. 	*/
	static Buffer synth(const int size, 
						UGen const& graph, 
						const bool allAtOnce = true) throw();
	
	/** Synthesise into an exisiting Buffer using a UGen graph. 
	 Be very careful that none of the UGens in the graph 
	 are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.
	 @param graph		The audio graph to process to synthesise the audio Buffer.	
	 @param offset		The start sample within the Buffer.
	 @param numSamples	The number of samples to process, 0 means all remaining samples. 
	 @param allAtOnce	If true the processing is done all in one go, if false it may yield the current thread.  */	
	void synthInPlace(UGen const& graph, 
					  const int offset = 0, 
					  const int numSamples = 0, 
					  const bool allAtOnce = true) throw();
	
	/** Process this Buffer and send it to a BufferReceiver.
	 This would be useful when run on a backgrond thread especially if the 
	 Buffer is large, since the processsing may take some time.
	 @param size		The number of samples to generate.
	 @param graph		The audio graph to process to synthesise the audio Buffer.
	 @param receiver	The BufferReceiver to recevie the Buffer when it's done.
	 @param bufferID	A number to pass to the third argument of handleBuffer() when the Buffer is sent. */
	static void synthAndSend(const int size, UGen const& graph, BufferReceiver* receiver, const int bufferID = 0) throw();
	
	/** Constuct a single-channel Buffer from data in a raw float array. 
	 Here there's an option to copy the data or just use the data from the original array directly. In the
	 latter case you must ensure that the original array exists for the same duration as this Buffer. */
	Buffer(const int size, float* sourceData, const bool copyTheData = true) throw();
	
	/** Constuct a multi-channel Buffer from data in a raw multi-dimensional float array. 
	 Here there's an option to copy the data or just use the data from the original array directly. In the
	 latter case you must ensure that the original array exists for the same duration as this Buffer. */
	Buffer(const int size, const int numChannels, float** sourceDataArray, const bool copyTheData = true) throw();
	
	/** Constuct a single-channel Buffer from another BufferChannelInternal. */
	Buffer(BufferChannelInternal *internalToUse) throw();
	
	/** Constuct a Buffer from an audio file on disk. 
	 Formats available are dependent on platform. */
	Buffer(const char *audioFilePath, int *bits = 0, double* sampleRate = 0, MetaData* metaData = 0) throw();
	
	/** Constuct a Buffer from an audio file on disk returning the sampleRate to the caller. 
	 Formats available are dependent on platform. */
	Buffer(Text const& audioFilePath, int *bits = 0, double* sampleRate = 0, MetaData* metaData = 0) throw();
	
	/** Write a Buffer to an audio file on disk. */
	bool write(Text const& audioFilePath, 
			   bool overwriteExisitingFile = false,
			   int bitDepth = 24,
			   MetaData const& metaData = MetaData()) throw();
	
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	/** Constuct a Buffer from a Juce AudioSampleBuffer. 
	 Here there's an option to copy the data or just use the data from the AudioSampleBuffer directly. In the
	 latter case you must ensure that the AudioSampleBuffer object exists for the same duration as this Buffer.*/	
	Buffer(AudioSampleBuffer& audioSampleBuffer, const bool copyTheData = true) throw();
		
	/** Constuct a Buffer from an audio file on disk given by a Juce String path optionally returning the sample rate, bit depth and meta data to the caller. */
	Buffer(String const& audioFilePath, double* sampleRate = 0, int* bits = 0, MetaData* metaData = 0) throw();
		
	/** Constuct a Buffer from an audio file on disk given by a Juce File optionally returning the sample rate, bit depth and meta data to the caller. */
	Buffer(const File& audioFile, double* sampleRate = 0, int* bits = 0, MetaData* metaData = 0) throw();
	
	/** Write a Buffer to a Juce File on disk. */
	bool write(const File& audioFile, 
			   bool overwriteExisitingFile = false, 
			   int bitDepth = 24, 
			   MetaData const& metaData = MetaData()) throw();
	
	/** Write a Buffer to a file in a special location on disk. */
	bool write(const File::SpecialLocationType directory, 
			   bool overwriteExisitingFile = false, 
			   int bitDepth = 24,
			   MetaData const& metaData = MetaData()) throw();
	
protected:
	double initFromJuceFile(const File& audioFile, int *bits = 0, MetaData* metaData = 0) throw();
	bool initFromJuceFile(const File& audioFile, 
						  bool overwriteExisitingFile, 
						  int bitDepth,
						  MetaData const& metaData) throw();
public:
#endif
#if defined(UGEN_IPHONE) || defined(DOXYGEN)
protected:
	double initFromAudioFile(const char* audioFilePath, int *bits = 0, MetaData* metaData = 0) throw();
	bool initFromAudioFileWav16(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();
	bool initFromAudioFileAiff16(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();
	bool initFromAudioFileWav24(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();
	bool initFromAudioFileAiff24(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();
	bool initFromAudioFileWav32(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();
	bool initFromAudioFileAiff32(const char* audioFilePath, bool overwriteExisitingFile, MetaData const& metaData = MetaData()) throw();


public:
#endif
	/** Constuct a Buffer from two other buffers by combining the channels. 
	 Here the result will be the size of the largest input Buffer. The
	 number of channels will be the sum of the channels in the two input
	 Buffers.*/
	Buffer(Buffer const& channels0, Buffer const& channels1) throw();
	~Buffer() throw();
	
	/** Copy constructor.
	 As this uses reference counted pointers internally this just copies the 
	 pointer(s) and increments the reference count(s). */
	Buffer(Buffer const& copy) throw();
	
	/** Make a deep copy of this Buffer. 
	 This always makes a deep copy of the raw data to which the Buffer refers.
	 This also applies to data which refers to another source. */
	Buffer copy() const throw();
	
	/** Assignment. */
	Buffer& operator= (Buffer const& other) throw();
	
	/** Make this Buffer refer to other data.
	 With a sourceSize of 0 the current size of the Buffer will be retained */
	void referTo(const int channel, float* data, const int sourceSize = 0) throw();
	
	/** Make this Buffer refer to other data.
	 With a sourceSize of 0 the current size of the Buffer will be retained */
	void referTo(const int numChannels, float** channelData, const int sourceSize = 0) throw();

	/** Make this Buffer refer to another Buffer. */
	void referTo(Buffer const& other, const int offset = 0, const int numSamples = 0) throw();
	
	
	/** Returns true if this and the other Buffer refer to identical internal objects.
	 This does NOT compare the data so. For example:
	 @code
		Buffer a = Buffer(0.0, 0.5, 1.0);
		Buffer b = a;
		Buffer c = a.copy();
		bool test1 = (a == b); // will be true
		bool test2 = (a == c); // will be false
	 @endcode */
	bool operator== (Buffer const& other) const throw();
	
	/** Returns true if this and the other Buffer do not refer to identical internal objects. */
	bool operator!= (Buffer const& other) const throw();
	
	/** @internal */
	void incrementInternals() throw();	
	
	/** @internal */
	void decrementInternals() throw();	
	
	/// @} <!-- end Construction and destruction ------------------------------------------------------ -->

	
	/// @name Data access and manipulation
	/// @{
	
	/** Returns a single channel Buffer with the channels of this Buffer interleaved. */
	Buffer interleave() throw();
	
	/** Returns a multi-channel Buffer from a channel-interleaved Buffer. */
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
	
	/** Convert the first channel of the buffer to an array of a specified type */
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
	
	/** Copy data from a source Buffer into this Buffer.
	 This does not increase the size of this Buffer so only data up to
	 this Buffer's size will be copied. The same applies to channels in that
	 only the number of channels this buffer has will be copied from the source. 
	 @param source The source Buffer. */
	void copyFrom(Buffer const& source) throw();
	
	/** Copy data from a source Buffer into this Buffer.
	 This does not increase the size of this Buffer so only data up to
	 this Buffer's size will be copied. The same applies to channels in that
	 only the number of channels this buffer has will be copied from the source. 
	 @param source The source Buffer.
	 @param sourceOffset The number of samples into the source Buffer to start copying form.
	 @param destOffset The number of samples into this Buffer to start copying to.
	 @param numSamples The number of samples to copy. */	
	void copyFrom(Buffer const& source, const int sourceOffset, const int destOffset, const int numSamples) throw();
	
	/** Return a pointer to the floating point data for a particular channel of this Buffer.
	 Channel indices are wrapped when the number of channels in the Buffer is exceed (so
	 channel index 2 in a 2-channel buffer would return data for channel 0). */
	inline float* getData(const int channel = 0) throw()								{ ugen_assert(channel >= 0); return channels[channel % numChannels_]->data; }
	
	/** Return a pointer to the floating point data for a particular channel of this Buffer.
	 Channel indices are wrapped when the number of channels in the Buffer is exceed (so
	 channel index 2 in a 2-channel buffer would return data for channel 0). */	
	inline const float* getData(const int channel = 0) const throw()					{ ugen_assert(channel >= 0); return channels[channel % numChannels_]->data; }
	
	/** Return a pointer to the floating point data for a particular channel of this Buffer.
	 Channel indices MUST be in range. */	
	inline float* getDataUnchecked(const int channel = 0) throw()						{ ugen_assert(channel >= 0); return channels[channel]->data; }
	
	/** Return a pointer to the floating point data for a particular channel of this Buffer.
	 Channel indices MUST be in range. */	
	inline const float* getDataUnchecked(const int channel = 0) const throw()			{ ugen_assert(channel >= 0); return channels[channel]->data; }
	
	/** Return the number of channels in this Buffer. */
	inline int getNumChannels() const throw()											{ return numChannels_; }
	
	/** Return the size in samples of each channel in this Buffer.
	 i.e., the number of sample 'frames'. */
	inline int size() const throw()														{ return size_; }
	
	/** Return the duration of this Buffer in seconds at the current sample rate. */
	double duration() const throw();
	
	inline int allocatedSize() const throw()											{ return channels[0]->allocatedSize; }
	
	/** Set all samples in all channels to zero. */
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
	
	/** Concatenate one Buffer with another and return a new Buffer. */
	Buffer append(Buffer const& other) const throw();
	
	//Buffer addChannels(Buffer const& newChannels) const throw();
	
	/** Return a Buffer containing only one channel of this Buffer. */
	Buffer getChannel(const int channel) const throw();
	
	/** Get a partiuclar region of this Buffer.
	 @param startSample		The index of sample in this Buffer that will be the first sample in the new Buffer.
	 @param endSample		The index of sample in this Buffer that will be the last sample in the new Buffer. 
							If this is -1 the all the samples to the end of the Buffer are used.
	 @param startChannel	The index of channel in this Buffer that will be the first channel in the new Buffer.
							-1 for default behaviour.
	 @param endChannel		The index of channel in this Buffer that will be the last channel in the new Buffer.
							-1 for default behaviour (all remaining channels).
	 @return A new Buffer containing a region of this Buffer in terms of sample positions and channels. */
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
	
	/** Return the sum of all samples in a particular channel. */
	float sum(const int channel) const throw();
	
	/** Return the sum of all samples in all channels. */
	float sum() const throw();
	
	/** Return a new Buffer which is this Buffer normalised based on the maximum amplitude across all channels. */
	Buffer normalise() const throw();
	
	/** Return a new Buffer which is this Buffer normalised when each channel is normalised separately. */
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
	 @code
		Buffer sound("test.wav");
		UGen audioin = AudioIn::AR(sound.getNumChannels());
		UGen filter = BLowPass::AR(audioin, 1000, 1);
		Buffer filteredBuffer = sound.process(audioin, filter);
	 @endcode
	 @param input		An AudioIn UGen which is at the top of the graph.
	 @param graph		The audio graph to process the audio with input at the top. 
	 @param offset		The start sample within the Buffer.
	 @param numSamples	The number of samples to process, 0 means all remaining samples. 
	 @param allAtOnce	If true the processing is done all in one go, if false it may yield the current thread. */
	Buffer process(UGen const& input, 
				   UGen const& graph, 
				   const int offset = 0, 
				   const int numSamples = 0, 
				   const bool allAtOnce = true) const throw();
	
	/** Process this Buffer in-place through a UGen graph.
	 Be very careful that none of the UGens in either the input or graph 
	 parameters are used elsewhere in another context e.g., for real-time use.
	 This graph must be completely separate from any other graphs otherwise
	 unusual results may be experienced.	 
	 @code
		Buffer sound("test.wav");
		UGen audioin = AudioIn::AR(sound.getNumChannels());
		UGen filter = BLowPass::AR(audioin, 1000, 1);
		sound.processInPlace(audioin, filter);
	 @endcode	 
	 @param input	An AudioIn UGen which is at the top of the graph.
	 @param graph	The audio graph to process the audio with input at the top.
	 @param offset		The start sample within the Buffer.
	 @param numSamples	The number of samples to process, 0 means all remaining samples. 
	 @param allAtOnce	If true the processing is done all in one go, if false it may yield the current thread.  */
	void processInPlace(UGen const& input, 
						UGen const& graph, 
						const int offset = 0, 
						const int numSamples = 0, 
						const bool allAtOnce = true) throw();
		
	/** Process this Buffer and send it to a BufferReceiver.
	 This would be useful when run on a backgrond thread especially if the 
	 Buffer is large, since the processsing may take some time.
	 @param input		An AudioIn UGen which is at the top of the graph.
	 @param graph		The audio graph to process the audio with input at the top. 
	 @param receiver	The BufferReceiver to recevie the Buffer when it's done.
	 @param bufferID	A number to pass to the third argument of handleBuffer() when the Buffer is sent. 
	 @param offset		The start sample within the Buffer.
	 @param numSamples	The number of samples to process, 0 means all remaining samples. */
	void processAndSend(UGen const& input, 
						UGen const& graph, 
						BufferReceiver* receiver, 
						const int bufferID = 0,
						const int offset = 0, 
						const int numSamples = 0) const throw();
	
	/// @} <!-- end Data access and manipulation ------------------------------ -->
	
	/** @name Unary Ops.
	 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except mix() and unary operator- 
	 
	 These return a new Buffer with the unary operator applied to all channels.
	 */
	/// @{
	
	Buffer operator- () const throw();
	UnaryOpMethodsDeclare(Buffer);
	Buffer reciprocalExceptZero() const throw();
	
	template<UnaryOpFunction op>
	Buffer unary() const throw()																				
	{																														
		Buffer newBuffer(BufferSpec(size_, numChannels_, false));															
		
		for(int channelIndex = 0; channelIndex < numChannels_; channelIndex++)												
		{																													
			int numSamples = size_;																						
			float* inputSamples = channels[channelIndex]->data;																
			float* outputSamples = newBuffer.channels[channelIndex]->data;													
			
			while(--numSamples >= 0) {																						
				*outputSamples++ = op(*inputSamples++);												
			}																											
		}																													
		
		return newBuffer;																									
	}
	
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
	
	/** Templated version of the binary operator functions. */
	template<BinaryOpFunction op>
	Buffer binary(Buffer rightOperand) throw()
	{
		const int newNumChannels = numChannels_ > rightOperand.numChannels_ ? numChannels_ : rightOperand.numChannels_;	
		const int maxSize = size_ > rightOperand.size_ ? size_ : rightOperand.size_;									
		const int minSize = size_ < rightOperand.size_ ? size_ : rightOperand.size_;									
		const int diffSize = maxSize-minSize;																			
		
		Buffer newBuffer(BufferSpec(maxSize, newNumChannels, false));													
		
		for(int channelIndex = 0; channelIndex < newNumChannels; channelIndex++)										
		{																												
			int numSamples;																								
			float* leftSamples = channels[channelIndex % numChannels_]->data;											
			float* rightSamples = rightOperand.channels[channelIndex % rightOperand.numChannels_]->data;				
			float* outputSamples = newBuffer.channels[channelIndex]->data;												
			
			if(size_ == 1 && numChannels_ == 1) {																		
				numSamples = maxSize;																					
				
				while(--numSamples >= 0) {																				
					*outputSamples++ = op(*leftSamples, *rightSamples++);						
				}																										
			} else if(rightOperand.size_ == 1 && rightOperand.numChannels_ == 1) {									
				numSamples = maxSize;																					
				
				while(--numSamples >= 0) {																				
					*outputSamples++ = op(*leftSamples++, *rightSamples);						
				}																										
			} else {																									
				numSamples = minSize;																					
				while(--numSamples >= 0) {																				
					*outputSamples++ = op(*leftSamples++, *rightSamples++);						
				}																										
				
				numSamples = diffSize;																					
				if(size_ > rightOperand.size_) {																		
					while(--numSamples >= 0) {																			
						*outputSamples++ = op(*leftSamples++, 0.f);								
					}																									
				}																										
				else if(rightOperand.size_ > size_) {																	
					while(--numSamples >= 0) {																			
						*outputSamples++ = op(0.f, *rightSamples++);								
					}																									
				}																										
			}																											
		}																												
		
		return newBuffer;																								
	}
	
	Buffer& operator+= (Buffer const& other) throw();
	Buffer& operator-= (Buffer const& other) throw();
	Buffer& operator*= (Buffer const& other) throw();
	Buffer& operator/= (Buffer const& other) throw();
	Buffer& operator<<= (Buffer const& other) throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
	/// @name Miscellaneous
	/// @{
	
	bool isNull() const throw() { return size_ == 0 || numChannels_ == 0; }
		
	static const Buffer& getTableSine512() throw();			///< A 512-point sine table.
	static const Buffer& getTableSine8192() throw();		///< A 8192-point sine table.
	static const Buffer& getTableCosine512() throw();		///< A 512-point cosine table.
	static const Buffer& getTableCosine8192() throw();		///< A 8192-point cosine table.
	static const Buffer& getTableConstantPan512() throw();	///< A 512-point pan table for 2-channel constant-power panning.
	
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
class BufferSender;
typedef ObjectArray<BufferReceiver*> BufferReceiverArray;
typedef ObjectArray<BufferSender*> BufferSenderArray;


/** Subclasses of this collect samples into a Buffer and transmit it to BufferReceiver objects.
 This mechanism is used by the ScopeGUI class and the Sender UGen to display time-domain sample data.
 The FFTSender UGen collects buffers and sends FFT-transformed buffers (e.g., to a ScopeGUI) for
 dealing with spectral data. But they can be used for other purposes too. */
class BufferSender
{
public:
	BufferSender() throw();
	virtual ~BufferSender();
	
	void addBufferReceiver(BufferReceiver* receiver) throw();
	void removeBufferReceiver(BufferReceiver* receiver) throw();
	void sendBuffer(Buffer const& buffer, const double value1 = 0.0, const int value2 = 0) throw();
	
protected:
	BufferReceiverArray receivers;
};

/** Subclasses of this receive Buffer objects from BufferSender objects. */
class BufferReceiver
{
public:
	
	BufferReceiver() throw();
	virtual ~BufferReceiver();
	
	/** This must be implmented.
	 The two values allow additional data to be sent along with the Buffer. This is commonly
	 used to send a time offset, FFT start bin and the FFT size if appropriate. */
	virtual void handleBuffer(Buffer const& buffer, const double value1, const int value2) = 0;
	
	/** This saves having to get the pointer to a BufferReceiver object, it will be casted automatically. */
	operator BufferReceiver*() throw() { return this; }
		
	friend class BufferSender;
	
protected:
	void addBufferSender(BufferSender* const sender) throw();
	void removeBufferSender(BufferSender* const sender) throw();

	BufferSenderArray senders;
};


#endif // _UGEN_ugen_Buffer_H_
	