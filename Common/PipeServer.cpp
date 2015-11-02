#include "StdAfx.h"
#include "PipeServer.h"
#include <process.h>

#define PIPE_TIMEOUT 500

CPipeServer::CPipeServer(void)
{
	m_pCmdProc = NULL;
	m_pParam = NULL;
	m_strEventName = L"";
	m_strPipeName = L"";

	m_hThread = NULL;

	m_hPipe = NULL;
	m_hEventConnect = NULL;
	m_hEventArray[0] = NULL;
	m_hEventArray[1] = NULL;
}

CPipeServer::~CPipeServer(void)
{
	StopServer();
}

BOOL CPipeServer::StartServer(LPCWSTR lpwszEventName, LPCWSTR lpwszPipeName, CMD_CALLBACK_PROC pfnCmdProc, void* pParam, int iThreadPriority)
{
	if( pfnCmdProc == NULL || pParam == NULL || lpwszEventName == NULL || lpwszPipeName == NULL ){
		return FALSE;
	}
	if( m_hThread != NULL ){
		return FALSE;
	}
	if( m_hPipe != NULL ){
		return FALSE;
	}

	m_pCmdProc = pfnCmdProc;
	m_pParam = pParam;
	m_strEventName = lpwszEventName;
	m_strPipeName = lpwszPipeName;
	m_iThreadPriority = iThreadPriority;

	_TCHAR DbgStr[MAX_PATH+64];
	_stprintf_s(DbgStr, _T("[Pt1Ctrl] invoke StartServer : %s\n"), m_strPipeName.c_str());
	::OutputDebugString(DbgStr);

	m_hEventArray[0] = _CreateEvent(FALSE, FALSE, NULL);					// StopEvent
	m_hEventArray[1] = _CreateEvent(FALSE, FALSE, NULL);					// コマンド受信イベント
	m_hEventConnect  = _CreateEvent(FALSE, FALSE, m_strEventName.c_str());	// 接続イベント

	BOOL bRes;
	bRes = FALSE;
	int nRetryCnt;
	nRetryCnt = 0;

	m_hPipe = _CreateNamedPipe(
				m_strPipeName.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_BYTE
				, 1
				, SEND_BUFF_SIZE
				, RES_BUFF_SIZE
				, PIPE_TIMEOUT
			);

	if (m_hPipe == INVALID_HANDLE_VALUE) {	// オープン失敗
		m_hPipe = NULL;	// オープン失敗
		CloseHandle(m_hStopEvent_PS);
		CloseHandle(m_hCmdRcvEvent_PS);
		CloseHandle(m_hEventConnect);
		m_hStopEvent_PS = NULL;
		m_hCmdRcvEvent_PS = NULL;
		m_hEventConnect = NULL;
		return FALSE;
	}

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, ServerThread, (LPVOID)this, CREATE_SUSPENDED, NULL);
	ResumeThread(m_hThread);

	return TRUE;
}

