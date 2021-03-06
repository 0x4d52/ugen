// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
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

#include "ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_UGen.h"
#include "ugen_UGenArray.h"
#include "ugen_ExternalControlSource.h"
#include "../basics/ugen_ScalarUGens.h"
#include "../basics/ugen_UnaryOpUGens.h"
#include "../basics/ugen_BinaryOpUGens.h"
#include "../basics/ugen_MixUGen.h"
#include "../basics/ugen_Plug.h"
#include "../basics/ugen_RawInputUGens.h"
#include "../basics/ugen_WrapFold.h"
#include "../basics/ugen_MappingUGens.h"
#include "../filters/control/ugen_Lag.h"
#include "../envelopes/ugen_EnvGen.h"
#include "../buffers/ugen_PlayBuf.h"

#ifndef UGEN_ANDROID
	#include "../spawn/ugen_VoicerBase.h"
	#include "../spawn/ugen_TSpawn.h"
#endif


//UGen::UGen() throw()
//:	userData(UGen::defaultUserData),
//	numInternalUGens(0),
//	internalUGens(0)
//{
//	static bool init = false;
//	
//	initInternal(1);
//	
//	if(init)
//		internalUGens[0] = getNullInternal();
//	else
//	{
//		internalUGens[0] = new NullUGenInternal();
//		init = true;
//	}
//}

UGen::UGen() throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{	
	initInternal(1);
	internalUGens[0] = NullUGenInternal::getInstance();
}


UGen::UGen(UGenInternal* internalUGenToUse, const int channel) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)	
{
	ugen_assert(internalUGenToUse != 0);
	
	initInternal(1);
		
	if(channel >= 0)
	{
		UGenInternal* internaUGenChannel = internalUGenToUse->getChannelInternal(channel);
		internalUGenToUse->decrementRefCount();
		internalUGenToUse = internaUGenChannel;
	}
	
	internalUGens[0] = internalUGenToUse;
}

UGen::UGen(const int numInternalUGensToUse, UGenInternal** internalUGensToUse) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	if(internalUGensToUse[0] == 0)
	{
		constructMultichannel(numInternalUGensToUse, 0);
	}
	else
	{
		initInternal(numInternalUGensToUse);
		
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i] = internalUGensToUse[i];
		}
	}
}

UGen::UGen(UGen const& copy) throw()
:	userData(copy.userData),
	numInternalUGens(0),
	internalUGens(0)
{
	initInternal(copy.numInternalUGens);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		internalUGens[i] = copy.getInternalUGen(i);
	}
}

UGen& UGen::operator= (UGen const& other) throw()
{
	if (this != &other)
    {				
		userData = other.userData;
		other.incrementInternals();
		initInternal(other.numInternalUGens);
		
		for(unsigned int i = 0; i < numInternalUGens; i++) 
		{
			internalUGens[i] = other.internalUGens[i]; //other.getInternalUGen(i);// add the increment above instead
		}
    }
	
    return *this;
}

UGen::UGen(const float value) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)			
{ 	
	initInternal(1);
	internalUGens[0] = new ScalarUGenInternal(value);	
}

UGen::UGen(const double value) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	initInternal(1);
	internalUGens[0] = new ScalarUGenInternal((float)value);
}

UGen::UGen(const int value) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	initInternal(1);
	internalUGens[0] = new ScalarUGenInternal((float)value);	
}

UGen::UGen(float const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)				
{ 	
	initInternal(1);
	internalUGens[0] = new FloatPtrUGenInternal(valuePtr);	
}

UGen::UGen(double const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)					
{ 	
	initInternal(1);
	internalUGens[0] = new DoublePtrUGenInternal(valuePtr);	
}

UGen::UGen(int const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	initInternal(1);
	internalUGens[0] = new IntPtrUGenInternal(valuePtr);
}

UGen::UGen(bool const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	initInternal(1);
	internalUGens[0] = new BoolPtrUGenInternal(valuePtr);
}

UGen::UGen(char const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{
	initInternal(1);
	internalUGens[0] = new CharPtrUGenInternal(valuePtr);
}

UGen::UGen(unsigned char const *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{
	initInternal(1);
	internalUGens[0] = new UnsignedCharPtrUGenInternal(valuePtr);
}

UGen::UGen(Buffer const& buffer) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	ugen_assert(buffer.size() != 0);
	
	initInternal(buffer.size());
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new ScalarUGenInternal(buffer.getSampleUnchecked(0, (int)i));	
	}
}

UGen::UGen(ValueArray const& array) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	ugen_assert(array.size() != 0);
	
	initInternal(array.size());
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new ValueUGenInternal(array[i]);	
	}
}

UGen::UGen(Value const& value) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	initInternal(1);
	internalUGens[0] = new ValueUGenInternal(value);	
}

UGenInternal* UGen::newScalarInternal(const float value) throw()
{
	return new ScalarUGenInternal(value);
}

UGen::UGen(UGenArray const& array) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	constructMultichannel(array.size(), array.getArray());
}

#ifndef UGEN_NOEXTGPL
UGen::UGen(ExternalControlSource const& externalControlSource) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	initInternal(1);
	internalUGens[0] = new ExternalControlSourceUGenInternal(externalControlSource);
}
#endif

