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

/* todo
 hypotApx .. hypotenuse approximation
 amclip .. two quadrant multiply   { 0 when b <= 0, a*b when b > 0 }
 scaleneg .. nonlinear amplification   { a when a >= 0, a*b when a < 0 }
 excess .. residual of clipping   a - clip2(a,b) 
 */

#ifndef UGEN_BINARYOPUGENS_H
#define UGEN_BINARYOPUGENS_H


#include "../core/ugen_UGen.h"
#include "../core/ugen_Value.h"
#include "ugen_InlineBinaryOps.h"



#define BinaryOpUGenConstructor(INTERNALUGENCLASSNAME, leftOperand_, rightOperand_)										\
		if(leftOperand_.getNumChannels() > rightOperand_.getNumChannels())												\
			initInternal(leftOperand_.getNumChannels());																\
		else																											\
			initInternal(rightOperand_.getNumChannels());																\
																														\
		for(int i = 0; i < numInternalUGens; i++)																		\
		{																												\
			internalUGens[i] = new INTERNALUGENCLASSNAME(leftOperand_, rightOperand_);									\
		}


#define BinaryOpSymbolUGenProcessBlock(shouldDelete_, blockID_, channel_, OPSYMBOL_INTERNAL)							\
		const int numSamplesToProcess = uGenOutput.getBlockSize();														\
		float* outputSamples = uGenOutput.getSampleData();																\
		const float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID_, channel_);			\
		const float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID_, channel_);			\
																														\
		for(int i = 0; i < numSamplesToProcess; ++i) {																	\
			outputSamples[i] = leftOperandSamples[i] OPSYMBOL_INTERNAL rightOperandSamples[i];							\
		}
	

#define BinaryOpSymbolUGenProcessBlock_K(shouldDelete_, blockID_, channel_, OPSYMBOL_INTERNAL)							\
		const int krBlockSize = UGen::getControlRateBlockSize();														\
		unsigned int blockPosition = blockID % krBlockSize;																\
		int numSamplesToProcess = uGenOutput.getBlockSize();															\
		float* outputSamples = uGenOutput.getSampleData();																\
		float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID_, channel_);					\
		float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID_, channel_);				\
																														\
		int numKrSamples = blockPosition % krBlockSize;																	\
																														\
			while(numSamplesToProcess > 0) {																			\
				float nextValue = value;																				\
				if(numKrSamples == 0)																					\
					nextValue = *leftOperandSamples OPSYMBOL_INTERNAL *rightOperandSamples;								\
																														\
				numKrSamples = krBlockSize - numKrSamples;																\
																														\
				blockPosition		+= numKrSamples;																	\
				leftOperandSamples	+= numKrSamples;																	\
				rightOperandSamples	+= numKrSamples;																	\
																														\
				if(nextValue == value) {																				\
					while(numSamplesToProcess && numKrSamples) {														\
						*outputSamples++ = nextValue;																	\
						--numSamplesToProcess;																			\
						--numKrSamples;																					\
					}																									\
				} else {																								\
					float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();					\
					while(numSamplesToProcess && numKrSamples) {														\
						*outputSamples++ = value;																		\
						value += valueSlope;																			\
						--numSamplesToProcess;																			\
						--numKrSamples;																					\
					}																									\
					value = nextValue;																					\
				}																										\
			}																											\


#define BinaryOpFunctionUGenProcessBlock(shouldDelete_, blockID_, channel_, OPFUNCTION_INTERNAL)						\
		const int numSamplesToProcess = uGenOutput.getBlockSize();														\
		float* outputSamples = uGenOutput.getSampleData();																\
		const float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID_, channel_);			\
		const float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID_, channel_);			\
																														\
		for(int i = 0; i < numSamplesToProcess; ++i) {																	\
			outputSamples[i] = OPFUNCTION_INTERNAL(leftOperandSamples[i], rightOperandSamples[i]);						\
		}


#define BinaryOpFunctionUGenProcessBlock_K(shouldDelete_, blockID_, channel_, OPFUNCTION_INTERNAL)						\
		const int krBlockSize = UGen::getControlRateBlockSize();														\
		unsigned int blockPosition = blockID % krBlockSize;																\
		int numSamplesToProcess = uGenOutput.getBlockSize();															\
		float* outputSamples = uGenOutput.getSampleData();																\
		float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID_, channel_);					\
		float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID_, channel_);				\
																														\
		int numKrSamples = blockPosition & krBlockSize;																	\
																														\
			while(numSamplesToProcess > 0) {																			\
				float nextValue = value;																				\
				if(numKrSamples == 0)																					\
					nextValue = OPFUNCTION_INTERNAL(*leftOperandSamples, *rightOperandSamples);							\
																														\
				numKrSamples = krBlockSize - numKrSamples;																\
																														\
				blockPosition		+= numKrSamples;																	\
				leftOperandSamples	+= numKrSamples;																	\
				rightOperandSamples	+= numKrSamples;																	\
																														\
				if(nextValue == value) {																				\
					while(numSamplesToProcess && numKrSamples) {														\
						*outputSamples++ = nextValue;																	\
						--numSamplesToProcess;																			\
						--numKrSamples;																					\
					}																									\
				} else {																								\
					float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();					\
					while(numSamplesToProcess && numKrSamples) {														\
						*outputSamples++ = value;																		\
						value += valueSlope;																			\
						--numSamplesToProcess;																			\
						--numKrSamples;																					\
					}																									\
					value = nextValue;																					\
				}																										\
			}																											\


