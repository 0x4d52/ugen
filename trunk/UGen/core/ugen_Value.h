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

#ifndef _UGEN_ugen_Value_H_
#define _UGEN_ugen_Value_H_

#include "../core/ugen_UGen.h"
#include "ugen_SmartPointer.h"
#include "ugen_Random.h"
#include "../basics/ugen_InlineUnaryOps.h"
#include "../basics/ugen_InlineBinaryOps.h"
#include "../buffers/ugen_Buffer.h"

class ValueArray;

/** Base class for internal value classes.
 Classes which generate values should inherit from this and implement getValue(). 
 @ingroup Values */
class ValueBaseInternal : public SmartPointer
{
public:	
	/** Evaluate the value. @return The value as a double. */
	virtual double getValue() throw() = 0;
	
	/** Set the current value. */
	virtual void setValue(const double newValue) throw() = 0;
	
	virtual bool isConst() const { return false; }
};

/** An internal value class with a constant value.
 This value of this is set on construction an doesn't change on each evaluation. 
 @ingroup Values */
class ValueInternal : public ValueBaseInternal
{
public:
	/** Constructor. @param value The constant value of this ValueInternal. */
	ValueInternal(double value = 0.0) throw();
	/** Return the value. @return The value. */
	double getValue() throw(); 
	void setValue(const double newValue) throw();
	bool isConst() const { return true; }

private:	
	double value_;
	
	ValueInternal (const ValueInternal&);
    const ValueInternal& operator= (const ValueInternal&);
};

/** This provides a means for providing a "specification" for a value which will be evaluated later.
 
 A good example is the RandomValue subclass, this allows a specification for a random number (e.g., 
 within particular limits) but the value will not be generated until it is required. 
 
 Many of the variants use Value in their specifications thus may be different on each evaluation.
 This enables powerful chaining of Value instances to produce complex behaviour.
 
 Value instances can be used a way of speficying a UGen using the ValueUGen, here the Value is evaluated each
 sample or control block. (A value passed to a UGen directly creates a ValueUGen.)
 
 To implement a new Value-type class you need you need to make two classes, one which inherits from 
 ValueBaseInternal which actually generates the value. And a factory class which inherits from Value and
 constructs a new internal class and passes it to the Value class constructor: 
 Value(ValueBaseInternal *internalToUse).
 
 @see RandomValue, R, GeomValue, G, SeriesValue, S, ValueArray 
 @ingroup Values */
class Value
{
public:	
	
	/// @name Construction and destruction
	/// @{
	
	/** Default constructor. 
	 Creates an empty value, this would be evaulated as 0.0.*/
	Value() throw();
	
	/** A construct a Value from a double.
	 @param value The double value to use. */
	Value(const double value) throw();
	
	/** A construct a Value from a float.
	 @param value The float value to use (although it is stored as a double). */
	Value(const float value) throw();
	
	/** A construct a Value from an int.
	 @param value The int value to use (although it is stored as a double). */
	Value(const int value) throw();
	
	
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	Value(Slider *slider) throw();
	Value(Button *button) throw();
	Value(Label *label) throw();
#endif
#if defined(UGEN_IPHONE) || defined(DOXYGEN)
	Value(UISlider *slider) throw();
#endif
	
	~Value();
	
	/** Copy constructor. */
	Value(Value const& copy) throw();
	
	/** Assignment operator. */
	Value& operator= (Value const& other) throw();
	
	
	
	/// @} <!-- end Construction and destruction ---------------------------------- -->
	
	/// @name Array construction
	/// @{
	
	ValueArray operator<< (ValueArray const& other) throw();
	ValueArray operator<< (Value const& other) throw();
	ValueArray operator, (ValueArray const& other) throw();
	ValueArray operator, (Value const& other) throw();
	
	/** Create a ValueArray containing a number of these Value instances. 
	 @param size The number of times to copy the Value into the ValueArray.
	 @return The ValueArray. */
	ValueArray fill(const int size) throw();
	
	/// @} <!-- end Array construction -------------------------------------------- -->
	
	/// @name Values
	/// @{
	
	/** Evaluate the value. @return The value as a double. */
	inline double getValue() throw() { return internal ? internal->getValue() : 0.0; } 
	
	/** Set the current value. */
	inline void setValue(const double newValue) throw() { if(internal) internal->setValue(newValue); } 
	
	/** Sets the value. */
	inline void setValue(Value const& other) throw() { *this = other; } 
	
	inline bool isConst() const throw() 
	{ 
		if(internal) 
			return internal->isConst();
		else
			return true; // will always be zero!
	}
	
