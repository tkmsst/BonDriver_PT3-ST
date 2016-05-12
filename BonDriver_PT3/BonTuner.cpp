#include "stdafx.h"
#include <Windows.h>
#include <process.h>

#include "inc/EARTH_PT3.h"
#include "inc/OS_Library.h"
using namespace EARTH;

#include "BonTuner.h"

static BOOL isISDB_S;

#define DATA_BUFF_SIZE	(188*256)
#define MAX_BUFF_COUNT	500

#pragma warning( disable : 4273 )
extern "C" __declspec(dllexport) IBonDriver * CreateBonDriver()
{
	// 同一プロセスからの複数インスタンス取得禁止
	// (非同期で取得された場合の排他処理がちゃんと出来ていないが放置)
	CBonTuner *p = NULL;
	if (CBonTuner::m_pThis == NULL) {
		p = new CBonTuner;
	}
	return p;
}
#pragma warning( default : 4273 )

CBonTuner * CBonTuner::m_pThis = NULL;
HINSTANCE CBonTuner::m_hModule = NULL;


CBonTuner::CBonTuner()
{
	m_pThis = this;
	m_hOnStreamEvent = NULL;

	m_LastBuff = NULL;

	m_dwCurSpace = 0xFF;
	m_dwCurChannel = 0xFF;

	m_iID = -1;
	m_hStopEvent = _CreateEvent(FALSE, FALSE, NULL);
	m_hThread = NULL;

	::InitializeCriticalSection(&m_CriticalSection);

	WCHAR strExePath[512] = L"";
	GetModuleFileName(m_hModule, strExePath, 512);

	WCHAR szPath[_MAX_PATH];	// パス
	WCHAR szDrive[_MAX_DRIVE];
	WCHAR szDir[_MAX_DIR];
	WCHAR szFname[_MAX_FNAME];
	WCHAR szExt[_MAX_EXT];
	_tsplitpath_s( strExePath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFname, _MAX_FNAME, szExt, _MAX_EXT );
	_tmakepath_s(  szPath, _MAX_PATH, szDrive, szDir, NULL, NULL );

	m_strPT1CtrlExe = szPath;
	m_strPT1CtrlExe += L"PT3Ctrl.exe";

	wstring strIni;
	strIni = szPath;
	strIni += L"BonDriver_PT3-ST.ini";

	m_dwSetChDelay = GetPrivateProfileIntW(L"SET", L"SetChDelay", 0, strIni.c_str());

	isISDB_S = TRUE;

	WCHAR szName[256];
	m_iTunerID = -1;
	if( wcslen(szFname) == wcslen(L"BonDriver_PT3-**") ){
		const WCHAR *TUNER_NAME2;
		if (szFname[14] == L'T'){
			isISDB_S = FALSE;
			TUNER_NAME2 = L"PT3 ISDB-T (%d)";
		}else{
			TUNER_NAME2 = L"PT3 ISDB-S (%d)";
		}
		m_iTunerID = _wtoi(szFname+wcslen(L"BonDriver_PT3-*"));
		wsprintfW(szName, TUNER_NAME2, m_iTunerID);
		m_strTunerName = szName;
	}else if( wcslen(szFname) == wcslen(L"BonDriver_PT3-*") ){
		const WCHAR *TUNER_NAME;
		if (szFname[14] == L'T'){
			isISDB_S = FALSE;
			TUNER_NAME = L"PT3 ISDB-T";
		}else{
			TUNER_NAME = L"PT3 ISDB-S";
		}
		m_strTunerName = TUNER_NAME;
	}else{
		m_strTunerName = L"PT3 ISDB-S";
	}

	wstring strChSet;
	strChSet = szPath;
	if (isISDB_S)
		strChSet += L"BonDriver_PT3-S.ChSet.txt";
	else
		strChSet += L"BonDriver_PT3-T.ChSet.txt";

	m_chSet.ParseText(strChSet.c_str());
}

CBonTuner::~CBonTuner()
{
	CloseTuner();

	::EnterCriticalSection(&m_CriticalSection);
	SAFE_DELETE(m_LastBuff);
	::LeaveCriticalSection(&m_CriticalSection);

	::CloseHandle(m_hStopEvent);
	m_hStopEvent = NULL;

	::DeleteCriticalSection(&m_CriticalSection);

	m_pThis = NULL;
}

