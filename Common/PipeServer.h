#pragma once

#include "StringUtil.h"
#include "PT1OutsideCtrlCmdDef.h"

typedef int (CALLBACK *CMD_CALLBACK_PROC)(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);

class CPipeServer
{
public:
	CPipeServer(void);
	~CPipeServer(void);

	BOOL StartServer(LPCWSTR lpwszEventName, LPCWSTR lpwszPipeName, CMD_CALLBACK_PROC pfnCmdProc, void* pParam, int iThreadPriority = THREAD_PRIORITY_NORMAL);
	void StopServer();

protected:
	CMD_CALLBACK_PROC m_pCmdProc;
	void* m_pParam;
	wstring m_strEventName;
	wstring m_strPipeName;

	int m_iThreadPriority;

	HANDLE m_hThread;
	HANDLE m_hPipe;
	HANDLE m_hEventConnect;
	OVERLAPPED m_stOver;
	HANDLE m_hEventArray[2];
#define m_hStopEvent_PS		m_hEventArray[0]
#define m_hCmdRcvEvent_PS	m_hEventArray[1]

protected:
	static UINT WINAPI ServerThread(LPVOID pParam);

};
