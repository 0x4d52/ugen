// $Id: ugen_Random.cpp 979 2010-01-09 15:47:39Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/core/ugen_Random.cpp $

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

#include "ugen_Random.h"


//Ran088::Ran088(unsigned long seed)
//{	
//	// humans tend to use small seeds - mess up the bits
//	//seed = (unsigned long)Hash((int)seed);
//	
//	// initialize seeds using the given seed value taking care of
//	// the requirements. The constants below are arbitrary otherwise
//	s1 = 1243598713U ^ seed; if (s1 <  2) s1 = 1243598713U;
//	s2 = 3093459404U ^ seed; if (s2 <  8) s2 = 3093459404U;
//	s3 = 1821928721U ^ seed; if (s3 < 16) s3 = 1821928721U;	
//}

// Ran088::Ran088(Ran088 const& copy)
//:	s1(copy.s1), s2(copy.s2), s3(copy.s3)
//{
//}
//
// Ran088& Ran088::operator= (Ran088 const& other)
//{
//	if (this != &other)
//    {	
//		s1 = other.s1;
//		s2 = other.s2;
//		s3 = other.s3;
//    }
//	
//    return *this;	
//}

//void Ran088::get(unsigned long& s1, unsigned long& s2, unsigned long& s3)
//{
//	s1 = this->s1;
//	s2 = this->s2;
//	s3 = this->s3;
//}
//
//void Ran088::set(unsigned long s1, unsigned long s2, unsigned long s3)
//{
//	this->s1 = s1;
//	this->s2 = s2;
//	this->s3 = s3;
//}
//
//unsigned long Ran088::next()
//{
//	// generate a random 32 bit number
//	s1 = ((s1 &  -2) << 12) ^ (((s1 << 13) ^  s1) >> 19);
//	s2 = ((s2 &  -8) <<  4) ^ (((s2 <<  2) ^  s2) >> 25);
//	s3 = ((s3 & -16) << 17) ^ (((s3 <<  3) ^  s3) >> 11);
//	return s1 ^ s2 ^ s3;
//}
//
//double Ran088::nextDouble()
//{
//	// return a double from 0.0 to 0.999...
//#if BYTE_ORDER == BIG_ENDIAN
//	union { struct { unsigned long hi, lo; } i; double f; } du;
//#else
//	union { struct { unsigned long lo, hi; } i; double f; } du;
//#endif
//	du.i.hi = 0x41300000; 
//	du.i.lo = next();
//	return du.f - 1048576.;
//}
//
//float Ran088::nextFloat()
//{
//	// return a float from 0.0 to 0.999...
//	union { unsigned long i; float f; } u;		// union for floating point conversion of result
//	u.i = 0x3F800000 | (next() >> 9);
//	return u.f - 1.f;
//}
//
//float Ran088::nextFloat0()
//{
//	// return a float from +1.0 to +1.999...
//	union { unsigned long i; float f; } u;		// union for floating point conversion of result
//	u.i = 0x3F800000 | (next() >> 9);
//	return u.f;
//}
//
//float Ran088::nextBiFloat()
//{
//	// return a float from -1.0 to +0.999...
//	union { unsigned long i; float f; } u;		// union for floating point conversion of result
//	u.i = 0x40000000 | (next() >> 9);
//	return u.f - 3.f;
//}
//
//float Ran088::nextFloat8()
//{
//	// return a float from -0.125 to +0.124999...
//	union { unsigned long i; float f; } u;		// union for floating point conversion of result
//	u.i = 0x3E800000 | (next() >> 9);
//	return u.f - 0.375f;
//}
//
//float Ran088::coin()
//{
//	// only return one of the two values -1.0 or +1.0
//	union { unsigned long i; float f; } u;		// union for floating point conversion of result
//	u.i = 0x3F800000 | (0x80000000 & next());
//	return u.f;
//}
//
//long Ran088::nextLong(long scale)
//{
//	// return an int from 0 to scale - 1
//	return (long)std::floor(scale * nextDouble());
//}
//
//long Ran088::nextBiLong(long scale)
//{
//	// return a int from -scale to +scale
//	return (long)std::floor((2. * scale + 1.) * nextDouble() - scale);
//}
//
//long Ran088::nextLinearLong(long scale)
//{
//	long a = nextLong(scale);
//	long b = nextLong(scale);
//	return min(a,b);
//}
//
//double Ran088::nextLinearDouble(double scale)
//{
//	double a = nextDouble();
//	double b = nextDouble();
//	return min(a,b) * scale;
//}
//
//long Ran088::nextBiLinearLong(long scale)
//{
//	long a = nextLong(scale);
//	long b = nextLong(scale);
//	return a - b;
//}
//
//double Ran088::nextBiLinearDouble(double scale)
//{
//	double a = nextDouble();
//	double b = nextDouble();
//	return (a - b) * scale;
//}
//
//double Ran088::nextExpRandRange(double lo, double hi)
//{
//	return lo * std::exp(std::log(hi / lo) * nextDouble());
//}
//
//double Ran088::nextExpRand(double scale)
//{
//	double z;
//	while ((z = nextDouble()) == 0.0) {}
//	return -std::log(z) * scale;
//}
//
//// double Ran088::nextBiExpRand(double scale)
////{
////	double z;
////	while ((z = drand2(1.)) == 0.0 || z == -1.0) {}
////	if (z > 0.0) z = std::log(z);
////	else z = -std::log(-z);
////	return z * scale;
////}
//
//double Ran088::nextGaussianApprox(double scale)
//{
//	// larry polansky's poor man's gaussian generator
//	return (nextDouble() + nextDouble() + nextDouble() - 1.5) * 0.666666667 * scale;
//}
//
//unsigned long Ran088::next( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	s1 = ((s1 &  -2) << 12) ^ (((s1 << 13) ^  s1) >> 19);
//	s2 = ((s2 &  -8) <<  4) ^ (((s2 <<  2) ^  s2) >> 25);
//	s3 = ((s3 & -16) << 17) ^ (((s3 <<  3) ^  s3) >> 11);
//	return s1 ^ s2 ^ s3;
//}
//
//double Ran088::nextDouble( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	union { struct { unsigned long hi, lo; } i; double f; } u;
//	u.i.hi = 0x41300000; 
//	u.i.lo = next(s1,s2,s3);
//	return u.f - 1048576.;
//}
//
//float Ran088::nextFloat( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	// return a float from 0.0 to 0.999...
//	union { unsigned long i; float f; } u;
//	u.i = 0x3F800000 | (next(s1,s2,s3) >> 9);
//	return u.f - 1.f;
//}
//
//float Ran088::nextFloat0( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	// return a float from +1.0 to +1.999...
//	union { unsigned long i; float f; } u;
//	u.i = 0x3F800000 | (next(s1,s2,s3) >> 9);
//	return u.f;
//}
//
//float Ran088::nextBiFloat( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	// return a float from -1.0 to +0.999...
//	union { unsigned long i; float f; } u;
//	u.i = 0x40000000 | (next(s1,s2,s3) >> 9);
//	return u.f - 3.f;
//}
//
//float Ran088::nextFloat8( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	// return a float from -0.125 to +0.124999...
//	union { unsigned long i; float f; } u;
//	u.i = 0x3E800000 | (next(s1,s2,s3) >> 9);
//	return u.f - 0.375f;
//}
//
//float Ran088::coin( unsigned long& s1, unsigned long& s2, unsigned long& s3 )
//{
//	// only return one of the two values -1.0 or +1.0
//	union { unsigned long i; float f; } u;
//	u.i = 0x3F800000 | (0x80000000 & next(s1,s2,s3));
//	return u.f;
//}



END_UGEN_NAMESPACE
