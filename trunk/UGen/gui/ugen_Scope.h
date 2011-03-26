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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_Scope_H_
#define _UGEN_ugen_Scope_H_

#include "../core/ugen_UGen.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../fft/ugen_FFTEngine.h"
#include "../core/ugen_Text.h"
#include "ugen_GUI.h"
	
/** This is a wrapper/controller for a platform dependent GUI object.
 
 In Juce the ScopeComponent inherits from ScopeGUI. On the iPhone ScopeView inherits 
 from ScopeGUI and is in a peer relationship with a UIScopeView (which is a UIView).
 
 @ingroup GUITools */
class ScopeGUI :	public Lock,
					public BufferReceiver
{
public:
	/** An enum to control the display style of the scope trace. */
	enum ScopeStyles
	{
		Lines,	///< The trace is drawn as a line when zoomed in far enough.
		Bars,	///< The trace is drawn as small bars from the peak to the zero line, i.e., filled.
		NumScopeStyles
	};
	
	/** Constructor. */
	ScopeGUI(const ScopeStyles style = Lines);
		
	/** An enum for x/y label scales in various formats. */
	enum Scales
	{
		LabelInvalid = -1,
		LabelXNone = 0,			///< No labelling of the x axis.
		LabelXMarks,		///< Only marks are drawn on the x axis.
		LabelXSamples,		///< The x axis is labelled with marks and units in samples.
		LabelXTime,			///< The x axis is labelled with marks and units in hours, mins, secs and ms
		LabelXFrequency,	///< The x axis is labelled with marks and units in frequency (based on FFT bins)
		LabelYNone,			///< No labelling of the y axis.
		LabelYMarks,		///< Only marks are drawn on the y axis.
		LabelYAmplitude,	///< The y axis is labelled with marks and units in amplitude.
		LabelYPercent,		///< The y axis is labelled with marks and units as a percentage amplitude 1.0
		LabelYDecibels		///< not yet...
	};
	
	/** Sets whether the scope is unipolar or bipolar.
	 If the argument is true it is bibolar i.e., above and below zero upto +-yMaximum. Otherwise
	 it is unipolar i.e., from 0 to yMaximum.
	 @see setYMaximum() */
	void setPolarity(const bool isBipolar) throw();
	
	/** Gets the current unipolar/bipolar state. */
	bool getPolarity() const throw() { return isBipolar; }
	
	/** Set the maximum value on the y axis. */
	void setYMaximum(const float maximum) throw();
	
	/** Get the current maximum value of the y axis scale. */
	float getYMaximum() const throw() { return yMaximum; }
	
	/** Set the lower margin below the scope display.
	 For rectangular scopes it's probably easier to change the size of the scope. This
	 is more useful for radial scopes such that there is a gap between the centre of the circle
	 and the scope image. */
	void setLowerMargin(const float margin) throw();
	
	/** Get the current lower margin value */
	float getLowerMargin() const throw() { return lowerMargin; }
	
	/** Set the labelling of the scale on the x axis.
	 @param scale			LabelXNone, LabelXMarks, LabelXSamples, LabelXTime or LabelXFrequency
	 @param markSpacing		In samples, how often to draw a mark on the x axis.
	 @param labelHop		How often to label a mark with a unit value. 
							These marks are also drawn 1.5 times longer than the other marks. 
	 @param labelFirst		If @c true the first marking will be labelled, if @c false it won't
							(setting this to false can help prevent the crowding of text around the
							origin where the two axes meet).*/
	void setScaleX(Scales scale, const double markSpacing = -1, const int labelHop = -1, const bool labelFirst = true) throw();
	Scales getScaleX() const throw() { return scaleX; }
	double getMarkSpacingX() const throw() { return markSpacingX; }
	int getLabelHopX() const throw() { return labelHopX; }
	bool getLabelFirstX() const throw() { return labelFirstX; }
	
	/** Set the labelling of the scale on the x axis.
	 @param scale			LabelYNone, LabelYMarks, LabelYAmplitude or LabelYDecibels
	 @param markSpacing		In pixels, how often to draw a mark on the y axis.
	 @param labelHop		How often to label a mark with a unit value. 
							These marks are also drawn 1.5 times longer than the other marks. 
	 @param labelFirst		If @c true the first marking will be labelled, if @c false it won't
							(setting this to false can help prevent the crowding of text around the
							origin where the two axes meet).*/	
	void setScaleY(Scales scale, const double markSpacing = -1, const int labelHop = -1, const int decimalPlaces = -1) throw();
	Scales getScaleY() const throw() { return scaleY; }
	double getMarkSpacingY() const throw() { return markSpacingY; }
	int getLabelHopY() const throw() { return labelHopY; }
	int getDecimalPlacesY() const throw() { return decimalPlacesY; }
	
	void initBuffers() throw();
	
	/* Set the currently displayed buffer. 
	 @param audioBufferToUse The buffer (any number of channels which may change on subsequent calls).
	 @param offset For time domain buffers this is a sample offset, for FFT-based buffers this is the first bin index.
	 @param fftSize The FFT size (for FFT-based buffers only). */
	void setAudioBuffer(Buffer const& audioBufferToUse, const double offset = 0.0, const int fftSize = -1) throw();
	const Buffer& getAudioBuffer() const throw() { return audioBuffer; }
	
	void handleBuffer(Buffer const& buffer, const double offset = 0.0, const int fftSize = -1) throw();
	
	void setWrap(const double amount) throw();
	double getWrap() throw() { return bufferWrap; }
	
	void setSmoothing(Buffer const& smooth) throw() { smoothingBuffer = smooth; }
	const Buffer& getSmoothing() const throw() { return smoothingBuffer; }
	
	/** An enum for the various colours used by the scope. Use these as indices to the @c colours array member. */
	enum ScopeColours { Background, TopLine, ZeroLine, LabelMarks, TextX, TextY, TextChannel, Trace, NumScopeColours };
	void setScopeColour(const ScopeColours which, RGBAColour const& colour) throw();
	const RGBAColour& getScopeColour(const ScopeColours which) const throw();
	
	/** Set the height of the marks on the X axis. Note that marks every hopsize will be 1.5 times larger. */
	void setMarkXHeight(const int size) throw();
	/** Set the width of the marks on the Y axis. Note that marks every hopsize will be 1.5 times larger. */
	void setMarkYWidth(const int size) throw();
	/** Set the length of the marks on the X and Y axis. Note that marks every hopsize will be 1.5 times larger. */
	void setMarkSizes(const int size) throw();
	int getMarkXHeight() const throw() { return markXHeight; }
	int getMarkYWidth() const throw()  { return markYWidth;  }
	
	/** Set the font size of the labels on the X axis. */
	void setTextSizeX(const float size) throw();
	float getTextSizeX() const throw() { return textSizeX; }
	/** Set the font size of the labels on the Y axis. */
	void setTextSizeY(const float size) throw();
	float getTextSizeY() const throw() { return textSizeY; }
	/** Set the font size of the channel labels. */
	void setTextSizeChannel(const float size) throw();
	float getTextSizeChannel() const throw() { return textSizeChannel; }
	/** Set all the font sizes. */
	void setTextSizes(const float size) throw();
	
	void setLabelChannelsState(const bool state) throw() { labelChannels = state; }
	bool getLabelChannelsState() const throw() { return labelChannels; }
	
	/** Set the formatting strings for the channel labels.
	 Using printf-style formatting: a single int is passed to the string.
	 The default is "Ch%d" so channel 0 displays as "Ch0". */
	void setChannelLabels(TextArray const& labels, const int offset = 0) throw();
	const TextArray& getChannelLabels() const throw() { return channelLabels; }
	int getChannelLabelOffset() const throw() { return channelLabelOffset; }
	
	virtual int getDisplayBufferSize() const = 0;
	//virtual int getHeight() const = 0;
	virtual void updateGUI() = 0;
	virtual bool isValid() = 0;
	
	inline double getSampleOffset() throw() { return offsetSamples; }
	
protected:
	bool isBipolar:1;
	bool hasDisplayed:1;
	float yMaximum;
	float lowerMargin;
	
	ScopeStyles style_;
	Scales scaleX, scaleY;
	
	bool labelFirstX;
	
	double markSpacingY;
	int decimalPlacesY;
	unsigned int labelHopY;
	double markSpacingX;
	unsigned int labelHopX;
	int markXHeight, markYWidth;
	
	double offsetSamples;
	int fftSize;
	
	Buffer minDrawBuffer;
	Buffer maxDrawBuffer;
	Buffer audioBuffer;
	Buffer smoothingBuffer;
	double bufferWrap;
		
	RGBAColour colours[NumScopeColours];
	
	float textSizeX, textSizeY, textSizeChannel;
	bool labelChannels;
	TextArray channelLabels;
	int channelLabelOffset;
	
	void calculateBuffers();
	void resizedGUI();
};