#if defined(JUCE_VERSION)
#include "../juce/ugen_JuceSlider.h"
#include "../juce/gui/ugen_JuceMultiSlider.h"
UGen::UGen(Slider* slider) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(slider != 0);
	initInternal(1);
	Value value = SliderValue(slider);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}

UGen::UGen(MultiSlider* sliders) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{	
	if(sliders)// && sliders->isValidComponent())
	{
		initInternal(sliders->getNumSliders());
		for(int i = 0; i < sliders->getNumSliders(); i++)
		{
			Value value = SliderValue(sliders->getSlider(i));
			ValueUGenInternal *internal = new ValueUGenInternal(value);
			internalUGens[i] = internal;
//			internal->initValue((float)value.getValue());
		}
	}
	else
	{
		ugen_assertfalse;
	}
}

UGen::UGen(Button* button) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(button != 0);
	initInternal(1);
	Value value = ButtonValue(button);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}

UGen::UGen(ToggleButton* button) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(button != 0);
	initInternal(1);
	Value value = ButtonValue(button);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}

UGen::UGen(TextButton* button) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(button != 0);
	initInternal(1);
	Value value = ButtonValue(button);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}


UGen::UGen(Label* label) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(label != 0);
	initInternal(1);
	Value value = LabelValue(label);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}
#endif

#if defined(UGEN_IPHONE) || defined(DOXYGEN)
#include "ugen_UISlider.h"
UGen::UGen(UISlider *slider) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{ 	
	ugen_assert(slider != 0);
	initInternal(1);
	Value value = UISliderValue(slider);
	ValueUGenInternal *internal = new ValueUGenInternal(value);
	internalUGens[0] = internal;
//	internal->initValue((float)value.getValue());
}

UGen::UGen(signed char *valuePtr) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{
	initInternal(1);
	internalUGens[0] = new BOOLPtrUGenInternal(valuePtr);
}
#endif

#ifndef UGEN_NOEXTGPL
UGen::UGen(Env const& env) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)						
{
	initInternal(1);
	internalUGens[0] = new EnvGenUGenInternal(env, UGen::DeleteWhenDone);
}
#endif // gpl

void UGen::constructMultichannel(const int numUGens, const UGen uGenArray[]) throw()
{
	if(uGenArray == 0)
	{
		initInternal(numUGens);
		for(int i = 0; i < numUGens; i++)
		{
			internalUGens[i] = getNullInternal();
		}
	}
	else
	{
		int totalChannels = 0;
		
		for(int uGenIndex = 0; uGenIndex < numUGens; uGenIndex++)
		{
			totalChannels += uGenArray[uGenIndex].numInternalUGens;
		}
		
		initInternal(totalChannels);
		
		int channel = 0;
		
		for(int uGenIndex = 0; uGenIndex < numUGens; uGenIndex++)
		{
			for(unsigned int internalIndex = 0; internalIndex < uGenArray[uGenIndex].numInternalUGens; internalIndex++)
			{			
				// was: internalUGens[channel++] = uGenArray[uGenIndex].getInternalUGen(internalIndex);
				internalUGens[channel++] = uGenArray[uGenIndex].getChannel(internalIndex).getInternalUGen(0);
			}
		}
	}
}


UGen::UGen(UGen const& arg1, UGen const& arg2) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2 };
	constructMultichannel(2, uGenArray);
}	

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3 };
	constructMultichannel(3, uGenArray);
}

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3, arg4 };
	constructMultichannel(4, uGenArray);
}

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3, arg4, arg5 };
	constructMultichannel(5, uGenArray);
}

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3, arg4, arg5, arg6 };
	constructMultichannel(6, uGenArray);
}

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3, arg4, arg5, arg6, arg7 };
	constructMultichannel(7, uGenArray);
}

UGen::UGen(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) throw()
:	userData(UGen::defaultUserData),
	numInternalUGens(0),
	internalUGens(0)
{
	UGen uGenArray[] = { arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 };
	constructMultichannel(8, uGenArray);
}

UGen::~UGen() throw()
{
	decrementInternals();
	delete [] internalUGens;
}

UGenArray UGen::operator<< (UGen const& rightOperand) throw()
{
	return UGenArray(*this, rightOperand, true);
}

UGen UGen::operator, (UGen const& rightOperand) throw()
{
	if (isNull())
        return rightOperand;	
	else
		return UGen(*this, rightOperand);
}

UGenArray operator<< (float leftOperand, UGen const& rightOperand) throw()
{
	return UGenArray(UGen(leftOperand), rightOperand, true);
}

UGen operator, (float leftOperand, UGen const& rightOperand) throw()
{
	return UGen(leftOperand, rightOperand);
}

UGen UGen::operator^ (UGen const& ignoreButEvalute) throw()
{
	return IngoreRightOperandUGen(*this, ignoreButEvalute);
}

UGen& UGen::operator+= (UGen const& other) throw()
{
	if (isNull())
        operator= (other);
    else
        operator= (*this + other);
	
    return *this;
}

