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

#ifndef UGEN_INLINEUNARYOPS_H
#define UGEN_INLINEUNARYOPS_H

//#include "../core/ugen_StandardHeader.h"
#include "../core/ugen_Constants.h"

/** A macro for adding the declarations of a standard set of unary operations to a class. 
 @param CLASSNAME	This must be the name of the class within which this decaration is made.
 @see UGen, UGenArray, Value, ValueArray */
#define UnaryOpMethodsDeclare(CLASSNAME)																						\
	/** Unary negation. @return A new CLASSNAME which is the negative of this one.	*/											\
	CLASSNAME neg() const throw();																								\
	/** Unary absolute. @return A new CLASSNAME which is the absolute value of this one i.e., @f$ |x| @f$. */					\
	CLASSNAME abs() const throw();																								\
	/** Unary reciprocal. @return A new CLASSNAME which is the reciprocal of this one i.e., @f$ \frac{1}{x} @f$. */				\
	CLASSNAME reciprocal() const throw();																						\
	/** Unary sine. @return A new CLASSNAME which is the sine of this one i.e., @f$ sin(x) @f$.	*/								\
	CLASSNAME sin() const throw();																								\
	/** Unary cosine. @return A new CLASSNAME which is the cosine of this one i.e., @f$ cos(x) @f$.	*/							\
	CLASSNAME cos() const throw();																								\
	/** Unary tangent. @return A new CLASSNAME which is the tangent of this one i.e., @f$ tan(x) @f$.	*/						\
	CLASSNAME tan() const throw();																								\
	/** Unary hyperbolic sine.	@return A new CLASSNAME which is: @f$ sinh(x) @f$.	*/											\
	CLASSNAME sinh() const throw();																								\
	/** Unary hyperbolic cosine.  @return A new CLASSNAME which is: @f$ cosh(x) @f$.  */										\
	CLASSNAME cosh() const throw();																								\
	/** Unary hyperbolic tangent.  @return A new CLASSNAME which is: @f$ tanh(x) @f$.	*/										\
	CLASSNAME tanh() const throw();																								\
	/** Unary arcsine. @return A new CLASSNAME which is the arcsine of this one i.e., @f$ asin(x) @f$.	*/						\
	CLASSNAME asin() const throw();																								\
	/** Unary arccosine. @return A new CLASSNAME which is the arccosine of this one i.e., @f$ acos(x) @f$.	*/					\
	CLASSNAME acos() const throw();																								\
	/** Unary arctangent. @return A new CLASSNAME which is the arctangent of this one i.e., @f$ atan(x) @f$. */					\
	CLASSNAME atan() const throw();																								\
	/** Unary square root. @return A new CLASSNAME which is the square root of this one i.e., @f$ \sqrt{x} @f$.	*/				\
	CLASSNAME sqrt() const throw();																								\
	/** Unary natural logarithm. @return A new CLASSNAME which is the natural logarithm i.e., @f$ ln x @f$. */					\
	CLASSNAME log() const throw();																								\
	/** Unary base-2 logarithm. @return A new CLASSNAME which is the base-2 logarithm i.e., @f$ \log_2 x @f$. */				\
	CLASSNAME log2() const throw();																								\
	/** Unary base-10 logarithm. @return A new CLASSNAME which is the base-10 logarithm i.e., @f$ \log_{10} x @f$. */			\
	CLASSNAME log10() const throw();																							\
	/** Unary exponential. @return A new CLASSNAME which is the exponential of this one i.e. @f$ \exp \left( x \right) @f$ */	\
	CLASSNAME exp() const throw();																								\
	/** Unary MIDI note number to frequency. 
		@return		A new CLASSNAME which is the conversion of MIDI note number to cycles per second in Hertz.		
					MIDI note 69 is equal to 440.0Hz.  This uses the equation:		\n 
					@f$ f = 440 \times 2^{ \left( \frac{m-69}{12} \right) } @f$		\n
					where @f$ f @f$ is freqeuncy and @f$ m @f$ is MIDI note number.	
					@see cpsmidi */																								\
	CLASSNAME midicps() const throw();																							\
	/** Unary frequency to MIDI note number. 
		@return		A new CLASSNAME which is the conversion of cycles per second in Hertz to MIDI note number.		
					MIDI note 69 is equal to 440.0Hz. This uses the equation:					\n
					@f$ m = 12 \cdot \log_2 \left( \frac{f}{440} \right) + 69 @f$				\n
					where @f$ f @f$ is freqeuncy and @f$ m @f$ is MIDI note number. 
					@see midicps */																								\
	CLASSNAME cpsmidi() const throw();																							\
	CLASSNAME squared() const throw();	\
	CLASSNAME cubed() const throw();	\
	/** Unary distort. 
		@return		A new CLASSNAME which applies @f$ \frac{x}{1+|x|} @f$.			*/											\
	CLASSNAME distort() const throw();																							\
	CLASSNAME besselI0() const throw();																							\
	CLASSNAME ceil() const throw();																								\
	CLASSNAME floor() const throw();																							\
	CLASSNAME frac() const throw();																								\
	CLASSNAME sign() const throw();																								\
	CLASSNAME zap() const throw();																								\



