#include "stdafx.h"
#include <Windows.h>
#include <process.h>

#include "inc/EARTH_PT3.h"
#include "inc/OS_Library.h"
using namespace EARTH;

#include "BonTuner.h"

static BOOL isISDB_S;

#define DATA_BUFF_SIZE 188*256
#define MAX_BUFF_COUNT 500

#pragma warning( disable : 4273 )
extern "C" __declspec(dllexport) IBonDriver * CreateBonDriver()
{
	return (CBonTuner::m_pThis)? CBonTuner::m_pThis : ((IBonDriver *) new CBonTuner);
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

	m_bUseUHF = GetPrivateProfileInt(L"SET", L"UseUHF", 1, strIni.c_str());
	m_bUseCATV = GetPrivateProfileInt(L"SET", L"UseCATV", 1, strIni.c_str());
	m_bUseVHF = GetPrivateProfileInt(L"SET", L"UseVHF", 1, strIni.c_str());
	m_bUseBS = GetPrivateProfileInt(L"SET", L"UseBS", 1, strIni.c_str());
	m_bUseCS = GetPrivateProfileInt(L"SET", L"UseCS", 1, strIni.c_str());

	isISDB_S = TRUE;
	WCHAR TUNER_SPACE = L'S';
	WCHAR szName[256];
	m_iTunerID = -1;
	if( wcslen(szFname) == wcslen(L"BonDriver_PT3-**") ){
		if (szFname[14] == L'T'){
			isISDB_S = FALSE;
			TUNER_SPACE = L'T';
		}
		m_iTunerID = _wtoi(szFname+wcslen(L"BonDriver_PT3-*"));
		wsprintfW(szName, L"PT3 ISDB-%c (%d)", TUNER_SPACE, m_iTunerID);
	}else if( wcslen(szFname) == wcslen(L"BonDriver_PT3-*") ){
		if (szFname[14] == L'T'){
			isISDB_S = FALSE;
			TUNER_SPACE = L'T';
		}
		wsprintfW(szName, L"PT3 ISDB-%c", TUNER_SPACE);
	}else{
		wsprintfW(szName, L"PT3 ISDB-S");
	}
	m_strTunerName = szName;

	wstring strFname;
	wstring strChSet;
	strFname = L"BonDriver_PT3-*.ChSet.txt";
	strFname[14] = TUNER_SPACE;
	strChSet = szPath;
	strChSet += strFname;

	m_chSet.ParseText(strChSet.c_str());
}

CBonTuner::~CBonTuner()
{
	CloseTuner();
	::CloseHandle(m_hStopEvent);
	m_hStopEvent = NULL;

	::DeleteCriticalSection(&m_CriticalSection);

	m_pThis = NULL;
}

int CBonTuner::GetPTDeviceInfo(DWORD dwIndex)
{
	OS::Library* cLibrary = NULL;
	PT::Bus* cBus = NULL;

	cLibrary = new OS::Library();

	PT::Bus::NewBusFunction function = cLibrary->Function();
	if (function == NULL) {
		SAFE_DELETE(cLibrary);
		cLibrary = NULL;
		return -1;
	}
	status enStatus = function(&cBus);
	if( enStatus != PT::STATUS_OK ){
		SAFE_DELETE(cLibrary);
		cLibrary = NULL;
		return -1;
	}

	//バージョンチェック
	uint32 version;
	cBus->GetVersion(&version);
	if ((version >> 8) != 2) {
		cBus->Delete();
		SAFE_DELETE(cLibrary);
		cLibrary = NULL;
		cBus = NULL;
		return -1;
	}

	int iRet = -1;
	PT::Bus::DeviceInfo deviceInfo[9];
	uint32 deviceInfoCount = sizeof(deviceInfo)/sizeof(*deviceInfo);
	cBus->Scan(deviceInfo, &deviceInfoCount);
	if( dwIndex < deviceInfoCount ){
		iRet = deviceInfo[dwIndex].PTn;
	}

	if( cBus != NULL ){
		cBus->Delete();
		cBus = NULL;
	}
	if( cLibrary != NULL ){
		SAFE_DELETE(cLibrary);
		cLibrary = NULL;
	}

	return iRet;
}