	/** A null value (equal to 0.0) */
	inline static Value& getNull() { static Value null; null = 0.0; return null; }
	//static Value null;
	
	/** Return this value as a control rate UGen.
	 NB Value objects are converted to audio rate UGens by default. */
	inline UGen kr() { return UGen(*this).kr(); }
	
	/// @} <!-- end Values -------------------------------------------------------- -->
	
	
#ifndef UGEN_ANDROID
	/**
	 Tests whether this Value contains a particular ValueInternal derived class.
	 
	 @code
	 Value v = SeriesValue(0, 1, 10);
	 bool test = v.containsInternalType<SeriesValueInternal>();
	 @endcode
	 
	 ..here 'test' would be true. */
	template<class ValueInternalType>
	bool containsInternalType() const
	{
		if(internal == 0) 
			return false;
		else { //return dynamic_cast<ValueInternalType*> (internal) != 0;
			return typeid(ValueInternalType*) == typeid(internal);
		}
	}
#endif
	
	/** @name Unary Ops.
	 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except unary operator- */
	/// @{
	
	/** Unary negation. 
	 If the Value is already a Value with this operator applied its source will be returned rather than applying
	 the operator again. This save excessively long call chains if the operator is applied to alternately flip
	 a value over and over again. e.g.,
	 @code
	 Value a = 1;
	 Value b = -a; // b has the value 1 with the operator - applied i.e. -1
	 Value c = -b; // c is idential to a, it is not a with opertor - applied twice
	 @endcode
	 */
	Value operator- () const throw();
	UnaryOpMethodsDeclare(Value);
	
	/// @} <!-- end Unary Ops ------------------------------ -->
	
	/** @name Binary Ops. 
	 
	 The definition of these (except the assignment versions) is done by macros in ugen_BinaryOpUGens.cpp */
	/// @{
	BinaryOpMethodsDeclare(Value);
	
	/** Assignment and addition.
	 Creates a Value which the sum of this Value and another Value on evaluation.
	 This:
	 @code
	 Value x = 1.0;
	 Value y += 2.0;
	 @endcode
	 is equivalent to:
	 @code
	 Value x = 1.0;
	 Value y = y + 2.0;
	 @endcode
	 */
	Value& operator+= (Value const& other) throw();
	
	/** Assignment and subtraction.
	 Creates a Value which the difference of this Value and another Value on evaluation. */
	Value& operator-= (Value const& other) throw();
	
	/** Assignment and multiplication.
	 Creates a Value which the product of this Value and another Value on evaluation. */
	Value& operator*= (Value const& other) throw();
	
	/** Assignment and division.
	 Creates a Value which the equal to this Value "over" another Value on evaluation. */
	Value& operator/= (Value const& other) throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
protected:		
	Value(ValueBaseInternal *internalToUse) throw();
	ValueBaseInternal *internal;
};
#define V Value

#ifndef UGEN_NOEXTGPL

/** An intermediate internal class that contains a random number generator.
 This is an intermediate since it doesn't implement getValue(). 
 @ingroup Values */ 
class RandomValueBaseInternal : public ValueBaseInternal
{
public:
	RandomValueBaseInternal() throw();
	void setValue(const double newValue) throw();
	
protected:
	Ran088 random;
};

/** An internal random value generator between low and high limits. 
 @ingroup Values */
class RandomDoubleRangeValueInternal : public RandomValueBaseInternal
{
public:
	RandomDoubleRangeValueInternal(Value const& lo = 0.0, Value const& hi = 1.0) throw();	
	double getValue() throw(); 
	
private: 
	Value lo_, hi_;
};

/** An internal random value generator between low and high limits exponentially distrubuted. 
 @ingroup Values */
class RandomDoubleExpRangeValueInternal : public RandomValueBaseInternal
{
public:
	RandomDoubleExpRangeValueInternal(Value const& lo = 100.0, Value const& hi = 1000.0) throw();	
	double getValue() throw(); 
	
private: 
	Value lo_, hi_;
};

/** An internal random value generator between low and high limits returning values rounded to integers. 
 @ingroup Values */
class RandomIntRangeValueInternal : public RandomValueBaseInternal
{
public:
	RandomIntRangeValueInternal(const int lo = 0, const int hi = 100) throw();	
	double getValue() throw(); 
	
private: 
	const int lo_, range;
};

/** This produces a new random value according to a specification each time it is evaluated. 
 @ingroup Values */
