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

 midiratio .. convert an interval in MIDI notes into a frequency ratio
 ratiomidi .. convert a frequency ratio to an interval in MIDI notes
 dbamp .. decibels to linear amplitude
 ampdb .. linear amplitude to decibels
 octcps .. decimal octaves to cycles per second
 cpsoct .. cycles per second to decimal octaves

 softclip .. distortion
 isPositive .. 1 when a >= 0, else 0
 isNegative .. 1 when a < 0, else 0
 isStrictlyPositive .. 1 when a > 0, else 0
 */

#ifndef UGEN_UNARYOPUGENS_H
#define UGEN_UNARYOPUGENS_H


#include "../core/ugen_UGen.h"
#include "../core/ugen_Value.h"
#include "ugen_InlineUnaryOps.h"



#define UnaryOpUGenConstructor(INTERNALUGENCLASSNAME, operand_)																	\
		initInternal(operand_.getNumChannels());																				\
		for(int i = 0; i < numInternalUGens; i++)																				\
		{																														\
			internalUGens[i] = new INTERNALUGENCLASSNAME(operand_);																\
		}

#define UnaryOpUGenProcessBlock(shouldDelete_, blockID_, channel_, OPFUNCTION_INTERNAL)											\
		int numSamplesToProcess = uGenOutput.getBlockSize();																	\
		float* outputSamples = uGenOutput.getSampleData();																		\
		float* inputSamples = inputs[Operand].processBlock(shouldDelete_, blockID_, channel_);									\
																																\
			while(numSamplesToProcess--)																						\
			{																													\
				*outputSamples++ = OPFUNCTION_INTERNAL(*inputSamples++);														\
			}																													\

#define UnaryOpUGenProcessBlock_K(shouldDelete_, blockID_, channel_, OPFUNCTION_INTERNAL)										\
		const int krBlockSize = UGen::getControlRateBlockSize();																\
		unsigned int blockPosition = blockID % krBlockSize;																		\
		int numSamplesToProcess = uGenOutput.getBlockSize();																	\
		float* outputSamples = uGenOutput.getSampleData();																		\
		float* inputSamples = inputs[Operand].processBlock(shouldDelete_, blockID_, channel_);									\
																																\
		int numKrSamples = blockPosition % krBlockSize;																			\
																																\
			while(numSamplesToProcess > 0) {																					\
				float nextValue = value;																						\
				if(numKrSamples == 0) nextValue = OPFUNCTION_INTERNAL(*inputSamples);											\
																																\
				numKrSamples = krBlockSize - numKrSamples;																		\
																																\
				blockPosition		+= numKrSamples;																			\
				inputSamples		+= numKrSamples;																			\
																																\
				if(nextValue == value) {																						\
					while(numSamplesToProcess && numKrSamples) {																\
						*outputSamples++ = nextValue;																			\
						--numSamplesToProcess;																					\
						--numKrSamples;																							\
					}																											\
				} else {																										\
					float valueSlope = (nextValue - value) / (float)UGen::getControlRateBlockSize();							\
					while(numSamplesToProcess && numKrSamples) {																\
						*outputSamples++ = value;																				\
						value += valueSlope;																					\
						--numSamplesToProcess;																					\
						--numKrSamples;																							\
					}																											\
					value = nextValue;																							\
				}																												\
			}																													\

