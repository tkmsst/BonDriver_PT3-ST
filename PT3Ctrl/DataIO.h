#pragma once

#include "inc/EARTH_PT3.h"
#include "inc/Prefix.h"
#include "inc/EX_Buffer.h"
#include "../Common/PT1OutsideCtrlCmdDef.h"
#include "../Common/PipeServer.h"

using namespace EARTH;

class CDataIO
{
public:
	CDataIO(void);
	~CDataIO(void);

	void SetDevice(PT::Device* pcDevice){ m_pcDevice = pcDevice; };
	void SetVirtualCount(UINT uiVirtualCount){ VIRTUAL_COUNT = uiVirtualCount*8; };
	void Run();
	void Stop();
	BOOL EnableTuner(int iID, BOOL bEnable);
	void StartPipeServer(int iID);
	void StopPipeServer(int iID);
	void ClearBuff(int iID);

	DWORD GetOverFlowCount(int iID);

protected:
	UINT VIRTUAL_COUNT;
	PT::Device* m_pcDevice;

	HANDLE m_hStopEvent;
	HANDLE m_hThread;

	CPipeServer m_cPipeT0;
	CPipeServer m_cPipeT1;
	CPipeServer m_cPipeS0;
	CPipeServer m_cPipeS1;

	typedef struct _BUFF_DATA {
		BYTE* pbBuff;
		DWORD dwSize;
		_BUFF_DATA(DWORD dw) : dwSize(dw){
			pbBuff = new BYTE[dw];
		}
		~_BUFF_DATA(){
			delete[] pbBuff;
		}
	} BUFF_DATA;

	deque<BUFF_DATA*> m_T0Buff;
	deque<BUFF_DATA*> m_T1Buff;
	deque<BUFF_DATA*> m_S0Buff;
	deque<BUFF_DATA*> m_S1Buff;

	EARTH::EX::Buffer* m_T0SetBuff;
	EARTH::EX::Buffer* m_T1SetBuff;
	EARTH::EX::Buffer* m_S0SetBuff;
	EARTH::EX::Buffer* m_S1SetBuff;

	uint32 m_T0WriteIndex;
	uint32 m_T1WriteIndex;
	uint32 m_S0WriteIndex;
	uint32 m_S1WriteIndex;

	DWORD m_dwT0OverFlowCount;
	DWORD m_dwT1OverFlowCount;
	DWORD m_dwS0OverFlowCount;
	DWORD m_dwS1OverFlowCount;

	HANDLE m_hEvent1;
	HANDLE m_hEvent2;
	HANDLE m_hEvent3;
	HANDLE m_hEvent4;

protected:
	static UINT WINAPI RecvThread(LPVOID pParam);

	void Lock1();
	void UnLock1();
	void Lock2();
	void UnLock2();
	void Lock3();
	void UnLock3();
	void Lock4();
	void UnLock4();

	void ChkTransferInfo();

	static int CALLBACK OutsideCmdCallbackT0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackT1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackS0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackS1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	
	void CmdSendData(DWORD dwID, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);

	bool CheckReady(EARTH::EX::Buffer* buffer, uint32 index);
	bool ReadAddBuff(EARTH::EX::Buffer* buffer, uint32 index, deque<BUFF_DATA*> &tsBuff);

	void Flush(deque<BUFF_DATA*> &buf) {
		while (!buf.empty()){
			BUFF_DATA *p = buf.front();
			buf.pop_front();
			delete p;
		}
	};
};