#ifdef _WIN32
inline float log2 (const float a) throw()			{	return (float)(::log(a) * oneOverLog2);		}
inline double log2 (const double a) throw()			{	return ::log(a) * oneOverLog2;				}
#else
inline float log2 (const float a) throw()			{	return (float)::log2(a);					}
inline double log2 (const double a) throw()			{	return ::log2(a);							}
#endif

inline float neg (const float a) throw()			{	return -a;									}
inline double neg (const double a) throw()			{	return -a;									}
inline float abs (const float a) throw()			{	return fabs(a);								}
inline double abs (const double a) throw()			{	return ::abs(a);							}
inline float reciprocal (const float a) throw()		{	return 1.f	/ a;							}
inline double reciprocal (const double a) throw()	{	return 1.0	/ a;							}
inline float sin (const float a) throw()			{	return (float)::sin(a);						}
//inline double sin (const double a) throw()			{	return ::sin(a);							}
inline float cos (const float a) throw()			{	return (float)::cos(a);						}
//inline double cos (const double a) throw()			{	return ::cos(a);							}
inline float tan (const float a) throw()			{	return (float)::tan(a);						}
//inline double tan (const double a) throw()			{	return ::tan(a);							}
inline float asin (const float a) throw()			{	return (float)::asin(a);					}
//inline double asin (const double a) throw()			{	return ::asin(a);							}
inline float acos (const float a) throw()			{	return (float)::acos(a);					}
//inline double acos (const double a) throw()			{	return ::acos(a);							}
inline float atan (const float a) throw()			{	return (float)::atan(a);					}
//inline double atan (const double a) throw()			{	return ::atan(a);							}
inline float sinh (const float a) throw()			{	return (float)::sinh(a);						}
inline float cosh (const float a) throw()			{	return (float)::cosh(a);						}
inline float tanh (const float a) throw()			{	return (float)::tanh(a);						}
inline float sqrt (const float a) throw()			{	return (float)::sqrt(a);					}
//inline double sqrt (const double a) throw()			{	return ::sqrt(a);							}
inline float log (const float a) throw()			{	return (float)::log(a);						}
//inline double log (const double a) throw()			{	return ::log(a);							}
inline float log10 (const float a) throw()			{	return (float)::log10(a);					}
//inline double log10 (const double a) throw()		{	return ::log10(a);							}
inline float exp (const float a) throw()			{	return (float)::exp(a);						}
//inline double exp (const double a) throw()			{	return ::exp(a);							}

inline float squared(const float a) throw()			{	return a*a;													}
inline double squared(const double a) throw()		{	return a*a;													}
inline float cubed(const float a) throw()			{	return a*a*a;												}
inline double cubed(const double a) throw()			{	return a*a*a;												}

// from music-dsp list
inline float zap(const float x) throw()
{
	float absx = std::abs(x);
	return (absx > (float)1e-15 && absx < (float)1e15) ? x : 0.f;
}