UGen& UGen::operator-= (UGen const& other) throw()
{
	if (isNull())
        operator= (other.neg());
    else
        operator= (*this - other);
	
    return *this;
}

UGen& UGen::operator*= (UGen const& other) throw()
{
	if (isNull() || other.isNull())
        operator= (0.f);
    else
        operator= (*this * other);
	
    return *this;
}

UGen& UGen::operator/= (UGen const& other) throw()
{
	if (isNull())
        operator= (other);
    else
        operator= (*this / other);
	
    return *this;
}

UGen& UGen::operator<<= (UGen const& other) throw()
{
	if (isNull())
        operator= (other);
    else
        operator= (UGen(*this, other));
	
    return *this;
}

UGen UGen::expand() const throw()
{
	UGen newUGen;
	
	for(int i = 0; i < (int)numInternalUGens; i++)
	{
		newUGen <<= UGen(getInternalUGen(i), i);
	}
	
	newUGen.userData = userData;
	
	return newUGen;
}

UGen UGen::operator[] (const int index) const throw()
{	
	return wrapAt(index);
}

UGen UGen::operator[] (IntArray const& indices) const throw()
{	
	return wrapAt(indices);
}

UGen UGen::at(const int index) const throw()
{	
	if(index < 0 || (unsigned int)index >= numInternalUGens)
	{
		ugen_assertfalse;
		return getNull();
	}
	else
		return UGen(getInternalUGen(index), index);
}

UGen UGen::at(IntArray const& indices) const throw()
{	
	const int numChannels = indices.length();
	
	if(numChannels == 0)
	{
		ugen_assertfalse;
		return getNull();
	}
	else
	{
		UGenInternal** newInternals = new UGenInternal*[numChannels];
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			const int index = indices[channel];
			
			if(index < 0 || (unsigned int)index >= numInternalUGens) 
				newInternals[channel] = getNullInternal();
			else
				newInternals[channel] = getInternalUGen(index);
		}
		
		return UGen(numChannels, newInternals);
	}
}


UGen UGen::wrapAt(const int index) const throw()
{	
	int indexToUse = index;
	while(indexToUse < 0) indexToUse += numInternalUGens;
	
	return at(indexToUse % numInternalUGens);
}

UGen UGen::wrapAt(IntArray const& indices) const throw()
{	
	const int numChannels = indices.length();
	
	if(numChannels == 0)
	{
		ugen_assertfalse;
		return getNull();
	}
	else
	{
		UGenInternal** newInternals = new UGenInternal*[numChannels];
		
		for(int channel = 0; channel < numChannels; channel++)
		{
			int index = indices[channel];
			while(index < 0) index += numInternalUGens;
			
			newInternals[channel] = getInternalUGen(index % numInternalUGens);
		}
		
		return UGen(numChannels, newInternals);
	}
}

UGen UGen::range(const int startIndex, const int endIndex) const throw()
{
	ugen_assert(startIndex >= 0);
	ugen_assert((unsigned int)endIndex < numInternalUGens);
	ugen_assert(startIndex < endIndex);
	
	const int startIndexChecked = clip(startIndex, 0, numInternalUGens);
	const int endIndexChecked = clip(endIndex, 0, numInternalUGens);
	
	const int numChannels = endIndexChecked - startIndexChecked;
	
	if(numChannels <= 0)
	{
		ugen_assertfalse;
		return getNull();
	}
	else
	{
		UGenInternal** newInternals = new UGenInternal*[numChannels];
		
		for(int i = 0; i < numChannels; i++)
		{
			newInternals[i] = getInternalUGen(i + startIndexChecked);
		}
		
		return UGen(numChannels, newInternals);
	}
}

UGen UGen::from(const int startIndex) const throw()
{
	return range(startIndex);
}

UGen UGen::range(const int startIndex) const throw()
{
	return range(startIndex, 0x7fffffff);
}

UGen UGen::to(const int endIndex) const throw()
{
	return range(0, endIndex);
}

// could optimise here if the new size is less than the current size and not realloc...?
void UGen::initInternal(const int numInternalUGensToInit) throw()
{
	/* it's possible here that you passed an uninitialised variable to a UGen input. */
	ugen_assert(numInternalUGensToInit > 0); 
	
	if((numInternalUGens > 0) && (internalUGens != 0))
	{
		decrementInternals();
		delete [] internalUGens;
	}
	
	numInternalUGens = numInternalUGensToInit;
	internalUGens = (numInternalUGens > 0)
					? new UGenInternal*[numInternalUGens]
					: 0;
}

void UGen::incrementInternals() const throw()
{	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i]->incrementRefCount();
	}
}

void UGen::decrementInternals() const throw()
{	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i]->decrementRefCount();
	}
}

UGen UGen::withNumChannels(const int numChannels, const bool addedChannelsWrap) const throw()
{
	UGenInternal** newInternals = new UGenInternal*[numChannels];
	
	if(addedChannelsWrap || ((unsigned int)numChannels <= numInternalUGens))
	{
		for(int i = 0; i < numChannels; i++)
		{
			newInternals[i] = getInternalUGen(i % numInternalUGens);
		}
	}
	else
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			newInternals[i] = getInternalUGen(i);
		}
		
		for(int i = numInternalUGens; i < numChannels; i++)
		{
			newInternals[i] = getNullInternal();
		}
	}
	
	return UGen(numChannels, newInternals);
}

