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
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#ifndef _UGEN_ugen_Text_H_
#define _UGEN_ugen_Text_H_

#include "ugen_Arrays.h"

//typedef CharArray Text;

/** A class for storing and manipulating text strings.
 Similar to std:string and juce::String this is UGen++'s default text format. 
 Internally Text is just a null terminated C string (array of chars) and is just
 a specialised versio of CharArray or NumericalArra<char>. */
class Text : public CharArray
{
public:
	static const char space;
	static const Text empty;
	
	/** Creates an emply text string. */
	Text() throw();
	
	/** Creates a text string containing a single character. */
	Text(const char c) throw();
		
	/** Copy numerical values from another numerical array type to a new Text string. */
	template<class CopyType>
	Text(NumericalArray<CopyType> const& copy) throw()
	:	CharArray(copy)
	{
	}
	
	/** Copy values from another array type to a new Text string. */
	template<class CopyType>
	Text(ObjectArray<CopyType> const& copy) throw()
	:	CharArray(copy)
	{
	}
	
#if defined(JUCE_VERSION) || defined(DOXYGEN)
	/** Copy a Juce string to a Text string. */
	Text(String const& string) throw();
	
	/* Cast a Text stirng to a Juce String. */
	operator String () const throw();
#endif
		
	/** Create a Text string from a wide char string. */
	Text(const wchar_t* string) throw();
	
	ObjectArrayAssignmentDefinition(Text, char);
	
	/** Concatentate two Text strings into one. */
	Text(Text const& array0, Text const& array1) throw();
		
	Text(NumericalArraySpec const& spec) throw();
	Text(NumericalArraySpec const& spec, const bool needsNullTermination) throw();
	static Text withSize(const int size, const bool zeroData = false) throw();	
	static Text newClear(const int size) throw();
	static Text fromValue(const int num) throw();
	static Text fromValue(const float num) throw();

	Text& operator= (CharArray const& other) throw();
	Text& operator= (const char* other) throw();
	Text& operator= (const wchar_t* other) throw();
	
	Text operator+ (Text const& rightOperand) const throw();
	Text operator+= (Text const& rightOperand) throw();

	Text offset(Text const& rightOperand) const throw();
	Text& add(Text const& other) throw();
	
	Text(const char* nullTerminatedSourceArray);
	Text(const int size, const char* sourceArray, const bool needsNullTermination = false);
		
	Text toUpper() const throw();
	Text toLower() const throw();
	
	bool equalsIgnoreCase(Text const& other) const throw();
	
	bool containsIgnoreCase (Text const& toFind) const throw();	
	int indexOfIgnoreCase (Text const& toFind, const int startIndex = 0) const throw();
	int indexOfAnyIgnoreCase (Text const& toFind, const int startIndex = 0) const throw();
	Text replaceIgnoreCase(Text const& find, Text const& substitute) const throw();
	
	void print(const char *prefix = 0) const throw();
	
	static const Text& getPrintable() throw();
	static const Text& getAlphabetLower() throw();
	static const Text& getAlphabetUpper() throw();
	static const Text& getAlphabetAll() throw();
	static const Text& getDigits() throw();
	static const Text& getAlphabetAndDigits() throw();
	static const Text& getPunctuation() throw();
	static const Text& getStandardDelimiters() throw();
	
		
};

Text operator+ (const char* text1, Text const& text2) throw();
Text operator+ (const wchar_t* text1, Text const& text2) throw();


//typedef NumericalArray<wchar_t>			UnicodeArray;
//typedef UnicodeArray UnicodeText;

//typedef CharArray2D TextArray;

//typedef ObjectArray<Text> TextArray;
//typedef ObjectArray2D<Text> TextArray2D;

//typedef ObjectArray<Text> TextArray;

class TextArray : public ObjectArray2DBase<char, Text>
{
public:
	TextArray(const int rows = 0)
	:	ObjectArray2DBase<char, Text> (rows < 0 ? 0 : rows)//, false) 
	{
	}
		
