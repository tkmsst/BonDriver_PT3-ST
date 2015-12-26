#include "StdAfx.h"
#include "PT3CtrlMain.h"

CPT3CtrlMain::CPT3CtrlMain(void)
{
	m_hStopEvent = _CreateEvent(TRUE, FALSE,NULL);
	m_bService = FALSE;
}

CPT3CtrlMain::~CPT3CtrlMain(void)
{
	StopMain();
	if( m_hStopEvent != NULL ){
		CloseHandle(m_hStopEvent);
	}
//	m_cPipeserver.StopServer();
}

void CPT3CtrlMain::StartMain(BOOL bService)
{
	BOOL bInit = TRUE;
	if( m_cPT3.LoadSDK() == FALSE ){
		OutputDebugString(L"PT SDKのロードに失敗");
		bInit = FALSE;
	}
	if( bInit == TRUE ){
		m_cPT3.Init();
	}
	m_bService = bService;

	//Pipeサーバースタート
	CPipeServer cPipeserver;
	cPipeserver.StartServer(CMD_PT1_CTRL_EVENT_WAIT_CONNECT, CMD_PT1_CTRL_PIPE, OutsideCmdCallback, this);

	while(1){
		if( WaitForSingleObject(m_hStopEvent, 15*1000) != WAIT_TIMEOUT ){
			break;
		}else{
			//アプリ層死んだ時用のチェック
			if( m_cPT3.CloseChk() == FALSE && m_bService == FALSE){
				break;
			}
		}
		if( bInit == FALSE ){
			break;
		}
	}

	cPipeserver.StopServer();
	m_cPT3.UnInit();
}

void CPT3CtrlMain::StopMain()
{
	if( m_hStopEvent != NULL ){
		SetEvent(m_hStopEvent);
	}
}

int CALLBACK CPT3CtrlMain::OutsideCmdCallback(void* pParam, CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	CPT3CtrlMain* pSys = (CPT3CtrlMain*)pParam;

	switch( pCmdParam->dwParam ){
		case CMD_CLOSE_EXE:
			pSys->CmdCloseExe(pCmdParam, pResParam);
			break;
		case CMD_OPEN_TUNER:
			pSys->CmdOpenTuner(pCmdParam, pResParam);
			break;
		case CMD_CLOSE_TUNER:
			pSys->CmdCloseTuner(pCmdParam, pResParam);
			break;
		case CMD_SET_CH:
			pSys->CmdSetCh(pCmdParam, pResParam);
			break;
		case CMD_GET_SIGNAL:
			pSys->CmdGetSignal(pCmdParam, pResParam);
			break;
		case CMD_OPEN_TUNER2:
			pSys->CmdOpenTuner2(pCmdParam, pResParam);
			break;
		default:
			pResParam->dwParam = CMD_NON_SUPPORT;
			break;
	}
	return 0;
}

//CMD_CLOSE_EXE PT1Ctrl.exeの終了
void CPT3CtrlMain::CmdCloseExe(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	pResParam->dwParam = CMD_SUCCESS;
	StopMain();
}

//CMD_OPEN_TUNER OpenTuner
void CPT3CtrlMain::CmdOpenTuner(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	BOOL bSate;
	CopyDefData((DWORD*)&bSate, pCmdParam->bData);
	int iID = m_cPT3.OpenTuner(bSate);
	if( iID != -1 ){
		pResParam->dwParam = CMD_SUCCESS;
	}else{
		pResParam->dwParam = CMD_ERR;
	}
	CreateDefStream(iID, pResParam);
}

//CMD_CLOSE_TUNER CloseTuner
void CPT3CtrlMain::CmdCloseTuner(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	int iID;
	CopyDefData((DWORD*)&iID, pCmdParam->bData);
	m_cPT3.CloseTuner(iID);
	Sleep(100);
	pResParam->dwParam = CMD_SUCCESS;
	if( m_cPT3.IsFindOpen() == FALSE && m_bService == FALSE){
		StopMain();
	}
}

//CMD_SET_CH SetChannel
void CPT3CtrlMain::CmdSetCh(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	int iID;
	DWORD dwCh;
	DWORD dwTSID;
	CopyDefData3((DWORD*)&iID, &dwCh, &dwTSID, pCmdParam->bData);
	if( m_cPT3.SetCh(iID,dwCh,dwTSID) == TRUE ){
		pResParam->dwParam = CMD_SUCCESS;
	}else{
		pResParam->dwParam = CMD_ERR;
	}
}

//CMD_GET_SIGNAL GetSignalLevel
void CPT3CtrlMain::CmdGetSignal(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	int iID;
	DWORD dwCn100;
	CopyDefData((DWORD*)&iID, pCmdParam->bData);
	dwCn100 = m_cPT3.GetSignal(iID);

	pResParam->dwParam = CMD_SUCCESS;
	CreateDefStream(dwCn100, pResParam);
}

void CPT3CtrlMain::CmdOpenTuner2(CMD_STREAM* pCmdParam, CMD_STREAM* pResParam)
{
	BOOL bSate;
	int iTunerID;
	CopyDefData2((DWORD*)&bSate, (DWORD*)&iTunerID, pCmdParam->bData);
	int iID = m_cPT3.OpenTuner2(bSate, iTunerID);
	if( iID != -1 ){
		pResParam->dwParam = CMD_SUCCESS;
	}else{
		pResParam->dwParam = CMD_ERR;
	}
	CreateDefStream(iID, pResParam);
}

BOOL CPT3CtrlMain::IsFindOpen()
{
	return m_cPT3.IsFindOpen();
}