UGen UGen::getChannel(const int channel) const throw()
{
	return wrapAt(channel);
}

bool UGen::isNull(const int index) const throw()
{
	if(index < 0)
		return numInternalUGens == 1 && internalUGens[0]->isNull();
	else if((unsigned int)index < numInternalUGens)
		return internalUGens[index]->isNull();
	else
		return false;
}

bool UGen::isNotNull(const int index) const throw()
{
	return ! isNull(index);
}

bool UGen::isNullKr() const throw()
{
	if(isNull())
		return internalUGens[0]->isControlRateOnly();
	else
		return false;
}

bool UGen::isScalar(const int index) const throw()
{	
	if(index < 0)
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			if(internalUGens[i]->isScalar() == false)
				return false;
		}
		return true;
	}
	else if((unsigned int)index < numInternalUGens)
		return internalUGens[index]->isScalar();
	else
		return false;
}

bool UGen::isConst(const int index) const throw()
{	
	if(index < 0)
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			if(internalUGens[i]->isConst() == false)
				return false;
		}
		return true;
	}
	else if((unsigned int)index < numInternalUGens)
		return internalUGens[index]->isConst();
	else
		return false;
}


bool UGen::containsIdenticalInternalsAs(UGen const& other, const bool mustBeInTheSameSequence) const throw()
{
	if(numInternalUGens != other.numInternalUGens) 
		return false;
	
	if(mustBeInTheSameSequence == true)
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			if(internalUGens[i] != other.internalUGens[i])
				return false;
		}
	}
	else
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			bool foundMatch = false;
			
			for(unsigned int j = 0; j < other.numInternalUGens; j++)
			{
				if(internalUGens[i] == other.internalUGens[j])
				{
					foundMatch = true;
					break;
				}
			}
			
			if(foundMatch == false)
				return false;
		}
	}
	
	return true;
}


float* UGen::prepareAndProcessBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{		
	if(channel < 0)
	{
		prepareForBlock(actualBlockSize, blockID, -1);
	}
	else
	{
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			prepareForBlock(actualBlockSize, blockID, i);
		}		
	}
	
	bool shouldDelete = false;
	return processBlock(shouldDelete, blockID, channel);
}


float* UGen::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{ 	
	ugen_assert(numInternalUGens > 0);
	ugen_assert(internalUGens != 0);
	
	if(channel < 0) 
	{
		// process all channels
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i]->processBlockInternal(shouldDelete, blockID, i);
		}
		
		return 0;
	} 
	else if(numInternalUGens == 1) 
	{
		float* block = internalUGens[0]->processBlockInternal(shouldDelete, blockID, channel);
		return block;
	} 
	else if(numInternalUGens == 2) 
	{
		float* block = internalUGens[channel & 1]->processBlockInternal(shouldDelete, blockID, channel);
		return block;
	}
	else 
	{
		unsigned int internalChannel = (unsigned int)channel % numInternalUGens;		
		float* block = internalUGens[internalChannel]->processBlockInternal(shouldDelete, blockID, channel);
		return block;
	}
	
}
 
void UGen::prepareForBlock(const int actualBlockSize, const unsigned int blockID, const int channel) throw()
{	
	ugen_assert(actualBlockSize > 0);
	ugen_assert(numInternalUGens > 0);
	ugen_assert(internalUGens != 0);
	
	bool shouldDelete = false;
	
	if(channel < 0)
	{
		for(unsigned int i = 0; i < numInternalUGens; i++) 
		{
			if(internalUGens[i]->shouldBeDeletedNow(blockID)) {
				shouldDelete = true;
				break;
			}
		}
		
		if(shouldDelete) 
		{
			for(unsigned int i = 0; i < numInternalUGens; i++)
			{	
				internalUGens[i]->userData = userData;
				internalUGens[i]->decrementRefCount();
			}
			
			numInternalUGens = 1;
			internalUGens[0] = getNullInternal();
			internalUGens[0]->prepareForBlockInternal(actualBlockSize, blockID, 0);		
		} 
		else 
		{
			for(unsigned int i = 0; i < numInternalUGens; i++)
			{
				internalUGens[i]->userData = userData;
				internalUGens[i]->prepareForBlockInternal(actualBlockSize, blockID, i);
			}
		}
	}
	else if((unsigned int)channel < numInternalUGens)
	{
		bool shouldDelete = internalUGens[channel]->shouldBeDeletedNow(blockID);
		
		if(shouldDelete) 
		{
			for(unsigned int i = 0; i < numInternalUGens; i++)
			{	
				internalUGens[i]->userData = userData;
				internalUGens[i]->decrementRefCount();
			}
			
			numInternalUGens = 1;
			internalUGens[0] = getNullInternal();
			internalUGens[0]->prepareForBlockInternal(actualBlockSize, blockID, 0);		
		} 
		else 
		{
			internalUGens[channel]->userData = userData;
			internalUGens[channel]->prepareForBlockInternal(actualBlockSize, blockID, channel);
		}		
	}
}