#define BinaryOpCommonUGenDeclaration(OPNAME)																			\
		/** Internal for Binary##OPNAME##UGen @ingroup UGenInternals */													\
		class Binary##OPNAME##UGenInternal : public BinaryOpUGenInternal												\
		{																												\
		public:																											\
			Binary##OPNAME##UGenInternal(UGen const& leftOperand, UGen const& rightOperand) throw();					\
			UGenInternal* getChannel(const int channel) throw();														\
			UGenInternal* getKr() throw();																				\
			void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();				\
			float getValue(const int channel) const throw();															\
		};																												\
		/** Control rate internal for Binary##OPNAME##UGen @ingroup UGenInternals */									\
		class Binary##OPNAME##UGenInternalK : public Binary##OPNAME##UGenInternal										\
		{																												\
		public:																											\
			Binary##OPNAME##UGenInternalK(UGen const& leftOperand, UGen const& rightOperand) throw();					\
			UGenInternal* getKr() throw() { incrementRefCount(); return this; }											\
			void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();				\
		private:																										\
			float value;																								\
		};																												\
		/** Internal for Binary##OPNAME##Value.	*/																		\
		class Binary##OPNAME##ValueInternal : public BinaryOpValueInternal												\
		{																												\
		public:																											\
			Binary##OPNAME##ValueInternal(Value const& leftOperand, Value const& rightOperand) throw();					\
			double getValue() throw();																					\
		};																												\
		/**	Applies OPNAME to the left and right values. */																\
		class Binary##OPNAME##Value : public Value																		\
		{																												\
		public:																											\
			Binary##OPNAME##Value(Value const& leftOperand, Value const& rightOperand) throw();							\
		};	

#define BinaryOpSymbolUGenDeclaration(OPNAME, OPSYMBOL)																	\
		/** A BinaryOpUGen usually created using the operator OPSYMBOL when applied to other UGen instances. 
			It is not normally required to use this UGen explicitly. 
			@code
				UGen result##OPNAME = SinOsc::AR(100, 0, 0.4) OPSYMBOL SinOsc::AR(400, 0, 0.3); // creates a Binary##OPNAME##UGen
			@endcode 
			@see ugen::operator##OPSYMBOL##(const float leftOperand, UGen const& rightOperand)	
			@see UGen::operator##OPSYMBOL##()	
			@ingroup AllUGens MathsUGens */																				\
		class Binary##OPNAME##UGen : public UGen																		\
		{																												\
		public:																											\
			/** Construct a Binary##OPNAME##UGen.
				For symbol-based binary operations these will be from expressions such as:
				@code left OPSYMBOL right; @endcode
				@param leftOperand	The left-hand-side of the operation.
				@param rightOperand The right-hand-side of the operation. */											\
			Binary##OPNAME##UGen(UGen const& leftOperand, UGen const& rightOperand) throw();							\
		};																												\
		BinaryOpCommonUGenDeclaration(OPNAME)																			\
		/** Global function for the UGen::operator##OPSYMBOL##() binary operator
			where the left-hand-side is not a UGen.
			@param leftOperand The left-hand-side of the operator (a float, double or int)
			@param rightOperand The right-hand-side of the operator. 
			@return The new UGen which is a result of the OPSYMBOL operator operation. 
			@see UGen::operator##OPSYMBOL##() */																		\
		UGen operator OPSYMBOL (const float leftOperand, UGen const& rightOperand) throw();								\
		/** Return a new Buffer with the binary operator applied to all channels. The resulting Buffer will
			be the size of the rightOperand and have the same number of channels. The constant 
			leftOperand will be applied to all channels and values using the OPSYMBOL operator.
			@see Buffer::operator##OPSYMBOL(Buffer const& rightOperand) */												\
		Buffer operator OPSYMBOL (const float leftOperand, Buffer const& rightOperand) throw();							\
		Value operator OPSYMBOL (const double leftOperand, Value const& rightOperand) throw();							\
		ValueArray operator OPSYMBOL (const double leftOperand, ValueArray const& rightOperand) throw();				\

