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

#ifndef _UGEN_ugen_TextFile_H_
#define _UGEN_ugen_TextFile_H_

#include "ugen_SmartPointer.h"
#include "ugen_Text.h"

//typedef CharArray Text;

class TextFileReaderInternal : public SmartPointer
{
public:
	TextFileReaderInternal(Text const& path) throw();
	~TextFileReaderInternal();
	
	Text readLine() throw(); 
	Text readAll(Text const& lineEndings) throw();
	bool isEof() const throw();
	
	bool readArray(NumericalArray<char> *array) throw();
	bool readArray(NumericalArray<int> *array) throw();
	bool readArray(NumericalArray<float> *array) throw();
	bool readArray(NumericalArray<double> *array) throw();
	
private:
	void *peer;
};

class TextFileWriterInternal : public SmartPointer
{
public:
	TextFileWriterInternal(Text const& path) throw();
	~TextFileWriterInternal();
	
	void write(Text const& text) throw();
	void write(const char* text) throw();
	void writeValue(const char value) throw();
	void writeValue(const int value) throw();
	void writeValue(const long value) throw();
	void writeValue(const unsigned long value) throw();
	void writeValue(const unsigned int value) throw();
	void writeValue(const float value) throw();
	void writeValue(const double value) throw();
	
	void writeArray(NumericalArray<char> const& array) throw();
	void writeArray(NumericalArray<int> const& array) throw();
	void writeArray(NumericalArray<float> const& array) throw();
	void writeArray(NumericalArray<double> const& array) throw();
	
	
private:
	void *peer;
	
	template<class NumericalType>
	void writeArrayInternal(NumericalArray<NumericalType> const& array, const char *header = 0) throw()
	{
		const int arraySize = array.size();
		
		if(arraySize < 1) return;
		
		if(header) this->write(header);
		
		const int size = 128;
		char buf[size];
		
		snprintf(buf, size, "size: %d\n", arraySize);
		this->write(buf);
		
		const NumericalType* arrayPtr = array.getArray();
		
		for(int i = 0; i < arraySize; i++)
		{
			snprintf(buf, size, "[%d]=", i);
			this->write(buf);
			this->writeValue(arrayPtr[i]);
			this->write("\n");
		}
	}
};

class TextFileReader : public SmartPointerContainer<TextFileReaderInternal>
{
public:
	TextFileReader(Text const& path) throw()
	:	SmartPointerContainer<TextFileReaderInternal>
		(new TextFileReaderInternal(path))
	{
	}
	
	TextFileReader(const char* path) throw()
	:	SmartPointerContainer<TextFileReaderInternal>
		(new TextFileReaderInternal(path))
	{
	}
	
	Text readLine() throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->readLine();
		}
		else return Text();
	}
	
	Text readAll(Text const& lineEndings = "\n") throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->readAll(lineEndings);
		}
		else return Text();
	}
	
	bool isEof() const throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->isEof();
		}
		else return true;
	}
	
	template<class NumericalType>
	bool readArray(NumericalArray<NumericalType> *array) throw()
	{
		if(getInternal() != 0)
		{
			return getInternal()->readArray(array);
		}	
		else return false;
	}
	
};


class TextFileWriter : public SmartPointerContainer<TextFileWriterInternal>
{
public:
	TextFileWriter(Text const& path) throw()
	:	SmartPointerContainer<TextFileWriterInternal>
		(new TextFileWriterInternal(path))
	{
	}

	TextFileWriter(const char* path) throw()
	:	SmartPointerContainer<TextFileWriterInternal>
		(new TextFileWriterInternal(path))
	{
	}
	
	void write(Text const& text) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->write(text);
		}
	}
	
	template<class NumericalType>
	void writeValue(const NumericalType value) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->writeValue(value);
		}		
	}	
	
	template<class NumericalType>
	void writeArray(NumericalArray<NumericalType> const& array) throw()
	{
		if(getInternal() != 0)
		{
			getInternal()->writeArray(array);
		}		
	}

};

#endif // _UGEN_ugen_TextFile_H_
