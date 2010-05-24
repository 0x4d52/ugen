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

#ifndef _UGEN_ugen_MulAdd_H_
#define _UGEN_ugen_MulAdd_H_

#include "../core/ugen_UGen.h"
#include "../basics/ugen_InlineBinaryOps.h"

#define Mul_Arg mul
#define Add_Arg add
#define MulAdd_ArgsDeclare UGen const& Mul_Arg = 1.f, UGen const& Add_Arg = 0.f
#define MulAdd_ArgsCall Mul_Arg, Add_Arg


/** Declare a UGen with multiply and add inputs.
 This uses the "expanding" version of MulAdd which is generally used for random-based UGen classes (e.g., WhiteNoise).
 This is "expanding" in the sense that multiple channels of the Base UGen are generated depending on the number
 of channels in the "mul" and/or "add" inputs.
 
 @param UGEN_NAME				Name of the UGen to create, this wiil derive from UGen
 @param ARGS_CALL				A comma separated list of the arguments the constructor takes when called. NB this
								excludes the mul and add arguments.
								This must be surrounded by parentheses. E.g., @code (freq, phase) @endcode
 @param ARGS_CALL_MULADD		A comma separated list of the arguments the ar/kr/AR/KR methods 
								take when called. This must be surrounded by parentheses. Commonly this should
								be the same as ARGS_CALL with the mul and add args added. You can use MulAdd_ArgsCall for this
								e.g., @code (freq, phase, MulAdd_ArgsCall) @endcode
 @param ARGS_DECLARE			A comma separated list of the arguments the constructor takes when declared. 
								NB this excludes the mul and add arguments.
								Each of these parameters must have a default. This must be surrounded by parentheses.
								E.g., @code (UGen const& freq = 440.f, UGen const& phase = 0.f) @endcode
 @param ARGS_DECLARE_MULADD		A comma separated list of the arguments the ar/kr/AR/KR methods take when declared. 
								Each of these parameters must have a default. This must be surrounded by parentheses.
								Commonly this should be the same as ARGS_CALL with the mul and add args added. You can 
								use MulAdd_ArgsDeclare for this 
								e.g., @code (UGen const& freq = 440.f, UGen const& phase = 0.f, MulAdd_ArgsDeclare) @endcode
 @param DOCS					Doxygen "param" documentation usually from a define. You can simply use
								DEFAULT_UGEN_DOCS or COMMON_UGEN_DOCS if you wish. Most UGen classes use COMMON_UGEN_DOCS 
								followed by their own param documentation followed by the MulAdd param documentation.	\n
								E.g., @code COMMON_UGEN_DOCS MyUGen_Docs MulAddArgs_Docs @endcode						\n
								- note these are separated by spaces so they form a single item. */