#define BinaryOpFunctionUGenDeclaration(OPNAME, OPFUNCTION)																\
		/** A BinaryOpUGen usually created using the function OPFUNCTION##() when applied to other UGen instances. 
			It is not normally required to use this UGen explicitly. 
			@code
				UGen result##OPNAME = SinOsc::AR(100, 0, 0.4).##OPFUNCTION##(SinOsc::AR(400, 0, 0.3)); // creates a Binary##OPNAME##UGen
			@endcode 
			You can also use a global version of this function, the above code would be equivalent as:
			@code
				UGen result##OPNAME = OPFUNCTION##(SinOsc::AR(100, 0, 0.4), SinOsc::AR(400, 0, 0.3)); // creates a Binary##OPNAME##UGen
			@endcode 
			
			@see OPFUNCTION##(UGen const& leftOperand, UGen const& rightOperand)
			@see OPFUNCTION##(const float leftOperand, UGen const& rightOperand)
			@see UGen::##OPFUNCTION##()
			@ingroup AllUGens MathsUGens */																				\
		class Binary##OPNAME##UGen : public UGen																		\
		{																												\
		public:																											\
			/** Construct a Binary##OPNAME##UGen.
				For function-based binary operations these will be from expressions such as:
				@code OPFUNCTION##(left, right); @endcode
				@param leftOperand	The left-hand-side of the oprtation.
				@param rightOperand The right-hand-side of the oprtation.*/												\
			Binary##OPNAME##UGen(UGen const& leftOperand, UGen const& rightOperand) throw();							\
		};																												\
		BinaryOpCommonUGenDeclaration(OPNAME)																			\
		/** Global function for the UGen::##OPFUNCTION##() binary operator.
			@param leftOperand The left-hand-side of the operator (a float, double or int)
			@param rightOperand The right-hand-side of the operator. 
			@return The new UGen which is a result of the OPFUNCTION##() operation. 
			@see UGen::##OPFUNCTION##() */																				\
		UGen OPFUNCTION(const float leftOperand, UGen const& rightOperand) throw();										\
		/** Global function for the UGen::##OPFUNCTION##() binary operator.
			This makes:
			@code
				UGen result##OPNAME = SinOsc::AR(100, 0, 0.4).##OPFUNCTION##(SinOsc::AR(400, 0, 0.3)); // creates a Binary##OPNAME##UGen
			@endcode
			..equivalent to:
			@code
				UGen result##OPNAME = OPFUNCTION##(SinOsc::AR(100, 0, 0.4), SinOsc::AR(400, 0, 0.3)); // creates a Binary##OPNAME##UGen
			@endcode 
			@param leftOperand The left-hand-side of the operator.
			@param rightOperand The right-hand-side of the operator. 
			@return The new UGen which is a result of the OPFUNCTION##() operation. 
			@see UGen::##OPFUNCTION##() */																				\
		UGen OPFUNCTION(UGen const& leftOperand, UGen const& rightOperand) throw();										\
		/** Return a new Buffer with the binary operator applied to all channels. The resulting Buffer will
			be the size of the rightOperand and have the same number of channels. The constant 
			leftOperand will be applied to all channels and values using the OPFUNCTION##() function.
			@see Buffer::##OPFUNCTION##(Buffer const& rightOperand)
		 */																												\
		Buffer OPFUNCTION(const float leftOperand, Buffer const& rightOperand) throw();									\
		/** Return a new Buffer with the binary operator applied to all channels. The resulting Buffer will
			be the size of the largest of the two input Buffers, the smaller Buffer will be effectively padded with
			zeroes. The resulting Buffer will have a number of channels equal to the highest number of channels in
			the two input Buffers. When applying the binary operator to two Buffers which have a different number
			of channels the smaller Buffer's channels will be accessed cyclically (i.e., using wrapped indices).
			@see Buffer::##OPFUNCTION##(Buffer const& rightOperand)
		*/																												\
		Buffer OPFUNCTION(Buffer const& leftOperand, Buffer const& rightOperand) throw();								\
		Value OPFUNCTION(const double leftOperand, Value const& rightOperand) throw();									\
		Value OPFUNCTION(Value const& leftOperand, Value const& rightOperand) throw();									\
		ValueArray OPFUNCTION(const double leftOperand, ValueArray const& rightOperand) throw();						\
		ValueArray OPFUNCTION(ValueArray const& leftOperand, ValueArray const& rightOperand) throw();									


#define BinaryOpCommonUGenDefinition(OPNAME)																			\
	Binary##OPNAME##UGenInternal::Binary##OPNAME##UGenInternal(UGen const& leftOperand,									\
															   UGen const& rightOperand) throw()						\
	:	BinaryOpUGenInternal(leftOperand, rightOperand)																	\
	{	}																												\
																														\
	UGenInternal* Binary##OPNAME##UGenInternal::getChannel(const int channel) throw()									\
	{																													\
		return new Binary##OPNAME##UGenInternal(inputs[LeftOperand].getChannel(channel),								\
												inputs[RightOperand].getChannel(channel));								\
	}																													\
																														\
	UGenInternal* Binary##OPNAME##UGenInternal::getKr() throw()															\
	{																													\
		return new Binary##OPNAME##UGenInternalK(inputs[LeftOperand].kr(), inputs[RightOperand].kr());					\
	}																													\
																														\
	Binary##OPNAME##UGenInternalK::Binary##OPNAME##UGenInternalK(UGen const& leftOperand,								\
																 UGen const& rightOperand) throw()						\
	:	Binary##OPNAME##UGenInternal(leftOperand, rightOperand), value(0.f)												\
	{																													\
		rate = ControlRate;																								\
	}																													\
																														\
	Binary##OPNAME##ValueInternal::Binary##OPNAME##ValueInternal(Value const& leftOperand, Value const& rightOperand) throw()		\
	:	BinaryOpValueInternal(leftOperand, rightOperand)																			\
	{																																\
	}																																\
																																	\
	Binary##OPNAME##Value::Binary##OPNAME##Value(Value const& leftOperand, Value const& rightOperand) throw()						\
	:	Value(new Binary##OPNAME##ValueInternal(leftOperand, rightOperand)) { }														\

#define BinaryOpSymbolUGenDefinition(OPNAME, OPSYMBOL, OPSYMBOL_INTERNAL)												\
	BinaryOpSymbolUGenDefinitionNoProcessBlock(OPNAME, OPSYMBOL, OPSYMBOL_INTERNAL)										\
																														\
	void Binary##OPNAME##UGenInternal::processBlock(bool& shouldDelete,													\
													const unsigned int blockID,											\
													const int channel) throw()											\
	{																													\
		BinaryOpSymbolUGenProcessBlock(shouldDelete, blockID, channel, OPSYMBOL_INTERNAL)								\
	}																													\


