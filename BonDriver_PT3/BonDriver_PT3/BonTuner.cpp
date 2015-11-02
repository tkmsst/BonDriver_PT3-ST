#include "stdafx.h"
#include <Windows.h>
#include <process.h>

#include "inc/EARTH_PT3.h"
#include "inc/OS_Library.h"
using namespace EARTH;

//#include "Resource.h"
#include "BonTuner.h"

//#define _ISDB_T
#ifdef _ISDB_T
#define TUNER_NAME L"PT3 ISDB-T"
#define TUNER_NAME2 L"PT%d ISDB-T (%d)"
#define SPACE_1 L"地デジ(UHF)"
#define SPACE_2 L"地デジ(CATV)"
#define SPACE_3 L"地デジ(VHF)"
#else
#define TUNER_NAME L"PT3 ISDB-S"
#define TUNER_NAME2 L"PT%d ISDB-S (%d)"
#define SPACE_1 L"BS"
#define SPACE_2 L"CS110"
#endif

/*
static const CH_DATA stUHF[]=
{
	{L"13Ch", 0, 0, 63},
	{L"14Ch", 0, 1, 64},
	{L"15Ch", 0, 2, 65},
	{L"16Ch", 0, 3, 66},
	{L"17Ch", 0, 4, 67},
	{L"18Ch", 0, 5, 68},
	{L"19Ch", 0, 6, 69},
	{L"20Ch", 0, 7, 70},
	{L"21Ch", 0, 8, 71},
	{L"22Ch", 0, 9, 72},
	{L"23Ch", 0,10, 73},
	{L"24Ch", 0,11, 74},
	{L"25Ch", 0,12, 75},
	{L"26Ch", 0,13, 76},
	{L"27Ch", 0,14, 77},
	{L"28Ch", 0,15, 78},
	{L"29Ch", 0,16, 79},
	{L"30Ch", 0,17, 80},
	{L"31Ch", 0,18, 81},
	{L"32Ch", 0,19, 82},
	{L"33Ch", 0,20, 83},
	{L"34Ch", 0,21, 84},
	{L"35Ch", 0,22, 85},
	{L"36Ch", 0,23, 86},
	{L"37Ch", 0,24, 87},
	{L"38Ch", 0,25, 88},
	{L"39Ch", 0,26, 89},
	{L"40Ch", 0,27, 90},
	{L"41Ch", 0,28, 91},
	{L"42Ch", 0,29, 92},
	{L"43Ch", 0,30, 93},
	{L"44Ch", 0,31, 94},
	{L"45Ch", 0,32, 95},
	{L"46Ch", 0,33, 96},
	{L"47Ch", 0,34, 97},
	{L"48Ch", 0,35, 98},
	{L"49Ch", 0,36, 99},
	{L"50Ch", 0,37,100},
	{L"51Ch", 0,38,101},
	{L"52Ch", 0,39,102},
	{L"53Ch", 0,40,103},
	{L"54Ch", 0,41,104},
	{L"55Ch", 0,42,105},
	{L"56Ch", 0,43,106},
	{L"57Ch", 0,44,107},
	{L"58Ch", 0,45,108},
	{L"59Ch", 0,46,109},
	{L"60Ch", 0,47,110},
	{L"61Ch", 0,48,111},
	{L"62Ch", 0,49,112}
};

static const CH_DATA stCATV[]=
{
	{L"C13Ch", 1, 0,  3},
	{L"C14Ch", 1, 1,  4},
	{L"C15Ch", 1, 2,  5},
	{L"C16Ch", 1, 3,  6},
	{L"C17Ch", 1, 4,  7},
	{L"C18Ch", 1, 5,  8},
	{L"C19Ch", 1, 6,  9},
	{L"C20Ch", 1, 7, 10},
	{L"C21Ch", 1, 8, 11},
	{L"C22Ch", 1, 9, 12},
	{L"C23Ch", 1,10, 22},
	{L"C24Ch", 1,11, 23},
	{L"C25Ch", 1,12, 24},
	{L"C26Ch", 1,13, 25},
	{L"C27Ch", 1,14, 26},
	{L"C28Ch", 1,15, 27},
	{L"C29Ch", 1,16, 28},
	{L"C30Ch", 1,17, 29},
	{L"C31Ch", 1,18, 30},
	{L"C32Ch", 1,19, 31},
	{L"C33Ch", 1,20, 32},
	{L"C34Ch", 1,21, 33},
	{L"C35Ch", 1,22, 34},
	{L"C36Ch", 1,23, 35},
	{L"C37Ch", 1,24, 36},
	{L"C38Ch", 1,25, 37},
	{L"C39Ch", 1,26, 38},
	{L"C40Ch", 1,27, 39},
	{L"C41Ch", 1,28, 40},
	{L"C42Ch", 1,29, 41},
	{L"C43Ch", 1,30, 42},
	{L"C44Ch", 1,31, 43},
	{L"C45Ch", 1,32, 44},
	{L"C46Ch", 1,33, 45},
	{L"C47Ch", 1,34, 46},
	{L"C48Ch", 1,35, 47},
	{L"C49Ch", 1,36, 48},
	{L"C50Ch", 1,37, 49},
	{L"C51Ch", 1,38, 50},
	{L"C52Ch", 1,39, 51},
	{L"C53Ch", 1,40, 52},
	{L"C54Ch", 1,41, 53},
	{L"C55Ch", 1,42, 54},
	{L"C56Ch", 1,43, 55},
	{L"C57Ch", 1,44, 56},
	{L"C58Ch", 1,45, 57},
	{L"C59Ch", 1,46, 58},
	{L"C60Ch", 1,47, 59},
	{L"C61Ch", 1,48, 60},
	{L"C62Ch", 1,49, 61},
	{L"C63Ch", 1,49, 62}
};

static const CH_DATA stVHF[]=
{
	{L"1Ch",  2, 0,  0},
	{L"2Ch",  2, 1,  1},
	{L"3Ch",  2, 2,  2},
	{L"4Ch",  2, 3, 13},
	{L"5Ch",  2, 4, 14},
	{L"6Ch",  2, 5, 15},
	{L"7Ch",  2, 6, 16},
	{L"8Ch",  2, 7, 17},
	{L"9Ch",  2, 8, 18},
	{L"10Ch", 2, 9, 19},
	{L"11Ch", 2,10, 20},
	{L"12Ch", 2,11, 21},
};

static const CH_DATA stBS[]=
{
	{L"BS1/TS0 BS朝日",          0, 0,  0, 0x4010},
	{L"BS1/TS1 BS-i",            0, 1,  0, 0x4011},
	{L"BS3/TS0 WOWOW",           0, 2,  1, 0x4030},
	{L"BS3/TS1 BSジャパン",      0, 3,  1, 0x4031},
	{L"BS9/TS0 BS11",            0, 4,  4, 0x4090},
	{L"BS9/TS1 Star Channel HV", 0, 5,  4, 0x4091},
	{L"BS9/TS2 TwellV",          0, 6,  4, 0x4092},
	{L"BS13/TS0 BS日テレ",       0, 7,  6, 0x40D0},
	{L"BS13/TS1 BSフジ",         0, 8,  6, 0x40D1},
	{L"BS15/TS1 NHK BS1/2",      0, 9,  7, 0x40F1},
	{L"BS15/TS2 NHK BS-hi",      0,10,  7, 0x40F2},
	{L"BS17/TS1 難視聴対策",     0,11,  8, 0x4310},
	{L"BS17/TS2 難視聴対策",     0,12,  8, 0x4311},
};

static const CH_DATA stCS[]=
{
	{L"ND2",  1, 0, 12, 0x6020},
	{L"ND4",  1, 1, 13, 0x7040},
	{L"ND6",  1, 2, 14, 0x7060},
	{L"ND8",  1, 3, 15, 0x6080},
	{L"ND10", 1, 4, 16, 0x60A0},
	{L"ND12", 1, 5, 17, 0x70C0},
	{L"ND14", 1, 6, 18, 0x70E0},
	{L"ND16", 1, 7, 19, 0x7100},
	{L"ND18", 1, 8, 20, 0x7120},
	{L"ND20", 1, 9, 21, 0x7140},
	{L"ND22", 1,10, 22, 0x7160},
	{L"ND24", 1,11, 23, 0x7180}
};
*/
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
	m_strPT1CtrlExe+= L"PT3Ctrl.exe";

	wstring strIni;
	strIni = szPath;
	strIni += L"BonDriver_PT3-ST.ini";

	m_bUseUHF = GetPrivateProfileInt(L"SET", L"UseUHF", 1, strIni.c_str());
	m_bUseCATV = GetPrivateProfileInt(L"SET", L"UseCATV", 1, strIni.c_str());
	m_bUseVHF = GetPrivateProfileInt(L"SET", L"UseVHF", 1, strIni.c_str());
	m_bUseBS = GetPrivateProfileInt(L"SET", L"UseBS", 1, strIni.c_str());
	m_bUseCS = GetPrivateProfileInt(L"SET", L"UseCS", 1, strIni.c_str());

	m_iTunerID = -1;
	if( wcslen(szFname) == wcslen(L"BonDriver_PT3-**") ){
		m_iTunerID = _wtoi(szFname+wcslen(L"BonDriver_PT3-*"));
		int iPTn = GetPTDeviceInfo(m_iTunerID/2);
		if( iPTn != -1 ){
			WCHAR szName[256] = L"";
			wsprintf(szName, TUNER_NAME2, iPTn, m_iTunerID);
			m_strTunerName = szName;
		}else{
			WCHAR szName[256] = L"";
			wsprintf(szName, L"%s (%d)", TUNER_NAME, m_iTunerID);
			m_strTunerName = szName;
		}
	}else{
		m_strTunerName = TUNER_NAME;
	}

	wstring strChSet;
	strChSet = szPath;
