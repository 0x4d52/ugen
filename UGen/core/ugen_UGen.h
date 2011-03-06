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

#ifndef UGEN_UGEN_H
#define UGEN_UGEN_H


#include "ugen_Constants.h"
#include "ugen_UGenInternal.h"
#include "ugen_Deleter.h"
#include "../buffers/ugen_Buffer.h"
#include "ugen_Random.h"
#include "ugen_Arrays.h"



#define DEFAULT_UGEN_DOCS	<I>Not yet documented although the argument name should be reasonably clear</I>.
#define COMMON_UGEN_DOCS	<B>Generic %UGen argument documentation:</B>										\
							Arguments which are UGen type can be continuously controlled and these				\
							may generally be multichannel (although there are some exceptions).					\
							Arguments which are constant/literal values (e.g., float or int) are clearly		\
							not continuously controllable and not multichannel (therefore will					\
							generallly be applied identically to each channel should the UGen being				\
							created is made multichannel via other arguments.									\
							Arguments which are Buffer type may require a Buffer for some specific				\
							purpose (e.g, TableOsc uses a buffer for its wavetable) but single channel			\
							Buffer objects are also used to specify an array of literal values (e.g., LLine)	\
							\n\
							\n\

/** Macro for declaring a simple UGen.
 
 Use this version if all the arguments have defaults so that no default constructor is declared. It declares
 a constructor and static AR() and KR() functions. (And the SC compatabilty methods ar(), kr() and operator().)
 This macro should be used in a header file, only the constructor itself will need defining in the .cpp file.
 
 E.g.,
 @code
 UGenSublcassDeclarationNoDefault(MyOsc, (freq, control), (UGen const& freq	= 440.f, UGen const& control = 0.5f));
 @endcode
 
 @param UGEN_NAME						Name of the UGen to create, this wiil derive from UGen
 @param CONSTRUCTOR_ARGS_CALL			A comma separated list of the arguments the constructor and 
										ar/kr/AR/KR methods take when called. This must be surrounded
										by parentheses.
 @param CONSTRUCTOR_ARGS_DECLARE		A comma separated list of the arguments the constructor and 
										ar/kr/AR/KR methods take when declared. Each of these parameters
										must have a default. This must be surrounded by parentheses. 
 @param DOCS							Doxygen "param" documentation usually from a define. You can simply use
										DEFAULT_UGEN_DOCS or COMMON_UGEN_DOCS if you wish. Most UGen classes use 
										COMMON_UGEN_DOCS followed by their own param documentation.							\n
										E.g., @code COMMON_UGEN_DOCS MyUGen_Docs @endcode									\n
										- note these are separated by spaces so they form a single item.

 @see UGenSublcassDeclaration, DefineCustomUGen
 */
#define UGenSublcassDeclarationNoDefault(UGEN_NAME, CONSTRUCTOR_ARGS_CALL, CONSTRUCTOR_ARGS_DECLARE, DOCS)							\
		class UGEN_NAME : public UGen																								\
		{																															\
		public:																														\
			/** Base UGEN_NAME Constuctor, use the AR() and KR() versions in user code. DOCS */										\
			UGEN_NAME CONSTRUCTOR_ARGS_DECLARE throw();																				\
			/** Construct and return an audio rate UGEN_NAME. DOCS	
				@return The audio rate UGEN_NAME. */																				\
			static inline UGen AR CONSTRUCTOR_ARGS_DECLARE throw()	{ return UGEN_NAME CONSTRUCTOR_ARGS_CALL;				}		\
			/** Construct and return a control rate UGEN_NAME (if available, otherise returns an audio version by default). DOCS 
				@return The control rate UGEN_NAME.	 */																				\
			static inline UGen KR CONSTRUCTOR_ARGS_DECLARE throw()	{ return UGen(UGEN_NAME CONSTRUCTOR_ARGS_CALL).kr();	}		\
		}

/**
 Macro for declaring a simple UGen.
 
 Use this version if at least one argument does NOT have a default value so that a default constructor
 is declared.
 
 @param UGEN_NAME					Name of the UGen to create, this wiil derive from UGen
 @param CONSTRUCTOR_ARGS_CALL		A comma separated list of the arguments the constructor and 
									ar/kr/AR/KR methods take when called. This must be surrounded
									by parentheses.
 @param CONSTRUCTOR_ARGS_DECLARE	A comma separated list of the arguments the constructor and 
									ar/kr/AR/KR methods take when declared. Each of these parameters
									must have a default. This must be surrounded by parentheses. 
 @param DOCS						Doxygen "param" documentation usually from a define. You can simply use
									DEFAULT_UGEN_DOCS or COMMON_UGEN_DOCS if you wish. Most UGen classes use 
									COMMON_UGEN_DOCS followed by their own param documentation.							\n
									E.g., @code COMMON_UGEN_DOCS MyUGen_Docs @endcode									\n
									- note these are separated by spaces so they form a single item.
 
 @see UGenSublcassDeclarationNoDefault, DefineCustomUGen
 */
#define UGenSublcassDeclaration(UGEN_NAME, CONSTRUCTOR_ARGS_CALL, CONSTRUCTOR_ARGS_DECLARE, DOCS)								\
		class UGEN_NAME : public UGen																									\
		{																																\
		public:																															\
			/** @internal */																											\
			UGEN_NAME () throw() : UGen() { }																							\
			/** Base UGEN_NAME Constuctor, use the AR() and KR() versions in user code. DOCS */											\
			UGEN_NAME CONSTRUCTOR_ARGS_DECLARE throw();																					\
			/** Construct and return an audio rate UGEN_NAME. DOCS	
				@return The audio rate UGEN_NAME. */																					\
			static inline UGen AR CONSTRUCTOR_ARGS_DECLARE throw()		{ return UGEN_NAME CONSTRUCTOR_ARGS_CALL;				}		\
			/** Construct and return a control rate UGEN_NAME (if available, otherise returns an audio version by default). DOCS 
				@return The control rate UGEN_NAME.	 */																					\
			static inline UGen KR CONSTRUCTOR_ARGS_DECLARE throw()		{ return UGen(UGEN_NAME CONSTRUCTOR_ARGS_CALL).kr();	}		\
		}

/**
 Macro for declaring a custom UGen by combining other UGens.
 
 @param UGEN_NAME					Name of the UGen to create, this wiil derive from UGen
 @param CONSTRUCTOR_ARGS_CALL		The UGen expression to return, this is used as an argument to the UGen constructor
									during the derived class construction. This must be surrounded by parentheses.
 @param CONSTRUCTOR_ARGS_DECLARE	A comma separated list of the arguments the constructor and 
									ar/kr/AR/KR methods take when declared. Each of these parameters
									must have a default. This must be surrounded by parentheses. 
 @param DOCS						Doxygen "param" documentation usually from a define. You can simply use
									DEFAULT_UGEN_DOCS or COMMON_UGEN_DOCS if you wish. Most UGens pass COMMON_UGEN_DOCS 
									followed by their own param documentation.											\n
									E.g., @code COMMON_UGEN_DOCS MyUGen_Docs @endcode									\n
 - note these are separated by spaces so they form a single item.
 
 
 @see UGenSublcassDeclarationNoDefault, UGenSublcassDeclaration
 */
#define DefineCustomUGen(UGEN_NAME, CONSTRUCTOR_ARGS_CALL, CONSTRUCTOR_ARGS_DECLARE, DOCS)											\
		class UGEN_NAME : public UGen																								\
		{																															\
		public:																														\
			/** @internal */																										\
			UGEN_NAME () throw() : UGen() { }																						\
			/** Base UGEN_NAME Constuctor, use the AR() and KR() versions in user code. 
				@code CONSTRUCTOR_ARGS_CALL @endcode is used as the call to the UGen constructor 
				(including the parentheses which is why they are required).
				\n\n
				DOCS */																												\
			UGEN_NAME CONSTRUCTOR_ARGS_DECLARE throw() : UGen CONSTRUCTOR_ARGS_CALL { }												\
																																	\
			/** Construct and return an audio rate UGEN_NAME. DOCS	
				@return The audio rate UGEN_NAME. */																				\
			static inline UGen AR CONSTRUCTOR_ARGS_DECLARE throw()		{ return UGen CONSTRUCTOR_ARGS_CALL ;			}			\
			/** Construct and return a control rate UGEN_NAME (if available, otherise returns an audio version by default). DOCS 
				@return The control rate UGEN_NAME.	 */																				\
			static inline UGen KR CONSTRUCTOR_ARGS_DECLARE throw()		{ return UGen CONSTRUCTOR_ARGS_CALL .kr();		}			\
		}


/** Macro for declaring a control rate version of a UGenInternal
	
	Given the class name of a sublcass of UGenInternal, this declares (and defines) most of the functionality
	of a control rate version of the subclass appending a 'K' to the end of the class name. You need then only
	define MyInternalUGenK::processBlock() in your cpp file. Note you should use the 'value' member to store the 
	current value of the control rate output.
 */
#define UGenInternalControlRateDeclaration(UGENINTERNAL_NAME, CONSTRUCTOR_ARGS_CALL, CONSTRUCTOR_ARGS_DECLARE)				\
		class UGENINTERNAL_NAME##K : public UGENINTERNAL_NAME																\
		{																													\
		public:																												\
			UGENINTERNAL_NAME##K CONSTRUCTOR_ARGS_DECLARE throw()															\
			:	UGENINTERNAL_NAME CONSTRUCTOR_ARGS_CALL,																	\
				value(0.f)																									\
			{ rate = ControlRate; }																							\
																															\
			UGenInternal* getKr() throw()					{ incrementRefCount(); return this; }							\
			float getValue(const int channel) const throw()	{ return value;						}							\
			void setValue(const float newValue) throw()		{ value = newValue;					}							\
			void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();					\
		private:																											\
			float value;																									\
		}

#define ReportSize(CLASSNAME) printf(#CLASSNAME " size = %ld\n", sizeof(CLASSNAME))


class UGenArray;
class ExternalControlSource;
class MultiSlider;
class Env;
class RawInputUGenInternal;
class MetaDataReceiver;

/**	The UGen class!

 This is the main class you use to construct audio processing graphs.  Each UGen contains an 
 array of UGenInternal subclasses which do the actual processing.
	
 Subclasses of UGen should only fill this array with appropriate UGenInternal subclasses
 the UGenSublcassDeclarationNoDefault() may be used to declare a UGen subclass then only a
 constructor will need defining which initialises the UGenInternal objects.
	
 See the various subclasses of UGen for more examples.
 
 For arrays of UGens (i.e., arrays of arrays of channels) see the UGenArray class.
	
 @see AllUGens, UGenArray
 */
class UGen
{
public:

	/// @name Construction and Destruction. 
	/// @{
	
	/** Default constructor. */
	UGen() throw();
	
	/** Constructor for a UGen using an already existing UGenInternal. 
	 
	 @param internalUGenToUse	This should have already had its refCount incrememented by using an 
								appropriate function to obtain it e.g., getInternalUGen()
	 @param channel				If zero or greater, which channel of this internal should be used.
								This recursively extracts that channel number from any UGen instances owned by
								the internal (leaving an entirely single channel UGen). A value of -1 
								leaves all channels in place. */
	UGen(UGenInternal* internalUGenToUse, const int channel = -1) throw();
	
	/** Constructor for a UGen using an alrady existing array of internals. 
	 
	 @param numInternalUGensToUse	The number of internals in the array.
	 @param internalUGensToUse		The array of internals, these should have already had their refCount 
									incremement by using an appropriate function to obtain it 
									e.g., getInternalUGen() */
	UGen(const int numInternalUGensToUse, UGenInternal** internalUGensToUse) throw();
	
	/** Copy constructor. */
	UGen(UGen const& copy) throw();
	
	/** Assignment operator. */
	UGen& operator= (UGen const& other) throw();
	
	/** Scalar UGen. A UGen which generates a constant float value. 
	 @param value	The float value to use. */
	UGen(const float value) throw();
	
	/** Scalar UGen. A UGen which generates a constant double value. 
	 @param value	The double value to use, although this will be cast to a float. */
	UGen(const double value) throw();
	
	/** Scalar UGen. A UGen which generates a constant int value. 
	 @param value	The int value to use, although this will be cast to a float. */
	UGen(const int value) throw();
	
	/** Scalar UGen. A UGen which generates a value from a float pointer. 
	 @param valuePtr	A pointer to a float, the UGen output will change if the 
						float pointed to changes. */
	UGen(float const *valuePtr) throw();
	
	/** Scalar UGen. A UGen which generates a value from a double pointer.
	 @param valuePtr	A pointer to a double, the UGen output will change if the 
						double pointed to changes. This will be cast to a float. */
	UGen(double const *valuePtr) throw();
	
	/** Scalar UGen. A UGen which generates a value from an int pointer.
	 @param valuePtr	A pointer to an int, the UGen output will change if the 
						int pointed to changes. This will be cast to a float. */
	UGen(int const *valuePtr) throw();
	UGen(char const *valuePtr) throw();
	UGen(unsigned char const *valuePtr) throw();
	UGen(bool const *valuePtr) throw();
	
	/** Construct a multichannel UGen from an array of values.
	 Here channel 0 of the Buffer object is used as an array of floats. A UGen is
	 created for each value in the array.
	 @param buffer The Buffer object to use (channel 0 only). */
	UGen(Buffer const& buffer) throw();
	
	/** Construct a multichannel UGen from an array of values.
	 Here Value objects in a ValueArray are evaluated to generate a multichannel UGen.
	 @param array The ValueArray object to use. */
	UGen(ValueArray const& array) throw();
	UGen(Value const& value) throw();
	
	/** A function to create a scalar float UGen.
	 This is used by templated functions to keep include files out of the UGen header. */
	static UGenInternal* newScalarInternal(const float value) throw();
	
	template<class NumericalType>
	UGen(NumericalArray<NumericalType> const& array) throw()
	:	userData(UGen::defaultUserData),
		numInternalUGens(0),
		internalUGens(0)
	{
		ugen_assert(array.size() > 0);
		
		initInternal(array.size());
		for(int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = newScalarInternal((float)array[i]);	
		}
	}	
	
	/** Construct a multichannel UGen from an UGenArray of UGen instances.
	 Here the multiple channels will be flattened.
	 @param array The UGenArray object to use. */
	UGen(UGenArray const& array) throw();
	
	/** Construct a UGen from an ExternalControlSource. 
	 @param externalControlSource The ExternalControlSource to use.*/
	UGen(ExternalControlSource const& externalControlSource) throw();
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	/** Construct a UGen from a Juce Slider. 
	 This registers listeners to the Slider and changes the UGen output value accordingly.
	 @param slider The Slider to use.*/
	UGen(Slider* slider) throw();
	
	/** Construct a UGen from a MultiSlider. 
	 @param sliders The MultiSlider to use.*/
	UGen(MultiSlider* sliders) throw();
	
	/** Construct a UGen from a Juce Button. 
	 This registers listeners to the Button and changes the UGen output value accordingly.
	 Obviously this is more useful with ToggleButton type buttons (which would output
	 0 or 1 depending on their state).
	 @param button The Button to use.*/
	UGen(Button* button) throw();
	
	UGen(ToggleButton* button) throw();
	
	UGen(TextButton* button) throw();
	
	/** Construct a UGen from a Juce Label. 
	 This registers listeners to the Label and changes the UGen output value accordingly.
	 The contents of the label will be converted to a float.
	 @param label The Label to use.*/
	UGen(Label* label) throw();
#endif
#if defined(UGEN_IPHONE) || defined(DOXYGEN)
	/** Construct a UGen from a UISlider (iPhone) Slider. 
	 This registers listeners to the Slider and changes the UGen output value accordingly.
	 @param slider The Slider to use.*/
	UGen(UISlider *slider) throw();
	UGen(signed char *valuePtr) throw();
#endif
	
	UGen(Env const& env) throw();
	
	/** Constuct a multichannel UGen from two other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2) throw();
	
	template<class Type1, class Type2>
	UGen(Type1 arg1, Type2 arg2) throw() : userData(UGen::defaultUserData), numInternalUGens(0), internalUGens(0)  { *this = UGen(UGen(arg1), UGen(arg2)); }
	
	/** Constuct a multichannel UGen from three other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3) throw();
	
	/** Constuct a multichannel UGen from four other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) throw();
	
	/** Constuct a multichannel UGen from five other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) throw();
	
	/** Constuct a multichannel UGen from six other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) throw();
	
	/** Constuct a multichannel UGen from seven other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) throw();
	
	/** Constuct a multichannel UGen from eight other UGen instances. 
	 The result has all the channels from the source UGen instances appended. 
	 For more sources than eight use UGen::operator<< or UGen::operator,
	 @see UGen::operator<< and UGen::operator, */
	UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) throw();
	
	/** Destructor. This decremements all of its UGenInternal classes by one. */
	/*virtual... UGen doesn't actaully have any virtual functions!*/ 
	~UGen() throw();
	
	/// @} <!-- end Construcion and Destruction ---------------------------------------------------------- -->
	
	/// @name Channel manipulation
	/// @{
	
	/** Create a new UGenArray containing this and another UGen.
	 A new UGenArray is returned with two elements. The Mix documentation illustrates why this might
	 be useful since the operator<< can then be chained together to produce a multiple
	 entry UGenArray.
	 @see UGenArray, Mix */
	UGenArray operator<< (UGen const& other) throw();
	
	/** Create a new UGen by appending channels to this one.
	 A new UGen is created which has a total number of channels equal to the sum of the number
	 of channels in this UGen and the rightOperand parameter. Channels are appended to create the 
	 new UGen. NB this is not in place. 
	 
	 For example:
	 @code
	 UGen x = 1.0;
	 UGen y = 2.0;
	 UGen z = (x, y); // equivalent to z = UGen(1.0, 2.0);
	 @endcode
	 */
	UGen operator, (UGen const& other) throw();
	
	/** Append and assignment.
	 Appends channels from another UGen to this UGen. This may seem unituitive since UGen::operator<< returns
	 a UGenArray but the result would be the same if this resulting UGenArray was flattened back to a UGen. (Except
	 this currently does not remove null UGen instances from the result.)
	 @see UGen::operator<< */
	UGen& operator<<= (UGen const& other) throw();
	
	/** Create a UGen which contains a particular channel.
	 @param index The channel number to access, this is wrapped to ensure it is always in range.
	 @return the single channel UGen at the index requested.
	 @see at(), wrapAt(), getChannel()
	 */
	UGen operator[] (const int index) const throw();
	
	/** Create a UGen which contains particular channels.
	 @param indices		The channel numbers to access as an IntArray, these indices are wrapped
						so that they are always in range.
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), range() */
	UGen operator[] (IntArray const& indices) const throw();
	
	/** Create a UGen which contains a particular channel.
	 @param index The channel number to access, if this is out of range a null UGen will be returned.
	 @return the single channel UGen at the index requested.
	 @see UGen::operator[], wrapAt(), getChannel() */
	UGen at(const int index) const throw();
	
	/** Create a UGen which contains particular channels.
	 @param indices		The channel numbers to access as an IntArray, if an index is out of range a null 
						UGen will be used instead.
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), range() */	
	UGen at(IntArray const& indices) const throw();
	
	/** Create a UGen which contains on a particular channel.
	 @param index The channel number to access, this is wrapped to ensure it is always in range.
	 @return the single channel UGen at the index requested.
	 @see UGen::operator[], at(), getChannel() */
	UGen wrapAt(const int index) const throw();
	
	/** Create a UGen which contains particular channels.
	 @param indices		The channel numbers to access as an IntArray, these indices are wrapped
						so that they are always in range.
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), range() */	
	UGen wrapAt(IntArray const& indices) const throw();
	
	/** Create a UGen which contains a particular range of channels.
	 Returns channels from the start index upto but not inlcuding the end index.
	 @param startIndex	The start index (inclusive)
	 @param endIndex	The end index (exclusive)
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), from(), to() */	
	UGen range(const int startIndex, const int endIndex) const throw();
	
	/** Create a UGen which contains a particular range of channels.
	 Returns channels from the start index to the last channel.
	 @param startIndex	The start index (inclusive)
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), from(), to() */	
	UGen from(const int startIndex) const throw();
	
	/** Create a UGen which contains a particular range of channels.
	 Returns channels from the start index to the last channel.
	 @param startIndex	The start index (inclusive)
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), from(), to() */	
	UGen range(const int startIndex) const throw();
	
	/** Create a UGen which contains a particular range of channels.
	 Returns channels from index 0 upto but not inlcuding the end index.
	 @param endIndex	The end index (exclusive)
	 @return the new UGen at the indices requested.
	 @see at(), wrapAt(), getChannel(), from(), to() */	
	UGen to(const int endIndex) const throw();
	
	
	/** Creates a new UGen based on this one with more/fewer channels.
	 @param numChannels			The number of channels to use.
	 @param addedChannelsWrap	If @c true the added channels will wrap back around to the 0th channel
								and continue repeating channels upto numChannels. If @c false empty
								channels will be added.
	 @return the new UGen will the number of channels requested. */
	UGen withNumChannels(const int numChannels, const bool addedChannelsWrap = true) const throw();
	
	/** Create a UGen which contains a particular channel.
	 @param channel The channel number to access, this is wrapped to ensure it is always in range.
	 @return the single channel UGen at the index requested.
	 @see wrapAt(), UGen::operator[], at()
	 */
	UGen getChannel(const int channel) const throw();
	
	/** Group UGen channels into clumps of channels.
	 
	 A UGen with channels { a, b, c, d, e, f } and a clumps size of 2 should
	 return a UGenArray { {a,b}, {c,d}, {e,f} }.
	 
	 @param size The size of the clumps.
	 @return A UGenArray containing the clumped channels. */
	UGenArray group(const int size) const throw();
	
	/** Interleave UGen channels.
	 
	 A UGen with channels { a, b, c, d, e, f } and a size of 2 should
	 return a UGenArray { {a,c,e}, {b,d,f} }.
	 
	 @param size The size of the clumps.
	 @return A UGenArray containing the clumped channels. */
	UGenArray interleave(const int size) const throw();
	
	/** Convert a UGen to a UGenArray.
	 @return	A UGenArray with size equal to the number of channels in the
	 source UGen, each channel at a separate index in the UGenArray. */
	UGenArray toArray() const throw();
	
	//	UGen remove(UGen const& itemsToRemove) const;
	//	UGen removeAt(const int index) const;
	//	UGen insert(const int index, UGen const& itemsToInsert) const;
	//	int indexOf(UGen const& itemsToSearchFor) const;
	//	bool contains(UGen const& itemsToSearchFor) const;
	
	UGen groupMix(const int size) const throw();
	UGen groupMixScale(const int size) const throw();
	UGen interleaveMix(const int size) const throw();
	UGen interleave(UGen const& rightOperand) const throw();
	
	/// @} <!-- end Channel manipulation ----------------------------------------------------- -->
	
	/// @name Tests
	/// @{
	
	/** Tests whether a UGen is a null UGen. 
	 @param index	The index of the channel to test, if this is -1 then the UGen MUST contain 
					only one UGenInternal AND be a NullUGenInternal to return true.
	 @return true if the UGen or one of its channels is null.
	 @see isNotNull(), isNullKr(), isScalar(), isConst()
	 */
	bool isNull(const int index = -1) const throw();
	
	/** Tests whether a UGen is a null UGen. 
	 @param index	The index of the channel to test, if this is -1 then the UGen MUST contain 
					only one UGenInternal AND be a NullUGenInternal to return false.
	 @return false if the UGen or one of its channels is null.
	 @see isNull(), isNullKr(), isScalar(), isConst()
	 */
	bool isNotNull(const int index = -1) const throw();
	
	/** Tests whether the UGen is null AND is control rate. */
	bool isNullKr() const throw();
	
	/** Tests whether a UGen is a scalar UGen. 
	 
	 Scalar UGen classes include constants but also UGen instances created from pointers (where the value pointed
	 to may change, this includes the Slider-type UGen classes for Juce and iPhone for example).
	 
	 @param index	The index of the channel to test, if this is -1 then the UGen MUST contain 
					only one UGenInternal AND be a scalar UGen to return true.
	 @return true if the UGen or one of its channels is scalar.
	 
	 @see isNotNull(), isNullKr(), isNull(), isConst()
	 */
	bool isScalar(const int index = -1) const throw();
	
	/** Tests whether a UGen is a constant UGen. 
	 
	 @param index	The index of the channel to test, if this is -1 then the UGen MUST contain 
					only one UGenInternal AND be a constant UGen to return true.
	 @return true if the UGen or one of its channels is constant.
	 
	 @see isNotNull(), isNullKr(), isNull(), isScalar()
	 */
	bool isConst(const int index = -1) const throw();
	
	/** Tests whether a UGen is a control rate only UGen. 
	 
	 @param index	The index of the channel is wrapped around the number of channels
	 @return true if the UGen channel is control rate only. */
	inline bool isControlRateOnly(const int index) const throw() { return internalUGens[index % numInternalUGens]->isControlRateOnly(); }
		
	/** Tests whether a UGen contains the same internals as another UGen.
	 @param other						The other UGen to campare with.
	 @param mustBeInTheSameSequence		If this is true the internals must be in the same order
										in both UGen instances, if false the order is ignored.
	 @return @c true if the UGens contain identical internals.
	 */
	bool containsIdenticalInternalsAs(UGen const& other, const bool mustBeInTheSameSequence = true) const throw();
	
	inline bool operator== (UGen const& other) const throw() { return containsIdenticalInternalsAs(other); }
	inline bool operator!= (UGen const& other) const throw() { return !containsIdenticalInternalsAs(other); }
	
#ifndef UGEN_ANDROID
	/** Tests whether this UGen contains a particular UGenInternal derived class.
	 
	 Use this with care as many of the UGen constructors wrap UGen instances in other UGen classes. 
	 For example, the MulAdd UGen (which is used for many of the oscillators and filters) may wrap the base
	 UGen in a BinaryMulUGen, a BinaryAddUGen or a MulAdd UGen depending on the state and/or presence
	 of the "mul" and "add" arguments.
	 
	 @code
	 UGen env = LLine::AR(1.0, 0.0, 10); // line from 1-0 in 10s
	 bool test = env.containsInternalType<LLineUGenInternal>();
	 @endcode
	 
	 ..here 'test' would be @c true. 
	 
	 @tparam UGenInternalType	The type of UGenInternal subclass to test for. Since UGenInternal objects
								are always stored as pointers the test will be for a UGenInternalType* type.
								<b>DO NOT</b> insert your own asterix!
	 
	 @param index	Index of the channel of the UGenInternal to test, if this is -1 the 
					UGen must have only one channel and contain the requested UGenInternal type
					to return true.
	 @return		True if the channel index contains the type of UGenInternal requested otherwise false 
					(this will also return false if the index is -1 and the UGen contains more than 1 internal). */
	template<class UGenInternalType>
	bool containsInternalType(const int index = -1) const throw()
	{
		if(index < 0)
			return (numInternalUGens == 1) && (dynamic_cast<UGenInternalType*> (internalUGens[0]) != 0);
		else
			return dynamic_cast<UGenInternalType*> (internalUGens[index]) != 0;
	}
#endif
	
	/// @} <!-- end Tests ---------------------------------------------------------------------- -->
	
	
	/// @name Rendering
	/// @{
	
	/** Get the next sample blockID based on the required block size. 
	 
	 Block IDs are used to ensure UGen instances don't process their data more than once for a
	 particular time slice.
	 
	 @param blockSize The blocksize required.
	 @return The next block ID
	 */
	static inline int getNextBlockID(const int blockSize) throw()			{ return nextBlockID += blockSize;		}
	static inline int getCurrentBlockID() throw()							{ return nextBlockID;					}
	
	/** Processes one or more channels in this UGen.
	 
	 This causes UGenInternal classes to have their UGenInternal::processBlockInternal() method called (which may result
	 in recursive calls to UGen::processBlock() if the UGenInternal contains other UGen instances).
	 
	 @param shouldDelete		This may get modified by UGen instances in the graph if they have finsihed
								and have a DoneAction which is UGen::DeleteWhenDone. If this returns true, the
								UGen graph will be set to null on the next block.
	 @param blockID				The current block ID, normally obtained from getNextBlockID()
	 @param channel				The channel index to process or -1 ro process all channels.
	 @return	A pointer to the array of processed samples for a single channel process or 0 if this 
				is processing all channels. */
	float* processBlock(bool& shouldDelete, const unsigned int blockID, const int channel = -1) throw();
	
	/** Prepares for a block then processes it.
	 
	 This is the normal way of processing a block of audio to/from another host using UGen++.
	 
	 @param actualBlockSize		The actual block size to prepare for and process.
	 @param blockID				The sample block ID of the block to process.
	 @param channel				The channel index to process or -1 ro process all channels.
	 @return	A pointer to the array of processed samples for a single channel process or 0 if this 
				is processing all channels. 
	 @see		processBlock(), prepareForBlock() */
	float* prepareAndProcessBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	
	/** Prepares a UGen for processing. 
	 
	 This ensures buffer sizes are allocated to appropriate sizes and also sets UGen instances to null
	 which are scheduled for deletion this block.
	 
	 @param actualBlockSize		The actual block size to prepare.
	 @param blockID				The sample block ID of the block to prepare. 
	 @param channel				The channel index to process or -1 ro process all channels. */
	void prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw();
	
	/// @} <!-- end Rendering --------------------------------------------------- -->
	
	/// @name Input and output access
	/// @{
	
	/** Get the input to a UGen by index.
	 @param input		The index of the input, this should be according to the Inputs enum
						in the appropriate UGenInternal-derived class
	 @param channel		The index of the UGenInternal.
	 @return			A reference to the input to the UGen (or a null UGen if either of
						the indices are out of range) */
	const UGen& getInput(const int input, const int channel) throw();
	
	/** Get the output object of one of the internals.
	 @param		channel	The index of the UGenInternal.
	 @return			A pointer to the UGenOutput for the UGenInternal. */
	UGenOutput* getOutput(int channel = 0) const throw();
	
	/** Set the output for a particular UGenInternal.
	 
	 UGenInternal instances normally write their data to their UGenOutput, this function forces the UGenInternal to 
	 write its data to somewhere else. This is one way of getting UGen++ to write data to an external data structure.
	 
	 @param	block		A pointer to an array of floats which will receive the data when the UGen processes.
	 @param	blockSize	The size of the array and block size.
	 @param	channel		The index of the UGenInternal. */
	void setOutput(float* block, const int blockSize, const int channel = 0) throw();
	
	/** Set the outputs for the UGenInternal instances.
	 
	 UGenInternal instances normally write their data to their UGenOutput, this function forces the UGenInternal instances to 
	 write their data to somewhere else. This is one way of getting UGen++ to write data to an external data structure.
	 
	 @param	block		A pointer to an array of pointers to arrays of floats 
						which will receive the data when the UGen processes.
	 @param	blockSize	The size of the arrays of floats and block size.
	 @param	numChannels	The number of pointers in block. */
	void setOutputs(float** block, const int blockSize, const int numChannels) throw();
	
	/** Attempts to set the input data source of a particular UGenInternal. 
	 
	 This only works if the UGen contains RawInputUGenInternal class, usually created
	 using the AudioIn class.
	 
	 @param	block		A pointer to an array of floats which will be read.
	 @param	blockSize	The size of the array and block size.
	 @param	channel		The index of the RawInputUGenInternal. 
	 @see AudioIn, RawInputUGenInternal */
	bool setInput(const float* block, const int blockSize, const int channel = 0) throw();
	
	/** Attempts to set the input data sources for the UGen. 
	 
	 This only works if the UGen contains RawInputUGenInternal classes, usually created
	 using the AudioIn class.
	 
	 @param	block		A pointer to an array of pointers to arrays of floats 
						which will be read.
	 @param	blockSize	The size of the arrays of floats and block size.
	 @param	numChannels	The number of pointers in block.
	 @see AudioIn, RawInputUGenInternal */
	bool setInputs(const float** block, const int blockSize, const int numChannels) throw();
		
	/// @} <!-- end Input and output access -------------------------------------------- -->
	
	/// @name UGen-specific control messages
	/// @{
	
	/** Attempts to set the Value object of a ValueUGen */
	bool setValue(Value const& other) throw();
	
	/** Attempts to set the source of a Plug.
	 
	 This only works if the UGen contains PlugUGenInternal classes.
	 
	 @param		source					The new source for the Plug.
	 @param		releasePreviousSources	If true the source being replaced is retained by the Plug for later,
										this is useful if you want to ensure a UGen is continued to be
										processed (e.g., DiskIn) so that it reaches a correct point in time 
										should it become the Plug source at some point in the future. If false 
										the current source removed completely when it is no longer required
										(this may not be immediate if a fadeTime greater than 0 is used).
	 @param		fadeTime				Time in seconds for the crossfade between current and new sources.
	 @return							@c true if this successfully found a Plug to set @c false otherwise.
	 @see Plug */
	bool setSource(UGen const& source, const bool releasePreviousSources = false, const float fadeTime = 0.f) throw();
	
	/** Attempts to set the source of a Plug using a cross fade.
	 
	 This only works if the UGen contains PlugUGenInternal classes. The current source is retained by
	 the Plug.
	 
	 @param		source					The new source for the Plug.
	 @param		fadeTime				Time in seconds for the crossfade between current and new sources.
	 @return							@c true if this successfully found a Plug to set @c false otherwise.
	 @see Plug */
	inline bool fadeSource(UGen const& source, const float fadeTime = 0.f) throw() { return setSource(source, false, fadeTime); }
	
	/** Attempts to set the source of a Plug using a cross fade.
	 
	 This only works if the UGen contains PlugUGenInternal classes. The current source is release by
	 the Plug when the crossfade has finished.
	 
	 @param		source					The new source for the Plug.
	 @param		fadeTime				Time in seconds for the crossfade between current and new sources.
	 @return							@c true if this successfully found a Plug to set @c false otherwise.
	 @see Plug */	
	inline bool fadeSourceAndRelease(UGen const& source, const float fadeTime = 0.f) throw() { return setSource(source, true, fadeTime); }
	
	/** Attempts to get the source of a Plug.
	 
	 This only works if the UGen contains PlugUGenInternal classes.
	 
	 @return The Plug source, or a null UGen if this is not a Plug.
	 @see Plug */
	UGen getSource() throw();
	
	/** Attempts to release any releasable UGen instances in the UGen graph.
	 
	 This will only have an affect if the UGen graph contains a ReleasableUGenInternal or
	 one of its subclasses (examples are LLine, EnvGen). The UGen graph is searched recursively 
	 sending a release() message to every UGen in the graph.
	 
	 @see LLine, XLine, EnvGen, ASR */
	void release() throw();
	
	/** Attempts to steal any releasable UGen instances in the UGen graph.
	 
	 This will only have an affect if the UGen graph contains a ReleasableUGenInternal or
	 one of its subclasses (examples are LLine, EnvGen). The UGen graph is searched recursively 
	 sending a steal() message to every UGen in the graph.
	 
	 @param forcedSteal		If this is true any ReleasableUGenInternal will execute their
							DoneAction this block and will cause the deletion of the UGen graph
							on the next block. If false, the ReleasableUGenInternal should perform
							a very fast fade, then execute its DoneAction.
	 
	 @see LLine, XLine, EnvGen, ASR */
	void steal(const bool forcedSteal) throw();
	
	/** Attmepts to send a MIDI note message to a Voicer.
	 
	 This will only have an affect if the UGen contains a VoicerBaseUGenInternal.
	 
	 @param		midiChannel		The MIDI channel of the message 1-16
	 @param		midiNote		The MIDI note number 0-127
	 @param		velocity		The MIDI note velocity 0 (note off) and 1-127 (note on)
	 
	 @see Voicer */
	bool sendMidiNote(const int midiChannel, const int midiNote, const int velocity) throw();
	
#ifndef UGEN_ANDROID
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	void sendMidiBuffer(MidiBuffer const& midiMessages) throw();
#endif
#if (defined(UGEN_IPHONE) && defined(UGEN_IOS_COREMIDI)) || defined(DOXYGEN)
	void sendMidiBuffer(ByteArray const& midiMessages) throw();
#endif
#endif
	
	/** Attempts to send a trigger message to a TSpawn.
	 
	 This will only have an affect if the UGen contains a TSpawnUGenInternal.
	 
	 @param extraArgs	User defined.
	 
	 @see TSpawn, TrigXFade */
	bool trigger(void* extraArgs = 0) throw();
	
	/** Attempts to stop any running events in a Spawn-type UGen.
	 Useful for a panic e.g., "all notes off" type command. */
	bool stopAllEvents() throw();
	
	UGen& addBufferReceiver(BufferReceiver* const receiver) throw();
	void removeBufferReceiver(BufferReceiver* const receiver) throw();
	UGen& addBufferReceiver(UGen const& receiver) throw();
	void removeBufferReceiver(UGen const& receiver) throw();
	
	//#if defined(UGEN_IPHONE) || defined(DOXYGEN)
	//	void addBufferReceiver(UIScopeView* receiver) throw();
	//	void removeBufferReceiver(UIScopeView* receiver) throw();
	//#endif	
	
	UGen& addDoneActionReceiver(DoneActionReceiver* const receiver) throw();
	void removeDoneActionReceiver(DoneActionReceiver* const receiver) throw();
	UGen& addDoneActionReceiver(UGen const& receiver) throw();
	void removeDoneActionReceiver(UGen const& receiver) throw();
	
	UGen& addMetaDataReceiver(MetaDataReceiver* const receiver) throw();
	void removeMetaDataReceiver(MetaDataReceiver* const receiver) throw();
	UGen& addMetaDataReceiver(UGen const& receiver) throw();
	void removeMetaDataReceiver(UGen const& receiver) throw();
	
	
	double getDuration() throw();
	double getPosition() throw();
	bool setPosition(const double newPosition) throw();
	DoubleArray getDurations() throw();
	DoubleArray getPositions() throw();
	bool setPositions(DoubleArray const& newPositions) throw();
	
	
	
	/// @} <!-- end UGen-specific control messages ------------------------------------- -->
	
	
	/// @name Static functions
	/// @{
	
	/** Prepares UGen++ for a rendering session.
	 
	 This configures various variables needed for rendering, especially those dependent
	 on the sample rate and/or block size. It should be called before any calls to 
	 prepareAndProcessBlock for example. 
	 
	 @param		sampleRate					The sample rate of the host systems in Hz (e.g., 44100.0)
	 @param		estimatedSamplesPerBlock	An estimate of the host block size.
	 @param		newControlRateBlockSize		The control rate block size (default 64). This may be larger or
											smaller than the hardware block size. A value of -1 leaves the
											control rate block size as is. */
	static void prepareToPlay(const double sampleRate = 44100.0, const int estimatedSamplesPerBlock = 64, const int newControlRateBlockSize = -1) throw();
	
	/** Create a UGen with a number of empty (null) channels.
	 @param numChannels The number of empty channels the UGen should contain.
	 @return The UGen with the empty channels. */
	inline static UGen emptyChannels(const int numChannels) throw()
	{
		ugen_assert(numChannels > 0);
		
		UGenInternal* temp[] = { 0 };
		return UGen(numChannels < 1 ? 1 : numChannels, temp);
	}
	
	/** Get the current sample rate. @return The current sample rate. */
	inline static double			getSampleRate() throw()						{ return sampleRate_;													}
	
	/** Get the recirprocal of the current sample rate. @return 1.0 / (sample rate). */
	inline static double			getReciprocalSampleRate() throw()			{ return reciprocalSampleRate;											}
	
	/** Get the current control rate block size. @return The current control rate block size. */
	inline static int				getControlRateBlockSize() throw()			{ return controlRateBlockSize;											}
	
	/** Get the current estimated block size. @return The current estimated block size. */
	inline static int				getEstimatedBlockSize() throw()				{ return estimatedSamplesPerBlock_;										}
		
	/** Get a null internal. @return A null internal. */
	inline static UGenInternal*		getNullInternal() throw()					{ return getNull().getInternalUGen(0);									}
	
	/** Get a control rate null internal. @return A control rate null internal. */
	inline static UGenInternal*		getNullInternalKr() throw()					{ return getNullKr().getInternalUGen(0);								}
	
	/** Get the current Deleter. 
	 
	 A Deleter is used to delete SmartPointer objects (e.g., UGenInternal) rather than 
	 calling delete directly. The default Deleter does simply call delete but other hosts
	 could use another thread to delete objects so this is not done in an audio callback
	 thread (for example).
	 
	 So instead of this:
	 @code
	 delete mySmartPointerObj;
	 @endcode
	 
	 ..do this:
	 @code
	 UGen::getDeleter()->deleteInternal(mySmartPointerObj);
	 @endcode
	 
	 @return The current Deleter. 
	 
	 @see Deleter, JuceTimerDeleter */
	inline static Deleter*			getDeleter() throw()						{ return internalUGenDeleter;											}
	
	/** Set the current Deleter. 
	 
	 A Deleter is used to delete SmartPointer objects (e.g., UGenInternal) rather than 
	 calling delete directly. The default Deleter does simply call delete but other hosts
	 could use another thread to delete objects so this is not done in an audio callback
	 thread (for example).
	 
	 @param newDeleter	The new Deleter to use. It remains the caller's responsibility
						to delete the Deleter.
	 
	 @see Deleter, JuceTimerDeleter */
	inline static void				setDeleter(Deleter* newDeleter) throw()		{ internalUGenDeleter = newDeleter;										}
	
	/** Shutdown UGen++.
	 This should be done as application is closing down (or a plugin is being removed from a host).
	 @see initialise
	 */
	inline static void				shutdown() throw()							
	{ 
		getDeleter()->flush();
		setDeleter(&defaultDeleter);
		
#ifdef JUCE_VERSION
//		#include "../juce/io/ugen_JuceMIDIInputBroadcaster.h"
//		JuceMIDIInputBroadcaster::deleteInstance();
#endif
		isInitialised = false;
	}
	
	/** Initialise UGen++.
	 This should be called as soon as possible when an application or plugin loads.
	 @see shutdown
	 */
	inline static void initialise() throw()
	{
		if(isInitialised == false)
		{
			isInitialised = true;			
			ugen_debugprintf(("UGen++ v%d.%d.%d\n", UGEN_MAJOR_VERSION, UGEN_MINOR_VERSION, UGEN_BUILDNUMBER));
		}
	}
	
	
	
	/// @} <!-- end Static Functions ---------------------------------------------------------- -->
	
	/// @name SC compatability
	/// @{
	
	UGen ar() const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw(); /**< SC compatability. */
	UGen ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw(); /**< SC compatability. */
	UGen operator() () const throw(); /**< SC compatability. */
	UGen operator() (UGen const& other) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw(); /**< SC compatability. */
	UGen operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw(); /**< SC compatability. */
	UGen kr() const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw(); /**< SC compatability. */
	UGen kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw(); /**< SC compatability. */
	
	/// @} <!-- end SC compatability ---------------------------------------------------------------------------------- -->
	
	/** @name Unary Ops.
	 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except mix() and unary operator- 
	 The result of these unary operations is calculated on a sample-by-sample basis (or control period basis depending
	 on the rate).
	 
	 A global function is defined for each of these so:
	 @code
		UGen a = SinOsc::AR(100, 0, 0.4);
		UGen b = a.neg();
	 @endcode
	 ..is equivalent to:
	 @code
		UGen a = SinOsc::AR(100, 0, 0.4);
		UGen b = neg(a);
	 @endcode
	 */
	/// @{
	
	/** Wraps the UGen in a Mix UGen.
	 
	 This can also be used to protect one or more UGen from a auto deletion DoneAction i.e., DeleteWhenDone.
	 @return	The mixed UGen, if it is unneccessary to wrap the UGen in a Mix the source UGen will be returned. */
	UGen mix(bool shouldAllowAutoDelete = true) const throw();
	UGen mixScale(bool shouldAllowAutoDelete = true) const throw();
	
	/** Unary negation UGen. */
	UGen operator- () const throw();
	UnaryOpMethodsDeclare(UGen);
	
	template<UnaryOpFunction op>
	UGen unary() throw();
	
	/// @} <!-- end Unary Ops ------------------------------ -->
	
	/** @name Binary Ops. 
	 
	 The definition of these (except the assignment versions) is done by macros in ugen_BinaryOpUGens.cpp. 
	 The result of these binary operations is calculated on a sample-by-sample basis (or control period basis depending
	 on the rate). In the case of the comparison operators the resulting signal from the UGen will be 1.0 at times the 
	 comparison is true and 0.0 at times the comparison is false. The resulting UGen should have the same number of 
	 channels as the largest of the two UGen instances. If the UGen do not have the same number of channels, the channel 
	 index accessing the smaller UGen is wrapped to acces the channels cyclically. 
	 
	 A global function is defined for each of these so:
	 @code
		UGen a = SinOsc::AR(100, 0, 0.4);
		UGen b = SinOsc::AR(400, 0, 0.3);
		UGen c = a.max(b);
	 @endcode
	 ..is equivalent to:
	 @code
		UGen a = SinOsc::AR(100, 0, 0.4);
		UGen b = SinOsc::AR(400, 0, 0.3);
		UGen c = max(a, b);
	 @endcode
	 */
	
	/// @{
	BinaryOpMethodsDeclare(UGen);
	
	template<BinaryOpFunction op>
	UGen binary(UGen const& rightOperand) throw();
	
	/** Ignore right operator. 
	 
	 Useful to force UGen execution. Here only the signal from the receiving UGen will be used but
	 the right operand UGen will be executed. */
	UGen operator^ (UGen const& ignoreButEvalute) throw();
	
	
	/** Assignment and addition.
	 Creates a UGen which the sum of this UGen and another UGen.
	 This:
	 @code
	 UGen x = 1.0;
	 UGen y += 2.0;
	 @endcode
	 is equivalent to:
	 @code
	 UGen x = 1.0;
	 UGen y = y + 2.0;
	 @endcode
	 */
	UGen& operator+= (UGen const& other) throw();
	
	/** Assignment and subtraction.
	 Creates a UGen which the difference of this UGen and another UGen. */
	UGen& operator-= (UGen const& other) throw();
	
	/** Assignment and multiplication.
	 Creates a UGen which the product of this UGen and another UGen. */
	UGen& operator*= (UGen const& other) throw();
	
	/** Assignment and division.
	 Creates a UGen which the equal to this UGen "over" another UGen. */
	UGen& operator/= (UGen const& other) throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
	/// @name Miscellaneous
	/// @{
	
	/** Wraps the UGen in a Lag UGen.
	 
	 @param rightOperand	The lag time in seconds (time to be within 60dB of the destination value).
	 @return				The lagged UGen. */
	UGen lag(UGen const& rightOperand = 0.2) const throw();
	
	UGen wrap(UGen const& lower, UGen const& upper) throw();
	UGen fold(UGen const& lower, UGen const& upper) throw();
	
	UGen linlin(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw();
	UGen linsin(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw();
	UGen linexp(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw();
		
	/** @internal Expands multichannel UGen instances. */ 
	UGen expand() const throw();
	
	//void optimiseBlockSize(); // reduce block sizes to minimum
	
	/** Get the number of channels (i.e., number of internals). 
	 @return The number of channels. */
	inline int getNumChannels()	const throw()								{ return numInternalUGens;															}
	
	/** Get the number of channels (i.e., number of internals). 
	 @return The number of channels. */
	inline int size()	const throw()										{ return numInternalUGens;															}
	
	/** Get the number of channels (i.e., number of internals). 
	 @return The number of channels. */
	inline int length()	const throw()										{ return numInternalUGens;															}
	
	/** Get one of the internals.
	 @param		index	The index of the UGenInternal to return.
	 @return			One of the UGenInternal classes, this will increment the 
						UGenInternal's reference count before returning. */
	inline UGenInternal* getInternalUGen(const int index) const throw()		{ internalUGens[index]->incrementRefCount(); return internalUGens[index];			}
	
	/** Decrement the reference count of one of the internals.
	 
	 Make sure you know what you're doing with this and ensure the internal is no longer
	 used by this UGen.
	 
	 @param		index The index of the UGenInternal. */
	inline void decrementInternalUGen(const int index) const throw()		{ internalUGens[index]->decrementRefCount();										}
	
	/** Get the value of one of the channels.
	 
	 @param		index	The index of the UGenInternal, larger values will be wrapped to the number of channels.
	 @return			The current value if this is scalar, constant and control rate internal. */
	inline float getValue(const int index = 0) const throw()				{ return internalUGens[index % numInternalUGens]->getValue(index);					}
	
	inline float poll(const int index = 0) throw()							{ return internalUGens[index % numInternalUGens]->poll(index);						}
	
	
	/** Defines actions to be done at the end of an event e.g., a line or envelope. */
	enum DoneAction
	{
		DoNothing,			///< When the event ends it holds its final value
		DeleteWhenDone		///< When the event ends it signals a delete message which should delete an entire UGen graph
	};
	
	/** Defines simple warp factors. */
	enum Warp
	{
		Linear,
		Exponential
	};
	
	/// @} <!-- end Miscellaneous ------------------------------------------------------------ -->
	
	/// @name Public data
	/// @{
	
	/** A null UGen.
	 Equivalent but more convenient and efficient than using the default constructor UGen(). */
	inline static const UGen& getNull() throw() { static UGen null = UGen(); return null; }
	
	/** A control rate null UGen.
	 Equivalent but more convenient and efficient than using the default constructor and a kr() call: UGen().kr() */
	inline static const UGen& getNullKr() throw() { static UGen null = UGen().kr(); return null; }
	
	static const int defaultUserData; ///< the default state of the userData member in all UGen instances.
	int userData; /**<	May be used to store custom user data, the Voicer class uses it to store MIDI 
						note numbers for example so it knows which UGen to release when it gets
						a note off message. */
		
	/// @} <!-- end Public data ---------------------------------------------------------------- -->
	
protected:
	void constructMultichannel(const int numUGens, const UGen uGenArray[]) throw();
	void initInternal(const int numInternalUGensToInit) throw();
	static int findMaxInputChannels(const int count, const UGen * const array) throw();
	static int findMaxInputChannels(const int count, const Buffer * const array) throw();
	static int findMaxInputSizes(const int count, const Buffer * const array) throw();
	void purgeInternalMemory() throw();
	
	void generateFromProxyOwner(ProxyOwnerUGenInternal* proxyOwner) throw();
	
	unsigned int numInternalUGens;
	UGenInternal** internalUGens;
	
	
	static unsigned long nextBlockID;		// blockIDs count samples  and should be enough for about 27hrs @ 44.1kHz
											// a unsigned 64-bit int would be enough for nearly 5 billion days (or over 13 centuries)
	static double sampleRate_;
	static double reciprocalSampleRate;
	static int estimatedSamplesPerBlock_;
	static int controlRateBlockSize;
	static bool isInitialised;
	static Deleter defaultDeleter;
	static Deleter* internalUGenDeleter;
	
	
private:
	
	void incrementInternals() const throw();
	void decrementInternals() const throw();
	
};


UGenArray operator<< (float leftOperand, UGen const& rightOperand) throw();
UGen operator, (float leftOperand, UGen const& rightOperand) throw();



// UGen docs groups

/** A list of all convenience host classes and their brief descriptions.
 "Host" classes are designed to work specifically with a particular host environment (e.g., 
 another library, API, SDK etc). Currently, host classes exist for Juce and the iPhone.
 There are also example projects in the source tree for other environments but these have
 not yet been turned into "host" classes.
 
 @defgroup Hosts			All "host" classes (for easy integration into Juce, iPhone etc). */


/** A list of all UGen classes and their brief descriptions.
 @defgroup AllUGens			All UGen classes. */

/** @defgroup UGenCategogies UGen Categories */

/// @{

/** A list of all oscillator UGen classes and their brief descriptions.
 @defgroup OscUGens			Oscillator UGen classes. */

/** A list of all filtering UGen classes and their brief descriptions.
 This category contains a list of all the UGen classes which filter/EQ a signal.
 This also includes other kinds of signal shaping (e.g., Lag and Decay).
 @defgroup FilterUGens		Filter UGen classes. */

/** A list of all noise/random-based UGen classes and their brief descriptions.
 @defgroup NoiseUGens		Noise UGen classes. */

/** Alist of FFT related UGens.
 @defgroup FFTUGens			FFT-based UGen classes. */

/** A list of all mathematical operator type UGen classes and their brief descriptions.
 This listas all the arithmetic and mathematical operators which may be applied to UGen
 instances. This includes the (overloaded) arithmentic operators such as +, -, * and /.
 @defgroup MathsUGens		Mathemetical UGen classes.	*/

/** A list of all envelope type UGen classes and their brief descriptions.
 @defgroup EnvUGens			Envelope UGen classes.	*/

/** A list of all delay-based UGen classes and their brief descriptions.
 @defgroup DelayUGens		Delay UGen classes.	*/

/** A list of all control UGen classes and their brief descriptions.
 This lists UGen classes useful for control purposes. This inlcudes some
 GUI conrtols available on some platforms.
 @defgroup ControlUGens		Control UGen classes.	*/

/** A list of all soundfile-based UGen classes and their brief descriptions.
 @defgroup SoundFileUGens	Sound File UGen classes.	*/

/** A list of all event-based UGen classes (e.g., Spawn) and their brief descriptions.
 @defgroup EventUGens		Event-based UGen classes.	*/

/// @}

/** A list of all event classes (e.g., SpawnEventBase) and their brief descriptions.
 @defgroup Events			Events for event-based UGen classes.	*/

/** A list of all UGenInternal subclasses.
 Note unless you intend to develop your own internal classes you shouldn't need to look at these!
 
 @section Introduction Introduction
 Classes ending in an upper case 'K' suffix are (by convention) control rate versions. Audio rate versions
 do not normally have a special suffix and are generally the name of the (normally) owning UGen with 
 "UGenInternal" appended. For example, the LFSaw UGen uses an LFSawUGenInternal for its internal. There
 are some exceptions. For example, the SinOsc UGen uses the TableOscUGenInternal by passing it a sine wave
 table. 
 
 Multichannel UGen instances are usually made by creating several instances of the same UGenInternal subclass within
 a UGen. This is because UGenInternal class have only one output (although they may have any number of inputs)
 However, where there are multiple output channels which need to be interrelated (e.g., panning) a "proxy"
 system is used. This involves making the internal a subclass of ProxyOwnerUGenInternal (rather than
 just a UGenInternal). This can then create a number of ProxyUGenInternal objects which act as the additional
 outputs. In the case of Pan2UGenInternal this creates one ProxyUGenInternal which is the right output channel.
 
 @section SmartPointers SmartPointers
 Ref counting..
 
 @section Rendering Rendering
 processBlock()...
 
 @defgroup UGenInternals	UGenInternal classes (for advanced users and UGenInternal developers).	*/

/** A list of all Value and ValueInternal classes and their brief descriptions.
 @defgroup Values			Value classes which evaluate pattern, sequences or random values.	*/


#define DOC_SINGLE	This should be a single value or a single channel				\
					(multichannel UGen instances will be mixed to mono).			\



/** Wrap some text in a normal "pre" Doxygen comment, before the item to be documented. */
#define PREDOC(DOCS) /** DOCS */

/** Wrap some text in a "post" Doxygen comment, after the item to be documented. */
#define POSTDOC(DOCS) /**< DOCS */
//#define POSTDOC PREDOC


#define EVENT_DOCS_OWNED(UGEN_NAME, EVENT_NAME)	/**	An event class for use with a UGEN_NAME UGen.	
													You can use an OwnerType to reference another object when the UGen
													is constructed.	Use EVENT_NAME##<void> or EVENT_NAME##<> if there's no owner.
													@see UGEN_NAME, EVENT_NAME##<void>	
													@ingroup Events */

#define EVENT_DOCS_VOID(UGEN_NAME, EVENT_NAME)	/**	An event class for use with a UGEN_NAME UGen.		
													A specialised version of EVENT_NAME with no owner. 
													@see UGEN_NAME, EVENT_NAME	
													@ingroup Events */

#define EVENT_OWNEDCONSTRUCTOR(EVENT_NAME)		/** Event class constructor, you must pass the owner to the base class.
													e.g., @code MyEventClass(OwnerType* o) : EVENT_NAME< OwnerType > (o) { } @endcode 
													@param o The owner object (o), of type OwnerType, for this event. */


#define EVENT_SPAWNFUNCTION_DOCS					You must implement this virtual function in your subclass.						\
													The parent UGen calls this function each time it needs a new					\
													event instance.																	\
													@param spawn		Allows acces to the internal UGen so that its				\
																		behaviour can be modified (see Spawn for an example)		\
													@param eventCount	The index of the event instance starting at 0 for			\
																		the first event.											\
													@return				The UGen graph for this event instance.


#define EVENT_COMMON_UGEN_DOCS	<B>Generic Event-based %UGen argument documentation:</B>										\
								Event-based UGen instances are created using a template where the first template parameter		\
								is an "event" class (i.e., a subclass of EventBase<OwnerType>) and the second template			\
								parameter is an "owner" type. This must be the same owner type as the event.					\
								Generally a event UGen will be constructed in one of two ways: using a pointer to the			\
								owner which will be used to construct an event of the appropriate type (in turn passing			\
								the owner pointer); or the event UGen may be constructed by passing in an existing				\
								event. This is useful if the event is complex and/or has a custom constructor and				\
								the default constructor(s) are therfore insufficient. Using this method be sure that your		\
								event class can be safely copied (e.g., using normal copy constructors) so don`t store			\
								regular pointers owned by the event class. UGen++ ojects such as Buffer and UGen are fine		\
								since these enclose reference-counted objects.													\
								\n\n\
								COMMON_UGEN_DOCS

#define EVENT_OWNER_DOC																										\
	@tparam OwnerType				Optional owner type (defaults to void).													\

#define EVENT_UGEN_OWNEREVENT_DOC	@param	e/o		The event object (e) to use to generate event instances or a pointer	\
													to its owner (o). The owner (o) may be 0 if there`s no owner (and		\
													OwnerType is void).														\

#define EVENT_UGEN_NUMCHANNELS_DOC(UGEN_NAME)																				\
	@param	numChannels				Number of output channels the UGEN_NAME will have.

#define EVENT_UGEN_MAXREPEATS_DOC(UGEN_NAME)																				\
	@param	maxRepeats				The maximum number of events the UGEN_NAME will generate (or 0 for no maximum).

#define EVENT_TEMPLATESPECIAL_DOC(UGEN_NAME, EVENT_NAME, EVENT_BASE)											\
	@tparam	EVENT_NAME				The name of the EVENT_BASE to use for this UGEN_NAME object.				\
									The OwnerType of the EVENT_NAME must be the same as the the					\
									OwnerType for this UGEN_NAME.												\
									EVENT_OWNER_DOC

#define EVENT_TEMPLATE_DOC(UGEN_NAME)	EVENT_TEMPLATESPECIAL_DOC(UGEN_NAME, UGEN_NAME##EventType, UGEN_NAME##EventBase)


#define EVENT_CONSTRUCTOR_DOC(UGEN_NAME, DOCS)																	\
	/** Base UGEN_NAME Constuctor, use the AR() versions in user code. DOCS */	

#define EventUGenStaticMethodDeclare(UGEN_NAME, METHOD_NAME, EVENT_TYPE, OWNER_TYPE, ARGS_CALL, ARGS_DECLARE)	\
	static inline UGen METHOD_NAME ARGS_DECLARE throw() { return UGEN_NAME<EVENT_TYPE, OWNER_TYPE> ARGS_CALL; }


#define EventUGenMethodsDeclare(UGEN_NAME, EVENT_TYPE, OWNER_TYPE, OWNER_ARGS_CALL, OWNER_ARGS_DECLARE, EVENT_ARGS_CALL, EVENT_ARGS_DECLARE, DOCS) \
	/** Construct and return an audio rate UGEN_NAME creating the event using an owner (or NULL). DOCS	
		@return The audio rate UGEN_NAME. */																					\
	EventUGenStaticMethodDeclare(UGEN_NAME, AR,         EVENT_TYPE, OWNER_TYPE, OWNER_ARGS_CALL, OWNER_ARGS_DECLARE)			\
	/** Construct and return an audio rate UGEN_NAME passing an event to use. DOCS	
		@return The audio rate UGEN_NAME. */																					\
	EventUGenStaticMethodDeclare(UGEN_NAME, AR,         EVENT_TYPE, OWNER_TYPE, EVENT_ARGS_CALL, EVENT_ARGS_DECLARE)			\

/** The base for all event classes.
 Event classes are used to create new UGen graphs based on some kind of specification, the event classes
 provided a means of providing that specification. Commonly in one of the derived classes there will
 a virtual function which will need to be implemented which returns a UGen containing the UGen
 graph required.
 
 The event classes may have an "owner" which would normally be one of the classes in your application or
 plugin which might contain useful values (e.g., slider values or some other parameter data). The event
 classes are templated to allow the type of the owner to be specified at compile time and a pointer to 
 the owner to be passed to the event on construction at run time.
 
 @see EventBase<void>, MixFill, Spawn, TSpawn, XFadeTexture, OverlapTexture, TrigXFade */
template<class OwnerType = void>
class EventBase
{	
public:
	/** Construct an event with a particular owner.  @param o The owner.*/
	EventBase(OwnerType* o) : owner(o) { }
	/** Destructor. */
	virtual ~EventBase() { owner = 0; }
	
	/** Get the owner. @return The owner. */
	OwnerType* getOwner() { return owner; }
	
protected:
private:
	OwnerType *owner;
	EventBase();
};

/** Specialised event class with no owner (i.e., OwnerType is void) 
 @see EventBase<OwnerType>
 */
template<>
class EventBase<void>
{	
public:
	virtual ~EventBase() {  }
};



#endif // UGEN_UGEN_H