void CPipeServer::StopServer()
{
	if( m_hThread != NULL ){
		_TCHAR DbgStr[MAX_PATH+64];
		_stprintf_s(DbgStr, _T("[Pt1Ctrl] invoke StopServer : %s\n"), m_strPipeName.c_str());
		::OutputDebugString(DbgStr);

		::SetEvent(m_hStopEvent_PS);
		// スレッド終了待ち
		if ( ::WaitForSingleObject(m_hThread, 15000) == WAIT_TIMEOUT ){
			::TerminateThread(m_hThread, 0xffffffff);
			::OutputDebugString(_T("[Pt1Ctrl] ServerThread : abnormal end.\n"));

			ResetEvent(m_hEventConnect);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;

		FlushFileBuffers(m_hPipe);
		DisconnectNamedPipe(m_hPipe);	// クライアント切断

		CloseHandle(m_hPipe);
		m_hPipe = NULL;

		CloseHandle(m_hStopEvent_PS);
		CloseHandle(m_hCmdRcvEvent_PS);
		CloseHandle(m_hEventConnect);
		m_hStopEvent_PS = NULL;
		m_hCmdRcvEvent_PS = NULL;
		m_hEventConnect = NULL;

	}
}

UINT WINAPI CPipeServer::ServerThread(LPVOID pParam)
{
	::OutputDebugString(_T("[Pt1Ctrl] ServerThread : start thread.\n"));

	CPipeServer* pSys = (CPipeServer*)pParam;

	HANDLE hCurThread = GetCurrentThread();
	SetThreadPriority(hCurThread, pSys->m_iThreadPriority);
	
	for (;;) {
		ZeroMemory(&pSys->m_stOver, sizeof(OVERLAPPED));
		pSys->m_stOver.hEvent = pSys->m_hCmdRcvEvent_PS;
		ConnectNamedPipe(pSys->m_hPipe, &(pSys->m_stOver));
		SetEvent(pSys->m_hEventConnect);

		DWORD dwRes;
		dwRes = WaitForMultipleObjects(2, pSys->m_hEventArray, FALSE, INFINITE);
		if ( dwRes == WAIT_OBJECT_0 ) {
			//STOP
			ResetEvent(pSys->m_hEventConnect);
			::OutputDebugString(_T("[Pt1Ctrl] ServerThread : normal end.\n"));
			break;
		}
		else if ( dwRes == WAIT_OBJECT_0+1 ) {
			//コマンド受信
			if( pSys->m_pCmdProc != NULL) {
				CMD_STREAM stCmd;
				CMD_STREAM stRes;
				DWORD dwRead = 0;
				DWORD dwWrite = 0;
				do {
					if ( ReadFile(pSys->m_hPipe, &stCmd, sizeof(DWORD)*2, &dwRead, NULL ) == FALSE ) {
						break;
					}

					if ( stCmd.dwSize > 0 ) {
						stCmd.bData = new BYTE[stCmd.dwSize];
						DWORD dwReadNum = 0;
						while (dwReadNum < stCmd.dwSize ) {
							DWORD dwReadSize = 0;
							if ( stCmd.dwSize-dwReadNum < SEND_BUFF_SIZE ) {
								dwReadSize = stCmd.dwSize-dwReadNum;
							}
							else {
								dwReadSize = SEND_BUFF_SIZE;
							}
							if ( ReadFile(pSys->m_hPipe, stCmd.bData+dwReadNum, dwReadSize, &dwRead, NULL ) == FALSE ) {
								break;
							}
							dwReadNum+=dwRead;
						}
						if ( dwReadNum < stCmd.dwSize ) {
							break;
						}
					}

					pSys->m_pCmdProc(pSys->m_pParam, &stCmd, &stRes);

					if( WriteFile(pSys->m_hPipe, &stRes, sizeof(DWORD)*2, &dwWrite, NULL ) == FALSE ) {
						break;
					}
					if ( stRes.dwSize > 0 ){
						if( stRes.bData == NULL ) {
							break;
						}
						DWORD dwSendNum = 0;
						while (dwSendNum < stRes.dwSize ) {
							DWORD dwSendSize = 0;
							if ( stRes.dwSize-dwSendNum < RES_BUFF_SIZE ) {
								dwSendSize = stRes.dwSize-dwSendNum;
							}
							else {
								dwSendSize = RES_BUFF_SIZE;
							}
							if ( WriteFile(pSys->m_hPipe, stRes.bData+dwSendNum, dwSendSize, &dwWrite, NULL ) == FALSE ) {
								break;
							}
							dwSendNum+=dwWrite;
						}
						if ( dwSendNum < stRes.dwSize ) {
							break;
						}
					}
					ResetEvent(pSys->m_hCmdRcvEvent_PS);
				}
				while (stRes.dwParam == CMD_NEXT); // Emun用の繰り返し
			}
		}
		FlushFileBuffers(pSys->m_hPipe);
		DisconnectNamedPipe(pSys->m_hPipe);
	}
	return 0;
}