#ifdef _ISDB_T
	strChSet += L"BonDriver_PT3-T.ChSet.txt";
#else
	strChSet += L"BonDriver_PT3-S.ChSet.txt";
#endif

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
#ifdef _ISDB_T
	if( m_iTunerID >= 0 ){
		dwRet = SendOpenTuner2(FALSE, m_iTunerID, &m_iID);
	}else{
		dwRet = SendOpenTuner(FALSE, &m_iID);
	}
#else
	if( m_iTunerID >= 0 ){
		dwRet = SendOpenTuner2(TRUE, m_iTunerID, &m_iID);
	}else{
		dwRet = SendOpenTuner(TRUE, &m_iID);
	}
#endif
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
/*
#ifdef _ISDB_T
	switch(dwSpace){
		case 0:
			return SPACE_1;
		case 1:
			return SPACE_2;
		case 2:
			return SPACE_3;
		default:
			return NULL;
	}
#else
	switch(dwSpace){
		case 0:
			return SPACE_1;
		case 1:
			return SPACE_2;
		default:
			return NULL;
	}
#endif
*/
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
/*
#ifdef _ISDB_T
	switch(dwSpace){
		case 0:
			if( sizeof(stUHF)/sizeof(CH_DATA) > dwChannel && m_bUseUHF == TRUE){
				return stUHF[dwChannel].wszName;
			}
			break;
		case 1:
			if( sizeof(stCATV)/sizeof(CH_DATA) > dwChannel && m_bUseCATV == TRUE){
				return stCATV[dwChannel].wszName;
			}
			break;
		case 2:
			if( sizeof(stVHF)/sizeof(CH_DATA) > dwChannel && m_bUseVHF == TRUE){
				return stVHF[dwChannel].wszName;
			}
			break;
		default:
			return NULL;
	}
#else
	switch(dwSpace){
		case 0:
			if( sizeof(stBS)/sizeof(CH_DATA) > dwChannel && m_bUseBS == TRUE){
				return stBS[dwChannel].wszName;
			}
			break;
		case 1:
			if( sizeof(stCS)/sizeof(CH_DATA) > dwChannel && m_bUseCS == TRUE){
				return stCS[dwChannel].wszName;
			}
			break;
		default:
			return NULL;
	}
#endif
	return NULL;
*/
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
/*
#ifdef _ISDB_T
		switch(dwSpace){
			case 0:
				dwRet=SendSetCh(m_iID, stUHF[dwChannel].dwPT1Ch, 0);
				break;
			case 1:
				dwRet=SendSetCh(m_iID, stCATV[dwChannel].dwPT1Ch, 0);
				break;
			case 2:
				dwRet=SendSetCh(m_iID, stVHF[dwChannel].dwPT1Ch, 0);
				break;
			default:
				return FALSE;
		}
#else
		switch(dwSpace){
			case 0:
				dwRet=SendSetCh(m_iID, stBS[dwChannel].dwPT1Ch, stBS[dwChannel].dwTSID);
				break;
			case 1:
				dwRet=SendSetCh(m_iID, stCS[dwChannel].dwPT1Ch, stCS[dwChannel].dwTSID);
				break;
			default:
				return FALSE;
		}
#endif
*/
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
