#ifndef __PT1_OUTSIDE_CTRL_CMD_DEF_H__
#define __PT1_OUTSIDE_CTRL_CMD_DEF_H__

#include "Util.h"

#define SEND_BUFF_SIZE 1024*64
#define RES_BUFF_SIZE 1024*64

//パイプ名
#define CMD_PT1_CTRL_PIPE _T("\\\\.\\pipe\\PT3CtrlPipe")
#define CMD_PT1_DATA_PIPE _T("\\\\.\\pipe\\PT3DataPipe_")

//接続待機用イベント
#define CMD_PT1_CTRL_EVENT_WAIT_CONNECT _T("Global\\PT3CtrlConnect")
#define CMD_PT1_DATA_EVENT_WAIT_CONNECT _T("Global\\PT3DataConnect_")

//モジュール内コマンド実行イベント
#define CMD_CTRL_EVENT_WAIT _T("CtrlCmdEvent")

//コマンド
#define CMD_CLOSE_EXE 1
#define CMD_OPEN_TUNER 2
#define CMD_CLOSE_TUNER 3
#define CMD_SET_CH 4
#define CMD_GET_SIGNAL 5
#define CMD_OPEN_TUNER2 6
#define CMD_SEND_DATA 10

//エラーコード
#define CMD_SUCCESS			0 //成功
#define CMD_ERR				1 //汎用エラー
#define CMD_NEXT			2 //Enumコマンド用、続きあり
#define CMD_NON_SUPPORT		3 //未サポートのコマンド
#define CMD_ERR_INVALID_ARG	4 //引数エラー
#define CMD_ERR_CONNECT		5 //サーバーにコネクトできなかった
#define CMD_ERR_DISCONNECT	6 //サーバーから切断された
#define CMD_ERR_TIMEOUT		7 //タイムアウト発生
#define CMD_ERR_BUSY		8 //ビジー状態で現在処理できない

//コマンド送受信ストリーム
typedef struct _CMD_STREAM{
	DWORD dwParam; //送信時コマンド、受信時エラーコード
	DWORD dwSize; //bDataサイズ
	BYTE* bData;
	_CMD_STREAM(void){
		dwSize = 0;
		bData = NULL;
	}
	~_CMD_STREAM(void){
		SAFE_DELETE_ARRAY(bData);
	}
} CMD_STREAM;


#endif