#define ExpandingMulAddUGenDeclaration(UGEN_NAME, ARGS_CALL, ARGS_CALL_MULADD, ARGS_DECLARE, ARGS_DECLARE_MULADD, DOCS)					\
	class UGEN_NAME : public UGen																										\
	{																																	\
	public:																																\
		/** Base MulAdd UGEN_NAME Constuctor (without "mul" or "add"), use the AR() and KR() versions in user code. 
			This uses the "expanding" version of MulAdd which is generally used for random-based UGen classes (e.g., WhiteNoise).
			This is "expanding" in the sense that multiple channels of the Base UGen are generated depending on the number
			of channels in the "mul" and/or "add" inputs. */																			\
		UGEN_NAME ARGS_DECLARE throw();																									\
																																		\
		/** Construct and return an audio rate UGEN_NAME adding "mul" and/or "add" operations as necessary. 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS	
			@return The audio rate UGEN_NAME. */																						\
		static inline UGen AR ARGS_DECLARE_MULADD throw()																				\
		{																																\
			const int mulNumChannels = Mul_Arg.getNumChannels();																		\
			const int addNumChannels = Add_Arg.getNumChannels();																		\
			const bool mulIsOne = (mulNumChannels == 1 && Mul_Arg.isConst(0) && Mul_Arg.getValue(0) == 1.f);							\
			const bool addIsZero = (addNumChannels == 1 && Add_Arg.isConst(0) && Add_Arg.getValue(0) == 0.f);							\
			UGen source = UGEN_NAME ARGS_CALL;																							\
			if(mulIsOne && addIsZero)																									\
				return source;																											\
			else if(addIsZero) {																										\
				while(source.getNumChannels() < mulNumChannels) source <<= UGEN_NAME ARGS_CALL;											\
				const int numChannels = ugen::max(mulNumChannels, source.getNumChannels());												\
				return source.withNumChannels(numChannels) * Mul_Arg;																	\
			} else if(mulIsOne)	{																										\
				while(source.getNumChannels() < addNumChannels) source <<= UGEN_NAME ARGS_CALL;											\
				const int numChannels = ugen::max(addNumChannels, source.getNumChannels());												\
				return source.withNumChannels(numChannels) + Add_Arg;																	\
			} else {																													\
				const int numMulAddChannels = ugen::max(mulNumChannels, addNumChannels);												\
				while(source.getNumChannels() < numMulAddChannels) source <<= UGEN_NAME ARGS_CALL;										\
				const int numChannels = ugen::max(numMulAddChannels, source.getNumChannels());											\
				return MulAdd (source.withNumChannels(numChannels), MulAdd_ArgsCall);													\
			}																															\
			return source;																												\
		}																																\
																																		\
		/** Construct and return a control rate UGEN_NAME (if available, otherise returns an audio version by default). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The control rate UGEN_NAME.	 */																						\
		static inline UGen KR ARGS_DECLARE_MULADD throw()																				\
		{																																\
			const int mulNumChannels = Mul_Arg.getNumChannels();																		\
			const int addNumChannels = Add_Arg.getNumChannels();																		\
			const bool mulIsOne = (mulNumChannels == 1 && Mul_Arg.isConst(0) && Mul_Arg.getValue(0) == 1.f);							\
			const bool addIsZero = (addNumChannels == 1 && Add_Arg.isConst(0) && Add_Arg.getValue(0) == 0.f);							\
			UGen source = UGEN_NAME ARGS_CALL;																							\
			if(mulIsOne && addIsZero)																									\
				return UGen(source).kr();																								\
			else if(addIsZero) {																										\
				while(source.getNumChannels() < mulNumChannels) source <<= UGEN_NAME ARGS_CALL;											\
				const int numChannels = ugen::max(mulNumChannels, source.getNumChannels());												\
				return UGen(source.withNumChannels(numChannels) * Mul_Arg).kr();														\
			} else if(mulIsOne)	{																										\
				while(source.getNumChannels() < addNumChannels) source <<= UGEN_NAME ARGS_CALL;											\
				const int numChannels = ugen::max(addNumChannels, source.getNumChannels());												\
				return UGen(source.withNumChannels(numChannels) + Add_Arg).kr();														\
			} else {																													\
				const int numMulAddChannels = ugen::max(mulNumChannels, addNumChannels);												\
				while(source.getNumChannels() < numMulAddChannels) source <<= UGEN_NAME ARGS_CALL;										\
				const int numChannels = ugen::max(numMulAddChannels, source.getNumChannels());											\
				return UGen(MulAdd (source.withNumChannels(numChannels), MulAdd_ArgsCall)).kr();										\
			}																															\
			return UGen(source).kr();																									\
		}																																\
																																		\
		/** Construct and return an audio rate UGEN_NAME (SC compatability). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The audio rate UGEN_NAME.	 	*/																					\
		inline UGen ar ARGS_DECLARE_MULADD throw() { return UGEN_NAME::AR ARGS_CALL_MULADD; }											\
																																		\
		/** Construct and return a control rate UGEN_NAME (SC compatability). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The control rate UGEN_NAME.	 	*/																					\
		inline UGen kr ARGS_DECLARE_MULADD throw() { return UGEN_NAME::KR ARGS_CALL_MULADD; }											\
																																		\
		/** @internal SC compatability */																								\
		inline UGen operator() ARGS_DECLARE_MULADD throw() { return UGEN_NAME::AR ARGS_CALL_MULADD; }									\
	}