const BOOL CBonTuner::OpenTuner(void)
{
	//イベント
	m_hOnStreamEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);

	BOOL bRet = CreateProcessW( NULL, (LPWSTR)m_strPT1CtrlExe.c_str(), NULL, NULL, FALSE, GetPriorityClass(GetCurrentProcess()), NULL, NULL, &si, &pi );
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	DWORD dwRet;
	if( m_iTunerID >= 0 ){
		dwRet = SendOpenTuner2(isISDB_S, m_iTunerID, &m_iID);
	}else{
		dwRet = SendOpenTuner(isISDB_S, &m_iID);
	}

	_RPT3(_CRT_WARN, "*** CBonTuner::OpenTuner() ***\nm_hOnStreamEvent[%p] bRet[%s] dwRet[%u]\n", m_hOnStreamEvent, bRet ? "TRUE" : "FALSE", dwRet);

	if( dwRet != CMD_SUCCESS ){
		return FALSE;
	}

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, (LPVOID)this, CREATE_SUSPENDED, NULL);
	ResumeThread(m_hThread);

	return TRUE;
}

void CBonTuner::CloseTuner(void)
{
	if( m_hThread != NULL ){
		::SetEvent(m_hStopEvent);
		// スレッド終了待ち
		if ( ::WaitForSingleObject(m_hThread, 15000) == WAIT_TIMEOUT ){
			::TerminateThread(m_hThread, 0xffffffff);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	m_dwCurSpace = 0xFF;
	m_dwCurChannel = 0xFF;

	::CloseHandle(m_hOnStreamEvent);
	m_hOnStreamEvent = NULL;

	if( m_iID != -1 ){
		SendCloseTuner(m_iID);
		m_iID = -1;
	}

	//バッファ解放
	::EnterCriticalSection(&m_CriticalSection);
	while (!m_TsBuff.empty()){
		TS_DATA *p = m_TsBuff.front();
		m_TsBuff.pop_front();
		delete p;
	}
	::LeaveCriticalSection(&m_CriticalSection);
}

const BOOL CBonTuner::SetChannel(const BYTE bCh)
{
	return TRUE;
}

const float CBonTuner::GetSignalLevel(void)
{
	if( m_iID == -1 ){
		return 0;
	}
	DWORD dwCn100;
	if( SendGetSignal(m_iID, &dwCn100) == CMD_SUCCESS ){
		return ((float)dwCn100) / 100.0f;
	}else{
		return 0;
	}
}

const DWORD CBonTuner::WaitTsStream(const DWORD dwTimeOut)
{
	if( m_hOnStreamEvent == NULL ){
		return WAIT_ABANDONED;
	}
	// イベントがシグナル状態になるのを待つ
	const DWORD dwRet = ::WaitForSingleObject(m_hOnStreamEvent, (dwTimeOut)? dwTimeOut : INFINITE);

	switch(dwRet){
		case WAIT_ABANDONED :
			// チューナが閉じられた
			return WAIT_ABANDONED;

		case WAIT_OBJECT_0 :
		case WAIT_TIMEOUT :
			// ストリーム取得可能
			return dwRet;

		case WAIT_FAILED :
		default:
			// 例外
			return WAIT_FAILED;
	}
}

const DWORD CBonTuner::GetReadyCount(void)
{
	DWORD dwCount = 0;
	::EnterCriticalSection(&m_CriticalSection);
	dwCount = (DWORD)m_TsBuff.size();
	::LeaveCriticalSection(&m_CriticalSection);
	return dwCount;
}

const BOOL CBonTuner::GetTsStream(BYTE *pDst, DWORD *pdwSize, DWORD *pdwRemain)
{
	BYTE *pSrc = NULL;

	if(GetTsStream(&pSrc, pdwSize, pdwRemain)){
		if(*pdwSize){
			::CopyMemory(pDst, pSrc, *pdwSize);
		}
		return TRUE;
	}
	return FALSE;
}

const BOOL CBonTuner::GetTsStream(BYTE **ppDst, DWORD *pdwSize, DWORD *pdwRemain)
{
	BOOL bRet;
	::EnterCriticalSection(&m_CriticalSection);
	if( m_TsBuff.size() != 0 ){
		delete m_LastBuff;
		m_LastBuff = m_TsBuff.front();
		m_TsBuff.pop_front();
		*pdwSize = m_LastBuff->dwSize;
		*ppDst = m_LastBuff->pbBuff;
		*pdwRemain = (DWORD)m_TsBuff.size();
		bRet = TRUE;
	}else{
		*pdwSize = 0;
		*pdwRemain = 0;
		bRet = FALSE;
	}
	::LeaveCriticalSection(&m_CriticalSection);
	return bRet;
}

void CBonTuner::PurgeTsStream(void)
{
	//バッファ解放
	::EnterCriticalSection(&m_CriticalSection);
	while (!m_TsBuff.empty()){
		TS_DATA *p = m_TsBuff.front();
		m_TsBuff.pop_front();
		delete p;
	}
	::LeaveCriticalSection(&m_CriticalSection);
}

LPCTSTR CBonTuner::GetTunerName(void)
{
	return m_strTunerName.c_str();
}

const BOOL CBonTuner::IsTunerOpening(void)
{
	return FALSE;
}

LPCTSTR CBonTuner::EnumTuningSpace(const DWORD dwSpace)
{
	map<DWORD, SPACE_DATA>::iterator itr;
	itr = m_chSet.spaceMap.find(dwSpace);
	if( itr == m_chSet.spaceMap.end() ){
		return NULL;
	}else{
		return itr->second.wszName.c_str();
	}
}

LPCTSTR CBonTuner::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel)
{
	DWORD key = dwSpace<<16 | dwChannel;
	map<DWORD, CH_DATA>::iterator itr;
	itr = m_chSet.chMap.find(key);
	if( itr == m_chSet.chMap.end() ){
		return NULL;
	}else{
		return itr->second.wszName.c_str();
	}
}

const BOOL CBonTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	DWORD key = dwSpace<<16 | dwChannel;
	map<DWORD, CH_DATA>::iterator itr;
	itr = m_chSet.chMap.find(key);
	if (itr == m_chSet.chMap.end()) {
		return FALSE;
	}

	if (m_iID == -1) {
		return FALSE;
	}

	if ((SendSetCh(m_iID, itr->second.dwPT1Ch, itr->second.dwTSID)) != CMD_SUCCESS) {
		return FALSE;
	}

	if (m_dwSetChDelay)
		Sleep(m_dwSetChDelay);

	PurgeTsStream();

	m_dwCurSpace = dwSpace;
	m_dwCurChannel = dwChannel;
	return TRUE;
}