#define UnaryOpUGenDeclaration(OPNAME, OPFUNCTION)																				\
		/** Internal for Unary##OPNAME##UGen. @ingroup UGenInternals */															\
		class Unary##OPNAME##UGenInternal : public UnaryOpUGenInternal															\
		{																														\
		public:																													\
			Unary##OPNAME##UGenInternal(UGen const& operand) throw();															\
			UGenInternal* getChannel(const int channel) throw();																\
			UGenInternal* getKr() throw();																						\
			void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();						\
			float getValue(const int channel) const throw();																	\
		};																														\
		/** Control rate internal for Unary##OPNAME##UGen. @ingroup UGenInternals */											\
		class Unary##OPNAME##UGenInternalK : public Unary##OPNAME##UGenInternal													\
		{																														\
		public:																													\
			Unary##OPNAME##UGenInternalK(UGen const& operand) throw();															\
			UGenInternal* getKr() throw() {  incrementRefCount(); return this; }												\
			void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();						\
		private:																												\
			float value;																										\
		};																														\
																																\
		/** A UnaryOpUGen usually created using the function OPFUNCTION##() when applied to other UGen instances. 
			It is not normally required to use this UGen explicitly. 
			@code
				UGen result##OPNAME = SinOsc::AR(100, 0, 0.4).##OPFUNCTION(); // creates a Unary##OPNAME##UGen
			@endcode 
			You can also use a global version of this function, the above code would be equivalent as:
			@code
				UGen result##OPNAME = OPFUNCTION##(SinOsc::AR(100, 0, 0.4)); // creates a Unary##OPNAME##UGen
			@endcode 
			@see OPFUNCTION##(UGen const& operand)
			@see UGen::##OPFUNCTION##()
			@ingroup AllUGens MathsUGens */																						\
		class Unary##OPNAME##UGen : public UGen																					\
		{																														\
		public:																													\
			Unary##OPNAME##UGen(UGen const& operand) throw();																	\
		};																														\
		/** Global function for OPFUNCTION##(UGen const& operand). @see UGen::##OPFUNCTION##() */								\
		UGen OPFUNCTION(UGen const& operand) throw();																			\
		/** Internal for Unary##OPNAME##Value. */																				\
		class Unary##OPNAME##ValueInternal : public UnaryOpValueInternal														\
		{																														\
		public:																													\
			Unary##OPNAME##ValueInternal(Value const& operand) throw();															\
			double getValue() throw();																							\
		};																														\
		/** Applies the OPFUNCTION##() operator to the input Value. */															\
		class Unary##OPNAME##Value : public Value																				\
		{																														\
		public:																													\
			Unary##OPNAME##Value(Value const& operand) throw();																	\
		};																														\
		/** Global function for OPFUNCTION##(Value const& operand). @see Value::##OPFUNCTION##() */								\
		Value OPFUNCTION(Value const& operand) throw();																			\
		/** Global function for OPFUNCTION##(ValueArray const& operand). @see ValueArray::##OPFUNCTION##() */					\
		ValueArray OPFUNCTION(ValueArray const& operand) throw();																

#define UnaryOpUGenDefinition(OPNAME, OPFUNCTION, OPFUNCTION_INTERNAL)															\
		UnaryOpUGenDefinitionNoProcessBlock(OPNAME, OPFUNCTION, OPFUNCTION_INTERNAL)											\
																																\
		void Unary##OPNAME##UGenInternal::processBlock(bool& shouldDelete,														\
													   const unsigned int blockID,												\
													   const int channel) throw()												\
		{																														\
			UnaryOpUGenProcessBlock(shouldDelete, blockID, channel, OPFUNCTION_INTERNAL);										\
		}