/** Declare a UGen with multiply and add inputs.
 This uses the "direct" version of MulAdd which is generally used for non randomly-based UGen classes. It assumes that multiple
 channels generated by multiple channels of "mul" and/or "add" would differ only by these scale and offset values (i.e.,
 the channels are otherwise the same "shape" which is unintuitive in the case of noise since one would expect each
 channel of noise to be fundamentally different).
 
 @param UGEN_NAME				Name of the UGen to create, this wiil derive from UGen
 @param ARGS_CALL				A comma separated list of the arguments the constructor takes when called. NB this
								excludes the mul and add arguments.
								This must be surrounded by parentheses. E.g., @code (freq, phase) @endcode
 @param ARGS_CALL_MULADD		A comma separated list of the arguments the ar/kr/AR/KR methods 
								take when called. This must be surrounded by parentheses. Commonly this should
								be the same as ARGS_CALL with the mul and add args added. You can use MulAdd_ArgsCall for this
								e.g., @code (freq, phase, MulAdd_ArgsCall)  @endcode
 @param ARGS_DECLARE			A comma separated list of the arguments the constructor takes when declared. 
								NB this excludes the mul and add arguments.
								Each of these parameters must have a default. This must be surrounded by parentheses.
								E.g., @code (UGen const& freq = 440.f, UGen const& phase = 0.f) @endcode
 @param ARGS_DECLARE_MULADD		A comma separated list of the arguments the ar/kr/AR/KR methods take when declared. 
								Each of these parameters must have a default. This must be surrounded by parentheses.
								Commonly this should be the same as ARGS_CALL with the mul and add args added. You can 
								use MulAdd_ArgsDeclare for this 
								e.g., @code (UGen const& freq = 440.f, UGen const& phase = 0.f, MulAdd_ArgsDeclare) @endcode
 @param DOCS					Doxygen "param" documentation usually from a define. You can simply use
								DEFAULT_UGEN_DOCS or COMMON_UGEN_DOCS if you wish. Most UGen classes use COMMON_UGEN_DOCS 
								followed by their own param documentation followed by the MulAdd param documentation.	\n
								E.g., @code COMMON_UGEN_DOCS MyUGen_Docs MulAddArgs_Docs @endcode						\n
								- note these are separated by spaces so they form a single item. */