#define BinaryOpSymbolUGenDefinitionNoProcessBlock(OPNAME, OPSYMBOL, OPSYMBOL_INTERNAL)									\
	BinaryOpCommonUGenDefinition(OPNAME)																				\
																														\
	Binary##OPNAME##UGen::Binary##OPNAME##UGen(UGen const& leftOperand, UGen const& rightOperand) throw()				\
	{																													\
		BinaryOpUGenConstructor(Binary##OPNAME##UGenInternal, leftOperand, rightOperand)								\
	}																													\
																														\
	float Binary##OPNAME##UGenInternal::getValue(const int channel) const throw()										\
	{																													\
		return inputs[LeftOperand].getValue(channel) OPSYMBOL inputs[RightOperand].getValue(channel);					\
	}																													\
																														\
	void Binary##OPNAME##UGenInternalK::processBlock(bool& shouldDelete,												\
													 const unsigned int blockID,										\
													 const int channel) throw()											\
	{																													\
		BinaryOpSymbolUGenProcessBlock_K(shouldDelete, blockID, channel, OPSYMBOL_INTERNAL)								\
	}																													\
																														\
	UGen operator OPSYMBOL (const float leftOperand, UGen const& rightOperand) throw()									\
	{																													\
		if(rightOperand.isNull())	return UGen(leftOperand);															\
		else						return UGen(leftOperand) OPSYMBOL rightOperand;										\
	}																													\
																														\
	UGen UGen::operator OPSYMBOL (UGen const& rightOperand)	const throw()												\
	{																													\
		if(isNull())	return rightOperand;																			\
		else			return Binary##OPNAME##UGen(*this, rightOperand);												\
	}																													\
																														\
	UGenArray UGenArray::operator OPSYMBOL (UGenArray const& rightOperand) const throw()								\
	{																													\
		int size;																										\
																														\
		if(internal->size() > rightOperand.internal->size())  size = internal->size();									\
		else												  size = rightOperand.internal->size();						\
																														\
		UGenArray newArray(size);																						\
																														\
		for(int i = 0; i < size; i++) {																					\
			newArray.internal->getArray()[i] = wrapAt(i) OPSYMBOL rightOperand.wrapAt(i);								\
		}																												\
																														\
		return newArray;																								\
	}																													\
																														\
	Buffer Buffer::operator OPSYMBOL (Buffer const& rightOperand) const throw()											\
	{																													\
		const int newNumChannels = numChannels_ > rightOperand.numChannels_ ? numChannels_ : rightOperand.numChannels_;	\
		const int maxSize = size_ > rightOperand.size_ ? size_ : rightOperand.size_;									\
		const int minSize = size_ < rightOperand.size_ ? size_ : rightOperand.size_;									\
		const int diffSize = maxSize-minSize;																			\
																														\
		Buffer newBuffer(BufferSpec(maxSize, newNumChannels, false));													\
																														\
		for(int channelIndex = 0; channelIndex < newNumChannels; channelIndex++)										\
		{																												\
			int numSamples;																								\
			float* leftSamples = channels[channelIndex % numChannels_]->data;											\
			float* rightSamples = rightOperand.channels[channelIndex % rightOperand.numChannels_]->data;				\
			float* outputSamples = newBuffer.channels[channelIndex]->data;												\
																														\
			if(size_ == 1 && numChannels_ == 1) {																		\
				numSamples = maxSize;																					\
																														\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = *leftSamples OPSYMBOL_INTERNAL *rightSamples++;									\
				}																										\
			} else if(rightOperand.size_ == 1 && rightOperand.numChannels_ == 1) {										\
				numSamples = maxSize;																					\
																														\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = *leftSamples++ OPSYMBOL_INTERNAL *rightSamples;									\
				}																										\
			} else {																									\
				numSamples = minSize;																					\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = *leftSamples++ OPSYMBOL_INTERNAL *rightSamples++;								\
				}																										\
																														\
				numSamples = diffSize;																					\
				if(size_ > rightOperand.size_) {																		\
					while(--numSamples >= 0) {																			\
						*outputSamples++ = *leftSamples++ OPSYMBOL_INTERNAL 0.f;										\
					}																									\
				}																										\
				else if(rightOperand.size_ > size_) {																	\
					while(--numSamples >= 0) {																			\
						*outputSamples++ = 0.f OPSYMBOL_INTERNAL *rightSamples++;										\
					}																									\
				}																										\
			}																											\
		}																												\
																														\
		return newBuffer;																								\
	}																													\
																														\
	Buffer operator OPSYMBOL(const float leftOperand, Buffer const& rightOperand) throw()								\
	{																													\
		if(rightOperand.isNull())	return Buffer(leftOperand);															\
		else						return Buffer(leftOperand) OPSYMBOL rightOperand;									\
	}																													\
																														\
	double Binary##OPNAME##ValueInternal::getValue() throw()															\
	{																													\
		return leftOperand_.getValue() OPSYMBOL rightOperand_.getValue();												\
	}																													\
																														\
	Value operator OPSYMBOL (const double leftOperand, Value const& rightOperand) throw()								\
	{																													\
		return Value(leftOperand) OPSYMBOL rightOperand;																\
	}																													\
																														\
	Value Value::operator OPSYMBOL (Value const& rightOperand)	const throw()											\
	{																													\
		return Binary##OPNAME##Value(*this, rightOperand);																\
	}																													\
																														\
	ValueArray operator OPSYMBOL (const double leftOperand, ValueArray const& rightOperand) throw()						\
	{																													\
		return ValueArray(leftOperand) OPSYMBOL rightOperand;															\
	}																													\
																														\
	ValueArray ValueArray::operator OPSYMBOL (ValueArray const& rightOperand)	const throw()							\
	{																													\
		if(size() == 0)																									\
			return rightOperand;																						\
		else if(rightOperand.size() == 0)																				\
			return *this;																								\
		else {																											\
			const int maxSize = ugen::max(size(), rightOperand.size());													\
			ValueArray newArray(maxSize);																				\
			for(int i = 0; i < maxSize; i++) {																			\
				newArray.put(i, wrapAt(i) OPSYMBOL rightOperand.wrapAt(i));												\
			}																											\
			return newArray;																							\
		}																												\
	}	


	