void UGen::purgeInternalMemory() throw()
{
	UGenInternal** oldInternalUGens = internalUGens;
	internalUGens = new UGenInternal*[numInternalUGens];

	memcpy(internalUGens, oldInternalUGens, numInternalUGens * sizeof(UGenInternal*));
	
	delete [] oldInternalUGens;
	// sorts the "memory leak"
}

const UGen& UGen::getInput(const int input, const int channel) throw()
{
	if(channel < 0 || (unsigned int)channel >= numInternalUGens)
	{
		ugen_assertfalse;
		return getNull();
	}	
	
	return internalUGens[channel]->getInput(input);
}

UGenOutput* UGen::getOutput(const int channel) const throw()						
{ 
	if(channel < 0 || (unsigned int)channel >= numInternalUGens)
	{
		ugen_assertfalse;
		return 0;
	}
		
	return internalUGens[channel]->getOutputPtr();								
}

void UGen::setOutput(float* block, const int blockSize, const int channel) throw()		
{ 
	if(block == 0 || channel < 0 || (unsigned int)channel >= numInternalUGens)
	{
		ugen_assertfalse;
		return;
	}	
	
	internalUGens[channel]->getOutputPtr()->useExternalOutput(block, blockSize);	
}

void UGen::setOutputs(float** block, const int blockSize, const int numChannels) throw()
{
	ugen_assert(block);
	ugen_assert(blockSize > 0);
	ugen_assert(numChannels > 0);
	
	for(int i = 0; i < numChannels; i++)
	{
		setOutput(block[i], blockSize, i);//, shouldReplace);
	}
}

bool UGen::setInput(const float* block, const int /*blockSize*/, const int channel) throw()
{	
	if(block == 0 || channel < 0 || (unsigned int)channel >= numInternalUGens) return false;
			
	return internalUGens[channel]->setInput(block, channel);		
}

bool UGen::setInputs(const float** block, const int blockSize, const int numChannels) throw()
{
	ugen_assert(block);
	ugen_assert(blockSize > 0);
	ugen_assert(numChannels > 0);

	bool result = false;
	
	for(int i = 0; i < numChannels; i++)
	{
		result = setInput(block[i], blockSize, i) || result;
	}
	
	return result;
}

bool UGen::setValue(Value const& other) throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		if(internalUGens[i]->setValue(other)) return true;
	}
	
	return false;
}

//void UGen::setSource(UGen const& source, const bool releasePreviousSources, const float fadeTime) throw()
//{
//	ugen_assert(fadeTime >= 0.f);
//	
//	for(int i = 0; i < numInternalUGens; i++)
//	{
//		PlugUGenInternal* plug = dynamic_cast<PlugUGenInternal*>(internalUGens[i]);
//		
//		if(plug != 0)
//		{
//			plug->setSource(source, releasePreviousSources, fadeTime);
//			return;
//		}
//	}
//}

bool UGen::setSource(UGen const& source, const bool releasePreviousSources, const float fadeTime) throw()
{
	ugen_assert(fadeTime >= 0.f);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		if(internalUGens[i]->setSource(source, releasePreviousSources, fadeTime)) return true;
	}
	
	return false;
}


UGen UGen::getSource() throw()
{
	UGen sourcesUGen;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		UGen source = internalUGens[i]->getSource();
		
		if(source.isNotNull()) sourcesUGen <<= source;
	}
	
	return sourcesUGen;
}

void UGen::release() throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i]->releaseInternal();
	}
}

void UGen::steal(const bool forcedSteal) throw()
{
	if(forcedSteal == true)
	{
		// just dispose the current UGens and set them to null
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i]->decrementRefCount();
			internalUGens[i] = getNullInternal();
		}
	}
	else
	{
		// allow the UGens to handle stealing more gracefully
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			internalUGens[i]->stealInternal();
		}		
	}
}


bool UGen::sendMidiNote(const int midiChannel, const int midiNote, const int velocity) throw()
{
	bool result = false;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		result = internalUGens[i]->sendMidiNote(midiChannel, midiNote, velocity) || result;
	}
	
	return result;
}

#if defined(JUCE_VERSION)
#include "../juce/ugen_JuceVoicer.h"
void UGen::sendMidiBuffer(MidiBuffer const& midiMessages) throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		VoicerUGenInternal* voicer = dynamic_cast<VoicerUGenInternal*> (internalUGens[i]);
		
		if(voicer != 0) voicer->sendMidiBuffer(midiMessages);
	}
}
#endif

#if defined(UGEN_IPHONE) && defined(UGEN_IOS_COREMIDI)
#include "../iphone/ugen_iOSMidiInput.h"
void UGen::sendMidiBuffer(ByteArray const& midiMessages) throw()
{
	for(int i = 0; i < numInternalUGens; i++)
	{
		VoicerUGenInternal* voicer = dynamic_cast<VoicerUGenInternal*> (internalUGens[i]);
		
		if(voicer != 0) voicer->sendMidiBuffer(midiMessages);
	}
}
#endif

bool UGen::trigger(void* extraArgs) throw()
{
	bool result = false;

	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		result = internalUGens[i]->trigger(extraArgs) || result;
	}
	
	return result;
}