class RandomValue : public Value
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	/** Construct a RandomValue with a range 0.0-scale. */
	RandomValue(const double scale) throw();
	
	/** Construct a RandomValue with a range 0.0-scale. 
	 
	 Here the scale is evaluated each time (thus may be different for each evaulation).
	 */
	RandomValue(Value const& scale) throw();
	
	/** Construct a RandomValue within a range. 
	 @param lo				The lower limit of the range.
	 @param hi				The upper limit of the range.
	 @param exponential		If true the random numbers will be exponentially distributed. 
							If false they will be uniformly distributed. */
	RandomValue(const double lo, const double hi, const bool exponential = false) throw();
	
	/** Construct a RandomValue within a range. 
	 @param lo				The lower limit of the range (evaluated on each evaluation).
	 @param hi				The upper limit of the range (evaluated on each evaluation).
	 @param exponential		If true the random numbers will be exponentially distributed. 
							If false they will be uniformly distributed. */
	RandomValue(Value const& lo, Value const& hi, const bool exponential = false) throw();
	
	/** Construct a RandomValue with a range 0-scale which are rounded to integers. */
	RandomValue(const int scale) throw();
	
	/** Construct a RandomValue within an interer range. 
	 @param lo				The lower limit of the range (inclusive).
	 @param hi				The upper limit of the range (exclusive). */
	RandomValue(const int lo, const int hi) throw();
	
	/// @} <!-- end Construction and destruction ------------------------------------ -->
	
private:
	RandomValue(); // no default constructor!
};
#define R	RandomValue

#endif // gpl

/** An intermediate internal class that generates sequences.
 This is an intermediate since it doesn't implement getValue(). 
 @ingroup Values */ 
class SequenceValueInternal : public ValueBaseInternal
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	SequenceValueInternal(Value const& start, Value const& grow) throw();
	
	enum LoopType { 
		NoLoop, 
		FixedNumSteps, 
		ThresholdIncreasing, 
		ThresholdDecreasing,
		NumLoopTypes 
	};
	
	enum LoopDirection {
		Normal, Alternate, NumLoopDirections
	};
	
	SequenceValueInternal(Value const& start, Value const& grow, const int numSteps, LoopDirection loopDirection) throw();
	SequenceValueInternal(Value const& start, Value const& grow, Value const& threshold, LoopDirection loopDirection) throw();
	
	/// @} <!-- end Construction and destruction ------------------------------------------------------------------------- -->
	
	/// @name Tests
	/// @{
	
	/** Check whether the sequence has hit a loop end point and adjust the parameters as necessary.
	 @return True if the loop occurred false if not. */
	bool checkLoop() throw();
	
	/// @} <!-- end Tests ------------------------- -->
	
	void setValue(const double newValue) throw();
	
protected:
	Value start_;
	double currentValue;
	Value grow_;
	Value threshold_;
	LoopType loopType;
	LoopDirection loopDirection_;
	int numSteps_, currentStep, loopCount;
	
};

/** @ingroup Values */
class SeriesValueInternal : public SequenceValueInternal
{
public:
	SeriesValueInternal(Value const& start, Value const& grow) throw();
	SeriesValueInternal(Value const& start, Value const& grow, const int numSteps, LoopDirection loopDirection) throw();
	SeriesValueInternal(Value const& start, Value const& grow, Value const& threshold, LoopDirection loopDirection) throw();
	double getValue() throw();
};

/** This provides a specification for generating a linear series of values. 
 @ingroup Values */
class SeriesValue : public Value
{
public:
	/** Construct a SeriesValue which starts at start and increments by grow.
	 
	 @param start	The starting value, this is evaluated once on instantiation.
	 @param grow	The grow value, this is evaluated on each evaulation and
					is added to the previous value to produce the result. */
	SeriesValue(Value const& start, Value const& grow) throw();
	SeriesValue(Value const& start, 
				Value const& grow, 
				const int numSteps, 
				SequenceValueInternal::LoopDirection loopDirection = SequenceValueInternal::Normal) throw();
	SeriesValue(Value const& start, 
				Value const& grow, 
				Value const& threshold, 
				SequenceValueInternal::LoopDirection loopDirection = SequenceValueInternal::Normal) throw();
	
private:
	SeriesValue();
};
#define S SeriesValue

/** @ingroup Values */
class GeomValueInternal : public SequenceValueInternal
{
public:
	GeomValueInternal(Value const& start, Value const& grow) throw();
	GeomValueInternal(Value const& start, Value const& grow, const int numSteps, LoopDirection loopDirection) throw();
	GeomValueInternal(Value const& start, Value const& grow, Value const& threshold, LoopDirection loopDirection) throw();
	double getValue() throw();
};

