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


#include "ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Value.h"
#include "../basics/ugen_UnaryOpUGens.h"


ValueInternal::ValueInternal(const double value) throw()
:	value_(value)
{ 
}

double ValueInternal::getValue() throw()
{ 
	return value_; 
}

void ValueInternal::setValue(const double newValue) throw()
{
	value_ = newValue;
}

Value::Value() throw()
:	internal(new ValueInternal(0.0))  
{
}

Value::Value(const double value) throw()
:	internal(new ValueInternal(value))  
{ 
}

Value::Value(const float value) throw()
:	internal(new ValueInternal((double)value))  
{ 
}

Value::Value(const int value) throw()   
:	internal(new ValueInternal((double)value))  
{ 
}

#if defined(JUCE_VERSION)
#include "../juce/ugen_JuceSlider.h"
Value::Value(Slider *slider) throw()
:	internal(new SliderValueInternal(slider))
{
}

Value::Value(Button *button) throw()
:	internal(new ButtonValueInternal(button))
{
}

Value::Value(Label *label) throw()
:	internal(new LabelValueInternal(label))
{
}
#endif

Value::Value(ValueBaseInternal *internalToUse) throw()
:	internal(internalToUse) 
{
}

Value::~Value()
{ 
	internal->decrementRefCount(); 
}

Value::Value(Value const& copy) throw()
{
	internal = copy.internal;
	internal->incrementRefCount();
}

Value& Value::operator= (Value const& other) throw()
{
	if (this != &other)
    {		
		other.internal->incrementRefCount();
		internal->decrementRefCount();
		internal = other.internal;
    }
	
    return *this;
}

