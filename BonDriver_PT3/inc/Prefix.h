// ========================================================================================
//	Prefix.h
// ========================================================================================

#ifndef _Prefix_H
#define _Prefix_H

#pragma warning(disable : 4127)
#pragma warning(disable : 4512)

#include <conio.h>
#include <crtdbg.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Prefix_Integer.h"

#define delete_(p)	{ delete p; p = NULL; }

#define BIT_SHIFT_MASK(value, shift, mask) (((value) >> (shift)) & ((1<<(mask))-1))

//#define ENV_FACTORY
//#define PRIVATE_DLL_PATH

#endif