/** This provides a specification for generating a geometic series of values. 
 @ingroup Values */
class GeomValue : public Value
{
public:
	/** Construct a GeomValue which starts at start and increments by a factor of grow.
	 
	 @param start	The starting value, this is evaluated once on instantiation.
	 @param grow	The grow value, this is evaluated on each evaulation and
					is multiplied by the previous value to produce the result. */
	GeomValue(Value const& start, Value const& grow) throw();
	GeomValue(Value const& start, 
			  Value const& grow, 
			  const int numSteps, 
			  SequenceValueInternal::LoopDirection loopDirection = SequenceValueInternal::Normal) throw();
	GeomValue(Value const& start, 
			  Value const& grow, 
			  Value const& threshold, 
			  SequenceValueInternal::LoopDirection loopDirection = SequenceValueInternal::Normal) throw();
	
private:
	GeomValue();
};
#define G GeomValue


/** Store and manipulate an array of Value objects.
 Note that if you have the same value in an array and evaluate each copy in the array it may
 return a different numerical value each time (if it is a RandomValue, SeriesValue or GeomValue
 for example). 
 @ingroup Values */
class ValueArray
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	ValueArray() throw();
	explicit ValueArray(const int size) throw();
	ValueArray(Value const& value) throw();
	ValueArray(const double value) throw();
	ValueArray(Value const& value, const int size) throw();
	ValueArray(ValueArray const& copy) throw();
	ValueArray(Buffer const& copy) throw();
	ValueArray(ValueArray const& array0, ValueArray const& array1) throw();
	~ValueArray() throw();
	
	ValueArray& operator= (ValueArray const& other) throw();
	
	/// @} <!-- end Construction and destruction ---------------------- -->
	
	/** @internal */
	class ValueArrayInternal : public SmartPointer
	{
	public:
		ValueArrayInternal(const int size = 1) throw();
		~ValueArrayInternal() throw();
		
		friend class ValueArray;
		
	private:
		int size_;
		Value* array;
		
		ValueArrayInternal (const ValueArrayInternal&);
		const ValueArrayInternal& operator= (const ValueArrayInternal&);
	};
	
	/// @name Array manipulation
	/// @{
	
	/** Appends/concatenates this array with another, returning the result. 
	 @param other	The other array.
	 @return		The two arrays concatenated. */
	ValueArray operator<< (ValueArray const& other) throw();	
	
	/** Appends/concatenates a Value to the end of this array, returning the result. 
	 @param other	The value.
	 @return		The two arrays concatenated. */
	ValueArray operator<< (Value const& other) throw();
	
	/** Appends/concatenates this array with another, returning the result. 
	 @param other	The other array.
	 @return		The two arrays concatenated. */
	ValueArray operator, (ValueArray const& other) throw();	
	
	/** Appends/concatenates a Value to the end of this array, returning the result. 
	 @param other	The value.
	 @return		The two arrays concatenated. */
	ValueArray operator, (Value const& other) throw();	
	ValueArray& operator<<= (ValueArray const& other) throw();
	
	/** Get the number of values in the array.
	 @return The number of values (which may be zero). */
	inline int size() const throw() { return internal == 0 ? 0 : internal->size_; }
	
	/** Replace a value at a specified index.
	 @param index	This must be bertween 0 and size-1.
	 @param item	The Value to place at the specified index. */
	void put(const int index, Value const& item) throw();
	
	/** Get a value at a specified index.
	 @param index	This must be bertween 0 and size-1
	 @return		A reference to the value at the index or a reference to a null value
					if the index is out of range (or if the array size is 0). 
	 @see at */
	Value& operator[] (const int index) const throw();
	
	/** Get a value at a specified index.
	 @param index	This must be bertween 0 and size-1
	 @return		A reference to the value at the index or a reference to a null value
					if the index is out of range (or if the array size is 0). 
	 @see ValueArray::operator[] */
	Value& at(const int index) const throw();
	
	/** Get a value at a specified index.
	 @param index	This will be wrapped into the range 0 and size-1
	 @return		A reference to the value at the wrapped index or a reference to a 
					null value if the array size is 0. 
	 @see at, ValueArray::operator[] */
	Value& wrapAt(const int index) const throw();
	
	/** Get the first value in the array.
	 @return A reference to the first value or a null value if the array size is 0. */
	Value& first() const throw();
	
	/** Get the last value in the array.
	 @return A reference to the last value or a null value if the array size is 0. */
	Value& last() const throw();
	
	/// @} <!-- Array manipulation ------------------------------- -->
	
	/** @name Unary Ops.
	 
	 The definition of most these is done by macros in ugen_UnaryOpUGens.cpp except unary operator- */
	/// @{
	
	/** Unary negation. */
	ValueArray operator- () const throw();
	UnaryOpMethodsDeclare(ValueArray);
	
	/// @} <!-- end Unary Ops ------------------------------ -->
	
	/** @name Binary Ops. 
	 
	 The definition of these (except the assignment versions) is done by macros in ugen_BinaryOpUGens.cpp */
	/// @{
	BinaryOpMethodsDeclare(ValueArray);
	
	/** Assignment and addition.
	 Creates a ValueArray which the sum of this ValueArray and another ValueArray on evaluation. */
	ValueArray& operator+= (ValueArray const& other) throw();
	
	/** Assignment and subtraction.
	 Creates a ValueArray which the difference of this ValueArray and another ValueArray on evaluation. */
	ValueArray& operator-= (ValueArray const& other) throw();
	
	/** Assignment and multiplication.
	 Creates a ValueArray which the product of this ValueArray and another ValueArray on evaluation. */
	ValueArray& operator*= (ValueArray const& other) throw();
	
	/** Assignment and division.
	 Creates a ValueArray which the equal to this ValueArray "over" another ValueArray on evaluation. */
	ValueArray& operator/= (ValueArray const& other) throw();
	
	/// @} <!-- end Binary Ops --------------------------------------------- -->
	