//class ScopeMetaDataGUI
//{
//public:
//	
//private:
//	
//};

//#define ScopeGUIPtrPtr void*

///** @ingroup UGenInternals GUITools */
//class ScopeUGenInternal :	public UGenInternal
//{
//public:
//	ScopeUGenInternal(ScopeGUIPtrPtr scopeGUI, UGen const& input, UGen const& duration) throw();	
//	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
//	
//	enum Inputs { Input, Duration, NumInputs };
//	
//private:
//	ScopeGUIPtrPtr scopeGUIref;
//	Buffer audioBuffer;
//	int bufferIndex;
//	int audioBufferSizeUsed;
//	unsigned int samplesProcessed;
//};
//
///** Scope UGen for use with a ScopeGUI.
// @ingroup GUITools */
//UGenSublcassDeclarationNoDefault(Scope, 
//								 (scopeGUI, input, duration), 
//								 (ScopeGUIPtrPtr scopeGUI, UGen const& input, UGen const& duration = 0.1), COMMON_UGEN_DOCS);


class BufferSenderUGenInternal :	public UGenInternal,
									public BufferSender
{
public:
	BufferSenderUGenInternal(UGen const& input, UGen const& duration) throw();	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Duration, NumInputs };
	
private:
	Buffer audioBuffer;
	int bufferIndex;
	int audioBufferSizeUsed;
	unsigned int samplesProcessed;
};

