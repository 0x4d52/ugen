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

#include "ugen_StandardHeader.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

BEGIN_UGEN_NAMESPACE

#include "ugen_TextFile.h"


TextFileReaderInternal::TextFileReaderInternal(Text const& path) throw()
{
	peer = new ifstream(path.getArray());	
}

TextFileReaderInternal::~TextFileReaderInternal()
{
	ifstream *file = (ifstream*)peer;
	
	if(file && file->is_open())
		file->close();
	
	delete file;	
}

Text TextFileReaderInternal::readLine() throw()
{
	ifstream *file = (ifstream*)peer;
	string line;
	
	Text text;
	
	if (file && file->is_open() && !file->eof())
	{
		getline (*file,line);
		text = Text(line.c_str());
	}
	
	return text;
}

Text TextFileReaderInternal::readAll(Text const& lineEndings) throw()
{
	Text text;
	
	while(isEof() == false)
	{
		text <<= readLine();
		
		if(isEof() == false && lineEndings.length() > 0)
			text <<= lineEndings;
	}
	
	return text;
}

bool TextFileReaderInternal::isEof() const throw()
{
	ifstream *file = (ifstream*)peer;
	
	if (file && file->is_open() && !file->eof())
		return false;
	else
		return true;	
}

bool TextFileReaderInternal::readArray(NumericalArray<char> *array) throw()
{
	ugen_assert(array != 0);
	
	if(isEof()) return false;
	
	Text text = readLine();
	
	if(text != "NumericalArray<char>:v1") return false; // should reset the file position before read
	
	text = readLine(); // should be size info;
	int size;
	sscanf(text.getArray(), "size:%d", &size);
	
	NumericalArray<char> newArray = NumericalArray<char>::withSize(size);
	char *newArrayPtr = newArray.getArray();
	
	for(int i = 0; i < size; i++)
	{
		text = readLine();
		int index;
		int value;
		sscanf(text.getArray(), "[%d]=%d", &index, &value);
		
		if(index != i) return false;
		
		newArrayPtr[i] = value;
	}
	
	*array = newArray;
	return true;
}

bool TextFileReaderInternal::readArray(NumericalArray<int> *array) throw()
{
	ugen_assert(array != 0);
	
	if(isEof()) return false;
	
	Text text = readLine();
	
	if(text != "NumericalArray<int>:v1") return false; // should reset the file position before read
	
	text = readLine(); // should be size info;
	int size;
	sscanf(text.getArray(), "size:%d", &size);
	
	NumericalArray<int> newArray = NumericalArray<int>::withSize(size);
	int *newArrayPtr = newArray.getArray();
	
	for(int i = 0; i < size; i++)
	{
		text = readLine();
		int index;
		int value;
		sscanf(text.getArray(), "[%d]=%d", &index, &value);
		
		if(index != i) return false;
		
		newArrayPtr[i] = value;
	}
	
	*array = newArray;
	return true;
}

bool TextFileReaderInternal::readArray(NumericalArray<float> *array) throw()
{
	ugen_assert(array != 0);
	
	if(isEof()) return false;
	
	Text text = readLine();
	
	if(text != "NumericalArray<float>:v1") return false; // should reset the file position before read
	
	text = readLine(); // should be size info;
	int size;
	sscanf(text.getArray(), "size:%d", &size);
	
	NumericalArray<float> newArray = NumericalArray<float>::withSize(size);
	float *newArrayPtr = newArray.getArray();
	
	for(int i = 0; i < size; i++)
	{
		text = readLine();
		int index;
		float value;
		sscanf(text.getArray(), "[%d]=%f", &index, &value);
		
		if(index != i) return false;
		
		newArrayPtr[i] = value;
	}
	
	*array = newArray;
	return true;
}

bool TextFileReaderInternal::readArray(NumericalArray<double> *array) throw()
{
	ugen_assert(array != 0);
	
	if(isEof()) return false;
	
	Text text = readLine();
	
	if(text != "NumericalArray<double>:v1") return false; // should reset the file position before read
	
	text = readLine(); // should be size info;
	int size;
	sscanf(text.getArray(), "size:%d", &size);
	
	NumericalArray<double> newArray = NumericalArray<double>::withSize(size);
	double *newArrayPtr = newArray.getArray();
	
	for(int i = 0; i < size; i++)
	{
		text = readLine();
		int index;
		double value;
		sscanf(text.getArray(), "[%d]=%lg", &index, &value);
		
		if(index != i) return false;
		
		newArrayPtr[i] = value;
	}
	
	*array = newArray;
	return true;	
}

TextFileWriterInternal::TextFileWriterInternal(Text const& path) throw()
{
	peer = new ofstream(path.getArray(), ios_base::out | ios_base::trunc);
}

TextFileWriterInternal::~TextFileWriterInternal()
{
	ofstream *file = (ofstream*)peer;
	
	if(file && file->is_open())
		file->close();
	
	delete file;
}

void TextFileWriterInternal::write(Text const& text) throw()
{
	ofstream *file = (ofstream*)peer;
	if(file && file->is_open())
		*file << text.getArray();
}

void TextFileWriterInternal::write(const char* text) throw()
{
	ugen_assert(text != 0);
	
	ofstream *file = (ofstream*)peer;
	if(file && file->is_open())
		*file << text;
}

void TextFileWriterInternal::writeValue(const char value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%d", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const int value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%d", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const long value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%ld", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const unsigned long value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%lu", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const unsigned int value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%u", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const float value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%.16f", value);
	write(buf);
}

void TextFileWriterInternal::writeValue(const double value) throw()
{
	const int size = 64;
	char buf[size];
	snprintf(buf, size, "%.16f", (float)value);
	write(buf);
}

void TextFileWriterInternal::writeArray(NumericalArray<char> const& array) throw()
{
	writeArrayInternal(array, "NumericalArray<char>:v1\n");
}

void TextFileWriterInternal::writeArray(NumericalArray<int> const& array) throw()
{
	writeArrayInternal(array, "NumericalArray<int>:v1\n");
}

void TextFileWriterInternal::writeArray(NumericalArray<float> const& array) throw()
{
	writeArrayInternal(array, "NumericalArray<float>:v1\n");
}

void TextFileWriterInternal::writeArray(NumericalArray<double> const& array) throw()
{
	writeArrayInternal(array, "NumericalArray<double>:v1\n");
}


END_UGEN_NAMESPACE