private:
	ValueArrayInternal* internal;
};
#define A(a) ugen::ValueArray(a)


/** A UGenInternal which uses a Value as its source. 
 The Value is evaluated for each output sample.
 @see ValueUGen, ValueUGenInternalK 
 @ingroup UGenInternals Values 
 */
class ValueUGenInternal : public UGenInternal
{	
public:
	ValueUGenInternal(Value const& value);
	UGenInternal* getKr() throw();	
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	void setValue(Value const& other) throw();
	
	bool isValueUGenInternal() const throw() { return true; }

	
protected:
	Value valueObject;
};

/** A UGenInternal which uses a Value as its source. 
 The Value is evaluated at control rate and then interpolated. 
 (Perhaps interpolation should be an option?) 
 @see ValueUGenInternal, ValueUGen 
 @ingroup UGenInternals Values 
 */
UGenInternalControlRateDeclaration(ValueUGenInternal, 
								   (value), 
								   (Value const& value));

#define ValueUGen_Docs	 @param values	The array of Value objects to use as the source. The number of UGen	\
										channels is goverened by the size of the array.


/** A UGen which uses a ValueArray as its source. 
 The Value instance(s) will be evaluated each sample or control period depending on whether
 the UGen is audio rate or control rate respectively. 
 @ingroup AllUGens ControlUGens MathsUGens Values 
 @see ValueUGenInternal, ValueUGenInternalK */
UGenSublcassDeclaration(ValueUGen, (values), (ValueArray const& values), COMMON_UGEN_DOCS ValueUGen_Docs);

/** @ingroup Values */
template<class OwnerType = void>
class ValueEventBase : public EventBase<OwnerType>
{
public:
	ValueEventBase(OwnerType* o) : EventBase<OwnerType> (o) { }
	virtual double getValue() = 0;
};

/** @ingroup Values */
template<>
class ValueEventBase<void> : public EventBase<void>
{
public:
	virtual double getValue() = 0;
};

/** @ingroup Values */
template<class ValueEventType, class OwnerType = void>
class FunctionValueInternal : public ValueInternal
{
public:
	FunctionValueInternal(OwnerType* o) throw() : function(o) { }
	FunctionValueInternal(ValueEventType const& f) throw() : function(f) { }
	double getValue() throw() { return function.getValue(); }

private:
	ValueEventType function;
};

/** @ingroup Values */
template<class ValueEventType>
class FunctionValueInternal<ValueEventType, void> : public ValueInternal
{
public:
	FunctionValueInternal(void* o) throw() { }
	FunctionValueInternal(ValueEventType const& f) throw() : function(f) { }
	double getValue() throw() { return function.getValue(); }

private:
	ValueEventType function;
};

/** @ingroup Values */
template<class ValueEventType, class OwnerType = void>
class FunctionValue : public Value
{
public:
	FunctionValue(OwnerType* o = 0) throw()
	:	Value(new FunctionValueInternal<ValueEventType,OwnerType>(o)) { }

	FunctionValue(ValueEventType const& f) throw()
	:	Value(new FunctionValueInternal<ValueEventType,OwnerType>(f)) { }
};



#endif // _UGEN_ugen_Value_H_