#define DirectMulAddUGenDeclaration(UGEN_NAME, ARGS_CALL, ARGS_CALL_MULADD, ARGS_DECLARE, ARGS_DECLARE_MULADD, DOCS)					\
	class UGEN_NAME : public UGen																										\
	{																																	\
	public:																																\
		/** Base MulAdd UGEN_NAME Constuctor (without "mul" or "add"), use the AR() and KR() versions in user code. 
			This uses the "direct" version of MulAdd which is generally used for non randomly-based UGen classes. It assumes that multiple
			channels generated by multiple channels of "mul" and/or "add" would differ only by these scale and offset values (i.e.,
			the channels are otherwise the same "shape" which is unintuitive in the case of noise since one would expect each
			channel of noise to be fundamentally different). */																			\
		UGEN_NAME ARGS_DECLARE throw();																									\
																																		\
		/** Construct and return an audio rate UGEN_NAME adding "mul" and/or "add" operations as necessary. 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS	
			@return The audio rate UGEN_NAME. */																						\
		static inline UGen AR ARGS_DECLARE_MULADD throw()																				\
		{																																\
			const bool mulIsOne = (Mul_Arg.getNumChannels() == 1 && Mul_Arg.isConst(0) && Mul_Arg.getValue(0) == 1.f);					\
			const bool addIsZero = (Add_Arg.getNumChannels() == 1 && Add_Arg.isConst(0) && Add_Arg.getValue(0) == 0.f);					\
			UGen source = UGEN_NAME ARGS_CALL;																							\
			if(mulIsOne && addIsZero)	return source;																					\
			else if(addIsZero) 			return source * Mul_Arg;																		\
			else if(mulIsOne)			return source + Add_Arg;										\
			else						return MulAdd (source, MulAdd_ArgsCall);														\
			return source;																												\
		}																																\
																																		\
		/** Construct and return a control rate UGEN_NAME (if available, otherise returns an audio version by default). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The control rate UGEN_NAME.	 */																						\
		static inline UGen KR ARGS_DECLARE_MULADD throw()																				\
		{																																\
			const bool mulIsOne = (Mul_Arg.getNumChannels() == 1 && Mul_Arg.isConst(0) && Mul_Arg.getValue(0) == 1.f);					\
			const bool addIsZero = (Add_Arg.getNumChannels() == 1 && Add_Arg.isConst(0) && Add_Arg.getValue(0) == 0.f);					\
			UGen source = UGEN_NAME ARGS_CALL;																							\
			if(mulIsOne && addIsZero)	return UGen(source).kr();																		\
			else if(addIsZero)			return UGen(source * Mul_Arg).kr();																\
			else if(mulIsOne)			return UGen(source + Add_Arg).kr();																\
			else 						return UGen(MulAdd (source, MulAdd_ArgsCall)).kr();												\
			return UGen(source).kr();																									\
		}																																\
																																		\
		/** Construct and return an audio rate UGEN_NAME (SC compatability). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The audio rate UGEN_NAME.	 	*/																					\
		inline UGen ar ARGS_DECLARE_MULADD throw() { return UGEN_NAME::AR ARGS_CALL_MULADD; }											\
																																		\
		/** Construct and return a control rate UGEN_NAME (SC compatability). 
			A "mul" operation will not be added if the multiplier is 1.0, an "add" operation will not be
			added if the "add" value is 0.0. 
			\n\n
			DOCS 
			@return The control rate UGEN_NAME.	 	*/																					\
		inline UGen kr ARGS_DECLARE_MULADD throw() { return UGEN_NAME::KR ARGS_CALL_MULADD; }											\
																																		\
		/** @internal SC compatability */																								\
		inline UGen operator() ARGS_DECLARE_MULADD throw() { return UGEN_NAME::AR ARGS_CALL_MULADD; }									\
	}

/** @ingroup UGenInternals */
class MulAddUGenInternal : public UGenInternal
{
public:
	MulAddUGenInternal(UGen const& input, UGen const& mul, UGen const& add) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, Mul, Add, NumInputs };
	
protected:	
};

#define MulAddArgs_Docs		@param mul	This is multiplied by the output.							\
							@param add	This is added to the output (after mul has been applied).

#define MulAdd_Docs			@param input  The input source to modify.								\
							MulAddArgs_Docs

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(MulAddUGenInternal, (input, mul, add),
								   (UGen const& input, UGen const& mul, UGen const& add));

/** Wrap a UGen with multiply and add inputs. 
 You may use this directly but it is more common to declare a new UGen by
 using the macros DirectMulAddUGenDeclaration() or ExpandingMulAddUGenDeclaration() which
 tests the mul and add inputs to see if the mul and add inputs really need adding. 
 @ingroup AllUGens MathsUGens 
 @see BinaryOpMulUGen, BinaryOpAddUGen, UGen::operator*(), UGen::operator+() */
UGenSublcassDeclarationNoDefault(MulAdd, (input, mul, add),
								 (UGen const& input, UGen const& mul = 1.f, UGen const& add = 0.f), 
								 COMMON_UGEN_DOCS MulAdd_Docs);


#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define MulAdd MulAdd()
#endif


#endif // _UGEN_ugen_MulAdd_H_