bool UGen::stopAllEvents() throw()
{
	bool result = false;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		result = internalUGens[i]->stopAllEvents() || result;
	}
	
	return result;
}

UGen& UGen::addBufferReceiver(BufferReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BufferSender* sender = dynamic_cast<BufferSender*> (internalUGens[i]);
		
		if(sender != 0) sender->addBufferReceiver(receiver);
	}
#endif
	
	return *this;
}

void UGen::removeBufferReceiver(BufferReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		BufferSender* sender = dynamic_cast<BufferSender*> (internalUGens[i]);
		
		if(sender != 0) sender->removeBufferReceiver(receiver);
	}	
#endif
}

UGen& UGen::addBufferReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		BufferSender* sender = dynamic_cast<BufferSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				BufferReceiver* receiver = dynamic_cast<BufferReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->addBufferReceiver(receiver);
			}
		}
	}
#endif
	return *this;
}

void UGen::removeBufferReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		BufferSender* sender = dynamic_cast<BufferSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				BufferReceiver* receiver = dynamic_cast<BufferReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->removeBufferReceiver(receiver);
			}
		}
	}
#endif
}

UGen& UGen::addDoneActionReceiver(DoneActionReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		DoneActionSender* sender = dynamic_cast<DoneActionSender*> (internalUGens[i]);
		
		if(sender != 0) sender->addDoneActionReceiver(receiver);
	}
#endif
	
	return *this;	
}

void UGen::removeDoneActionReceiver(DoneActionReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		DoneActionSender* sender = dynamic_cast<DoneActionSender*> (internalUGens[i]);
		
		if(sender != 0) sender->removeDoneActionReceiver(receiver);
	}	
#endif
}

UGen& UGen::addDoneActionReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		DoneActionSender* sender = dynamic_cast<DoneActionSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				DoneActionReceiver* receiver = dynamic_cast<DoneActionReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->addDoneActionReceiver(receiver);
			}
		}
	}
#endif
	
	return *this;	
}

void UGen::removeDoneActionReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		DoneActionSender* sender = dynamic_cast<DoneActionSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				DoneActionReceiver* receiver = dynamic_cast<DoneActionReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->removeDoneActionReceiver(receiver);
			}
		}
	}	
#endif
}

UGen& UGen::addMetaDataReceiver(MetaDataReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		MetaDataSender* sender = dynamic_cast<MetaDataSender*> (internalUGens[i]);
		
		if(sender != 0) sender->addMetaDataReceiver(receiver);
	}
#endif
	
	return *this;		
}

void UGen::removeMetaDataReceiver(MetaDataReceiver* const receiver) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		MetaDataSender* sender = dynamic_cast<MetaDataSender*> (internalUGens[i]);
		
		if(sender != 0) sender->removeMetaDataReceiver(receiver);
	}	
#endif	
}

UGen& UGen::addMetaDataReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		MetaDataSender* sender = dynamic_cast<MetaDataSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				MetaDataReceiver* receiver = dynamic_cast<MetaDataReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->addMetaDataReceiver(receiver);
			}
		}
	}
#endif
	
	return *this;		
}

void UGen::removeMetaDataReceiver(UGen const& receiverUGen) throw()
{
#if !defined(UGEN_ANDROID) || defined(UGEN_JUCE)
	for(unsigned int src = 0; src < numInternalUGens; src++)
	{
		MetaDataSender* sender = dynamic_cast<MetaDataSender*> (internalUGens[src]);
		
		if(sender != 0) 
		{
			for(unsigned int dst = 0; dst < receiverUGen.numInternalUGens; dst++)
			{
				MetaDataReceiver* receiver = dynamic_cast<MetaDataReceiver*> (receiverUGen.internalUGens[dst]);
				
				if(receiver != 0) sender->removeMetaDataReceiver(receiver);
			}
		}
	}	
#endif	
}

double UGen::getDuration() throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		double duration = internalUGens[i]->getDuration();
		
		if(duration >= 0.0) return duration;
	}
	
	return -1.0;
}

double UGen::getPosition() throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		double position = internalUGens[i]->getPosition();
		
		if(position >= 0.0) return position;
	}
	
	return -1.0;
}

bool UGen::setPosition(const double newPosition) throw()
{
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		if(internalUGens[i]->setPosition(newPosition)) return true;
	}
	
	return false;
}

DoubleArray UGen::getDurations() throw()
{
	DoubleArray result;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		double duration = internalUGens[i]->getDuration();

		if(duration >= 0.0) result.add(duration);
	}	
	
	return result;
}

DoubleArray UGen::getPositions() throw()
{
	DoubleArray result;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		double position = internalUGens[i]->getPosition();
		
		if(position >= 0.0) result.add(position);
	}	
	
	return result;
}

bool UGen::setPositions(DoubleArray const& newPositions) throw()
{
	ugen_assert(newPositions.length() > 0);
	
	bool result = false;
	int posIndex = 0;
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{
		result = internalUGens[i]->setPosition(newPositions.wrapAt(posIndex++)) || result;
	}
	
	return result;
}


