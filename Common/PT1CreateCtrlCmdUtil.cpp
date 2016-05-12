#include "stdafx.h"
#include "PT1CreateCtrlCmdUtil.h"

BOOL CreateDefStream(DWORD dwValue, CMD_STREAM* pCmd)
{
	if( pCmd == NULL ){
		return FALSE;
	}
	pCmd->dwSize = sizeof(DWORD);
	pCmd->bData = new BYTE[sizeof(DWORD)];

	DWORD dwPos = 0;

	memcpy(pCmd->bData + dwPos, &dwValue, sizeof(DWORD));
//	dwPos+=sizeof(DWORD);

	return TRUE;
}

BOOL CopyDefData(DWORD* pdwValue, BYTE* pBuff)
{
	if( pdwValue == NULL || pBuff == NULL ){
		return FALSE;
	}
	DWORD dwPos = 0;

	*pdwValue = *(DWORD*)(pBuff+dwPos);
//	dwPos+=sizeof(DWORD);

	return TRUE;
}

BOOL CreateDefStream2(DWORD dwValue, DWORD dwValue2, CMD_STREAM* pCmd)
{
	if( pCmd == NULL ){
		return FALSE;
	}
	pCmd->dwSize = sizeof(DWORD)*2;
	pCmd->bData = new BYTE[sizeof(DWORD)*2];

	DWORD dwPos = 0;

	memcpy(pCmd->bData + dwPos, &dwValue, sizeof(DWORD));
	dwPos+=sizeof(DWORD);

	memcpy(pCmd->bData + dwPos, &dwValue2, sizeof(DWORD));
//	dwPos+=sizeof(DWORD);

	return TRUE;
}

BOOL CopyDefData2(DWORD* pdwValue, DWORD* pdwValue2, BYTE* pBuff)
{
	if( pdwValue == NULL || pdwValue2 == NULL || pBuff == NULL ){
		return FALSE;
	}
	DWORD dwPos = 0;

	*pdwValue = *(DWORD*)(pBuff+dwPos);
	dwPos+=sizeof(DWORD);

	*pdwValue2 = *(DWORD*)(pBuff+dwPos);
//	dwPos+=sizeof(DWORD);

	return TRUE;
}

BOOL CreateDefStream3(DWORD dwValue, DWORD dwValue2, DWORD dwValue3, CMD_STREAM* pCmd)
{
	if( pCmd == NULL ){
		return FALSE;
	}
	pCmd->dwSize = sizeof(DWORD)*3;
	pCmd->bData = new BYTE[sizeof(DWORD)*3];

	DWORD dwPos = 0;

	memcpy(pCmd->bData + dwPos, &dwValue, sizeof(DWORD));
	dwPos+=sizeof(DWORD);

	memcpy(pCmd->bData + dwPos, &dwValue2, sizeof(DWORD));
	dwPos+=sizeof(DWORD);

	memcpy(pCmd->bData + dwPos, &dwValue3, sizeof(DWORD));
//	dwPos+=sizeof(DWORD);

	return TRUE;
}

BOOL CopyDefData3(DWORD* pdwValue, DWORD* pdwValue2, DWORD* pdwValue3, BYTE* pBuff)
{
	if( pdwValue == NULL || pdwValue2 == NULL || pdwValue3 == NULL || pBuff == NULL ){
		return FALSE;
	}
	DWORD dwPos = 0;

	*pdwValue = *(DWORD*)(pBuff+dwPos);
	dwPos+=sizeof(DWORD);

	*pdwValue2 = *(DWORD*)(pBuff+dwPos);
	dwPos+=sizeof(DWORD);

	*pdwValue3 = *(DWORD*)(pBuff+dwPos);
//	dwPos+=sizeof(DWORD);

	return TRUE;
}