ValueArray Value::operator<< (ValueArray const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray Value::operator<< (Value const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray Value::operator, (ValueArray const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray Value::operator, (Value const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray Value::fill(const int size) throw()
{
	return ValueArray(*this, size);
}

Value Value::operator- () const throw()
{
	if(internal == 0) 
		return *this;
	else if(containsInternalType<UnaryNegValueInternal>())
		return (dynamic_cast<UnaryNegValueInternal*> (internal))->getOperand();
	else
		return UnaryNegValue(*this);
}

Value& Value::operator+= (Value const& other) throw()
{
	operator= (*this + other);
	return *this;
}

Value& Value::operator-= (Value const& other) throw()
{
	operator= (*this - other);
	return *this;
}

Value& Value::operator*= (Value const& other) throw()
{
	operator= (*this * other);
	return *this;
}

Value& Value::operator/= (Value const& other) throw()
{
	operator= (*this / other);
	return *this;
}

//Value Value::null = 0.0;

#ifndef UGEN_NOEXTGPL

RandomValueBaseInternal::RandomValueBaseInternal() throw()
: random(Ran088::defaultGenerator().next()) 
{ 
}

void RandomValueBaseInternal::setValue(const double newValue) throw()
{
	random.setSeed(newValue);
}

RandomDoubleRangeValueInternal::RandomDoubleRangeValueInternal(Value const& lo, Value const& hi) throw()
:	lo_(lo), hi_(hi)
{
}

double RandomDoubleRangeValueInternal::getValue() throw()
{
	double lo = lo_.getValue();
	double hi = hi_.getValue();
	return random.nextDouble() * (hi-lo) + lo;
}


RandomDoubleExpRangeValueInternal::RandomDoubleExpRangeValueInternal(Value const& lo, Value const& hi) throw()
:	lo_(lo), hi_(hi)
{
}

double RandomDoubleExpRangeValueInternal::getValue() throw()
{
	double lo = lo_.getValue();
	double hi = hi_.getValue();
	return random.nextExpRandRange(lo, hi);
}

RandomIntRangeValueInternal::RandomIntRangeValueInternal(const int lo, const int hi) throw()
:	lo_(lo), range(hi-lo)
{
}

double RandomIntRangeValueInternal::getValue() throw()
{
	return random.nextLong(range) + lo_;
}

RandomValue::RandomValue(const double scale) throw()
:	Value(new RandomDoubleRangeValueInternal(0.0, scale))
{
}

RandomValue::RandomValue(Value const& scale) throw()
:	Value(new RandomDoubleRangeValueInternal(0.0, scale))
{
}

RandomValue::RandomValue(const double lo, const double hi, const bool exponential) throw()
:	Value(exponential ? 
		  static_cast<ValueBaseInternal*> (new RandomDoubleExpRangeValueInternal(lo, hi))
		  :
		  static_cast<ValueBaseInternal*> (new RandomDoubleRangeValueInternal(lo, hi)))
{
}

RandomValue::RandomValue(Value const& lo, Value const& hi, const bool exponential) throw()
:	Value(exponential ? 
		  static_cast<ValueBaseInternal*> (new RandomDoubleExpRangeValueInternal(lo, hi))
		  :
		  static_cast<ValueBaseInternal*> (new RandomDoubleRangeValueInternal(lo, hi)))
{
}

RandomValue::RandomValue(const int scale) throw()
:	Value(new RandomIntRangeValueInternal(0, scale))
{	
}

RandomValue::RandomValue(const int lo, const int hi) throw()
:	Value(new RandomIntRangeValueInternal(lo, hi))
{
}

#endif // gpl

SequenceValueInternal::SequenceValueInternal(Value const& start, Value const& grow) throw()
:	start_(start),
	currentValue(start_.getValue()),
	grow_(grow),
	threshold_(0.0),
	loopType(NoLoop),
	loopDirection_(Normal),
	numSteps_(0), 
	currentStep(0),
	loopCount(0)
{
}

SequenceValueInternal::SequenceValueInternal(Value const& start, 
											 Value const& grow, 
											 const int numSteps, 
											 LoopDirection loopDirection) throw()
:	start_(start),
	currentValue(start_.getValue()),
	grow_(grow), 
	threshold_(0.0),
	loopType(FixedNumSteps),
	loopDirection_(loopDirection),
	numSteps_(numSteps),
	currentStep(0),
	loopCount(0)
{
}

SequenceValueInternal::SequenceValueInternal(Value const& start, 
											 Value const& grow, 
											 Value const& threshold, 
											 LoopDirection loopDirection) throw()
:	start_(start),
	currentValue(start_.getValue()),
	grow_(grow),
	threshold_(threshold),
	loopType(threshold_.getValue() > currentValue ? ThresholdIncreasing : ThresholdDecreasing),
	loopDirection_(loopDirection),
	numSteps_(0),
	currentStep(0),
	loopCount(0)
{
}

bool SequenceValueInternal::checkLoop() throw()
{
	bool loopedThisTime = false;
	
	switch (loopType) 
	{
		case FixedNumSteps: {
			currentStep++;
			if(currentStep >= numSteps_)
			{
				loopCount++;
				currentStep = 0;
				loopedThisTime = true;
				
				if(loopDirection_ == Normal)
					currentValue = start_.getValue();
			}
		} break;
		case ThresholdIncreasing: {
			if(currentValue >= threshold_.getValue())
			{
				loopCount++;
				loopedThisTime = true;
				
				if(loopDirection_ == Normal)
					currentValue = start_.getValue();
				else {					
					Value temp = start_;
					start_ = threshold_;
					threshold_ = temp;
					
					loopType = ThresholdDecreasing;
				}
			}
		} break;
		case ThresholdDecreasing: {
			if(currentValue <= threshold_.getValue())
			{
				loopCount++;
				loopedThisTime = true;
				
				if(loopDirection_ == Normal)
					currentValue = start_.getValue();
				else {					
					Value temp = start_;
					start_ = threshold_;
					threshold_ = temp;
					
					loopType = ThresholdIncreasing;
					
					//?currentValue = start_.getValue();
				}
			}
		} break;
		default:
			; // otherwise do nothing
	}
	
	return loopedThisTime;
}

void SequenceValueInternal::setValue(const double newValue) throw()
{
	currentValue = newValue;
}

SeriesValueInternal::SeriesValueInternal(Value const& start, Value const& grow) throw()
:	SequenceValueInternal(start, grow)
{
}

SeriesValueInternal::SeriesValueInternal(Value const& start, Value const& grow, const int numSteps, LoopDirection loopDirection) throw()
:	SequenceValueInternal(start, grow, numSteps, loopDirection)
{
}

SeriesValueInternal::SeriesValueInternal(Value const& start, Value const& grow, Value const& threshold, LoopDirection loopDirection) throw()
:	SequenceValueInternal(start, grow, threshold, loopDirection)
{
}


double SeriesValueInternal::getValue() throw()
{
	double returnValue = currentValue;
	double growValue = grow_.getValue();
	currentValue += growValue;
	
	if(checkLoop())
	{
	   grow_ = -grow_;
	   
	   if(loopDirection_ == Alternate)
	   {
		   currentValue -= growValue;
		   if(loopCount % 2) 
			   currentValue -= growValue;
	   }
	}
	
	return returnValue;
}

SeriesValue::SeriesValue(Value const& start, Value const& grow) throw()
:	Value(new SeriesValueInternal(start, grow))
{
}

SeriesValue::SeriesValue(Value const& start, Value const& grow, const int numSteps, SequenceValueInternal::LoopDirection loopDirection) throw()
:	Value(new SeriesValueInternal(start, grow, numSteps, loopDirection))
{
}

SeriesValue::SeriesValue(Value const& start, Value const& grow, Value const& threshold, SequenceValueInternal::LoopDirection loopDirection) throw()
:	Value(new SeriesValueInternal(start, grow, threshold, loopDirection))
{
}

GeomValueInternal::GeomValueInternal(Value const& start, Value const& grow) throw()
:	SequenceValueInternal(start, grow)
{
}

GeomValueInternal::GeomValueInternal(Value const& start, Value const& grow, const int numSteps, LoopDirection loopDirection) throw()
:	SequenceValueInternal(start, grow, numSteps, loopDirection)
{
}

GeomValueInternal::GeomValueInternal(Value const& start, Value const& grow, Value const& threshold, LoopDirection loopDirection) throw()
:	SequenceValueInternal(start, grow, threshold, loopDirection)
{
}

double GeomValueInternal::getValue() throw()
{
	double returnValue = currentValue;
	double growValue = grow_.getValue();
	currentValue *= growValue;
	
	if(checkLoop())
	{
	   grow_ = Value(1.0) / grow_;
	   
	   if(loopDirection_ == Alternate)
	   {
		   if((loopCount % 2) == 1) {
			   currentValue /= growValue;
			   currentValue /= growValue;
		   }
	   }
	}
	
	return returnValue;
}

GeomValue::GeomValue(Value const& start, Value const& grow) throw()
:	Value(new GeomValueInternal(start, grow))
{
}

GeomValue::GeomValue(Value const& start, Value const& grow, const int numSteps, SequenceValueInternal::LoopDirection loopDirection) throw()
:	Value(new GeomValueInternal(start, grow, numSteps, loopDirection))
{
}

GeomValue::GeomValue(Value const& start, Value const& grow, Value const& threshold, SequenceValueInternal::LoopDirection loopDirection) throw()
:	Value(new GeomValueInternal(start, grow, threshold, loopDirection))
{
}

ValueArray::ValueArrayInternal::ValueArrayInternal(const int size) throw()
:	size_(size),
	array(new Value[size_])
{
}

ValueArray::ValueArrayInternal::~ValueArrayInternal() throw()
{
	delete [] array;
	array = 0;
	size_ = 0;
}


ValueArray::ValueArray() throw()
:	internal(0)
{
}

ValueArray::ValueArray(const int size) throw()
:	internal(new ValueArrayInternal(size))
{
}

ValueArray::ValueArray(Value const& value) throw()
:	internal(new ValueArrayInternal(1))
{
	internal->array[0] = value;
}

ValueArray::ValueArray(const double value) throw()
:	internal(new ValueArrayInternal(1))
{
	internal->array[0] = value;
}

ValueArray::ValueArray(Value const& value, const int size) throw()
:	internal(new ValueArrayInternal(size))
{
	for(int i = 0; i < size; i++)
	{
		internal->array[i] = value;
	}
}

ValueArray::ValueArray(ValueArray const& copy) throw()
:	internal(copy.internal)
{
	if(internal != 0)
		internal->incrementRefCount();
}

ValueArray::ValueArray(Buffer const& copy) throw()
:	internal(copy.size() > 0 ? new ValueArrayInternal(copy.size()) : 0)
{
	if(internal != 0)
	{
		for(int i = 0; i < internal->size_; i++)
		{
			internal->array[i] = copy.getSampleUnchecked(i);
		}
	}
}

ValueArray::ValueArray(ValueArray const& array0, ValueArray const& array1) throw()
:	internal(0)
{
	int newSize = array0.size() + array1.size();
	
	if(newSize != 0) {
		internal = new ValueArrayInternal(newSize);
		
		int newIndex = 0;
		
		for(int i = 0; i < array0.size(); i++, newIndex++) {
			internal->array[newIndex] = array0.internal->array[i];
		}
		
		for(int i = 0; i < array1.size(); i++, newIndex++) {
			internal->array[newIndex] = array1.internal->array[i];
		}
	}
}

ValueArray::~ValueArray() throw()
{
	if(internal != 0)
		internal->decrementRefCount();
}

ValueArray ValueArray::operator<< (ValueArray const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray ValueArray::operator<< (Value const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray ValueArray::operator, (ValueArray const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray ValueArray::operator, (Value const& other) throw()
{
	return ValueArray(*this, other);
}

ValueArray& ValueArray::operator= (ValueArray const& other) throw()
{
	if (this != &other)
    {		
		if(internal != 0)
			internal->decrementRefCount();
		
		if(other.internal != 0)
			other.internal->incrementRefCount();
		
		internal = other.internal;
    }
	
    return *this;
}

ValueArray& ValueArray::operator+= (ValueArray const& other) throw()
{
	if(internal == 0)
		operator= (other);
	else
		operator= (*this + other);
	
	return *this;	
}

ValueArray& ValueArray::operator-= (ValueArray const& other) throw()
{
	if(internal == 0)
		operator= (other.neg());
	else
		operator= (*this - other);
	
	return *this;	
}

ValueArray& ValueArray::operator*= (ValueArray const& other) throw()
{
	if(internal == 0)
		operator= (ValueArray(0.0));
	else
		operator= (*this * other);
	
	return *this;	
}

ValueArray& ValueArray::operator/= (ValueArray const& other) throw()
{
	if(internal == 0)
		operator= (other);
	else
		operator= (*this / other);
	
	return *this;	
}


ValueArray& ValueArray::operator<<= (ValueArray const& other) throw()
{
	return operator= (ValueArray(*this, other));
}

void ValueArray::put(const int index, Value const& item) throw()
{
	if(internal == 0 || index < 0 || index >= internal->size_) return;
	
	internal->array[index] = item;
}

Value& ValueArray::operator[] (const int index) const throw()
{
	if(internal == 0 || index < 0 || index >= internal->size_) return Value::getNull();
	
	return internal->array[index];
}

Value& ValueArray::at(const int index) const throw()
{
	if(internal == 0 || index < 0 || index >= internal->size_) return Value::getNull();
	
	return internal->array[index];
}

Value& ValueArray::wrapAt(const int index) const throw()
{
	if(internal == 0) return Value::getNull();
	
	int indexToUse = index;
	while(indexToUse < 0)
		indexToUse += internal->size_;
	
	return internal->array[index % internal->size_];
}

Value& ValueArray::first() const throw()
{
	if(internal == 0) return Value::getNull();
	
	return internal->array[0];
}

Value& ValueArray::last() const throw()
{
	if(internal == 0) return Value::getNull();
	
	return internal->array[internal->size_ - 1];
}

ValueArray ValueArray::operator- () const throw()
{
	if(size() == 0)																										
		return *this;																									
	else {																												
		ValueArray newArray(internal->size_);																			
		for(int i = 0; i < internal->size_; i++)																		
			newArray.put(i, -(internal->array[i]));															
		return newArray;																								
	}																														
}

ValueUGenInternal::ValueUGenInternal(Value const& value) 
:	UGenInternal(0), 
	valueObject(value),
	isConst(valueObject.containsInternalType<ValueInternal>())
{				 
}

UGenInternal* ValueUGenInternal::getKr() throw()
{
	return new ValueUGenInternalK(valueObject);
}

void ValueUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	if(!isConst)
	{
		while(numSamplesToProcess--)
		{
			*outputSamples++ = (float)valueObject.getValue();
		}
	}
	else
	{	
		float value = (float)valueObject.getValue();
		while(numSamplesToProcess--)
		{
			*outputSamples++ = value;
		}
	}
}

void ValueUGenInternal::setValue(Value const& other) throw()
{
	valueObject = other;
	isConst = valueObject.containsInternalType<ValueInternal>();
}

void ValueUGenInternalK::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	const int krBlockSize = UGen::getControlRateBlockSize();
	unsigned int blockPosition = blockID % krBlockSize;
	int numSamplesToProcess = uGenOutput.getBlockSize();
	float* outputSamples = uGenOutput.getSampleData();
	
	int numKrSamples = blockPosition % krBlockSize;
	
	while(numSamplesToProcess > 0)
	{
		float nextValue = value;
		
		if(numKrSamples == 0)			
		{
			nextValue = (float)valueObject.getValue();
		}
		
		numKrSamples = krBlockSize - numKrSamples;		
		blockPosition		+= numKrSamples;
		
		if(nextValue == value)
		{
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = nextValue;
				--numSamplesToProcess;
				--numKrSamples;
			}
		}
		else
		{
			float valueSlope = (nextValue - value) * UGen::getControlSlopeFactor();
			
			while(numSamplesToProcess && numKrSamples)
			{
				*outputSamples++ = value;
				value += valueSlope;
				--numSamplesToProcess;
				--numKrSamples;
			}
			
			value = nextValue;
		}
	}
	
}



ValueUGen::ValueUGen(ValueArray const& values) throw()
{
	initInternal(values.size());
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new ValueUGenInternal(values[i]);
	}
}

END_UGEN_NAMESPACE