//#if defined(UGEN_IPHONE)
//#include "../iphone/ugen_ScopeView.h"
//void UGen::addBufferReceiver(UIScopeView* receiver) throw()
//{
//	addBufferReceiver(receiver.peer);
//}
//
//void UGen::removeBufferReceiver(UIScopeView* receiver) throw()
//{
//	removeBufferReceiver(receiver.peer);
//}
//#endif


void UGen::prepareToPlay(const double sampleRate, const int estimatedSamplesPerBlock, const int newControlRateBlockSize) throw()
{	
	if(sampleRate > 0.0)
	{
		sampleRate_ = sampleRate;
		reciprocalSampleRate = 1.0 / sampleRate_;
	}
	
	if(estimatedSamplesPerBlock > 0)
		estimatedSamplesPerBlock_ = estimatedSamplesPerBlock;
	
	if(newControlRateBlockSize > 0)
		controlRateBlockSize = newControlRateBlockSize;
}

int UGen::findMaxInputChannels(const int count, const UGen * const array) throw()
{
	ugen_assert(count > 0);
	ugen_assert(array != 0);
	
	int numChannels = 0;
	
	for(int i = 0; i < count; i++)
	{
		int inputChannels = array[i].getNumChannels();
		if(inputChannels > numChannels)
			numChannels = inputChannels;
	}
	
	return numChannels;
}

int UGen::findMaxInputChannels(const int count, const Buffer * const array) throw()
{
	ugen_assert(count > 0);
	ugen_assert(array != 0);

	int numChannels = 0;
	
	for(int i = 0; i < count; i++)
	{
		int inputChannels = array[i].getNumChannels();
		if(inputChannels > numChannels)
			numChannels = inputChannels;
	}
	
	return numChannels;
}

int UGen::findMaxInputSizes(const int count, const Buffer * const array) throw()
{
	ugen_assert(count > 0);
	ugen_assert(array != 0);

	int size = 0;
	
	for(int i = 0; i < count; i++)
	{
		int inputSize = array[i].size();
		if(inputSize > size)
			size = inputSize;
	}
	
	return size;
}

void UGen::generateFromProxyOwner(ProxyOwnerUGenInternal* proxyOwner) throw()
{
	ugen_assert(proxyOwner != 0);
	
	internalUGens[0] = proxyOwner;
	
	for(unsigned int i = 1; i < numInternalUGens; i++)
		internalUGens[i] = proxyOwner->getProxy(i);	
}

UGenArray UGen::group(const int size) const throw()
{
	ugen_assert(size > 0);
	
	int outputSize = numInternalUGens / size;
	if(numInternalUGens % size != 0) outputSize++;
	
	UGenArray array(outputSize);
	
	int inputIndex = 0;
	
	for(int outputIndex = 0; outputIndex < outputSize; outputIndex++)
	{
		UGen item;
		
		for(int i = 0; i < size && inputIndex < (int)numInternalUGens; i++, inputIndex++)
		{
			item <<= operator[] (inputIndex);
		}
		
		array.put(outputIndex, item);
	}
	
	return array;
}

UGenArray UGen::interleave(const int size) const throw()
{	
	ugen_assert(size > 0);
	
	UGenArray array(size);
		
	for(int outputIndex = 0; outputIndex < size; outputIndex++)
	{
		UGen item;
		
		for(unsigned int inputIndex = outputIndex; inputIndex < numInternalUGens; inputIndex += size)
		{
			item <<= operator[] (inputIndex);
		}
		
		array.put(outputIndex, item);
	}
	
	return array;
}

UGenArray UGen::toArray() const throw()
{
	UGenArray array(numInternalUGens);
	
	for(unsigned int i = 0; i < numInternalUGens; i++)
	{		
		array.put(i, operator[] (i));
	}
	
	return array;
}

UGen UGen::groupMix(const int size) const throw()
{
	UGen newUGen(group(size).mix());
	newUGen.userData = userData;
	return newUGen;
}

UGen UGen::groupMixScale(const int size) const throw()
{
	UGenArray grouped = group(size);
	UGen newUGen(grouped.mix() / grouped.size());
	newUGen.userData = userData;
	return newUGen;
}

UGen UGen::interleaveMix(const int size) const throw()
{
	UGen newUGen(interleave(size).mix());
	newUGen.userData = userData;
	return newUGen;
}

UGen UGen::interleave(UGen const& rightOperand) const throw()
{
	// need to check this...
	
	if(rightOperand.isNull())
		return *this;
	else if(isNull())
		return rightOperand;
	else
	{
		UGen interleavedChannels;
		int maxNumChannels;
		if(numInternalUGens > rightOperand.numInternalUGens)
			maxNumChannels = numInternalUGens;
		else
			maxNumChannels = rightOperand.numInternalUGens;
		
		for(int i = 0; i < maxNumChannels; i++)
		{
			interleavedChannels <<= wrapAt(i);
			interleavedChannels <<= rightOperand.wrapAt(i);
		}
		
		return interleavedChannels;
	}
}

UGen UGen::ar() const throw()
{
	return *this;
}

