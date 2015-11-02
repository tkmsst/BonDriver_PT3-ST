#include "StdAfx.h"
#include "DataIO.h"
#include <process.h>

#define DATA_BUFF_SIZE (188*256)
#define MAX_DATA_BUFF_COUNT 500
#define NOT_SYNC_BYTE 0x74

CDataIO::CDataIO(void)
{
	VIRTUAL_COUNT = 8*8;

	m_hStopEvent = _CreateEvent(FALSE, FALSE, NULL);
	m_hThread = NULL;

	m_pcDevice = NULL;

	mQuit = false;
	
	m_T0SetBuff = NULL;
	m_T1SetBuff = NULL;
	m_S0SetBuff = NULL;
	m_S1SetBuff = NULL;
	/*
	for( int i=0; i<4; i++ ){
		BUFF_DATA* pDataBuff = new BUFF_DATA;
		pDataBuff->dwSize = DATA_BUFF_SIZE;
		pDataBuff->pbBuff = new BYTE[DATA_BUFF_SIZE];
		switch(i){
			case 1:
				m_T0SetBuff = pDataBuff;
				break;
			case 2:
				m_T1SetBuff = pDataBuff;
				break;
			case 3:
				m_S0SetBuff = pDataBuff;
				break;
			case 4:
				m_S1SetBuff = pDataBuff;
				break;
			default:
				break;
		}
	}
	*/
	m_hEvent1 = _CreateEvent(FALSE, TRUE, NULL );
	m_hEvent2 = _CreateEvent(FALSE, TRUE, NULL );
	m_hEvent3 = _CreateEvent(FALSE, TRUE, NULL );
	m_hEvent4 = _CreateEvent(FALSE, TRUE, NULL );

	m_dwT0OverFlowCount = 0;
	m_dwT1OverFlowCount = 0;
	m_dwS0OverFlowCount = 0;
	m_dwS1OverFlowCount = 0;

	//m_bDMABuff = NULL;
}

CDataIO::~CDataIO(void)
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
	if( m_hStopEvent != NULL ){
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}
	
	SAFE_DELETE(m_T0SetBuff);
	SAFE_DELETE(m_T1SetBuff);
	SAFE_DELETE(m_S0SetBuff);
	SAFE_DELETE(m_S1SetBuff);
	
	for( int i=0; i<(int)m_T0Buff.size(); i++ ){
		SAFE_DELETE(m_T0Buff[i]);
	}
	for( int i=0; i<(int)m_T1Buff.size(); i++ ){
		SAFE_DELETE(m_T1Buff[i]);
	}
	for( int i=0; i<(int)m_S0Buff.size(); i++ ){
		SAFE_DELETE(m_S0Buff[i]);
	}
	for( int i=0; i<(int)m_S1Buff.size(); i++ ){
		SAFE_DELETE(m_S1Buff[i]);
	}

	if( m_hEvent1 != NULL ){
		UnLock1();
		CloseHandle(m_hEvent1);
		m_hEvent1 = NULL;
	}
	if( m_hEvent2 != NULL ){
		UnLock2();
		CloseHandle(m_hEvent2);
		m_hEvent2 = NULL;
	}
	if( m_hEvent3 != NULL ){
		UnLock3();
		CloseHandle(m_hEvent3);
		m_hEvent3 = NULL;
	}
	if( m_hEvent4 != NULL ){
		UnLock4();
		CloseHandle(m_hEvent4);
		m_hEvent4 = NULL;
	}

	//SAFE_DELETE_ARRAY(m_bDMABuff);

}

void CDataIO::Lock1()
{
	if( m_hEvent1 == NULL ){
		return ;
	}
	if( WaitForSingleObject(m_hEvent1, 10*1000) == WAIT_TIMEOUT ){
		OutputDebugString(L"time out1");
	}
}

void CDataIO::UnLock1()
{
	if( m_hEvent1 != NULL ){
		SetEvent(m_hEvent1);
	}
}

void CDataIO::Lock2()
{
	if( m_hEvent2 == NULL ){
		return ;
	}
	if( WaitForSingleObject(m_hEvent2, 10*1000) == WAIT_TIMEOUT ){
		OutputDebugString(L"time out2");
	}
}

void CDataIO::UnLock2()
{
	if( m_hEvent2 != NULL ){
		SetEvent(m_hEvent2);
	}
}

void CDataIO::Lock3()
{
	if( m_hEvent3 == NULL ){
		return ;
	}
	if( WaitForSingleObject(m_hEvent3, 10*1000) == WAIT_TIMEOUT ){
		OutputDebugString(L"time out3");
	}
}

void CDataIO::UnLock3()
{
	if( m_hEvent3 != NULL ){
		SetEvent(m_hEvent3);
	}
}

void CDataIO::Lock4()
{
	if( m_hEvent4 == NULL ){
		return ;
	}
	if( WaitForSingleObject(m_hEvent4, 10*1000) == WAIT_TIMEOUT ){
		OutputDebugString(L"time out4");
	}
}

void CDataIO::UnLock4()
{
	if( m_hEvent4 != NULL ){
		SetEvent(m_hEvent4);
	}
}

uint32 CDataIO::Size()
{
	return 4096 * UNIT_PER_4096 * WRITE_PER_UNIT;
}

void CDataIO::ClearBuff(int iID)
{
	int iDevID = iID>>16;
	PT::Device::ISDB enISDB = (PT::Device::ISDB)((iID&0x0000FF00)>>8);
	uint32 iTuner = iID&0x000000FF;

	if( enISDB == PT::Device::ISDB_T ){
		if( iTuner == 0 ){
			Lock1();
			m_dwT0OverFlowCount = 0;
			//SAFE_DELETE(m_T0SetBuff);
			for( int i=0; i<(int)m_T0Buff.size(); i++ ){
				SAFE_DELETE(m_T0Buff[i]);
			}
			m_T0Buff.clear();
			UnLock1();
		}else{
			Lock2();
			m_dwT1OverFlowCount = 0;
			//SAFE_DELETE(m_T1SetBuff);
			for( int i=0; i<(int)m_T1Buff.size(); i++ ){
				SAFE_DELETE(m_T1Buff[i]);
			}
			m_T1Buff.clear();
			UnLock2();
		}
	}else{
		if( iTuner == 0 ){
			Lock3();
			m_dwS0OverFlowCount = 0;
			//SAFE_DELETE(m_S0SetBuff);
			for( int i=0; i<(int)m_S0Buff.size(); i++ ){
				SAFE_DELETE(m_S0Buff[i]);
			}
			m_S0Buff.clear();
			UnLock3();
		}else{
			Lock4();
			m_dwS1OverFlowCount = 0;
			//SAFE_DELETE(m_S1SetBuff);
			for( int i=0; i<(int)m_S1Buff.size(); i++ ){
				SAFE_DELETE(m_S1Buff[i]);
			}
			m_S1Buff.clear();
			UnLock4();
		}
	}
}

