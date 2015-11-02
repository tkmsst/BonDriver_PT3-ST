#ifndef __PT1_CREATE_CTRL_CMD_UTIL_H__
#define __PT1_CREATE_CTRL_CMD_UTIL_H__

#include "PT1OutsideCtrlCmdDef.h"

BOOL CreateDefStream(DWORD dwValue, CMD_STREAM* pCmd);
BOOL CopyDefData(DWORD* pdwValue, BYTE* pBuff);

BOOL CreateDefStream2(DWORD dwValue, DWORD dwValue2, CMD_STREAM* pCmd);
BOOL CopyDefData2(DWORD* pdwValue, DWORD* pdwValue2, BYTE* pBuff);

BOOL CreateDefStream3(DWORD dwValue, DWORD dwValue2, DWORD dwValue3, CMD_STREAM* pCmd);
BOOL CopyDefData3(DWORD* pdwValue, DWORD* pdwValue2, DWORD* pdwValue3, BYTE* pBuff);

#endif