UGen UGen::ar(UGen const& arg1) const throw()
{
	return UGen(arg1);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2) const throw()
{
	return UGen(arg1, arg2);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw()
{
	return UGen(arg1, arg2, arg3);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw()
{
	return UGen(arg1, arg2, arg3, arg4);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw()
{
	return UGen(arg1, arg2, arg3, arg4, arg5);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw()
{
	return UGen(arg1, arg2, arg3, arg4, arg5, arg6);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw()
{
	return UGen(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

UGen UGen::ar(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw()
{
	return UGen(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

UGen UGen::operator() () const throw()
{
	if(isNullKr())
		return kr(*this);
	else
		return ar(*this);
}

UGen UGen::operator() (UGen const& other) const throw()
{
	if(isNullKr())
		return kr(other);
	else
		return ar(other);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2);
	else
		return ar(arg1, arg2);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3);
	else
		return ar(arg1, arg2, arg3);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3, arg4);
	else
		return ar(arg1, arg2, arg3, arg4);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3, arg4, arg5);
	else
		return ar(arg1, arg2, arg3, arg4, arg5);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3, arg4, arg5, arg6);
	else
		return ar(arg1, arg2, arg3, arg4, arg5, arg6);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	else
		return ar(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

UGen UGen::operator() (UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw()
{
	if(isNullKr())
		return kr(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	else
		return ar(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

UGen UGen::kr() const throw()
{
	if(isNull())
	{
		UGenInternal* newInternal = internalUGens[0]->getKr();
		return UGen(newInternal);
	}
	else
	{
		UGenInternal** newInternals = new UGenInternal*[numInternalUGens];
		
		for(unsigned int i = 0; i < numInternalUGens; i++)
		{
			newInternals[i] = internalUGens[i]->getKr();
		}
		
		UGen newUGen = UGen((int)numInternalUGens, newInternals);
				
		delete [] newInternals;
		
		return newUGen;
	}
}

UGen UGen::kr(UGen const& arg1) const throw()
{
	return UGen(arg1.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2) const throw()
{
	return UGen(arg1.kr(), arg2.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr(), arg4.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr(), arg4.kr(), arg5.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr(), arg4.kr(), arg5.kr(), arg6.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr(), arg4.kr(), arg5.kr(), arg6.kr(), arg7.kr());
}

UGen UGen::kr(UGen const& arg1, UGen const& arg2, UGen const& arg3, UGen const& arg4, UGen const& arg5, UGen const& arg6, UGen const& arg7, UGen const& arg8) const throw()
{
	return UGen(arg1.kr(), arg2.kr(), arg3.kr(), arg4.kr(), arg5.kr(), arg6.kr(), arg7.kr(), arg8.kr());
}

UGen UGen::mix(bool shouldAllowAutoDelete) const throw()
{
	if(numInternalUGens == 1 && shouldAllowAutoDelete == true)
		return *this;
	else	
		return Mix(*this, shouldAllowAutoDelete);
}

UGen UGen::mixScale(bool shouldAllowAutoDelete) const throw()
{
	if(numInternalUGens == 1 && shouldAllowAutoDelete == true)
		return *this;
	else	
		return Mix(*this, shouldAllowAutoDelete) * (1.f / (float)numInternalUGens);
}

UGen UGen::operator- () const throw()
{
	return UnaryNegUGen(*this);
}

#ifndef UGEN_NOEXTGPL
UGen UGen::lag(UGen const& rightOperand) const throw()
{
	if(isNull())
		return rightOperand;
	else if(numInternalUGens == 1 && internalUGens[0]->isControlRateOnly())
		return UGen(Lag(*this, rightOperand.kr())).kr();
	else
		return Lag(*this, rightOperand);
}

UGen UGen::wrap(UGen const& lower, UGen const& upper) throw()
{
	return Wrap::AR(*this, lower, upper);
}

UGen UGen::fold(UGen const& lower, UGen const& upper) throw()
{
	return Fold::AR(*this, lower, upper);
}

UGen UGen::linlin(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw()
{
	return LinLin::AR(*this, inLow, inHigh, outLow, outHigh);
}

UGen UGen::linsin(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw()
{
	return LinSin::AR(*this, inLow, inHigh, outLow, outHigh);
}

UGen UGen::linexp(UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh) throw()
{
	return LinExp::AR(*this, inLow, inHigh, outLow, outHigh);
}

#endif // gpl


unsigned long	UGen::nextBlockID					= 0;
double			UGen::sampleRate_					= 44100.0;
double			UGen::reciprocalSampleRate			= 1.0 / UGen::sampleRate_;
int				UGen::estimatedSamplesPerBlock_		= 512;
int				UGen::controlRateBlockSize			= 32;//64;
//int				UGen::blockSize						= 0;
//double			UGen::slopeFactor					= 1.0 / estimatedSamplesPerBlock_;
//double			UGen::controlSlopeFactor			= (double)controlRateBlockSize / estimatedSamplesPerBlock_;

bool			UGen::isInitialised					= false;

Deleter UGen::defaultDeleter;
Deleter* UGen::internalUGenDeleter = &UGen::defaultDeleter;


const int		UGen::defaultUserData				= 0x7FFFFFFF;





END_UGEN_NAMESPACE