#define BinaryOpFunctionUGenDefinition(OPNAME, OPFUNCTION, OPFUNCTION_INTERNAL)											\
	BinaryOpCommonUGenDefinition(OPNAME)																				\
																														\
	Binary##OPNAME##UGen::Binary##OPNAME##UGen(UGen const& leftOperand, UGen const& rightOperand) throw()				\
	{																													\
		BinaryOpUGenConstructor(Binary##OPNAME##UGenInternal, leftOperand, rightOperand)								\
	}																													\
																														\
	void Binary##OPNAME##UGenInternal::processBlock(bool& shouldDelete,													\
													const unsigned int blockID,											\
													const int channel) throw()											\
	{																													\
		BinaryOpFunctionUGenProcessBlock(shouldDelete, blockID, channel, OPFUNCTION_INTERNAL)							\
	}																													\
																														\
	float Binary##OPNAME##UGenInternal::getValue(const int channel) const throw()										\
	{																													\
		return OPFUNCTION_INTERNAL(inputs[LeftOperand].getValue(channel), inputs[RightOperand].getValue(channel));		\
	}																													\
																														\
	void Binary##OPNAME##UGenInternalK::processBlock(bool& shouldDelete,												\
													 const unsigned int blockID,										\
													 const int channel) throw()											\
	{																													\
		BinaryOpFunctionUGenProcessBlock_K(shouldDelete, blockID, channel, OPFUNCTION_INTERNAL)							\
	}																													\
																														\
	UGen OPFUNCTION(const float leftOperand, UGen const& rightOperand) throw()											\
	{																													\
		if(rightOperand.isNull())	return UGen(leftOperand);															\
		else						return UGen(leftOperand).OPFUNCTION(rightOperand);									\
	}																													\
	UGen OPFUNCTION(UGen const& leftOperand, UGen const& rightOperand) throw()											\
	{																													\
		if(rightOperand.isNull())	return leftOperand;																	\
		else						return leftOperand.OPFUNCTION(rightOperand);										\
	}																													\
																														\
	UGen UGen::OPFUNCTION(UGen const& rightOperand)	const throw()														\
	{																													\
		if(isNull())	return rightOperand;																			\
		else			return Binary##OPNAME##UGen(*this, rightOperand);												\
	}																													\
																														\
	UGenArray UGenArray::OPFUNCTION(UGenArray const& rightOperand) const throw()										\
	{																													\
		if(internal == 0) return UGenArray();																			\
																														\
		int size;																										\
																														\
		if(internal->size() > rightOperand.internal->size())  size = internal->size();									\
		else												  size = rightOperand.internal->size();						\
																														\
		UGenArray newArray(size);																						\
																														\
		for(int i = 0; i < size; i++) {																					\
			newArray.internal->getArray()[i] = wrapAt(i).OPFUNCTION(rightOperand.wrapAt(i));							\
		}																												\
																														\
		return newArray;																								\
	}																													\
																														\
	Buffer Buffer::OPFUNCTION (Buffer const& rightOperand) const throw()												\
	{																													\
		const int newNumChannels = numChannels_ > rightOperand.numChannels_ ? numChannels_ : rightOperand.numChannels_;	\
		const int maxSize = size_ > rightOperand.size_ ? size_ : rightOperand.size_;									\
		const int minSize = size_ < rightOperand.size_ ? size_ : rightOperand.size_;									\
		const int diffSize = maxSize-minSize;																			\
																														\
		Buffer newBuffer(BufferSpec(maxSize, newNumChannels, false));													\
																														\
		for(int channelIndex = 0; channelIndex < newNumChannels; channelIndex++)										\
		{																												\
			int numSamples;																								\
			float* leftSamples = channels[channelIndex % numChannels_]->data;											\
			float* rightSamples = rightOperand.channels[channelIndex % rightOperand.numChannels_]->data;				\
			float* outputSamples = newBuffer.channels[channelIndex]->data;												\
																														\
			if(size_ == 1 && numChannels_ == 1) {																		\
				numSamples = maxSize;																					\
																														\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = ugen::OPFUNCTION_INTERNAL(*leftSamples, *rightSamples++);						\
				}																										\
			} else if(rightOperand.size_ == 1 && rightOperand.numChannels_ == 1) {										\
				numSamples = maxSize;																					\
																														\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = ugen::OPFUNCTION_INTERNAL(*leftSamples++, *rightSamples);						\
				}																										\
			} else {																									\
				numSamples = minSize;																					\
				while(--numSamples >= 0) {																				\
					*outputSamples++ = ugen::OPFUNCTION_INTERNAL(*leftSamples++, *rightSamples++);						\
				}																										\
																														\
				numSamples = diffSize;																					\
				if(size_ > rightOperand.size_) {																		\
					while(--numSamples >= 0) {																			\
						*outputSamples++ = ugen::OPFUNCTION_INTERNAL(*leftSamples++, 0.f);								\
					}																									\
				}																										\
				else if(rightOperand.size_ > size_) {																	\
					while(--numSamples >= 0) {																			\
						*outputSamples++ = ugen::OPFUNCTION_INTERNAL(0.f, *rightSamples++);								\
					}																									\
				}																										\
			}																											\
		}																												\
																														\
		return newBuffer;																								\
	}																													\
																														\
	Buffer OPFUNCTION(const float leftOperand, Buffer const& rightOperand) throw()										\
	{																													\
		if(rightOperand.isNull())	return Buffer(leftOperand);															\
		else						return Buffer(leftOperand).OPFUNCTION(rightOperand);								\
	}																													\
																														\
	Buffer OPFUNCTION(Buffer const& leftOperand, Buffer const& rightOperand) throw()									\
	{																													\
		if(rightOperand.isNull())		return leftOperand;																\
		else if(leftOperand.isNull())	return rightOperand;															\
		else							return Buffer(leftOperand).OPFUNCTION(rightOperand);							\
	}																													\
																														\
	double Binary##OPNAME##ValueInternal::getValue() throw()															\
	{																													\
		return ugen::OPFUNCTION_INTERNAL(leftOperand_.getValue(), rightOperand_.getValue());							\
	}																													\
																														\
	Value OPFUNCTION(const double leftOperand, Value const& rightOperand) throw()										\
	{																													\
		return Value(leftOperand).OPFUNCTION(rightOperand);																\
	}																													\
																														\
	Value OPFUNCTION(Value const& leftOperand, Value const& rightOperand) throw()										\
	{																													\
		return leftOperand.OPFUNCTION(rightOperand);																	\
	}																													\
																														\
	Value Value::OPFUNCTION(Value const& rightOperand)	const throw()													\
	{																													\
		return Binary##OPNAME##Value(*this, rightOperand);																\
	}																													\
																														\
	ValueArray OPFUNCTION(ValueArray const& leftOperand, ValueArray const& rightOperand) throw()						\
	{																													\
		return leftOperand.OPFUNCTION(rightOperand);																	\
	}																													\
																														\
	ValueArray ValueArray::OPFUNCTION(ValueArray const& rightOperand) const throw()										\
	{																													\
		if(size() == 0)																									\
			return rightOperand;																						\
		else if(rightOperand.size() == 0)																				\
			return *this;																								\
		else {																											\
			const int maxSize = ugen::max(size(), rightOperand.size());													\
			ValueArray newArray(maxSize);																				\
			for(int i = 0; i < maxSize; i++) {																			\
				newArray.put(i, wrapAt(i).OPFUNCTION(rightOperand.wrapAt(i)));											\
			}																											\
			return newArray;																							\
		}																												\
	}	