inline double midicps(const int a) throw()			{	return 440.0 * ::pow(2.0, (a - 69.0) * oneOver12);				}
inline float midicps(const float a) throw()			{	return (float)(440.0 * ::pow(2.0, (a - 69.0) * oneOver12));		}
inline double midicps(const double a) throw()		{	return 440.0 * ::pow(2.0, (a - 69.0) * oneOver12);				}
inline double cpsmidi(const int a) throw()			{	return log2(a * oneOver440) * 12.0 + 69.0;						}
inline float cpsmidi(const float a) throw()			{	return (float)(log2(a * oneOver440) * 12.0 + 69.0);				}
inline double cpsmidi(const double a) throw()		{	return log2(a * oneOver440) * 12.0 + 69.0;						}


#ifndef UGEN_NOEXTGPL
inline float distort(const float a) throw()			{	return a / (1.f + fabs(a));									}
inline double distort(const double a) throw()		{	return a / (1.0 + ::abs(a));								}

inline double besselI0(double p) throw()
{
	static const int ITERLIMIT=15;
	static const double CONVERGE=1.0E-8;
	
	p = p/2;
	double n = 1, t = 1, d = 1;
	int k = 1;
	double v;
	
	do {
		n = n * p;           d = d * k;
		v = n / d;           t = t + v * v;
	} while ((++k < ITERLIMIT) && (v > CONVERGE));
	
	return t;
}

//inline double besselI0(double x) throw()
//{
//	// Compute the modified Bessel function I_0(x) for any real x.
//	//
//	//--- NvE 12-mar-2000 UU-SAP Utrecht
//
//	// Parameters of the polynomial approximation
//	const double	p1=1.0,          p2=3.5156229,    p3=3.0899424,
//					p4=1.2067492,    p5=0.2659732,    p6=3.60768e-2,  p7=4.5813e-3;
//
//	const double	q1= 0.39894228,  q2= 1.328592e-2, q3= 2.25319e-3,
//					q4=-1.57565e-3,  q5= 9.16281e-3,  q6=-2.057706e-2,
//					q7= 2.635537e-2, q8=-1.647633e-2, q9= 3.92377e-3;
//
//	const double k1 = 3.75;
//	double ax = abs(x);
//
//	double y=0, result=0;
//
//	if (ax < k1) {
//		double xx = x/k1;
//		y = xx*xx;
//		result = p1+y*(p2+y*(p3+y*(p4+y*(p5+y*(p6+y*p7)))));
//	} else {
//		y = k1/ax;
//		result = (exp(ax)/sqrt(ax))*(q1+y*(q2+y*(q3+y*(q4+y*(q5+y*(q6+y*(q7+y*(q8+y*q9))))))));
//	}
//	
//	return result;
//}

inline float besselI0(float p) throw()
{
	return (float)besselI0((double)p);
}


//inline double zap(double x) throw()
//{
//	double absx = std::abs(x);
//	return (absx > 1e-15 && absx < 1e15) ? x : 0.;
//}

inline int roundFloatToInt (const float value) throw()
{
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;
	
    return n.asInt [0];
}
#endif // gpl

inline double ceil(double a) throw()	{ int n = (int)a; return (double)(n+1); }
inline float ceil(float a) throw()		{ int n = (int)a; return (float)(n+1); }
inline double floor(double a) throw()	{ int n = (int)a; return (double)n; }
inline float floor(float a) throw()		{ int n = (int)a; return (float)n; }
inline double frac(double a) throw()	{ int n = (int)a; return a - (double)n; }
inline float frac(float a) throw()		{ int n = (int)a; return a - (float)n; }
inline double sign(double a) throw()	{ int n = (a == 0.0) ? 0 : (a < 0.0) ? -1 : 1; return (double)n; }
inline float sign(float a) throw()		{ int n = (a == 0.f) ? 0 : (a < 0.f) ? -1 : 1; return (float)n; }


#endif // UGEN_INLINEUNARYOPS_H