// $Id: ugen_MappingUGens.h 998 2010-03-08 10:33:23Z mgrobins $
// $HeadURL: http://dsrowlan@164.11.131.73/svn/CEMS/mt/other/UGenProject/UGen/basics/ugen_MappingUGens.h $

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

#ifndef _UGEN_ugen_MappingUGens_H_
#define _UGEN_ugen_MappingUGens_H_


#include "../core/ugen_UGen.h"
#include "../basics/ugen_MulAdd.h"

#define MappingBase_InputsWithTypesAndDefaults	UGen const& input, UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh
#define MappingBase_InputsWithTypesOnly			UGen const& input, UGen const& inLow, UGen const& inHigh, UGen const& outLow, UGen const& outHigh
#define MappingBase_InputsNoTypes				input, inLow, inHigh, outLow, outHigh
#define MappingBase_InputsEnum					Input, InLow, InHigh, OutLow, OutHigh

/** @ingroup UGenInternals */
class MappingBaseUGenInternal : public UGenInternal
{
public:
	MappingBaseUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw() = 0;
	
	enum Inputs { MappingBase_InputsEnum, NumInputs };
	
protected:
};

/** @ingroup UGenInternals */
class LinExpSignalUGenInternal : public MappingBaseUGenInternal
{
public:
	LinExpSignalUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinExpSignalUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class LinExpScalarUGenInternal : public MappingBaseUGenInternal
{
public:
	LinExpScalarUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinExpScalarUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class LinLinSignalUGenInternal : public MappingBaseUGenInternal
{
public:
	LinLinSignalUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinLinSignalUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class LinLinScalarUGenInternal : public MappingBaseUGenInternal
{
public:
	LinLinScalarUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinLinScalarUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class LinSinSignalUGenInternal : public MappingBaseUGenInternal
{
public:
	LinSinSignalUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinSinSignalUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class LinSinScalarUGenInternal : public MappingBaseUGenInternal
{
public:
	LinSinScalarUGenInternal(MappingBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(LinSinScalarUGenInternal, (MappingBase_InputsNoTypes), (MappingBase_InputsWithTypesAndDefaults));



#define MapTableBase_InputsWithTypesAndDefaults		UGen const& input, UGen const& inLow, UGen const& inHigh, Buffer const& table
#define MapTableBase_InputsWithTypesOnly			UGen const& input, UGen const& inLow, UGen const& inHigh, Buffer const& table
#define MapTableBase_InputsNoTypes					input, inLow, inHigh, table
#define MapTableBase_InputsEnum						Input, InLow, InHigh

/** @ingroup UGenInternals */
class MapTableBaseUGenInternal : public UGenInternal
{
public:
	MapTableBaseUGenInternal(MapTableBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw() = 0;
	
	enum Inputs { MapTableBase_InputsEnum, NumInputs };
	
protected:
	Buffer table_;
	const float tableSize1;
};

/** @ingroup UGenInternals */
class MapTableSignalUGenInternal : public MapTableBaseUGenInternal
{
public:
	MapTableSignalUGenInternal(MapTableBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
protected:	
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(MapTableSignalUGenInternal, 
								   (MapTableBase_InputsNoTypes), 
								   (MapTableBase_InputsWithTypesAndDefaults));

/** @ingroup UGenInternals */
class MapTableScalarUGenInternal : public MapTableBaseUGenInternal
{
public:
	MapTableScalarUGenInternal(MapTableBase_InputsWithTypesAndDefaults) throw();
	UGenInternal* getChannel(const int channel) throw();
	UGenInternal* getKr() throw();
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
protected:	
};

/** @ingroup UGenInternals */
UGenInternalControlRateDeclaration(MapTableScalarUGenInternal, 
									(MapTableBase_InputsNoTypes), 
									(MapTableBase_InputsWithTypesAndDefaults));


#define MappingBaseBase_Docs	@param input	The input UGen to map.													\
								@param inLow	The lower range of the input value (note this is not clipped so			\
												it is the user`s responsibility to make sure the value is in range)		\
								@param inHigh	The upper range of the input value (note this is not clipped so			\
												it is the user`s responsibility to make sure the value is in range)		\

#define MappingBase_Docs		MappingBaseBase_Docs																	\
								@param outLow	The lower range of the desired output value.							\
								@param outHigh	The upper range of the desired output value.

/** Map a linear to exponential scale.
 @ingroup AllUGens MathsUGens
 @see LinLin, LinSin, Map */
UGenSublcassDeclaration(LinExp, (MappingBase_InputsNoTypes),
								(MappingBase_InputsWithTypesAndDefaults), 
						COMMON_UGEN_DOCS MappingBase_Docs);

/** Map a linear to linear scale.
 @ingroup AllUGens MathsUGens
 @see LinExp, LinSin, Map */
UGenSublcassDeclaration(LinLin, (MappingBase_InputsNoTypes),
								(MappingBase_InputsWithTypesAndDefaults), 
						COMMON_UGEN_DOCS MappingBase_Docs);

/** Map a linear to sine interpolated scale.
 @ingroup AllUGens MathsUGens
 @see LinExp, LinLin, Map*/
UGenSublcassDeclaration(LinSin, (MappingBase_InputsNoTypes),
								(MappingBase_InputsWithTypesAndDefaults), 
						COMMON_UGEN_DOCS MappingBase_Docs);

#define Map_Docs				MappingBaseBase_Docs																	\
								@param table	A single channel Buffer used as a lookup table. Here an input			\
												value of inLow would output the first value in the table, an			\
												input value of inHigh would output the last value in the table.			\
												An input value half way between inLow and inHigh would output the		\
												value in the middle of the table, and so on.
					

/** Map uaing a lookup table.
 @ingroup AllUGens MathsUGens
 @see LinExp, LinLin, LinSin*/
DirectMulAddUGenDeclaration(Map, 
							(input, inLow, inHigh, table), 
							(input, inLow, inHigh, table, MulAdd_ArgsCall), 
							(UGen const& input, UGen const& inLow, UGen const& inHigh, Buffer const& table), 
							(UGen const& input, UGen const& inLow, UGen const& inHigh, Buffer const& table, MulAdd_ArgsDeclare), 
							COMMON_UGEN_DOCS Map_Docs MulAddArgs_Docs);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define LinExp LinExp()
#define LinLin LinLin()
#define LinSin LinSin()
#define Map Map()
#endif


#endif // _UGEN_ugen_MappingUGens_H_