const BOOL CBonTuner::OpenTuner(void)
{
	//イベント
	m_hOnStreamEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);

	BOOL bRet = CreateProcess( NULL, (LPWSTR)m_strPT1CtrlExe.c_str(), NULL, NULL, FALSE, GetPriorityClass(GetCurrentProcess()), NULL, NULL, &si, &pi );
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	DWORD dwRet;
	if( m_iTunerID >= 0 ){
		dwRet = SendOpenTuner2(isISDB_S, m_iTunerID, &m_iID);
	}else{
		dwRet = SendOpenTuner(isISDB_S, &m_iID);
	}
	if( dwRet != CMD_SUCCESS ){
		return FALSE;
	}

	//初期チャンネル

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
	Sleep(100);

	//バッファ解放
	::EnterCriticalSection(&m_CriticalSection);
	if( m_LastBuff != NULL ){
		SAFE_DELETE(m_LastBuff);
	}
	for( int i=0; i<(int)m_TsBuff.size(); i++ ){
		SAFE_DELETE(m_TsBuff[i])
	}
	m_TsBuff.clear();
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
	::EnterCriticalSection(&m_CriticalSection);
	if( m_LastBuff != NULL ){
		SAFE_DELETE(m_LastBuff);
	}
	::LeaveCriticalSection(&m_CriticalSection);
	BOOL bRet = TRUE;
	if( m_TsBuff.size() != 0 ){
		::EnterCriticalSection(&m_CriticalSection);
		m_LastBuff = m_TsBuff[0];
		m_TsBuff.erase( m_TsBuff.begin() );
		::LeaveCriticalSection(&m_CriticalSection);

		*pdwSize = m_LastBuff->dwSize;
		*ppDst = m_LastBuff->pbBuff;
		*pdwRemain = (DWORD)m_TsBuff.size();
	}else{
		*pdwSize = 0;
		*pdwRemain = 0;
		bRet = FALSE;
	}
	return bRet;
}

void CBonTuner::PurgeTsStream(void)
{
	//バッファ解放
	::EnterCriticalSection(&m_CriticalSection);
	if( m_LastBuff != NULL ){
		SAFE_DELETE(m_LastBuff);
	}
	for( int i=0; i<(int)m_TsBuff.size(); i++ ){
		SAFE_DELETE(m_TsBuff[i])
	}
	m_TsBuff.clear();
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
//	if(!EnumChannelName(dwSpace, dwChannel))return FALSE;
	DWORD key = dwSpace<<16 | dwChannel;
	map<DWORD, CH_DATA>::iterator itr;
	itr = m_chSet.chMap.find(key);
	if( itr == m_chSet.chMap.end() ){
		return FALSE;
	}

	DWORD dwRet=CMD_ERR;
	if( m_iID != -1 ){
		dwRet=SendSetCh(m_iID, itr->second.dwPT1Ch, itr->second.dwTSID);
	}else{
		return FALSE;
	}
	Sleep(100);

	PurgeTsStream();

	if( dwRet != CMD_SUCCESS ){
		return FALSE;
	}else{
		m_dwCurSpace = dwSpace;
		m_dwCurChannel = dwChannel;
		return TRUE;
	}
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

	while(1){
		if( WaitForSingleObject( pSys->m_hStopEvent, 0 ) != WAIT_TIMEOUT ){
			//中止
			break;
		}
		TS_DATA* pData = new TS_DATA;
		pData->dwSize = DATA_BUFF_SIZE;
		pData->pbBuff = new BYTE[pData->dwSize];

		if( SendSendData(pSys->m_iID, pData->pbBuff, &pData->dwSize, strEvent, strPipe) == CMD_SUCCESS ){
			::EnterCriticalSection(&pSys->m_CriticalSection);
			if( pSys->m_TsBuff.size() > MAX_BUFF_COUNT ){
				while( pSys->m_TsBuff.size() > MAX_BUFF_COUNT ){
					SAFE_DELETE(pSys->m_TsBuff[0]);
					pSys->m_TsBuff.erase( pSys->m_TsBuff.begin() );
				}
			}
			pSys->m_TsBuff.push_back(pData);
			::LeaveCriticalSection(&pSys->m_CriticalSection);
			SetEvent(pSys->m_hOnStreamEvent);
		}else{
			SAFE_DELETE(pData);
			Sleep(5);
		}
	}

	return 0;
}
