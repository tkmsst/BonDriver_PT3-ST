#ifndef __PT1_SEND_CTRL_CMD_UTIL_H__
#define __PT1_SEND_CTRL_CMD_UTIL_H__

#include "PT1OutsideCtrlCmdDef.h"
#include "PT1CreateCtrlCmdUtil.h"
#include "StringUtil.h"

#define CONNECT_TIMEOUT 30*1000

DWORD SendCloseExe(DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendOpenTuner(BOOL bSate, int* piID, DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendOpenTuner2(BOOL bSate, int iTunerID, int* piID, DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendCloseTuner(int iID, DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendSetCh(int iID, DWORD dwCh, DWORD dwTSID, DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendGetSignal(int iID, DWORD* pdwCn100, DWORD dwConnectTimeOut = CONNECT_TIMEOUT);
DWORD SendSendData(int iID, BYTE* pbData, DWORD* pdwSize, wstring strEvent, wstring strPipe, DWORD dwConnectTimeOut = CONNECT_TIMEOUT );

#endif
