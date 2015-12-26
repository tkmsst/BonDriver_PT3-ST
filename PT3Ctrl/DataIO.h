#pragma once

#include "inc/EARTH_PT3.h"
#include "inc/Prefix.h"
#include "inc/EX_Buffer.h"
#include "../Common/PT1OutsideCtrlCmdDef.h"
#include "../Common/PipeServer.h"

//#define TRANSFER_SIZE (4096*PT::Device::BUFFER_PAGE_COUNT)
//#define VIRTUAL_IMAGE_COUNT 4
//#define VIRTUAL_COUNT 8
//#define LOCK_SIZE 4
//#define READ_BLOCK_COUNT 8
//#define READ_BLOCK_SIZE (TRANSFER_SIZE / READ_BLOCK_COUNT)

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

	uint32 mVirtualIndex;
	uint32 mImageIndex;
	uint32 mBlockIndex;
	volatile bool mQuit;

	HANDLE m_hStopEvent;
	HANDLE m_hThread;

	CPipeServer m_cPipeT0;
	CPipeServer m_cPipeT1;
	CPipeServer m_cPipeS0;
	CPipeServer m_cPipeS1;

	typedef struct _BUFF_DATA{
		BYTE* pbBuff;
		DWORD dwSize;
		DWORD dwSetSize;
		_BUFF_DATA(void){
			pbBuff = NULL;
			dwSize = 0;
			dwSetSize = 0;
		}
		~_BUFF_DATA(void){
			SAFE_DELETE_ARRAY(pbBuff)
		}
	}BUFF_DATA;

	vector<BUFF_DATA*> m_T0Buff;
	vector<BUFF_DATA*> m_T1Buff;
	vector<BUFF_DATA*> m_S0Buff;
	vector<BUFF_DATA*> m_S1Buff;

	EARTH::EX::Buffer* m_T0SetBuff;
	EARTH::EX::Buffer* m_T1SetBuff;
	EARTH::EX::Buffer* m_S0SetBuff;
	EARTH::EX::Buffer* m_S1SetBuff;

	uint32 m_T0WriteIndex;
	uint32 m_T1WriteIndex;
	uint32 m_S0WriteIndex;
	uint32 m_S1WriteIndex;
/*
	CMicroPacketUtil m_cT0Micro;
	CMicroPacketUtil m_cT1Micro;
	CMicroPacketUtil m_cS0Micro;
	CMicroPacketUtil m_cS1Micro;
*/
	DWORD m_dwT0OverFlowCount;
	DWORD m_dwT1OverFlowCount;
	DWORD m_dwS0OverFlowCount;
	DWORD m_dwS1OverFlowCount;

	HANDLE m_hEvent1;
	HANDLE m_hEvent2;
	HANDLE m_hEvent3;
	HANDLE m_hEvent4;

	//BYTE* m_bDMABuff;

	enum {
		UNIT_PER_4096    = 47,		// ユニットサイズ÷4096
		WRITE_PER_UNIT   =  1,		// 1回あたりの書き込みは何ユニット分か
		//BUFFER_PER_WRITE = 32		// バッファは何書き込み分か
	};

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

	uint32 Size();
	void ChkTransferInfo();
	/*
	bool WaitBlock();
	void CopyBlock();
	bool DispatchBlock();
	void Clear(uint virtualIndex, uint imageIndex, uint blockIndex);
	uint Read(uint virtualIndex, uint imageIndex, uint blockIndex) const;
	uint Offset(uint imageIndex, uint blockIndex, uint additionalOffset = 0) const;
	void MicroPacket(BYTE* pbPacket);
*/
	static int CALLBACK OutsideCmdCallbackT0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackT1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackS0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	static int CALLBACK OutsideCmdCallbackS1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);
	
	void CmdSendData(DWORD dwID, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam);

	void ResetDMA();

	bool CheckReady(EARTH::EX::Buffer* buffer, uint32 index);
	bool ReadAddBuff(EARTH::EX::Buffer* buffer, uint32 index, vector<BUFF_DATA*>* tsBuff);
};
