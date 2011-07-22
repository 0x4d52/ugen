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

#include "ugen_Text.h"

const char Text::space = ' ';
const Text Text::empty = "";

Text::Text() throw()
:	CharArray(NumericalArraySpec(1, true), true)
{
}

Text::Text(const char c) throw()
:	CharArray(NumericalArraySpec(2, true), true)
{
	this->put(0, c);
}

#if defined(JUCE_VERSION) || defined(DOXYGEN)
Text::Text(String const& string) throw()
:	CharArray((const char*)string.toUTF8())
{
}

Text::operator String () const throw()
{
	return String((const char*)this->getArray());
}
#endif

Text::Text(const wchar_t* string) throw()
:	CharArray(NumericalArray<wchar_t>(string))
{
}


Text::Text(Text const& array0, Text const& array1) throw()
:	CharArray(array0, array1)
{
}

Text::Text(NumericalArraySpec const& spec) throw()
:	CharArray(spec, true)
{
	if(spec.zeroData_)
		zero();
}

Text::Text(NumericalArraySpec const& spec, const bool needsNullTermination) throw()
:	CharArray(spec, needsNullTermination)
{
	if(spec.zeroData_)
		zero();
}

Text Text::withSize(const int size, const bool zeroData) throw()
{
	return Text(NumericalArraySpec(size, zeroData));
}

Text Text::newClear(const int size) throw()
{
	return Text(NumericalArraySpec(size, true));
}
 
Text Text::fromValue(const int num) throw()
{
	const int size = 64;
	char buf[size];
	
	snprintf(buf, size, "%d", num);
	
	return buf;
}

Text Text::fromValue(const float num) throw()
{
	const int size = 64;
	char buf[size];
	
	snprintf(buf, size, "%f", num);
	
	return buf;
}

Text& Text::operator= (CharArray const& other) throw()
{
	return operator= (static_cast<Text const&> (other) );
}	

Text& Text::operator= (const char* other) throw()
{
	return operator= (Text(other));
}

Text& Text::operator= (const wchar_t* other) throw()
{
	return operator= (Text(other));
}

Text Text::operator+ (Text const& rightOperand) const throw()
{
	return Text(*this, rightOperand);
}

Text Text::operator+= (Text const& rightOperand) throw()
{
	return operator= (Text(*this, rightOperand));
}

Text operator+ (const char* text1, Text const& text2) throw()
{
	return Text(text1) + text2;
}

Text operator+ (const wchar_t* text1, Text const& text2) throw()
{
	return Text(text1) + text2;
}

Text Text::offset(Text const& rightOperand) const throw()
{
	CharArray left = *this;
	CharArray right = rightOperand;
	return left + right;
}

Text& Text::add(Text const& other) throw()
{
	const int length = other.length();
	for(int i = 0; i < length; i++)
	{
		CharArray::add(other[i]);
	}
	
	return *this;
}

Text::Text(const char* nullTerminatedSourceArray)
:	CharArray(nullTerminatedSourceArray)
{
}

Text::Text(const int size, const char* sourceArray, const bool needsNullTermination)
:	CharArray(CharArray::withArray(size, sourceArray, needsNullTermination))
{
}																								

Text Text::toUpper() const throw()
{
	const char *thisArray = this->getArray();
	const int size = this->size();
	
	if(thisArray == 0 || size <= 0) return *this;
	
	Text newText = Text::withSize(size, false);
	char *newArray = newText.getArray();		
	
	for(int i = 0; i < size; i++)
	{
		char c = thisArray[i];
		
		if(c >= 'a' && c <= 'z')
			c -= 32;
		
		newArray[i] = c;
	}			
	
	return newText;
}

