// BonTuner.h: CBonTuner クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "IBonDriver2.h"
#include "../Common/PT1SendCtrlCmdUtil.h"
#include "ParseChSet.h"

#define BUFF_SIZE (188*256)


class CBonTuner : public IBonDriver2
{
public:
	CBonTuner();
	virtual ~CBonTuner();

// IBonDriver
	const BOOL OpenTuner(void);
	void CloseTuner(void);

	const BOOL SetChannel(const BYTE bCh);
	const float GetSignalLevel(void);

	const DWORD WaitTsStream(const DWORD dwTimeOut = 0);
	const DWORD GetReadyCount(void);

	const BOOL GetTsStream(BYTE *pDst, DWORD *pdwSize, DWORD *pdwRemain);
	const BOOL GetTsStream(BYTE **ppDst, DWORD *pdwSize, DWORD *pdwRemain);

	void PurgeTsStream(void);

// IBonDriver2(暫定)
	LPCTSTR GetTunerName(void);

	const BOOL IsTunerOpening(void);
	
	LPCTSTR EnumTuningSpace(const DWORD dwSpace);
	LPCTSTR EnumChannelName(const DWORD dwSpace, const DWORD dwChannel);

	const BOOL SetChannel(const DWORD dwSpace, const DWORD dwChannel);
	
	const DWORD GetCurSpace(void);
	const DWORD GetCurChannel(void);

	void Release(void);

	static CBonTuner * m_pThis;
	static HINSTANCE m_hModule;

protected:
	CRITICAL_SECTION m_CriticalSection;
	HANDLE m_hOnStreamEvent;

	DWORD m_dwCurSpace;
	DWORD m_dwCurChannel;

	typedef struct _TS_DATA{
		BYTE* pbBuff;
		DWORD dwSize;
		_TS_DATA(BYTE* pb, DWORD dw) : pbBuff(pb), dwSize(dw){
		}
		~_TS_DATA(void){
			delete[] pbBuff;
		}
	} TS_DATA;
	deque<TS_DATA*> m_TsBuff;
	TS_DATA* m_LastBuff;

	HANDLE m_hStopEvent;
	HANDLE m_hThread;

	int m_iID;
	int m_iTunerID;
	DWORD m_dwSetChDelay;

	wstring m_strPT1CtrlExe;

	wstring m_strTunerName;

	CParseChSet m_chSet;

protected:
	static UINT WINAPI RecvThread(LPVOID pParam);
};
