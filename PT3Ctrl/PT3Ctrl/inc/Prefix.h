#ifndef _Prefix_H
#define _Prefix_H

// PUBLIC_LEVEL	用途
// 0			一般公開用
// 1			組立工場検査用
// 2			FPGA 回路更新用
// 3			アースソフト社内用

#include <conio.h>
#include <crtdbg.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Prefix_Integer.h"

#define delete_(p)	{ delete p; p = NULL; }

#define BIT_SHIFT_MASK(value, shift, mask) (((value) >> (shift)) & ((1<<(mask))-1))

#endif
