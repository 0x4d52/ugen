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


#ifndef UGEN_INLINEBINARYOPS_H
#define UGEN_INLINEBINARYOPS_H

#include <math.h>
#include "../core/ugen_Constants.h"

typedef float (*BinaryOpFunction)(float,float);

#define BinaryOpMethodsDeclare(CLASSNAME)													\
	/** Binary addition. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left + right. 
		@see ugen::operator+(const float leftOperand, UGen const& rightOperand)*/		\
	CLASSNAME operator+ (CLASSNAME const& rightOperand) const throw();						\
	/** Binary subtraction. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left - right. 
		@see ugen::operator-(const float leftOperand, UGen const& rightOperand)*/		\
	CLASSNAME operator- (CLASSNAME const& rightOperand) const throw();						\
	/** Binary multiplication. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left * right. 
		@see ugen::operator*(const float leftOperand, UGen const& rightOperand)*/		\
	CLASSNAME operator* (CLASSNAME const& rightOperand) const throw();						\
	/** Binary division. 
		This CLASSNAME is the left-hand-side of the operation. 
		@param	rightOperand	The right-hand-side of the operation.
								If this is a constant the operation will often reorganised
								as a multiplication for efficiency.
		@return					A new CLASSNAME which the result of left / right. 
		@see ugen::operator/(const float leftOperand, UGen const& rightOperand)*/		\
	CLASSNAME operator/ (CLASSNAME const& rightOperand) const throw();						\
	/** Ignore right operator.  Useful to forced execution without using its value. */		\
	CLASSNAME operator^ (CLASSNAME const& rightOperand) const throw();						\
	/** Binary equal to. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left == right. 
								Usually this means the result is 1 for true and 0 for 
								false.  Note that many uses of this operator are applied
								to float or double values and care should be taken when
								dealing with equality due to the precision of these data
								types. The == operator should be used only for checking 
								equality of objects.
		@see ugen::isEqualTo(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME isEqualTo (CLASSNAME const& rightOperand) const throw();						\
	/** Binary not equal to. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left != right. 
								Usually this means the result is 1 for true and 0 for 
								false.  Note that many uses of this operator are applied
								to float or double values and care should be taken when
								dealing with equality due to the precision of these data
								types. The != operator should be used only for checking 
								equality of objects.
		@see ugen::isNotEqualTo(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME isNotEqualTo (CLASSNAME const& rightOperand) const throw();						\
	/** Binary less-than. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left < right. 
								Usually this means the result is 1 for true and 0 for 
								false. 
		@see ugen::operator<(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME operator< (CLASSNAME const& rightOperand) const throw();						\
	/** Binary greater-than. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left > right. 
								Usually this means the result is 1 for true and 0 for 
								false. 
		@see ugen::operator>(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME operator> (CLASSNAME const& rightOperand) const throw();						\
	/** Binary less-than or equal to. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left <= right. 
								Usually this means the result is 1 for true and 0 for 
								false. Note that many uses of this operator are applied
								to float or double values and care should be taken when
								dealing with equality due to the precision of these data
								types. 
		@see ugen::operator<=(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME operator<= (CLASSNAME const& rightOperand) const throw();						\
	/** Binary greater-than or equal to. 
		This CLASSNAME is the left-hand-side of the operation.
		@param	rightOperand	The right-hand-side of the operation.
		@return					A new CLASSNAME which the result of left >= right. 
								Usually this means the result is 1 for true and 0 for 
								false. Note that many uses of this operator are applied
								to float or double values and care should be taken when
								dealing with equality due to the precision of these data
								types. 
		@see ugen::operator>=(const float leftOperand, UGen const& rightOperand)*/													\
	CLASSNAME operator>= (CLASSNAME const& rightOperand) const throw();						\
	/** Binary power. 
		This CLASSNAME is the left-hand-side (L) of the operation.
		@param	rightOperand	The right-hand-side (R) of the operation.
		@return					A new CLASSNAME which the result of 
								@f$ L^{R} @f$ 
		@see ugen::pow() */																	\
	CLASSNAME pow(CLASSNAME const& rightOperand) const throw();								\
	/** Binary hypotenuse. 
		This CLASSNAME is the left-hand-side (L) of the operation.
		@param	rightOperand	The right-hand-side (R) of the operation.
		@return					A new CLASSNAME which is the result of 
								@f$ \sqrt{L^{2}+R^{2}} @f$ 
		@see ugen::hypot() */																\
	CLASSNAME hypot(CLASSNAME const& rightOperand) const throw();							\
	/** Binary acrtangent using atan2.
		This CLASSNAME is the x in of the atan2 operation.
		@param	rightOperand	The is y of the atan2 operation.
		@return					A new CLASSNAME which the result of @f$ atan2(x,y) @f$. 
		@see ugen::atan2	*/																\
	CLASSNAME atan2(CLASSNAME const& rightOperand) const throw();							\
	/** Binary minimum. 
		This CLASSNAME is the left-hand-side (L) of the operation.
		@param	rightOperand	The right-hand-side (R) of the operation.
		@return					A new CLASSNAME which the result of @f$ min(L,R) @f$. 
		@see ugen::min() */																	\
	CLASSNAME min(CLASSNAME const& rightOperand) const throw();								\
	/** Binary maximum. 
		This CLASSNAME is the left-hand-side (L) of the operation.
		@param	rightOperand	The right-hand-side (R) of the operation.
		@return					A new CLASSNAME which the result of @f$ max(L,R) @f$. 
		@see ugen::max() */																	\
	CLASSNAME max(CLASSNAME const& rightOperand) const throw();								\
	/** TBA */																				\
	CLASSNAME clip2(CLASSNAME const& rightOperand) const throw();							\
	/** TBA */																				\
	CLASSNAME wrap(CLASSNAME const& rightOperand) const throw();							\
	/** TBA */																				\
	CLASSNAME wrap2(CLASSNAME const& rightOperand) const throw();							\
	/** TBA */																				\
	CLASSNAME fold(CLASSNAME const& rightOperand) const throw();							\
	/** TBA */																				\
	CLASSNAME fold2(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME ring1(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME ring2(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME ring3(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME ring4(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME round(CLASSNAME const& rightOperand = 1) const throw();							\
	CLASSNAME trunc(CLASSNAME const& rightOperand = 1) const throw();							\
	CLASSNAME sumsqr(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME difsqr(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME sqrsum(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME sqrdif(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME absdif(CLASSNAME const& rightOperand) const throw();							\
	CLASSNAME thresh(CLASSNAME const& rightOperand) const throw();							\





inline float isEqualTo(float a, float b) throw()				{ return (a==b) ? 1.f : 0.f; }
inline float isNotEqualTo(float a, float b) throw()				{ return (a!=b) ? 1.f : 0.f; }
inline double isEqualTo(double a, double b) throw()				{ return (a==b) ? 1.0 : 0.0; }
inline double isNotEqualTo(double a, double b) throw()			{ return (a!=b) ? 1.0 : 0.0; }

#ifdef _WIN32
inline double hypot (float a, float b) throw()				{ return (float) ::_hypot ((double)a, (double)b);	}
inline double hypot (double a, double b) throw()			{ return ::_hypot (a, b);							}
#else
inline double hypot (float a, float b) throw()				{ return (float) ::hypot (a, b);	}
inline double hypot (double a, double b) throw()			{ return ::hypot (a, b);			}
#endif

inline float pow (const float a, const float b)	throw()		{ return (float)::powf(a, b);		}
inline double pow (const double a, const double b) throw()	{ return ::pow(a, b);				}
inline float atan2 (const float a, const float b) throw()	{ return (float)::atan2(a, b);		}
inline float atan2 (const double a, const double b) throw()	{ return ::atan2(a, b);				}
inline float min (const float a, const float b) throw()		{ return (a > b) ? b : a;			}
inline float max (const float a, const float b) throw()		{ return (a < b) ? b : a;			}
inline double min (const double a, const double b) throw()	{ return (a > b) ? b : a;			}
inline double max (const double a, const double b) throw()	{ return (a < b) ? b : a;			}
inline int min (const int a, const int b) throw()			{ return (a > b) ? b : a;			}
inline int max (const int a, const int b) throw()			{ return (a < b) ? b : a;			}
inline long min (const long a, const long b) throw()		{ return (a > b) ? b : a;			}
inline long max (const long a, const long b) throw()		{ return (a < b) ? b : a;			}
inline unsigned int min (const unsigned int a, const unsigned int b) throw()			{ return (a > b) ? b : a;			}
inline unsigned int max (const unsigned int a, const unsigned int b) throw()			{ return (a < b) ? b : a;			}
inline unsigned long min (const unsigned long a, const unsigned long b) throw()			{ return (a > b) ? b : a;			}
inline unsigned long max (const unsigned long a, const unsigned long b) throw()			{ return (a < b) ? b : a;			}


inline double clip(const double value, const double lower, const double upper) throw() // actually ternery!
{
	double ret = value;
	ret = max(lower, ret);
	ret = min(ret, upper);
	return ret;
}

inline float clip(const float value, const float lower, const float upper) throw() // actually ternery!
{
	float ret = value;
	ret = max(lower, ret);
	ret = min(ret, upper);
	return ret;
}

inline int clip(const int value, const int lower, const int upper) throw() // actually ternery!
{
	int ret = value;
	ret = max(lower, ret);
	ret = min(ret, upper);
	return ret;
}

inline float clip2 (const float value, const float range) throw()		{ return clip(value, -range, range); }
inline double clip2 (const double value, const double range) throw()	{ return clip(value, -range, range); }

#ifndef UGEN_NOEXTGPL

inline int wrap(int in, int lo, int hi) throw()
{
	int range;
	if (in >= hi) 
	{
		range = hi - lo;
		in -= range;
		if (in < hi) return in;
	} 
	else if (in < lo) 
	{
		range = hi - lo;
		in += range;
		if (in >= lo) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	//return in - range * std::floor((in - lo)/range); 
	
	return in % range + lo;
}


#ifndef UGEN_ANDROID
inline float wrap(float in, float lo, float hi) throw()
{
	float range;
	if (in >= hi) 
	{
		range = hi - lo;
		in -= range;
		if (in < hi) return in;
	} 
	else if (in < lo) 
	{
		range = hi - lo;
		in += range;
		if (in >= lo) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	return in - range * std::floor((in - lo)/range); 
}
#else
// android
inline float wrap(float in, float lo, float hi) throw()
{
	float range;
	if (in >= hi) 
	{
		range = hi - lo;
		in -= range;
		if (in < hi) return in;
	} 
	else if (in < lo) 
	{
		range = hi - lo;
		in += range;
		if (in >= lo) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	return in - range * floorf((in - lo)/range); 
}
#endif

inline float wrap(float in, float hi) throw() { return wrap(in, 0.f, hi); }
inline float wrap2(float in, float hi) throw() { return wrap(in, -hi, hi); }

#ifndef UGEN_ANDROID
inline double wrap(double in, double lo, double hi) throw() 
{
	double range;
	if (in >= hi) 
	{
		range = hi - lo;
		in -= range;
		if (in < hi) return in;
	} 
	else if (in < lo) 
	{
		range = hi - lo;
		in += range;
		if (in >= lo) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	return in - range * std::floor((in - lo)/range); 
}
#else
// android
inline double wrap(double in, double lo, double hi) throw() 
{
	double range;
	if (in >= hi) 
	{
		range = hi - lo;
		in -= range;
		if (in < hi) return in;
	} 
	else if (in < lo) 
	{
		range = hi - lo;
		in += range;
		if (in >= lo) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	return in - range * floor((in - lo)/range); 
}
#endif

inline double wrap(double in, double hi) throw() { return wrap(in, 0.0, hi); }
inline double wrap2(double in, double hi) throw() { return wrap(in, -hi, hi); }

#ifndef UGEN_ANDROID
inline float fold(float in, float lo, float hi) throw() 
{
	float x;
	float c, range, range2;
	x = in - lo;
	
	if (in >= hi) 
	{
		in = hi + hi - in;
		if (in >= lo) return in;
	} 
	else if (in < lo) 
	{
		in = lo + lo - in;
		if (in < hi) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;

	range = hi - lo;
	range2 = range + range;
	c = x - range2 * std::floor(x / range2);
	if (c>=range) c = range2 - c;
	return c + lo;
}
#else
// android
inline float fold(float in, float lo, float hi) throw() 
{
	float x;
	float c, range, range2;
	x = in - lo;
	
	if (in >= hi) 
	{
		in = hi + hi - in;
		if (in >= lo) return in;
	} 
	else if (in < lo) 
	{
		in = lo + lo - in;
		if (in < hi) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	
	range = hi - lo;
	range2 = range + range;
	c = x - range2 * floorf(x / range2);
	if (c>=range) c = range2 - c;
	return c + lo;
}
#endif

inline float fold(float in, float hi) throw() { return fold(in, 0.f, hi); }
inline float fold2(float in, float hi) throw() { return fold(in, -hi, hi); }

#ifndef UGEN_ANDROID
inline double fold(double in, double lo, double hi) throw() 
{
	double x;
	double c, range, range2;
	x = in - lo;
	
	if (in >= hi) 
	{
		in = hi + hi - in;
		if (in >= lo) return in;
	} 
	else if (in < lo) 
	{
		in = lo + lo - in;
		if (in < hi) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	
	range = hi - lo;
	range2 = range + range;
	c = x - range2 * std::floor(x / range2);
	if (c>=range) c = range2 - c;
	return c + lo;
}
#else
// android
inline double fold(double in, double lo, double hi) throw() 
{
	double x;
	double c, range, range2;
	x = in - lo;
	
	if (in >= hi) 
	{
		in = hi + hi - in;
		if (in >= lo) return in;
	} 
	else if (in < lo) 
	{
		in = lo + lo - in;
		if (in < hi) return in;
	} 
	else return in;
	
	if (hi == lo) return lo;
	
	range = hi - lo;
	range2 = range + range;
	c = x - range2 * floor(x / range2);
	if (c>=range) c = range2 - c;
	return c + lo;
}
#endif

inline double fold(double in, double hi) throw() { return fold(in, 0.0, hi); }
inline double fold2(double in, double hi) throw() { return fold(in, -hi, hi); }

inline double ring1(double a, double b) throw() { return a * (b + 1.0); } 
inline double ring2(double a, double b) throw() { return a*b + a + b; } 
inline double ring3(double a, double b) throw() { return a*a*b; } 
inline double ring4(double a, double b) throw() { return a*a*b - a*b*b; } 

inline float ring1(float a, float b) throw() { return a * (b + 1.f); } 
inline float ring2(float a, float b) throw() { return a*b + a + b; } 
inline float ring3(float a, float b) throw() { return a*a*b; } 
inline float ring4(float a, float b) throw() { return a*a*b - a*b*b; } 

inline double round(double a, double b) throw()
{
	double offset = a < 0 ? -0.5 : 0.5;
	int n = (int)(a / b + offset);
	return b * (double)n;
}

inline float round(float a, float b) throw()
{
	float offset = a < 0 ? -0.5f : 0.5f;
	int n = (int)(a / b + offset);
	return b * (float)n;
}

inline double trunc(double a, double b) throw()
{
	int n = (int)(a / b);
	return b * (double)n;
}

inline float trunc(float a, float b) throw()
{
	int n = (int)(a / b);
	return b * (float)n;
}

inline double sumsqr(double a, double b) throw() { return a*a + b*b; }
inline double difsqr(double a, double b) throw() { return a*a - b*b; }
inline double sqrsum(double a, double b) throw() { double c = a+b; return c*c; }
inline double sqrdif(double a, double b) throw() { double c = a-b; return c*c; }
inline double absdif(double a, double b) throw() { return ::abs(a-b); }
inline double thresh(double a, double b) throw() { return (a < b) ? 0.0 : a; }

inline float sumsqr(float a, float b) throw() { return a*a + b*b; }
inline float difsqr(float a, float b) throw() { return a*a - b*b; }
inline float sqrsum(float a, float b) throw() { float c = a+b; return c*c; }
inline float sqrdif(float a, float b) throw() { float c = a-b; return c*c; }
inline float absdif(float a, float b) throw() { return fabs(a-b); }
inline float thresh(float a, float b) throw() { return (a < b) ? 0.f : a; }

#endif

inline float linlin(const float input, 
					const float inLow, const float inHigh,
					const float outLow, const float outHigh) throw()
{
	float inRange = inHigh - inLow;
	float outRange = outHigh - outLow;
	return (input - inLow) * outRange / inRange + outLow;
}

inline double linlin(const double input, 
					const double inLow, const double inHigh,
					const double outLow, const double outHigh) throw()
{
	double inRange = inHigh - inLow;
	double outRange = outHigh - outLow;
	return (input - inLow) * outRange / inRange + outLow;
}


inline float linlin2(const float input, 
					 const float inLow, const float inRange,
					 const float outLow, const float outRange) throw()
{
	return (input - inLow) * outRange / inRange + outLow;
}

#ifndef UGEN_NOEXTGPL

inline float linsin(const float input, 
					const float inLow, const float inHigh,
					const float outLow, const float outHigh) throw()
{
	float inRange = inHigh - inLow;
	float outRange = outHigh - outLow;
		
	float inPhase = (input - inLow) * pi / inRange + pi;
#ifndef UGEN_ANDROID
	float cosInPhase = (float)std::cos((double)inPhase) * 0.5f + 0.5f;
#else
	float cosInPhase = cosf(inPhase) * 0.5f + 0.5f;
#endif
	
	return cosInPhase * outRange + outLow;	
}

inline double linsin(const double input, 
					 const double inLow, const double inHigh,
					 const double outLow, const double outHigh) throw()
{
	double inRange = inHigh - inLow;
	double outRange = outHigh - outLow;
	
	double inPhase = (input - inLow) * pi / inRange + pi;
#ifndef UGEN_ANDROID
	double cosInPhase = std::cos(inPhase) * 0.5 + 0.5;
#else
	double cosInPhase = cos(inPhase) * 0.5 + 0.5;
#endif
	
	return cosInPhase * outRange + outLow;	
}


inline float linsin2(const float input, 
					 const float inLow, const float inRange,
					 const float outLow, const float outRange) throw()
{	
	float inPhase = (input - inLow) * pi / inRange + pi;
#ifndef UGEN_ANDROID
	float cosInPhase = (float)std::cos((double)inPhase) * 0.5f + 0.5f;
#else
	float cosInPhase = cosf(inPhase) * 0.5f + 0.5f;
#endif
	return cosInPhase * outRange + outLow;	
}


inline float linexp(const float input, 
					const float inLow, const float inHigh,
					const float outLow, const float outHigh) throw()
{
	float outRatio = outHigh / outLow;
	float reciprocalInRange = 1.f / (inHigh - inLow);
	float negInLowOverInRange = reciprocalInRange * -inLow;
	return outLow * pow(outRatio, input * reciprocalInRange + negInLowOverInRange);	
}

inline double linexp(const double input, 
					 const double inLow, const double inHigh,
					 const double outLow, const double outHigh) throw()
{
	double outRatio = outHigh / outLow;
	double reciprocalInRange = 1.0 / (inHigh - inLow);
	double negInLowOverInRange = reciprocalInRange * -inLow;
	return outLow * pow(outRatio, input * reciprocalInRange + negInLowOverInRange);	
}


inline float linexp2(const float input, 
					 const float reciprocalInRange, const float negInLowOverInRange,
					 const float outLow, const float outRatio) throw()
{
	return outLow * pow(outRatio, input * reciprocalInRange + negInLowOverInRange);	
}

inline float linwelch(const float input, 
					  const float inLow, const float inHigh,
					  const float outLow, const float outHigh) throw()
{
	float inRange = inHigh - inLow;
	float outRange = outHigh - outLow;
	float inPos = (input - inLow) / inRange;

#ifndef UGEN_ANDROID
	if (outLow < outHigh)
		return outLow + outRange * std::sin(piOverTwo * inPos);
	else
		return outHigh - outRange * std::sin(piOverTwo - piOverTwo * inPos);
#else
	if (outLow < outHigh)
		return outLow + outRange * sinf(piOverTwo * inPos);
	else
		return outHigh - outRange * sinf(piOverTwo - piOverTwo * inPos);	
#endif
}

inline double explin(double input, double inLow, double inHigh, double outLow, double outHigh)
{
    if (input <= inLow) return outLow;
    if (input >= inHigh) return outHigh;
    return log(input/inLow) / log(inHigh/inLow) * (outHigh-outLow) + outLow;
}

inline float explin(float input, float inLow, float inHigh, float outLow, float outHigh)
{
    if (input <= inLow) return outLow;
    if (input >= inHigh) return outHigh;
    return logf(input/inLow) / logf(inHigh/inLow) * (outHigh-outLow) + outLow;
}



#endif // gpl

#endif // UGEN_INLINEBINARYOPS_H

