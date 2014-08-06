#pragma once

// Remove asserts and Windows7 warnings for speed
// These were mnoved to the makefile (they are defined for the release build configuration)
//#define BOOST_DISABLE_ASSERTS
//#define _SCL_SECURE_NO_WARNINGS


#include "NullptrWorkaround.h"   // for older gcc compiler



typedef unsigned char byte; 

// Note there are no types for uint8 that are interpreted as a number (if unsigned char is used it gets interpretted as a character)
typedef unsigned short ushort;
typedef unsigned int   uint;

// Windows7 operating system definitions
typedef uint uint32;
typedef int int32;   
typedef double float64;
typedef float float32;
typedef bool bool8;   // 1 byte




/*

Fundamental types in ISO/IEC C++

bool			1   true / false		
char			1   -128	to 127
signed char		1	-128	to 127
unsigned char	1	0		to 255
short			2   -32768	to 32767
unsigned short  2   0		to 65535
int				4	-2.147Bn	to +2.147Bn
unsigned int	4   0		to 4.29Bn			10U, 64000U
long			4	-2.147Bn	to +2.147Bn		-77L, 12543L
unsigned long	4	0		to	4.29Bn	
long long		8	-9^18   to  9^18       
u. long long	8	0		to 18^18			55ULL, 855ull
float			4   +/- 3.4E+/-38 with 7 digits accuracy
double			8   1.7E+/-308  with 15 digits accuaracy

*/