class BinaryOpUGenInternal : public UGenInternal
{
public:
	BinaryOpUGenInternal(UGen const& leftOperand, UGen const& rightOperand, const int channel = -1) throw();
	
	enum Inputs { LeftOperand, RightOperand, NumInputs };
	
protected:
};

class BinaryOpValueInternal : public ValueInternal
{
public:
	BinaryOpValueInternal(Value const& leftOperand, Value const& rightOperand) throw();
		
	Value getLeftOperand() throw()	{ return leftOperand_;  }
	Value getRightOperand() throw()	{ return rightOperand_; }
	
protected:
	Value leftOperand_, rightOperand_;
};



BinaryOpSymbolUGenDeclaration(Add,					+);
BinaryOpSymbolUGenDeclaration(Subtract,				-);
BinaryOpSymbolUGenDeclaration(Multiply,				*);
//BinaryOpSymbolUGenDeclaration(Divide,				/);
BinaryOpSymbolUGenDeclaration(LessThan,				<);
BinaryOpSymbolUGenDeclaration(GreaterThan,			>);
BinaryOpSymbolUGenDeclaration(LessThanOrEquals,		<=);
BinaryOpSymbolUGenDeclaration(GreaterThanOrEquals,	>=);
//BinaryOpSymbolUGenDeclaration(Equal,				==);
//BinaryOpSymbolUGenDeclaration(NotEqual,				!=);

BinaryOpFunctionUGenDeclaration(IsEqualTo,				isEqualTo);
BinaryOpFunctionUGenDeclaration(IsNotEqualTo,			isNotEqualTo);

BinaryOpFunctionUGenDeclaration(Pow,				pow);
BinaryOpFunctionUGenDeclaration(Hypot,				hypot);
BinaryOpFunctionUGenDeclaration(Atan2,				atan2);
BinaryOpFunctionUGenDeclaration(Min,				min);
BinaryOpFunctionUGenDeclaration(Max,				max);

BinaryOpFunctionUGenDeclaration(Clip2,				clip2);
BinaryOpFunctionUGenDeclaration(Wrap,				wrap);
BinaryOpFunctionUGenDeclaration(Wrap2,				wrap2);
BinaryOpFunctionUGenDeclaration(Fold,				fold);
BinaryOpFunctionUGenDeclaration(Fold2,				fold2);