void CDataIO::Run()
{
	if( m_hThread != NULL ){
		return ;
	}
	/*
	status enStatus;

	bool bEnalbe = true;
	enStatus = m_pcDevice->GetTransferEnable(&bEnalbe);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	if( bEnalbe == true ){
		enStatus = m_pcDevice->SetTransferEnable(false);
		if( enStatus != PT::STATUS_OK ){
			return ;
		}
	}

	enStatus = m_pcDevice->SetBufferInfo(NULL);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	PT::Device::BufferInfo bufferInfo;
	bufferInfo.VirtualSize  = VIRTUAL_IMAGE_COUNT;
	bufferInfo.VirtualCount = VIRTUAL_COUNT;
	bufferInfo.LockSize     = LOCK_SIZE;
	enStatus = m_pcDevice->SetBufferInfo(&bufferInfo);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}

	if( m_bDMABuff == NULL ){
		m_bDMABuff = new BYTE[READ_BLOCK_SIZE];
	}

	// DMA 転送がどこまで進んだかを調べるため、各ブロックの末尾を 0 でクリアする
	for (uint i=0; i<VIRTUAL_COUNT; i++) {
		for (uint j=0; j<VIRTUAL_IMAGE_COUNT; j++) {
			for (uint k=0; k<READ_BLOCK_COUNT; k++) {
				Clear(i, j, k);
			}
		}
	}

	// 転送カウンタをリセットする
	enStatus = m_pcDevice->ResetTransferCounter();
	if( enStatus != PT::STATUS_OK ){
		return ;
	}

	// 転送カウンタをインクリメントする
	for (uint i=0; i<VIRTUAL_IMAGE_COUNT*VIRTUAL_COUNT; i++) {
		enStatus = m_pcDevice->IncrementTransferCounter();
		if( enStatus != PT::STATUS_OK ){
			return ;
		}

	}

	// DMA 転送を許可する
	enStatus = m_pcDevice->SetTransferEnable(true);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	*/
	::ResetEvent(m_hStopEvent);
	mQuit = false;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, (LPVOID)this, CREATE_SUSPENDED, NULL);
	SetThreadPriority( m_hThread, THREAD_PRIORITY_ABOVE_NORMAL );
	ResumeThread(m_hThread);
}
/*
void CDataIO::ResetDMA()
{
	status enStatus;

	bool bEnalbe = true;
	enStatus = m_pcDevice->GetTransferEnable(&bEnalbe);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	if( bEnalbe == true ){
		enStatus = m_pcDevice->SetTransferEnable(false);
		if( enStatus != PT::STATUS_OK ){
			return ;
		}
	}

	mVirtualIndex=0;
	mImageIndex=0;
	mBlockIndex=0;

	// DMA 転送がどこまで進んだかを調べるため、各ブロックの末尾を 0 でクリアする
	for (uint i=0; i<VIRTUAL_COUNT; i++) {
		for (uint j=0; j<VIRTUAL_IMAGE_COUNT; j++) {
			for (uint k=0; k<READ_BLOCK_COUNT; k++) {
				Clear(i, j, k);
			}
		}
	}

	// 転送カウンタをリセットする
	enStatus = m_pcDevice->ResetTransferCounter();
	if( enStatus != PT::STATUS_OK ){
		return ;
	}

	// 転送カウンタをインクリメントする
	for (uint i=0; i<VIRTUAL_IMAGE_COUNT*VIRTUAL_COUNT; i++) {
		enStatus = m_pcDevice->IncrementTransferCounter();
		if( enStatus != PT::STATUS_OK ){
			return ;
		}

	}

	// DMA 転送を許可する
	enStatus = m_pcDevice->SetTransferEnable(true);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
}
*/
void CDataIO::Stop()
{
	if( m_hThread != NULL ){
		mQuit = true;
		::SetEvent(m_hStopEvent);
		// スレッド終了待ち
		if ( ::WaitForSingleObject(m_hThread, 15000) == WAIT_TIMEOUT ){
			::TerminateThread(m_hThread, 0xffffffff);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	/*
	status enStatus;

	bool bEnalbe = true;
	enStatus = m_pcDevice->GetTransferEnable(&bEnalbe);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	if( bEnalbe == true ){
		enStatus = m_pcDevice->SetTransferEnable(false);
		if( enStatus != PT::STATUS_OK ){
			return ;
		}
	}
	*/
}

void CDataIO::StartPipeServer(int iID)
{
	int iDevID = iID>>16;
	PT::Device::ISDB enISDB = (PT::Device::ISDB)((iID&0x0000FF00)>>8);
	uint32 iTuner = iID&0x000000FF;

	wstring strPipe = L"";
	wstring strEvent = L"";
	Format(strPipe, L"%s%d", CMD_PT1_DATA_PIPE, iID );
	Format(strEvent, L"%s%d", CMD_PT1_DATA_EVENT_WAIT_CONNECT, iID );

	status enStatus = m_pcDevice->SetTransferTestMode(enISDB, iTuner);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	if( enISDB == PT::Device::ISDB_T ){
		if( iTuner == 0 ){
			Lock1();
			if( m_T0SetBuff == NULL ){
				m_T0SetBuff = new EARTH::EX::Buffer(m_pcDevice);
				m_T0SetBuff->Alloc(Size(), VIRTUAL_COUNT);
				m_T0WriteIndex = 0;

				uint8	*ptr = static_cast<uint8 *>(m_T0SetBuff->Ptr(0));
				for (uint32 i=0; i<VIRTUAL_COUNT; i++) {
					ptr[Size()*i] = NOT_SYNC_BYTE;	// 同期バイト部分に NOT_SYNC_BYTE を書き込む
					m_T0SetBuff->SyncCpu(i);		// CPU キャッシュをフラッシュ
				}

				uint64	pageAddress = m_T0SetBuff->PageDescriptorAddress();

				enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

				m_dwT0OverFlowCount = 0;
			}
			UnLock1();
			m_cPipeT0.StartServer(strEvent.c_str(), strPipe.c_str(), OutsideCmdCallbackT0, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}else{
			Lock2();
			if( m_T1SetBuff == NULL ){
				m_T1SetBuff = new EARTH::EX::Buffer(m_pcDevice);
				m_T1SetBuff->Alloc(Size(), VIRTUAL_COUNT);
				m_T1WriteIndex = 0;

				uint8	*ptr = static_cast<uint8 *>(m_T1SetBuff->Ptr(0));
				for (uint32 i=0; i<VIRTUAL_COUNT; i++) {
					ptr[Size()*i] = NOT_SYNC_BYTE;	// 同期バイト部分に NOT_SYNC_BYTE を書き込む
					m_T1SetBuff->SyncCpu(i);		// CPU キャッシュをフラッシュ
				}

				uint64	pageAddress = m_T1SetBuff->PageDescriptorAddress();

				enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

				m_dwT1OverFlowCount = 0;
			}
			UnLock2();
			m_cPipeT1.StartServer(strEvent.c_str(), strPipe.c_str(), OutsideCmdCallbackT1, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}
	}else{
		if( iTuner == 0 ){
			Lock3();
			if( m_S0SetBuff == NULL ){
				m_S0SetBuff = new EARTH::EX::Buffer(m_pcDevice);
				m_S0SetBuff->Alloc(Size(), VIRTUAL_COUNT);
				m_S0WriteIndex = 0;

				uint8	*ptr = static_cast<uint8 *>(m_S0SetBuff->Ptr(0));
				for (uint32 i=0; i<VIRTUAL_COUNT; i++) {
					ptr[Size()*i] = NOT_SYNC_BYTE;	// 同期バイト部分に NOT_SYNC_BYTE を書き込む
					m_S0SetBuff->SyncCpu(i);		// CPU キャッシュをフラッシュ
				}

				uint64	pageAddress = m_S0SetBuff->PageDescriptorAddress();

				enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

				m_dwS0OverFlowCount = 0;
			}
			UnLock3();
			m_cPipeS0.StartServer(strEvent.c_str(), strPipe.c_str(), OutsideCmdCallbackS0, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}else{
			Lock4();
			if( m_S1SetBuff == NULL ){
				m_S1SetBuff = new EARTH::EX::Buffer(m_pcDevice);
				m_S1SetBuff->Alloc(Size(), VIRTUAL_COUNT);
				m_S1WriteIndex = 0;

				uint8	*ptr = static_cast<uint8 *>(m_S1SetBuff->Ptr(0));
				for (uint32 i=0; i<VIRTUAL_COUNT; i++) {
					ptr[Size()*i] = NOT_SYNC_BYTE;	// 同期バイト部分に NOT_SYNC_BYTE を書き込む
					m_S1SetBuff->SyncCpu(i);		// CPU キャッシュをフラッシュ
				}

				uint64	pageAddress = m_S1SetBuff->PageDescriptorAddress();

				enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

				m_dwS1OverFlowCount = 0;
			}
			UnLock4();
			m_cPipeS1.StartServer(strEvent.c_str(), strPipe.c_str(), OutsideCmdCallbackS1, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}
	}
}

void CDataIO::StopPipeServer(int iID)
{
	int iDevID = iID>>16;
	PT::Device::ISDB enISDB = (PT::Device::ISDB)((iID&0x0000FF00)>>8);
	uint32 iTuner = iID&0x000000FF;

	if( enISDB == PT::Device::ISDB_T ){
		if( iTuner == 0 ){
			m_cPipeT0.StopServer();
			Lock1();
			m_dwT0OverFlowCount = 0;
			SAFE_DELETE(m_T0SetBuff);
			for( int i=0; i<(int)m_T0Buff.size(); i++ ){
				SAFE_DELETE(m_T0Buff[i]);
			}
			m_T0Buff.clear();
			UnLock1();
		}else{
			m_cPipeT1.StopServer();
			Lock2();
			m_dwT1OverFlowCount = 0;
			SAFE_DELETE(m_T1SetBuff);
			for( int i=0; i<(int)m_T1Buff.size(); i++ ){
				SAFE_DELETE(m_T1Buff[i]);
			}
			m_T1Buff.clear();
			UnLock2();
		}
	}else{
		if( iTuner == 0 ){
			m_cPipeS0.StopServer();
			Lock3();
			m_dwS0OverFlowCount = 0;
			SAFE_DELETE(m_S0SetBuff);
			for( int i=0; i<(int)m_S0Buff.size(); i++ ){
				SAFE_DELETE(m_S0Buff[i]);
			}
			m_S0Buff.clear();
			UnLock3();
		}else{
			m_cPipeS1.StopServer();
			Lock4();
			m_dwS1OverFlowCount = 0;
			SAFE_DELETE(m_S1SetBuff);
			for( int i=0; i<(int)m_S1Buff.size(); i++ ){
				SAFE_DELETE(m_S1Buff[i]);
			}
			m_S1Buff.clear();
			UnLock4();
		}
	}
}

BOOL CDataIO::EnableTuner(int iID, BOOL bEnable)
{
	int iDevID = iID>>16;
	PT::Device::ISDB enISDB = (PT::Device::ISDB)((iID&0x0000FF00)>>8);
	uint32 iTuner = iID&0x000000FF;

	status enStatus = PT::STATUS_OK;

	if( bEnable == TRUE ){

		if( enISDB == PT::Device::ISDB_T ){
			if( iTuner == 0 ){
				Lock1();
				/*if( m_T0SetBuff == NULL ){
					m_T0SetBuff = new EARTH::EX::Buffer(m_pcDevice);
					m_T0SetBuff->Alloc(Size());
					m_T0WriteIndex = 0;

					uint64	pageAddress = m_T0SetBuff->PageDescriptorAddress();

					enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwT0OverFlowCount = 0;
				}*/
				if( m_T0SetBuff != NULL ){
					/*uint8	*ptr = static_cast<uint8 *>(m_T0SetBuff->Ptr());
					for( size_t i=0; i<Size(); i+= DATA_BUFF_SIZE){
						ptr[i] = NOT_SYNC_BYTE;
					}
					*/
					//uint64	pageAddress = m_T0SetBuff->PageDescriptorAddress();

					//enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwT0OverFlowCount = 0;
					m_T0WriteIndex = 0;
				}
				UnLock1();
			}else{
				Lock2();
				/*if( m_T1SetBuff == NULL ){
					m_T1SetBuff = new EARTH::EX::Buffer(m_pcDevice);
					m_T1SetBuff->Alloc(Size());
					m_T1WriteIndex = 0;

					uint64	pageAddress = m_T1SetBuff->PageDescriptorAddress();

					enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwT1OverFlowCount = 0;
				}*/
				if( m_T1SetBuff != NULL ){
					/*uint8	*ptr = static_cast<uint8 *>(m_T1SetBuff->Ptr());
					for( size_t i=0; i<Size(); i+= DATA_BUFF_SIZE){
						ptr[i] = NOT_SYNC_BYTE;
					}
					*/
					//uint64	pageAddress = m_T1SetBuff->PageDescriptorAddress();

					//enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwT1OverFlowCount = 0;
					m_T1WriteIndex = 0;
				}
				UnLock2();
			}
		}else{
			if( iTuner == 0 ){
				Lock3();
				/*if( m_S0SetBuff == NULL ){
					m_S0SetBuff = new EARTH::EX::Buffer(m_pcDevice);
					m_S0SetBuff->Alloc(Size());
					m_S0WriteIndex = 0;

					uint64	pageAddress = m_S0SetBuff->PageDescriptorAddress();

					enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwS0OverFlowCount = 0;
				}*/
				if( m_S0SetBuff != NULL ){
					/*uint8	*ptr = static_cast<uint8 *>(m_S0SetBuff->Ptr());
					for( size_t i=0; i<Size(); i+= DATA_BUFF_SIZE){
						ptr[i] = NOT_SYNC_BYTE;
					}
					*/
					//uint64	pageAddress = m_S0SetBuff->PageDescriptorAddress();

					//enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwS0OverFlowCount = 0;
					m_S0WriteIndex = 0;
				}
				UnLock3();
			}else{
				Lock4();
				/*if( m_S1SetBuff == NULL ){
					m_S1SetBuff = new EARTH::EX::Buffer(m_pcDevice);
					m_S1SetBuff->Alloc(Size());
					m_S1WriteIndex = 0;

					uint64	pageAddress = m_S1SetBuff->PageDescriptorAddress();

					enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwS1OverFlowCount = 0;

				}*/
				if( m_S1SetBuff != NULL ){
					/*uint8	*ptr = static_cast<uint8 *>(m_S1SetBuff->Ptr());
					for( size_t i=0; i<Size(); i+= DATA_BUFF_SIZE){
						ptr[i] = NOT_SYNC_BYTE;
					}
					*/
					//uint64	pageAddress = m_S1SetBuff->PageDescriptorAddress();

					//enStatus = m_pcDevice->SetTransferPageDescriptorAddress(enISDB, iTuner, pageAddress);

					m_dwS1OverFlowCount = 0;
					m_S1WriteIndex = 0;
				}
				UnLock4();
			}
		}
//		if( enStatus != PT::STATUS_OK ){
//			_OutputDebugString(L"★SetTransferEnabled true err");
//			return FALSE;
//		}
		enStatus = m_pcDevice->SetTransferTestMode(enISDB, iTuner);
		enStatus = m_pcDevice->SetTransferEnabled(enISDB, iTuner, true);
		_OutputDebugString(L"Device::SetTransferEnabled ISDB:%d tuner:%d enable:true●",enISDB,iTuner);
		if( enStatus != PT::STATUS_OK ){
			_OutputDebugString(L"★SetTransferEnabled true err");
			return FALSE;
		}
	}else{
		enStatus = m_pcDevice->SetTransferEnabled(enISDB, iTuner, false);
		_OutputDebugString(L"Device::SetTransferEnabled ISDB:%d tuner:%d enable:false■",enISDB,iTuner);
		if( enStatus != PT::STATUS_OK ){
			_OutputDebugString(L"★SetTransferEnabled false err");
//			return FALSE;
		}
		if( enISDB == PT::Device::ISDB_T ){
			if( iTuner == 0 ){
				Lock1();
				m_dwT0OverFlowCount = 0;
				//SAFE_DELETE(m_T0SetBuff);
				for( int i=0; i<(int)m_T0Buff.size(); i++ ){
					SAFE_DELETE(m_T0Buff[i]);
				}
				m_T0Buff.clear();
				UnLock1();
			}else{
				Lock2();
				m_dwT1OverFlowCount = 0;
				//SAFE_DELETE(m_T1SetBuff);
				for( int i=0; i<(int)m_T1Buff.size(); i++ ){
					SAFE_DELETE(m_T1Buff[i]);
				}
				m_T1Buff.clear();
				UnLock2();
			}
		}else{
			if( iTuner == 0 ){
				Lock3();
				m_dwS0OverFlowCount = 0;
				//SAFE_DELETE(m_S0SetBuff);
				for( int i=0; i<(int)m_S0Buff.size(); i++ ){
					SAFE_DELETE(m_S0Buff[i]);
				}
				m_S0Buff.clear();
				UnLock3();
			}else{
				Lock4();
				m_dwS1OverFlowCount = 0;
				//SAFE_DELETE(m_S1SetBuff);
				for( int i=0; i<(int)m_S1Buff.size(); i++ ){
					SAFE_DELETE(m_S1Buff[i]);
				}
				m_S1Buff.clear();
				UnLock4();
			}
		}
	}
	return TRUE;
}

void CDataIO::ChkTransferInfo()
{
	BOOL err = FALSE;
	PT::Device::TransferInfo transferInfo;

	if( m_T0SetBuff != NULL ){
		ZeroMemory(&transferInfo, sizeof(PT::Device::TransferInfo));
		m_pcDevice->GetTransferInfo((PT::Device::ISDB)0, 0, &transferInfo);

		if( transferInfo.InternalFIFO_A_Overflow == true ||
			transferInfo.InternalFIFO_A_Underflow == true ||
			transferInfo.InternalFIFO_B_Overflow == true ||
			transferInfo.InternalFIFO_B_Underflow == true ||
			transferInfo.ExternalFIFO_Overflow == true ||
			transferInfo.Status >= 0x100
			){
				_OutputDebugString(L"★TransferInfo err : isdb:%d, tunerIndex:%d status:%d InternalFIFO_A_Overflow:%d InternalFIFO_A_Underflow:%d InternalFIFO_B_Overflow:%d InternalFIFO_B_Underflow:%d ExternalFIFO_Overflow:%d",
					0, 0,transferInfo.Status,
					transferInfo.InternalFIFO_A_Overflow,
					transferInfo.InternalFIFO_A_Underflow,
					transferInfo.InternalFIFO_B_Overflow,
					transferInfo.InternalFIFO_B_Underflow,
					transferInfo.ExternalFIFO_Overflow
					);
				err = TRUE;
		}
	}
	if( m_T1SetBuff != NULL ){
		ZeroMemory(&transferInfo, sizeof(PT::Device::TransferInfo));
		m_pcDevice->GetTransferInfo((PT::Device::ISDB)0, 1, &transferInfo);

		if( transferInfo.InternalFIFO_A_Overflow == true ||
			transferInfo.InternalFIFO_A_Underflow == true ||
			transferInfo.InternalFIFO_B_Overflow == true ||
			transferInfo.InternalFIFO_B_Underflow == true ||
			transferInfo.ExternalFIFO_Overflow == true ||
			transferInfo.Status >= 0x100
			){
				_OutputDebugString(L"★TransferInfo err : isdb:%d, tunerIndex:%d status:%d InternalFIFO_A_Overflow:%d InternalFIFO_A_Underflow:%d InternalFIFO_B_Overflow:%d InternalFIFO_B_Underflow:%d ExternalFIFO_Overflow:%d",
					0, 1,transferInfo.Status,
					transferInfo.InternalFIFO_A_Overflow,
					transferInfo.InternalFIFO_A_Underflow,
					transferInfo.InternalFIFO_B_Overflow,
					transferInfo.InternalFIFO_B_Underflow,
					transferInfo.ExternalFIFO_Overflow
					);
				err = TRUE;
		}
	}
	if( m_S0SetBuff != NULL ){
		ZeroMemory(&transferInfo, sizeof(PT::Device::TransferInfo));
		m_pcDevice->GetTransferInfo((PT::Device::ISDB)1, 0, &transferInfo);

		if( transferInfo.InternalFIFO_A_Overflow == true ||
			transferInfo.InternalFIFO_A_Underflow == true ||
			transferInfo.InternalFIFO_B_Overflow == true ||
			transferInfo.InternalFIFO_B_Underflow == true ||
			transferInfo.ExternalFIFO_Overflow == true ||
			transferInfo.Status >= 0x100 
			){
				_OutputDebugString(L"★TransferInfo err : isdb:%d, tunerIndex:%d status:%d InternalFIFO_A_Overflow:%d InternalFIFO_A_Underflow:%d InternalFIFO_B_Overflow:%d InternalFIFO_B_Underflow:%d ExternalFIFO_Overflow:%d",
					1, 0,transferInfo.Status,
					transferInfo.InternalFIFO_A_Overflow,
					transferInfo.InternalFIFO_A_Underflow,
					transferInfo.InternalFIFO_B_Overflow,
					transferInfo.InternalFIFO_B_Underflow,
					transferInfo.ExternalFIFO_Overflow
					);
				err = TRUE;
		}
	}
	if( m_S1SetBuff != NULL ){
		ZeroMemory(&transferInfo, sizeof(PT::Device::TransferInfo));
		m_pcDevice->GetTransferInfo((PT::Device::ISDB)1, 1, &transferInfo);

		if( transferInfo.InternalFIFO_A_Overflow == true ||
			transferInfo.InternalFIFO_A_Underflow == true ||
			transferInfo.InternalFIFO_B_Overflow == true ||
			transferInfo.InternalFIFO_B_Underflow == true ||
			transferInfo.ExternalFIFO_Overflow == true ||
			transferInfo.Status >= 0x100 
			){
				_OutputDebugString(L"★TransferInfo err : isdb:%d, tunerIndex:%d status:%d InternalFIFO_A_Overflow:%d InternalFIFO_A_Underflow:%d InternalFIFO_B_Overflow:%d InternalFIFO_B_Underflow:%d ExternalFIFO_Overflow:%d",
					1, 1,transferInfo.Status,
					transferInfo.InternalFIFO_A_Overflow,
					transferInfo.InternalFIFO_A_Underflow,
					transferInfo.InternalFIFO_B_Overflow,
					transferInfo.InternalFIFO_B_Underflow,
					transferInfo.ExternalFIFO_Overflow
					);
				err = TRUE;
		}
	}

	if( err == TRUE ){
		for(int i=0; i<2; i++ ){
			for(int j=0; j<2; j++ ){
				int iID = (i<<8) | (j&0x000000FF);
				EnableTuner(iID, false);
			}
		}
		if( m_T0SetBuff != NULL ){
			int iID = (0<<8) | (0&0x000000FF);
			EnableTuner(iID, true);
		}
		if( m_T1SetBuff != NULL ){
			int iID = (0<<8) | (1&0x000000FF);
			EnableTuner(iID, true);
		}
		if( m_S0SetBuff != NULL ){
			int iID = (1<<8) | (0&0x000000FF);
			EnableTuner(iID, true);
		}
		if( m_S1SetBuff != NULL ){
			int iID = (1<<8) | (1&0x000000FF);
			EnableTuner(iID, true);
		}
	}
}

bool CDataIO::CheckReady(EARTH::EX::Buffer* buffer, uint32 index)
{
	status status = PT::STATUS_OK;
	//status = buffer->SyncIo(index);
	uint32 nextIndex = index+1;
	if( nextIndex >= VIRTUAL_COUNT ){
		nextIndex = 0;
	}
	volatile uint8	*ptr  = static_cast<uint8 *>(buffer->Ptr(nextIndex));
	uint8	data = ptr[0];
	status = buffer->SyncCpu(nextIndex);	// キャッシュライン全体に渡って正しいデータを読み込んだとは限らないため、CPU キャッシュをフラッシュ
	if( status != PT::STATUS_OK){
		return false;
	}

	if (data == 0x47) return true;
	if (data == NOT_SYNC_BYTE) return false;

	return false;
}

bool CDataIO::ReadAddBuff(EARTH::EX::Buffer* buffer, uint32 index, vector<BUFF_DATA*>* tsBuff)
{
	status	status = PT::STATUS_OK;
	status = buffer->SyncIo(index);
	if( status != PT::STATUS_OK){
		return false;
	}
	uint8	*ptr  = static_cast<uint8 *>(buffer->Ptr(index));
	/*
	HANDLE hFile = CreateFile(L"test.ts", GENERIC_WRITE , FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, 0, FILE_END);
	DWORD w;
	WriteFile(hFile, ptr, Size(), &w, NULL);
	CloseHandle(hFile);
	*/
	for( uint32 i=0; i<Size(); i+=DATA_BUFF_SIZE ){
		BUFF_DATA* pDataBuff = new BUFF_DATA;
		pDataBuff->dwSize = DATA_BUFF_SIZE;
		pDataBuff->pbBuff = new BYTE[pDataBuff->dwSize];

		memcpy(pDataBuff->pbBuff, ptr+i, pDataBuff->dwSize);
		tsBuff->push_back(pDataBuff);
	}

	ptr[0] = NOT_SYNC_BYTE;
	status = buffer->SyncCpu(index);

	return true;
}

UINT WINAPI CDataIO::RecvThread(LPVOID pParam)
{
	CDataIO* pSys = (CDataIO*)pParam;

	HANDLE hCurThread = GetCurrentThread();
	SetThreadPriority(hCurThread, THREAD_PRIORITY_HIGHEST);

	pSys->mVirtualIndex = 0;
	pSys->mImageIndex = 0;
	pSys->mBlockIndex = 0;

	uint32 maxCount = pSys->Size()/DATA_BUFF_SIZE;
	int chkCount = 0;

	while (true) {
		DWORD dwRes = WaitForSingleObject(pSys->m_hStopEvent, 1);
		if( dwRes == WAIT_OBJECT_0 ){
			//STOP
			break;
		}
		/*
		chkCount++;
		if( chkCount>50 ){
			pSys->ChkTransferInfo();
			chkCount = 0;
		}
		*/
		pSys->Lock1();
		if( pSys->m_T0SetBuff != NULL ){
			if( pSys->CheckReady(pSys->m_T0SetBuff, pSys->m_T0WriteIndex) == true ){
				if( pSys->ReadAddBuff(pSys->m_T0SetBuff, pSys->m_T0WriteIndex, &pSys->m_T0Buff) == true ){
					if( pSys->m_T0Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(pSys->m_T0Buff[0]);
						pSys->m_T0Buff.erase(pSys->m_T0Buff.begin());
						pSys->m_dwT0OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						pSys->m_dwT0OverFlowCount = 0;
					}

					pSys->m_T0WriteIndex++;
					if (pSys->VIRTUAL_COUNT <= pSys->m_T0WriteIndex) {
						pSys->m_T0WriteIndex = 0;
					}
				}
			}
			/*
			uint8	*ptr = static_cast<uint8 *>(pSys->m_T0SetBuff->Ptr());
			uint8	*startPtr = ptr + DATA_BUFF_SIZE * pSys->m_T0WriteIndex;
			uint32	nextWriteIndex = pSys->m_T0WriteIndex + 1;
			if (maxCount <= nextWriteIndex) {
				nextWriteIndex = 0;
			}
			uint8	*nextPtr  = ptr + DATA_BUFF_SIZE * nextWriteIndex;
			if (startPtr[0] == 0x47 && nextPtr[0] == 0x47){
				BUFF_DATA* pDataBuff = new BUFF_DATA;
				pDataBuff->dwSize = DATA_BUFF_SIZE;
				pDataBuff->pbBuff = new BYTE[pDataBuff->dwSize];

				memcpy(pDataBuff->pbBuff, startPtr, pDataBuff->dwSize);
				pSys->m_T0Buff.push_back(pDataBuff);
				if( pSys->m_T0Buff.size() > MAX_DATA_BUFF_COUNT ){
					SAFE_DELETE(pSys->m_T0Buff[0]);
					pSys->m_T0Buff.erase(pSys->m_T0Buff.begin());
					pSys->m_dwT0OverFlowCount++;
					OutputDebugString(L"Buff Full");
				}else{
					pSys->m_dwT0OverFlowCount = 0;
				}

				startPtr[0] = NOT_SYNC_BYTE;

				pSys->m_T0WriteIndex++;
				if (maxCount <= pSys->m_T0WriteIndex) {
					pSys->m_T0WriteIndex = 0;
				}
			}
			*/
		}
		pSys->UnLock1();

		pSys->Lock2();
		if( pSys->m_T1SetBuff != NULL ){
			if( pSys->CheckReady(pSys->m_T1SetBuff, pSys->m_T1WriteIndex) == true ){
				if( pSys->ReadAddBuff(pSys->m_T1SetBuff, pSys->m_T1WriteIndex, &pSys->m_T1Buff) == true ){
					if( pSys->m_T1Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(pSys->m_T1Buff[0]);
						pSys->m_T1Buff.erase(pSys->m_T1Buff.begin());
						pSys->m_dwT1OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						pSys->m_dwT1OverFlowCount = 0;
					}

					pSys->m_T1WriteIndex++;
					if (pSys->VIRTUAL_COUNT <= pSys->m_T1WriteIndex) {
						pSys->m_T1WriteIndex = 0;
					}
				}
			}

			/*
			uint8	*ptr = static_cast<uint8 *>(pSys->m_T1SetBuff->Ptr());
			uint8	*startPtr = ptr + DATA_BUFF_SIZE * pSys->m_T1WriteIndex;
			uint32	nextWriteIndex = pSys->m_T1WriteIndex + 1;
			if (maxCount <= nextWriteIndex) {
				nextWriteIndex = 0;
			}
			uint8	*nextPtr  = ptr + DATA_BUFF_SIZE * nextWriteIndex;
			if (startPtr[0] == 0x47 && nextPtr[0] == 0x47){
				BUFF_DATA* pDataBuff = new BUFF_DATA;
				pDataBuff->dwSize = DATA_BUFF_SIZE;
				pDataBuff->pbBuff = new BYTE[pDataBuff->dwSize];

				memcpy(pDataBuff->pbBuff, startPtr, pDataBuff->dwSize);
				pSys->m_T1Buff.push_back(pDataBuff);
				if( pSys->m_T1Buff.size() > MAX_DATA_BUFF_COUNT ){
					SAFE_DELETE(pSys->m_T1Buff[0]);
					pSys->m_T1Buff.erase(pSys->m_T1Buff.begin());
					pSys->m_dwT1OverFlowCount++;
					OutputDebugString(L"Buff Full");
				}else{
					pSys->m_dwT1OverFlowCount = 0;
				}

				startPtr[0] = NOT_SYNC_BYTE;

				pSys->m_T1WriteIndex++;
				if (maxCount <= pSys->m_T1WriteIndex) {
					pSys->m_T1WriteIndex = 0;
				}
			}
			*/
		}
		pSys->UnLock2();


		pSys->Lock3();
		if( pSys->m_S0SetBuff != NULL ){
			if( pSys->CheckReady(pSys->m_S0SetBuff, pSys->m_S0WriteIndex) == true ){
				if( pSys->ReadAddBuff(pSys->m_S0SetBuff, pSys->m_S0WriteIndex, &pSys->m_S0Buff) == true ){
					if( pSys->m_S0Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(pSys->m_S0Buff[0]);
						pSys->m_S0Buff.erase(pSys->m_S0Buff.begin());
						pSys->m_dwS0OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						pSys->m_dwS0OverFlowCount = 0;
					}

					pSys->m_S0WriteIndex++;
					if (pSys->VIRTUAL_COUNT <= pSys->m_S0WriteIndex) {
						pSys->m_S0WriteIndex = 0;
					}
				}
			}
			/*
			uint8	*ptr = static_cast<uint8 *>(pSys->m_S0SetBuff->Ptr());
			uint8	*startPtr = ptr + DATA_BUFF_SIZE * pSys->m_S0WriteIndex;
			uint32	nextWriteIndex = pSys->m_S0WriteIndex + 1;
			if (maxCount <= nextWriteIndex) {
				nextWriteIndex = 0;
			}
			uint8	*nextPtr  = ptr + DATA_BUFF_SIZE * nextWriteIndex;
			if (startPtr[0] == 0x47 && nextPtr[0] == 0x47){
				BUFF_DATA* pDataBuff = new BUFF_DATA;
				pDataBuff->dwSize = DATA_BUFF_SIZE;
				pDataBuff->pbBuff = new BYTE[pDataBuff->dwSize];

				memcpy(pDataBuff->pbBuff, startPtr, pDataBuff->dwSize);
				pSys->m_S0Buff.push_back(pDataBuff);
				if( pSys->m_S0Buff.size() > MAX_DATA_BUFF_COUNT ){
					SAFE_DELETE(pSys->m_S0Buff[0]);
					pSys->m_S0Buff.erase(pSys->m_S0Buff.begin());
					pSys->m_dwS0OverFlowCount++;
					OutputDebugString(L"Buff Full");
				}else{
					pSys->m_dwS0OverFlowCount = 0;
				}

				startPtr[0] = NOT_SYNC_BYTE;

				pSys->m_S0WriteIndex++;
				if (maxCount <= pSys->m_S0WriteIndex) {
					pSys->m_S0WriteIndex = 0;
				}
			}
			*/
		}
		pSys->UnLock3();

		pSys->Lock4();
		if( pSys->m_S1SetBuff != NULL ){
			if( pSys->CheckReady(pSys->m_S1SetBuff, pSys->m_S1WriteIndex) == true ){
				if( pSys->ReadAddBuff(pSys->m_S1SetBuff, pSys->m_S1WriteIndex, &pSys->m_S1Buff) == true ){
					if( pSys->m_S1Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(pSys->m_S1Buff[0]);
						pSys->m_S1Buff.erase(pSys->m_S1Buff.begin());
						pSys->m_dwS1OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						pSys->m_dwS1OverFlowCount = 0;
					}

					pSys->m_S1WriteIndex++;
					if (pSys->VIRTUAL_COUNT <= pSys->m_S1WriteIndex) {
						pSys->m_S1WriteIndex = 0;
					}
				}
			}
			/*
			uint8	*ptr = static_cast<uint8 *>(pSys->m_S1SetBuff->Ptr());
			uint8	*startPtr = ptr + DATA_BUFF_SIZE * pSys->m_S1WriteIndex;
			uint32	nextWriteIndex = pSys->m_S1WriteIndex + 1;
			if (maxCount <= nextWriteIndex) {
				nextWriteIndex = 0;
			}
			uint8	*nextPtr  = ptr + DATA_BUFF_SIZE * nextWriteIndex;
			if (startPtr[0] == 0x47 && nextPtr[0] == 0x47){
				BUFF_DATA* pDataBuff = new BUFF_DATA;
				pDataBuff->dwSize = DATA_BUFF_SIZE;
				pDataBuff->pbBuff = new BYTE[pDataBuff->dwSize];

				memcpy(pDataBuff->pbBuff, startPtr, pDataBuff->dwSize);
				pSys->m_S1Buff.push_back(pDataBuff);
				if( pSys->m_S1Buff.size() > MAX_DATA_BUFF_COUNT ){
					SAFE_DELETE(pSys->m_S1Buff[0]);
					pSys->m_S1Buff.erase(pSys->m_S1Buff.begin());
					pSys->m_dwS1OverFlowCount++;
					OutputDebugString(L"Buff Full");
				}else{
					pSys->m_dwS1OverFlowCount = 0;
				}

				startPtr[0] = NOT_SYNC_BYTE;

				pSys->m_S1WriteIndex++;
				if (maxCount <= pSys->m_S1WriteIndex) {
					pSys->m_S1WriteIndex = 0;
				}
			}
			*/
		}
		pSys->UnLock4();
		/*
		bool b;
		
		b = pSys->WaitBlock();
		if (b == false) break;

		pSys->CopyBlock();

		b = pSys->DispatchBlock();
		if (b == false) break;
		*/
	}

	return 0;
}
/*
// 1ブロック分 DMA 転送が終わるか mQuit が true になるまで待つ
bool CDataIO::WaitBlock()
{
	bool b = true;

	while (true) {
		if (mQuit) {
			b = false;
			break;
		}

		// ブロックの末尾が 0 でなければ、そのブロックの DMA 転送が完了したことになる
		if (Read(mVirtualIndex, mImageIndex, mBlockIndex) != 0) break;
		Sleep(3);
	}
	//::wprintf(L"(mVirtualIndex, mImageIndex, mBlockIndex) = (%d, %d, %d) の転送が終わりました。\n", mVirtualIndex, mImageIndex, mBlockIndex);

	return b;
}

// 1ブロック分のデータをテンポラリ領域にコピーする。CPU 側から見て DMA バッファはキャッシュが効かないため、
// キャッシュが効くメモリ領域にコピーしてからアクセスすると効率が高まります。
void CDataIO::CopyBlock()
{
	status enStatus;

	void *voidPtr;
	enStatus = m_pcDevice->GetBufferPtr(mVirtualIndex, &voidPtr);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}
	DWORD dwOffset = ((TRANSFER_SIZE*mImageIndex) + (READ_BLOCK_SIZE*mBlockIndex));

	memcpy(m_bDMABuff, (BYTE*)voidPtr+dwOffset, READ_BLOCK_SIZE);

	// コピーし終わったので、ブロックの末尾を 0 にします。
	uint *ptr = static_cast<uint *>(voidPtr);
	ptr[Offset(mImageIndex, mBlockIndex, READ_BLOCK_SIZE)-1] = 0;

	if (READ_BLOCK_COUNT <= ++mBlockIndex) {
		mBlockIndex = 0;

		// 転送カウンタは OS::Memory::PAGE_SIZE * PT::Device::BUFFER_PAGE_COUNT バイトごとにインクリメントします。
		enStatus = m_pcDevice->IncrementTransferCounter();
		if( enStatus != PT::STATUS_OK ){
			return ;
		}

		if (VIRTUAL_IMAGE_COUNT <= ++mImageIndex) {
			mImageIndex = 0;
			if (VIRTUAL_COUNT <= ++mVirtualIndex) {
				mVirtualIndex = 0;
			}
		}
	}
}

void CDataIO::Clear(uint virtualIndex, uint imageIndex, uint blockIndex)
{
	void *voidPtr;
	status enStatus = m_pcDevice->GetBufferPtr(virtualIndex, &voidPtr);
	if( enStatus != PT::STATUS_OK ){
		return ;
	}

	uint *ptr = static_cast<uint *>(voidPtr);
	ptr[Offset(imageIndex, blockIndex, READ_BLOCK_SIZE)-1] = 0;
}

uint CDataIO::Read(uint virtualIndex, uint imageIndex, uint blockIndex) const
{
	void *voidPtr;
	status enStatus = m_pcDevice->GetBufferPtr(virtualIndex, &voidPtr);
	if( enStatus != PT::STATUS_OK ){
		return 0;
	}

	volatile const uint *ptr = static_cast<volatile const uint *>(voidPtr);
	return ptr[Offset(imageIndex, blockIndex, READ_BLOCK_SIZE)-1];
}

uint CDataIO::Offset(uint imageIndex, uint blockIndex, uint additionalOffset) const
{
	uint offset = ((TRANSFER_SIZE*imageIndex) + (READ_BLOCK_SIZE*blockIndex) + additionalOffset) / sizeof(uint);

	return offset;
}

bool CDataIO::DispatchBlock()
{
	const uint *ptr = (uint*)m_bDMABuff;

	for (uint i=0; i<READ_BLOCK_SIZE; i+=4) {
		uint packetError = BIT_SHIFT_MASK(m_bDMABuff[i+3], 0, 1);

		if (packetError) {
			// エラーの原因を調べる
			PT::Device::TransferInfo info;
			status enStatus = m_pcDevice->GetTransferInfo(&info);
			if( enStatus == PT::STATUS_OK ){
				if (info.TransferCounter0) {
					OutputDebugString(L"★転送カウンタが 0 であるのを検出した。");
					ResetDMA();
					break;
				} else if (info.TransferCounter1) {
					OutputDebugString(L"★転送カウンタが 1 以下になりました。");
					ResetDMA();
					break;
				} else if (info.BufferOverflow) {
					OutputDebugString(L"★PCI バスを長期に渡り確保できなかったため、ボード上の FIFO が溢れました。");
					ResetDMA();
					break;
				} else {
					OutputDebugString(L"★転送エラーが発生しました。");
					break;
				}
			}else{
				OutputDebugString(L"GetTransferInfo() err");
				break;
			}
		}else{
			MicroPacket(m_bDMABuff+i);
		}
	}

	return true;
}

void CDataIO::MicroPacket(BYTE* pbPacket)
{
	uint packetId      = BIT_SHIFT_MASK(pbPacket[3], 5,  3);

	BOOL bCreate1TS = FALSE;
	switch(packetId){
		case 2:
			bCreate1TS = m_cT0Micro.MicroPacket(pbPacket);
			if( bCreate1TS == TRUE && m_T0SetBuff != NULL){
				Lock1();
				if( m_T0SetBuff == NULL ){
					UnLock1();
					return ;
				}
				memcpy(m_T0SetBuff->pbBuff+m_T0SetBuff->dwSetSize, m_cT0Micro.Get1TS(), 188);
				m_T0SetBuff->dwSetSize+=188;
				if( m_T0SetBuff->dwSetSize >= m_T0SetBuff->dwSize ){
					m_T0Buff.push_back(m_T0SetBuff);

					BUFF_DATA* pDataBuff = new BUFF_DATA;
					pDataBuff->dwSize = DATA_BUFF_SIZE;
					pDataBuff->pbBuff = new BYTE[DATA_BUFF_SIZE];
					m_T0SetBuff = pDataBuff;

					if( m_T0Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(m_T0Buff[0]);
						m_T0Buff.erase(m_T0Buff.begin());
						m_dwT0OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						m_dwT0OverFlowCount = 0;
					}
				}
				UnLock1();
			}
			break;
		case 4:
			bCreate1TS = m_cT1Micro.MicroPacket(pbPacket);
			if( bCreate1TS == TRUE && m_T1SetBuff != NULL){
				Lock2();
				if( m_T1SetBuff == NULL ){
					UnLock2();
					return ;
				}
				memcpy(m_T1SetBuff->pbBuff+m_T1SetBuff->dwSetSize, m_cT1Micro.Get1TS(), 188);
				m_T1SetBuff->dwSetSize+=188;
				if( m_T1SetBuff->dwSetSize >= m_T1SetBuff->dwSize ){
					m_T1Buff.push_back(m_T1SetBuff);

					BUFF_DATA* pDataBuff = new BUFF_DATA;
					pDataBuff->dwSize = DATA_BUFF_SIZE;
					pDataBuff->pbBuff = new BYTE[DATA_BUFF_SIZE];
					m_T1SetBuff = pDataBuff;

					if( m_T1Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(m_T1Buff[0]);
						m_T1Buff.erase(m_T1Buff.begin());
						m_dwT1OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						m_dwT1OverFlowCount = 0;
					}
				}
				UnLock2();
			}
			break;
		case 1:
			bCreate1TS = m_cS0Micro.MicroPacket(pbPacket);
			if( bCreate1TS == TRUE && m_S0SetBuff != NULL){
				Lock3();
				if( m_S0SetBuff == NULL ){
					UnLock3();
					return ;
				}
				memcpy(m_S0SetBuff->pbBuff+m_S0SetBuff->dwSetSize, m_cS0Micro.Get1TS(), 188);
				m_S0SetBuff->dwSetSize+=188;
				if( m_S0SetBuff->dwSetSize >= m_S0SetBuff->dwSize ){
					m_S0Buff.push_back(m_S0SetBuff);

					BUFF_DATA* pDataBuff = new BUFF_DATA;
					pDataBuff->dwSize = DATA_BUFF_SIZE;
					pDataBuff->pbBuff = new BYTE[DATA_BUFF_SIZE];
					m_S0SetBuff = pDataBuff;

					if( m_S0Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(m_S0Buff[0]);
						m_S0Buff.erase(m_S0Buff.begin());
						m_dwS0OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						m_dwS0OverFlowCount = 0;
					}
				}
				UnLock3();
			}
			break;
		case 3:
			bCreate1TS = m_cS1Micro.MicroPacket(pbPacket);
			if( bCreate1TS == TRUE && m_S1SetBuff != NULL){
				Lock4();
				if( m_S1SetBuff == NULL ){
					UnLock4();
					return ;
				}
				memcpy(m_S1SetBuff->pbBuff+m_S1SetBuff->dwSetSize, m_cS1Micro.Get1TS(), 188);
				m_S1SetBuff->dwSetSize+=188;
				if( m_S1SetBuff->dwSetSize >= m_S1SetBuff->dwSize ){
					m_S1Buff.push_back(m_S1SetBuff);

					BUFF_DATA* pDataBuff = new BUFF_DATA;
					pDataBuff->dwSize = DATA_BUFF_SIZE;
					pDataBuff->pbBuff = new BYTE[DATA_BUFF_SIZE];
					m_S1SetBuff = pDataBuff;

					if( m_S1Buff.size() > MAX_DATA_BUFF_COUNT ){
						SAFE_DELETE(m_S1Buff[0]);
						m_S1Buff.erase(m_S1Buff.begin());
						m_dwS1OverFlowCount++;
						OutputDebugString(L"Buff Full");
					}else{
						m_dwS1OverFlowCount = 0;
					}
				}
				UnLock4();
			}
			break;
		default:
			return;
			break;
	}
}
*/
int CALLBACK CDataIO::OutsideCmdCallbackT0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	CDataIO* pSys = (CDataIO*)pParam;
	switch( pCmdParam->dwParam ){
		case CMD_SEND_DATA:
			pSys->CmdSendData(0, pCmdParam, pResParam);
			break;
		default:
			pResParam->dwParam = CMD_NON_SUPPORT;
			break;
	}
	return 0;
}

int CALLBACK CDataIO::OutsideCmdCallbackT1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	CDataIO* pSys = (CDataIO*)pParam;
	switch( pCmdParam->dwParam ){
		case CMD_SEND_DATA:
			pSys->CmdSendData(1, pCmdParam, pResParam);
			break;
		default:
			pResParam->dwParam = CMD_NON_SUPPORT;
			break;
	}
	return 0;
}

int CALLBACK CDataIO::OutsideCmdCallbackS0(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	CDataIO* pSys = (CDataIO*)pParam;
	switch( pCmdParam->dwParam ){
		case CMD_SEND_DATA:
			pSys->CmdSendData(2, pCmdParam, pResParam);
			break;
		default:
			pResParam->dwParam = CMD_NON_SUPPORT;
			break;
	}
	return 0;
}

int CALLBACK CDataIO::OutsideCmdCallbackS1(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	CDataIO* pSys = (CDataIO*)pParam;
	switch( pCmdParam->dwParam ){
		case CMD_SEND_DATA:
			pSys->CmdSendData(3, pCmdParam, pResParam);
			break;
		default:
			pResParam->dwParam = CMD_NON_SUPPORT;
			break;
	}
	return 0;
}

void CDataIO::CmdSendData(DWORD dwID, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	pResParam->dwParam = CMD_SUCCESS;
	BOOL bSend = FALSE;

	switch(dwID){
		case 0:
			Lock1();
			if( m_T0Buff.size() > 0 ){
				pResParam->dwSize = m_T0Buff[0]->dwSize;
				pResParam->bData = new BYTE[pResParam->dwSize];
				memcpy(pResParam->bData, m_T0Buff[0]->pbBuff, pResParam->dwSize);
				bSend = TRUE;
				SAFE_DELETE(m_T0Buff[0]);
				m_T0Buff.erase(m_T0Buff.begin());
			}
			UnLock1();
			break;
		case 1:
			Lock2();
			if( m_T1Buff.size() > 0 ){
				pResParam->dwSize = m_T1Buff[0]->dwSize;
				pResParam->bData = new BYTE[pResParam->dwSize];
				memcpy(pResParam->bData, m_T1Buff[0]->pbBuff, pResParam->dwSize);
				bSend = TRUE;
				SAFE_DELETE(m_T1Buff[0]);
				m_T1Buff.erase(m_T1Buff.begin());
			}
			UnLock2();
			break;
		case 2:
			Lock3();
			if( m_S0Buff.size() > 0 ){
				pResParam->dwSize = m_S0Buff[0]->dwSize;
				pResParam->bData = new BYTE[pResParam->dwSize];
				memcpy(pResParam->bData, m_S0Buff[0]->pbBuff, pResParam->dwSize);
				bSend = TRUE;
				SAFE_DELETE(m_S0Buff[0]);
				m_S0Buff.erase(m_S0Buff.begin());
			}
			UnLock3();
			break;
		case 3:
			Lock4();
			if( m_S1Buff.size() > 0 ){
				pResParam->dwSize = m_S1Buff[0]->dwSize;
				pResParam->bData = new BYTE[pResParam->dwSize];
				memcpy(pResParam->bData, m_S1Buff[0]->pbBuff, pResParam->dwSize);
				bSend = TRUE;
				SAFE_DELETE(m_S1Buff[0]);
				m_S1Buff.erase(m_S1Buff.begin());
			}
			UnLock4();
			break;
	}

	if( bSend == FALSE ){
		pResParam->dwParam = CMD_ERR_BUSY;
	}
}

DWORD CDataIO::GetOverFlowCount(int iID)
{
	int iDevID = iID>>16;
	PT::Device::ISDB enISDB = (PT::Device::ISDB)((iID&0x0000FF00)>>8);
	uint32 iTuner = iID&0x000000FF;

	DWORD dwRet = 0;
	if( enISDB == PT::Device::ISDB_T ){
		if( iTuner == 0 ){
			dwRet = m_dwT0OverFlowCount;
		}else{
			dwRet = m_dwT1OverFlowCount;
		}
	}else{
		if( iTuner == 0 ){
			dwRet = m_dwS0OverFlowCount;
		}else{
			dwRet = m_dwS1OverFlowCount;
		}
	}
	return dwRet;
}

