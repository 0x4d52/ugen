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

#ifndef _UGEN_ugen_EnvCurve_H_
#define _UGEN_ugen_EnvCurve_H_


/** Specify curved sections of breakpoint envelopes.
 Curve type may be linear or from a selection of curve types, or a numerical
 value which specifies the degree of curvature.
 
 @see Env, EnvGen */
class EnvCurve
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	enum CurveType
	{
		Empty,
		Numerical,
		Step,
		Linear,
		Exponential,
		Sine,
		Welch
	};
		
	EnvCurve(float curve) throw()				: type_(Numerical), curve_(curve)			{ }
	//EnvCurve(double curve) throw()				: type_(Numerical), curve_((float)curve)	{ }
	EnvCurve(CurveType type = Empty) throw()	: type_(type), curve_(0.f)					{ }
	
	/// @} <!-- end Construction and destruction ------------------------------------------- -->
	
	/// @name Curve access
	/// @{
	
	CurveType getType() const throw()				{ return type_;	 }
	float getCurve() const throw()					{ return curve_; }
	void setType(const CurveType newType) throw()	{ type_ = newType;	 }
	void setCurve(const float newCurve) throw()		{ curve_ = newCurve; }

	bool equalsInfinity() const throw()	{ return type_ == Numerical && curve_ == INFINITY; }
    
    bool operator==(EnvCurve const& other) const
    {
        if (type_ != other.type_)
            return false;
        
        if (type_ == Numerical)
            return curve_ == other.curve_;
        
        return true;
    }
    
    bool operator!=(EnvCurve const& other) const { return !(*this==other); }

	
	/// @} <!-- end Curve access ----------------------------------------------------------- -->
	
private:
	CurveType type_;
	float curve_;	
};

/** An array of EnvCurve objects.
 
 @see EnvCurve, Env, EnvGen */
class EnvCurveList
{
public:
	
	/// @name Construction and destruction
	/// @{
	
	EnvCurveList(EnvCurve const& i00) throw();
	EnvCurveList(EnvCurve const& i00,
				 EnvCurve const& i01, 
				 EnvCurve const& i02 = INFINITY, 
				 EnvCurve const& i03 = INFINITY, 
				 EnvCurve const& i04 = INFINITY, 
				 EnvCurve const& i05 = INFINITY, 
				 EnvCurve const& i06 = INFINITY, 
				 EnvCurve const& i07 = INFINITY, 
				 EnvCurve const& i08 = INFINITY, 
				 EnvCurve const& i09 = INFINITY, 
				 EnvCurve const& i10 = INFINITY, 
				 EnvCurve const& i11 = INFINITY, 
				 EnvCurve const& i12 = INFINITY, 
				 EnvCurve const& i13 = INFINITY, 
				 EnvCurve const& i14 = INFINITY, 
				 EnvCurve const& i15 = INFINITY, 
				 EnvCurve const& i16 = INFINITY, 
				 EnvCurve const& i17 = INFINITY, 
				 EnvCurve const& i18 = INFINITY, 
				 EnvCurve const& i19 = INFINITY, 
				 EnvCurve const& i20 = INFINITY, 
				 EnvCurve const& i21 = INFINITY, 
				 EnvCurve const& i22 = INFINITY, 
				 EnvCurve const& i23 = INFINITY, 
				 EnvCurve const& i24 = INFINITY, 
				 EnvCurve const& i25 = INFINITY, 
				 EnvCurve const& i26 = INFINITY, 
				 EnvCurve const& i27 = INFINITY, 
				 EnvCurve const& i28 = INFINITY, 
				 EnvCurve const& i29 = INFINITY, 
				 EnvCurve const& i30 = INFINITY, 
				 EnvCurve const& i31 = INFINITY) throw();
	
	EnvCurveList(EnvCurve::CurveType type, const int size = 1) throw();
	EnvCurveList(const double curve, const int size = 1) throw();
	
	~EnvCurveList() throw();
	
	EnvCurveList(EnvCurveList const& copy) throw();
	EnvCurveList& operator= (EnvCurveList const& other) throw();
	
	
	/// @} <!-- end Construction and destruction ------------------------------------------- -->
	
	/// @internal
	class EnvCurveListInternal : public SmartPointer
	{
	public:
		EnvCurveListInternal(const int initSize) throw();
		~EnvCurveListInternal();
		
		friend class EnvCurveList;
	private:
		const int size;
		EnvCurve* data;
	};
	
	/// @name Array access and manipulation
	/// @{
	
	EnvCurve& operator[] (const int index) throw();
	const EnvCurve& operator[] (const int index) const throw();
	
	inline int size() const throw()									{ return internal->size;		}
	inline const EnvCurve* const getData() throw()					{ return internal->data;		}
	inline const EnvCurve* const getData() const throw()			{ return internal->data;		}
	//inline const EnvCurve* const getDataReadOnly() const throw()	{ return internal->data;		}
	
	EnvCurveList blend(EnvCurveList const& other, float fraction) const throw();
	
	/// @} <!-- end Array access and manipulation ----------------------------------------------- -->
	
private:
	EnvCurveListInternal* internal;
	static EnvCurve empty;
};


#endif // _UGEN_ugen_EnvCurve_H_