/** Collects samples and sends them as a Buffer to one or more receivers.
 This can be used to send data to a oscilloscope (e.g., ScopeComponent) or
 for other purposes (e.g., analysis). 
 @see FFTSender */
UGenSublcassDeclarationNoDefault(Sender, 
								 (input, duration), 
								 (UGen const& input, UGen const& duration = 0.1), COMMON_UGEN_DOCS);

///** @ingroup UGenInternals GUITools */
//class SpectralScopeUGenInternal :	public UGenInternal // this SHOULDN'T be a ProxyOwner so that
//{														// the number channels can be dynamic!
//public:
//	SpectralScopeUGenInternal(ScopeGUIPtrPtr scopeGUI, 
//							  UGen const& input, 
//							  FFTEngine::FFTModes mode,
//							  FFTEngine const& fft,
//							  const int overlap,
//							  const int firstBin,
//							  const int numBins) throw();	
//	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
//	
//	enum Inputs { Input, NumInputs };
//	
//private:
//	ScopeGUIPtrPtr scopeGUIref;
//	FFTEngine::FFTModes mode_;
//	FFTEngine fftEngine;
//	const int fftSize, fftSizeHalved, overlap_, firstBin_, maxNumBins, numBins_;
//	Buffer inputBuffer, outputBuffer;
//	int bufferIndex;
//};
//
///** Spectral scope UGen for use with a ScopeGUI.
// @ingroup GUITools FFTUGens*/
//UGenSublcassDeclarationNoDefault(SpectralScope, 
//								 (scopeGUI, input, mode, fft, overlap, firstBin, numBins), 
//								 (ScopeGUIPtrPtr scopeGUI, 
//								  UGen const& input, 
//								  FFTEngine::FFTModes mode,
//								  FFTEngine const& fft = 1024, 
//								  const int overlap = 1,
//								  const int firstBin = 1,
//								  const int numBins = 0), COMMON_UGEN_DOCS);


class FFTSenderUGenInternal :	public UGenInternal, // this SHOULDN'T be a ProxyOwner so that
								public BufferSender	 // the number channels can be dynamic!
{
public:
	FFTSenderUGenInternal(UGen const& input, 
						  FFTEngine::FFTModes mode,
						  FFTEngine const& fft,
						  const int overlap,
						  const int firstBin,
						  const int numBins) throw();	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs };
	
private:
	FFTEngine::FFTModes mode_;
	FFTEngine fftEngine;
	const int fftSize, fftSizeHalved, overlap_, firstBin_, maxNumBins, numBins_;
	Buffer inputBuffer, outputBuffer;
	int bufferIndex;
};

/** Collects samples and performs and FFT before sending to one or more receivers.
 @param input	The audio input to apply the FFT to.
 @param mode	The data can be cooked in various ways using one of the FFTModes before it's sent. 
				E.g., FFTEngine::Magnitude will just calculate the bin magnitudes. Using mode 
				FFTEngine::RealImagRaw will use less allocation on the audio thread so you could
				do the transformation.
 @param fft		The FFT size.
 @param overlap	The overlap factor for successive FFT frames.
 @param firstBin	The first bin reported in the cooked data sent (not used for modes
					FFTEngine::RealImagRaw or FFTEngine::RealImagRawSplit). Default is 1
					where bin 0 is the DC bin.
 @param numBins		The number of bins reported in the cooked data sent (not used for modes
					FFTEngine::RealImagRaw or FFTEngine::RealImagRawSplit). If firstBin is
					0 then the maximum number of bins is one greater than half the FFT size.
					Here the final bin would be the Nyquist bin. Default is 0 which means
					all bins from firstBin upwards will be sent.
 @see Sender, FFTEngine */
UGenSublcassDeclarationNoDefault(FFTSender, 
								 (input, mode, fft, overlap, firstBin, numBins), 
								 (UGen const& input, 
								  FFTEngine::FFTModes mode,
								  FFTEngine const& fft = 1024, 
								  const int overlap = 1,
								  const int firstBin = 1,
								  const int numBins = 0), COMMON_UGEN_DOCS);




#endif // _UGEN_ugen_Scope_H_
