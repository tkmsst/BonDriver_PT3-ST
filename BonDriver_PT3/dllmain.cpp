// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "BonTuner.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
			// モジュールハンドル保存
			CBonTuner::m_hModule = hModule;
			break;
	
		case DLL_PROCESS_DETACH:
			// 未開放の場合はインスタンス開放		
			if(CBonTuner::m_pThis)CBonTuner::m_pThis->Release();
			break;
	}  
    return TRUE;
}