BinaryOpFunctionUGenDeclaration(Ring1,				ring1);
BinaryOpFunctionUGenDeclaration(Ring2,				ring2);
BinaryOpFunctionUGenDeclaration(Ring3,				ring3);
BinaryOpFunctionUGenDeclaration(Ring4,				ring4);

BinaryOpFunctionUGenDeclaration(Round,				round);
BinaryOpFunctionUGenDeclaration(Trunc,				trunc);
BinaryOpFunctionUGenDeclaration(SumSqr,				sumsqr);
BinaryOpFunctionUGenDeclaration(DifSqr,				difsqr);
BinaryOpFunctionUGenDeclaration(SqrSum,				sqrsum);
BinaryOpFunctionUGenDeclaration(SqrDif,				sqrdif);
BinaryOpFunctionUGenDeclaration(AbsDif,				absdif);
BinaryOpFunctionUGenDeclaration(Thresh,				thresh);



/** Internal for BinaryDivideUGen */
class BinaryDivideUGenInternal : public BinaryOpUGenInternal 
{ 
public: 
	BinaryDivideUGenInternal(UGen const& leftOperand, UGen const& rightOperand) throw(); 
	UGenInternal* getChannel(const int channel) throw(); 
	UGenInternal* getKr() throw(); 
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw(); 
}; 

/** Control rate internal for BinaryDivideUGen */
class BinaryDivideUGenInternalK : public BinaryDivideUGenInternal 
{ 
public: 
	BinaryDivideUGenInternalK(UGen const& leftOperand, UGen const& rightOperand) throw(); 
	UGenInternal* getKr() throw() { incrementRefCount(); return this; } 
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw(); 
	float getValue(const int channel) const throw() 
	{ 
		return inputs[LeftOperand].getValue(channel) / inputs[RightOperand].getValue(channel);
	}
	
private: 
	float value; 
}; 

/** A BinaryOpUGen usually created using the operator / when applied to other UGen instances. 
 It is not normally required to use this UGen explicitly. 
 @code
 UGen resultDivide = SinOsc::AR(100, 0, 0.4) / SinOsc::AR(400, 0, 0.3); // creates a BinaryDivideUGen
 @endcode 
 However if rightOperand is a constant the UGen creates a multiplication operation instead (for
 efficiency) e.g,
 
 @code
 UGen resultDivide = SinOsc::AR(100, 0, 0.4) / 2; // creates a BinaryMulUGen
 @endcode
 
 ..would create an identical UGen graph to:
 
 @code
 UGen resultDivide = SinOsc::AR(100, 0, 0.4) * 0.5; // creates a BinaryMulUGen
 @endcode
 
 ..thus divide operations using constants may be used safely (to retain clarity of code) without
 the expense of a divide operation in the inner DSP loop.
 
 @ingroup AllUGens MathsUGens
 @see ugen::operator/(const float leftOperand, UGen const& rightOperand)	*/
class BinaryDivideUGen : public UGen 
{ 
public: 
	/** Construct a BinaryDivideUGen.
	 For symbol-based binary operations these will be from expressions such as:
	 @code left / right; @endcode
	 Althouth as discussed above the UGen may decide to create a multiplication operation 
	 instead (if this is possible and beneficial).
	 @param leftOperand		The left-hand-side of the operation.
	 @param rightOperand	The right-hand-side of the operation. */
	BinaryDivideUGen(UGen const& leftOperand, UGen const& rightOperand) throw(); 
}; 

/** Global function for the UGen::operator/() binary operator
 where the left-hand-side is not a UGen.
 @param leftOperand		The left-hand-side of the operator (a float, double or int)
 @param rightOperand	The right-hand-side of the operator. 
 @return The new UGen which is a result of the / operator operation. 
 @see UGen::operator/() */
UGen operator / (const float leftOperand, UGen const& rightOperand) throw(); 

/** Return a new Buffer with the binary operator applied to all channels. The resulting Buffer will
 be the size of the rightOperand and have the same number of channels. The constant 
 leftOperand will be applied to all channels and values using the / operator.
 Like UGen, Buffer does some optimising if rightOperand is a single value and implments
 a multiplication instead (if this is possible and beneficial).
 @see Buffer::operator/(Buffer const& rightOperand) */
Buffer operator / (const float leftOperand, Buffer const& rightOperand) throw();

Value operator / (const double leftOperand, Value const& rightOperand) throw();
ValueArray operator / (const double leftOperand, ValueArray const& rightOperand) throw();

/** Internal for BinaryDivideValue.	*/
class BinaryDivideValueInternal : public BinaryOpValueInternal													
{																												
public:																											
	BinaryDivideValueInternal(Value const& leftOperand, Value const& rightOperand) throw();					
	double getValue() throw();																					
};																												

/**	Applies a division to the left and right values. */
class BinaryDivideValue : public Value																			
{																												
public:																											
	BinaryDivideValue(Value const& leftOperand, Value const& rightOperand) throw();							
};	


/** Internal for IngoreRightOperandUGen */
class IngoreRightOperandUGenInternal : public BinaryOpUGenInternal 
{ 
public: 
	IngoreRightOperandUGenInternal(UGen const& leftOperand, UGen const& rightOperand) throw(); 
	UGenInternal* getChannel(const int channel) throw(); 
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw(); 
}; 

class IngoreRightOperandUGen : public UGen 
{ 
public: 
	IngoreRightOperandUGen(UGen const& leftOperand, UGen const& rightOperand) throw(); 
}; 


