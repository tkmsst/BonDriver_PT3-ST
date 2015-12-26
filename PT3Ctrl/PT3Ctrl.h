#pragma once

#include "resource.h"


void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
DWORD WINAPI service_ctrl(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
BOOL SendStatusScm(int iState, int iExitcode, int iProgress);

void StartMain(BOOL bService);
void StopMain();