const DWORD CBonTuner::GetCurSpace(void)
{
	return m_dwCurSpace;
}

const DWORD CBonTuner::GetCurChannel(void)
{
	return m_dwCurChannel;
}

void CBonTuner::Release()
{
	delete this;
}

UINT WINAPI CBonTuner::RecvThread(LPVOID pParam)
{
	CBonTuner* pSys = (CBonTuner*)pParam;

	wstring strEvent;
	wstring strPipe;
	Format(strEvent, L"%s%d", CMD_PT1_DATA_EVENT_WAIT_CONNECT, pSys->m_iID);
	Format(strPipe, L"%s%d", CMD_PT1_DATA_PIPE, pSys->m_iID);

	while (1) {
		if (::WaitForSingleObject( pSys->m_hStopEvent, 0 ) != WAIT_TIMEOUT) {
			//中止
			break;
		}
		DWORD dwSize;
		BYTE *pbBuff;
		if ((SendSendData(pSys->m_iID, &pbBuff, &dwSize, strEvent, strPipe) == CMD_SUCCESS) && (dwSize != 0)) {
			TS_DATA *pData = new TS_DATA(pbBuff, dwSize);
			::EnterCriticalSection(&pSys->m_CriticalSection);
			while (pSys->m_TsBuff.size() > MAX_BUFF_COUNT) {
				TS_DATA *p = pSys->m_TsBuff.front();
				pSys->m_TsBuff.pop_front();
				delete p;
			}
			pSys->m_TsBuff.push_back(pData);
			::LeaveCriticalSection(&pSys->m_CriticalSection);
			::SetEvent(pSys->m_hOnStreamEvent);
		}else{
			::Sleep(5);
		}
	}

	return 0;
}