// the start of templatising the macros...

template<BinaryOpFunction op>
class BinaryOpUGenInternalT : public BinaryOpUGenInternal
{
public:
	BinaryOpUGenInternalT(UGen const& leftOperand, UGen const& rightOperand)
	:	BinaryOpUGenInternal(leftOperand, rightOperand)
	{
	}
	
	UGenInternal* getChannel(const int channel) throw()
	{
		return new BinaryOpUGenInternalT<op>(inputs[LeftOperand].getChannel(channel),
											 inputs[RightOperand].getChannel(channel));
	}
	
	UGenInternal* getKr() throw();
	
	float getValue(const int channel) const throw()										
	{																													
		return op(inputs[LeftOperand].getValue(channel), inputs[RightOperand].getValue(channel));		
	}	
	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
	{ 
		int numSamplesToProcess = uGenOutput.getBlockSize(); 
		float* outputSamples = uGenOutput.getSampleData(); 
		const float* leftOperandSamples = inputs[LeftOperand].processBlock(shouldDelete, blockID, channel); 
		const float* rightOperandSamples = inputs[RightOperand].processBlock(shouldDelete, blockID, channel); 
		
		while(numSamplesToProcess--)
		{
			*outputSamples++ = op(*leftOperandSamples++,
								  *rightOperandSamples++);
		}
	}
};

template<BinaryOpFunction op>
class BinaryOpUGenInternalTK : public BinaryOpUGenInternalT<op>
{
private:
	float value;
	
public:
	BinaryOpUGenInternalTK(UGen const& leftOperand, UGen const& rightOperand) throw()
	:	BinaryOpUGenInternalT<op>(leftOperand, rightOperand), value(0.f)
	{
		this->rate = UGenInternal::ControlRate;
	}
	
	UGenInternal* getChannel(const int channel) throw()
	{
		return new BinaryOpUGenInternalTK<op>(this->inputs[BinaryOpUGenInternal::LeftOperand].getChannel(channel),
											  this->inputs[BinaryOpUGenInternal::RightOperand].getChannel(channel));
	}
	
	UGenInternal* getKr() throw() { this->incrementRefCount(); return this; }	
	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
	{
		const unsigned int krBlockSize = UGen::getControlRateBlockSize();														
		unsigned int blockPosition = blockID % krBlockSize;																
		int numSamplesToProcess = this->uGenOutput.getBlockSize();															
		float* outputSamples = this->uGenOutput.getSampleData();																
		float* leftOperandSamples = this->inputs[BinaryOpUGenInternal::LeftOperand].processBlock(shouldDelete, blockID, channel);					
		float* rightOperandSamples = this->inputs[BinaryOpUGenInternal::RightOperand].processBlock(shouldDelete, blockID, channel);				
		
		int numKrSamples = blockPosition & krBlockSize;																	
		
		while(numSamplesToProcess > 0) {																			
			float nextValue = this->value;																				
			if(numKrSamples == 0)																					
				nextValue = op(*leftOperandSamples, *rightOperandSamples);							
			
			numKrSamples = krBlockSize - numKrSamples;																
			
			blockPosition		+= numKrSamples;																	
			leftOperandSamples	+= numKrSamples;																	
			rightOperandSamples	+= numKrSamples;																	
			\
			if(nextValue == value) {																				
				while(numSamplesToProcess && numKrSamples) {														
					*outputSamples++ = nextValue;																	
					--numSamplesToProcess;																			
					--numKrSamples;																					
				}																									
			} else {																								
				float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();					
				while(numSamplesToProcess && numKrSamples) {														
					*outputSamples++ = value;																		
					value += valueSlope;																			
					--numSamplesToProcess;																			
					--numKrSamples;																					
				}																									
				value = nextValue;																					
			}																										
		}																													
	}
};

template<BinaryOpFunction op>
UGenInternal* BinaryOpUGenInternalT<op>::getKr() throw()
{
	return new BinaryOpUGenInternalTK<op>(inputs[LeftOperand].kr(),
										  inputs[RightOperand].kr());
}


template<BinaryOpFunction op>
class BinaryOpUGenT : public UGen
{
public:
	BinaryOpUGenT(UGen const& leftOperand, UGen const& rightOperand) throw()
	{
		const int numRightChannels = rightOperand.getNumChannels();
		const int numLeftChannels = leftOperand.getNumChannels();
		
		if(numLeftChannels > numRightChannels) 
			initInternal(numLeftChannels); 
		else 
			initInternal(numRightChannels); 
		
		for(int i = 0; i < numInternalUGens; i++) 
		{ 
			internalUGens[i] = new BinaryOpUGenInternalT<op>(leftOperand,rightOperand);
			internalUGens[i]->initValue(op(leftOperand.getValue(i),
										   rightOperand.getValue(i)));
		}
		
	}
	
	static UGen AR(UGen const& leftOperand, UGen const& rightOperand) { return BinaryOpUGenT<op>(leftOperand, rightOperand); }
	static UGen KR(UGen const& leftOperand, UGen const& rightOperand) { return UGen(BinaryOpUGenT<op>(leftOperand, rightOperand)).kr(); }
};

template<BinaryOpFunction op>
UGen UGen::binary(UGen const& rightOperand) throw()
{
	return BinaryOpUGenT<op>::AR(rightOperand);
}

#endif // BINARYOPUGENS_H