	TextArray(ObjectArray< ObjectArray<char> > const& other)
	:	ObjectArray2DBase<char, Text>(other.size())
	{
		const int rows = this->size();
		for(int row = 0; row < rows; row++)
		{
			this->put(row, Text(other[row]));
		}
	}
	
	TextArray(Text const& single)
	:	ObjectArray2DBase<char, Text>(single)
	{
	}
	
	TextArray(Text const &i00,
			  Text const &i01,
			  Text const &i02,
			  Text const &i03 = Text())
	:	ObjectArray2DBase<char, Text> (i00, i01, i02, i03) 
	{
	}
	
	TextArray(Text const &i00,
			  Text const &i01,
			  Text const &i02,
			  Text const &i03,
			  Text const &i04,
			  Text const &i05 = Text(),
			  Text const &i06 = Text(),
			  Text const &i07 = Text())
	:	ObjectArray2DBase<char, Text> (i00, i01, i02, i03,
									   i04, i05, i06, i07) 
	{
	}
	
	TextArray(Text const &i00,
			  Text const &i01,
			  Text const &i02,
			  Text const &i03,
			  Text const &i04,
			  Text const &i05,
			  Text const &i06,
			  Text const &i07,
			  Text const &i08,
			  Text const &i09 = Text(),
			  Text const &i10 = Text(),
			  Text const &i11 = Text(),
			  Text const &i12 = Text(),
			  Text const &i13 = Text(),
			  Text const &i14 = Text(),
			  Text const &i15 = Text())
	:	ObjectArray2DBase<char, Text>(i00, i01, i02, i03,
									  i04, i05, i05, i07,
									  i08, i09, i10, i11, 
									  i12, i13, i14, i15)
	{
	}
	
	TextArray(Text const &i00,
			  Text const &i01,
			  Text const &i02,
			  Text const &i03,
			  Text const &i04,
			  Text const &i05,
			  Text const &i06,
			  Text const &i07,
			  Text const &i08,
			  Text const &i09,
			  Text const &i10,
			  Text const &i11,
			  Text const &i12,
			  Text const &i13,
			  Text const &i14,
			  Text const &i15,
			  Text const &i16,
			  Text const &i17 = Text(),
			  Text const &i18 = Text(),
			  Text const &i19 = Text(),
			  Text const &i20 = Text(),
			  Text const &i21 = Text(),
			  Text const &i22 = Text(),
			  Text const &i23 = Text(),
			  Text const &i24 = Text(),
			  Text const &i25 = Text(),
			  Text const &i26 = Text(),
			  Text const &i27 = Text(),
			  Text const &i28 = Text(),
			  Text const &i29 = Text(),
			  Text const &i30 = Text(),
			  Text const &i31 = Text())
	:	ObjectArray2DBase<char, Text>(i00, i01, i02, i03,
									  i04, i05, i05, i07,
									  i08, i09, i10, i11, 
									  i12, i13, i14, i15, 
									  i16, i17, i18, i19,
									  i20, i21, i22, i23,
									  i24, i25, i26, i27,
									  i28, i29, i30, i31)
	{
	}
	
	ObjectArrayAssignmentDefinition(TextArray, Text);
	
	TextArray(Text const& array0, 
			  Text const& array1) throw()
	:	ObjectArray2DBase<char, Text>(array0, array1)
	{
	}
	
	TextArray(TextArray const& array0, 
			  TextArray const& array1) throw()
	:	ObjectArray2DBase<char, Text>(array0, array1)
	{
	}
	
	int indexOf(Text const& item, const int startIndex = 0) const throw();
	int indexOfIgnoreCase(Text const& item, const int startIndex = 0) const throw();
	
	void print(const char *prefix = 0) const throw();
	
};

typedef ObjectArray2D<Text, TextArray> TextArray2D;

#endif // _UGEN_ugen_Text_H_