#define UnaryOpUGenDefinitionNoProcessBlock(OPNAME, OPFUNCTION, OPFUNCTION_INTERNAL)											\
		Unary##OPNAME##UGenInternal::Unary##OPNAME##UGenInternal(UGen const& operand) throw()									\
		:	UnaryOpUGenInternal(operand)																						\
		{  }																													\
																																\
		UGenInternal* Unary##OPNAME##UGenInternal::getChannel(const int channel) throw()										\
		{																														\
			return new Unary##OPNAME##UGenInternal(inputs[Operand].getChannel(channel));										\
		}																														\
																																\
		UGenInternal* Unary##OPNAME##UGenInternal::getKr() throw()																\
		{																														\
			return new Unary##OPNAME##UGenInternalK(inputs[Operand].kr());														\
		}																														\
																																\
		float Unary##OPNAME##UGenInternal::getValue(const int channel) const throw()											\
		{																														\
			return OPFUNCTION_INTERNAL(inputs[Operand].getValue(channel));														\
		}																														\
																																\
		Unary##OPNAME##UGenInternalK::Unary##OPNAME##UGenInternalK(UGen const& operand) throw()									\
		:	Unary##OPNAME##UGenInternal(operand), value(0.f)																	\
		{																														\
			rate = ControlRate;																									\
		}																														\
																																\
		void Unary##OPNAME##UGenInternalK::processBlock(bool& shouldDelete,														\
														const unsigned int blockID,												\
														const int channel) throw()												\
		{																														\
			UnaryOpUGenProcessBlock_K(shouldDelete, blockID, channel, OPFUNCTION_INTERNAL);										\
		}																														\
																																\
		Unary##OPNAME##UGen::Unary##OPNAME##UGen(UGen const& operand) throw()													\
		{																														\
			UnaryOpUGenConstructor(Unary##OPNAME##UGenInternal, operand);														\
		}																														\
																																\
		UGen OPFUNCTION(UGen const& operand) throw()																			\
		{																														\
			return operand.OPFUNCTION();																						\
		}																														\
																																\
		UGen UGen::OPFUNCTION() const throw()																					\
		{																														\
			return Unary##OPNAME##UGen(*this);																					\
		}																														\
																																\
		UGenArray UGenArray::OPFUNCTION() const throw()																			\
		{																														\
			if(internal == 0) return UGenArray();																				\
																																\
			UGenArray newArray(internal->size());																				\
																																\
			for(int i = 0; i < internal->size(); i++)																			\
			{																													\
				newArray.internal->getArray()[i] = internal->getArray()[i].OPFUNCTION();										\
			}																													\
																																\
			return newArray;																									\
		}																														\
																																\
		Buffer Buffer::OPFUNCTION() const throw()																				\
		{																														\
			Buffer newBuffer(BufferSpec(size_, numChannels_, false));															\
																																\
			for(int channelIndex = 0; channelIndex < numChannels_; channelIndex++)												\
			{																													\
				int numSamples = size_;																							\
				float* inputSamples = channels[channelIndex]->data;																\
				float* outputSamples = newBuffer.channels[channelIndex]->data;													\
																																\
				while(--numSamples >= 0) {																						\
					*outputSamples++ = ugen::OPFUNCTION_INTERNAL(*inputSamples++);												\
				}																												\
			}																													\
																																\
			return newBuffer;																									\
		}																														\
																																\
		Unary##OPNAME##ValueInternal::Unary##OPNAME##ValueInternal(Value const& operand) throw()								\
		:	UnaryOpValueInternal(operand) { }																					\
																																\
		double Unary##OPNAME##ValueInternal::getValue() throw()																	\
		{																														\
			return ugen::OPFUNCTION_INTERNAL(operand_.getValue());																\
		}																														\
																																\
		Unary##OPNAME##Value::Unary##OPNAME##Value(Value const& operand) throw()												\
		:	Value(new Unary##OPNAME##ValueInternal(operand)) { }																\
																																\
		Value Value::OPFUNCTION() const throw()																					\
		{																														\
			return Unary##OPNAME##Value(*this);																					\
		}																														\
																																\
		Value OPFUNCTION(Value const& operand) throw()																			\
		{																														\
			return operand.OPFUNCTION();																						\
		}																														\
																																\
		ValueArray ValueArray::OPFUNCTION() const throw()																		\
		{																														\
			if(size() == 0)																										\
				return *this;																									\
			else {																												\
				ValueArray newArray(internal->size_);																			\
				for(int i = 0; i < internal->size_; i++)																		\
					newArray.put(i, internal->array[i].OPFUNCTION());															\
				return newArray;																								\
			}																													\
		}																														\
																																\
		ValueArray OPFUNCTION(ValueArray const& operand) throw()																\
		{																														\
			return operand.OPFUNCTION();																						\
		}																														\
																						



class UnaryOpUGenInternal : public UGenInternal
{
public:
	UnaryOpUGenInternal(UGen const& operand, const int channel = -1) throw();
	
	enum Inputs { Operand, NumInputs };
	
protected:
};

class UnaryOpValueInternal : public ValueInternal
{
public:
	UnaryOpValueInternal(Value const& operand) throw();
	
	Value getOperand() throw() { return operand_; }
	
protected:
	Value operand_;
};



UnaryOpUGenDeclaration(Neg,			neg);
UnaryOpUGenDeclaration(Abs,			abs);
UnaryOpUGenDeclaration(Reciprocal,	reciprocal);
UnaryOpUGenDeclaration(Sin,			sin);
UnaryOpUGenDeclaration(Cos,			cos);
UnaryOpUGenDeclaration(Tan,			tan);
UnaryOpUGenDeclaration(SinH,		sinh);
UnaryOpUGenDeclaration(CosH,		cosh);
UnaryOpUGenDeclaration(TanH,		tanh);
UnaryOpUGenDeclaration(ASin,		asin);
UnaryOpUGenDeclaration(ACos,		acos);
UnaryOpUGenDeclaration(ATan,		atan);
UnaryOpUGenDeclaration(Sqrt,		sqrt);
UnaryOpUGenDeclaration(Log,			log);
UnaryOpUGenDeclaration(Log2,		log2);
UnaryOpUGenDeclaration(Log10,		log10);
UnaryOpUGenDeclaration(Exp,			exp);
UnaryOpUGenDeclaration(MidiCPS,		midicps);
UnaryOpUGenDeclaration(CPSMidi,		cpsmidi);
UnaryOpUGenDeclaration(AmpdB,		ampdb);
UnaryOpUGenDeclaration(dBAmp,		dbamp);
UnaryOpUGenDeclaration(Squared,		squared);
UnaryOpUGenDeclaration(Cubed,		cubed);
UnaryOpUGenDeclaration(Distort,		distort);
UnaryOpUGenDeclaration(BesselI0,	besselI0);
UnaryOpUGenDeclaration(Ceil,		ceil);
UnaryOpUGenDeclaration(Floor,		floor);
UnaryOpUGenDeclaration(Frac,		frac);
UnaryOpUGenDeclaration(Sign,		sign);
UnaryOpUGenDeclaration(Zap,			zap);



// the start of templatising the macros...



template<UnaryOpFunction op>
class UnaryOpUGenInternalT : public UnaryOpUGenInternal
{
public:
	UnaryOpUGenInternalT(UGen const& operand)
	:	UnaryOpUGenInternal(operand)
	{
	}
	
	UGenInternal* getChannel(const int channel) throw()
	{
		return new UnaryOpUGenInternalT<op>(inputs[Operand].getChannel(channel));
	}
	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw() 
	{ 
		int numSamplesToProcess = uGenOutput.getBlockSize(); 
		float* outputSamples = uGenOutput.getSampleData(); 
		const float* operandSamples = inputs[Operand].processBlock(shouldDelete, blockID, channel); 
		
		while(numSamplesToProcess--)
		{
			*outputSamples++ = (float)op((float)*operandSamples);
		}
	}
};

template<UnaryOpFunction op>
class UnaryOpUGenT : public UGen
{
public:
	UnaryOpUGenT(UGen const& operand, UGen const& rightOperand) throw()
	{
		initInternal(operand.getNumChannels()); 
		
		for(int i = 0; i < numInternalUGens; i++) 
		{ 
			internalUGens[i] = new UnaryOpUGenInternalT<op>(operand);
		}
		
	}
	
	static UGen AR(UGen const& operand) { return UnaryOpUGenT<op>(operand); }
};

template<UnaryOpFunction op>
UGen UGen::unary() throw()
{
	return UnaryOpUGenT<op>::AR(*this);
}


#endif // UNARYOPUGENS_H