Text Text::toLower() const throw()
{
	const char *thisArray = this->getArray();
	const int size = this->size();
	
	if(thisArray == 0 || size <= 0) return *this;
	
	Text newText = Text::withSize(size, false);
	char *newArray = newText.getArray();		
	
	for(int i = 0; i < size; i++)
	{
		char c = thisArray[i];
		
		if(c >= 'A' && c <= 'Z')
			c += 32;
		
		newArray[i] = c;
	}			
	
	return newText;	
}

bool Text::equalsIgnoreCase(Text const& other) const throw()
{
	return this->toLower() == other.toLower();
}

bool Text::containsIgnoreCase (Text const& toFind) const throw()
{
	return this->toLower().contains(toFind.toLower());
}

int Text::indexOfIgnoreCase (Text const& toFind, const int startIndex) const throw()
{
	return this->toLower().indexOf(toFind.toLower(), startIndex);
}

int Text::indexOfAnyIgnoreCase (Text const& toFind, const int startIndex) const throw()
{
	return this->toLower().indexOfAny(toFind.toLower(), startIndex);
}

Text Text::replaceIgnoreCase(Text const& find, Text const& substitute) const throw()
{
	Text thisLower = this->toLower();
	Text findLower = find.toLower();
	const int size = this->size();
	const int findLength = find.length();
	int startIndex = 0;
	int findIndex;
	
	Text result;
	
	while((findIndex = thisLower.indexOf(findLower, startIndex)) >= 0)
	{
		result <<= this->range(startIndex, findIndex);
		result <<= substitute;
		startIndex = findIndex + findLength;
	}
	
	result <<= this->range(startIndex, size);
	
	return result;
}

void Text::print(const char *prefix) const throw()
{
	if(prefix)
		printf("%s: ", prefix);
	
	printf("%s\n", this->getArray());
}

const Text& Text::getPrintable() throw()
{
	static const Text t = Text::series(128-32-1, 32, 1);
	return t;
}

const Text& Text::getAlphabetLower() throw()
{
	static const Text t = "abcdefghijklmnopqrstuvwxyz";
	return t;
}

const Text& Text::getAlphabetUpper() throw()
{
	static const Text t = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return t;
}

const Text& Text::getAlphabetAll() throw()
{
	static const Text t = Text::getAlphabetUpper() << Text::getAlphabetLower();
	return t;
}

const Text& Text::getDigits() throw()
{
	static const Text t = "0123456789";
	return t;
}

const Text& Text::getAlphabetAndDigits() throw()
{
	static const Text t = Text::getAlphabetAll() << Text::getDigits();
	return t;
}

const Text& Text::getPunctuation() throw()
{
	static const Text t = Text::getPrintable().copy().removeItems(Text::getAlphabetAndDigits()).removeItem(Text::space);
	return t;
}

const Text& Text::getStandardDelimiters() throw()
{
	static const Text t = " :;,.\t";
	return t;
}

int TextArray::indexOf(Text const& item, const int startIndex) const throw()
{
	ugen_assert(startIndex >= 0);
	
	const Text *array = this->getArray();
	
	if(array != 0)
	{
		const int length = this->length();
		for(int i = startIndex < 0 ? 0 : startIndex; i < length; i++)
		{
			if(array[i] == item)
				return i;
		}
	}
	
	return -1;
}

int TextArray::indexOfIgnoreCase(Text const& item, const int startIndex) const throw()
{
	ugen_assert(startIndex >= 0);
	
	const Text *array = this->getArray();
	
	if(array != 0)
	{
		const int length = this->length();
		for(int i = startIndex < 0 ? 0 : startIndex; i < length; i++)
		{
			if(array[i].equalsIgnoreCase(item))
				return i;
		}
	}
	
	return -1;
}

void TextArray::print(const char *prefix) const throw()
{
	const Text *textArray = this->getArray();
	const int size = this->size();
	for(int i = 0; i < size; i++)
	{
		if(prefix)
			printf("%s[%d]:", prefix, i);
		else
			printf("[%d]:", i);
		
		printf("%s\n", (const char*)textArray[i]);
	}
}

END_UGEN_NAMESPACE
