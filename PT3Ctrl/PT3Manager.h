#pragma once

#include "inc/EARTH_PT3.h"
#include "inc/OS_Library.h"
#include "../Common/Util.h"
#include "DataIO.h"

using namespace EARTH;

class CPT3Manager
{
public:
	CPT3Manager(void);
	~CPT3Manager(void);

	BOOL LoadSDK();
	BOOL Init();
	void UnInit();

	int OpenTuner(BOOL bSate);
	int OpenTuner2(BOOL bSate, int iTunerID);
	BOOL CloseTuner(int iID);

	BOOL SetCh(int iID, unsigned long ulCh, DWORD dwTSID);
	DWORD GetSignal(int iID);

	BOOL IsFindOpen();

	BOOL CloseChk();

protected:
	OS::Library* m_cLibrary;
	PT::Bus* m_cBus;

	typedef struct _DEV_STATUS{
		PT::Bus::DeviceInfo stDevInfo;
		PT::Device* pcDevice;
		BOOL bOpen;
		BOOL bUseT0;
		BOOL bUseT1;
		BOOL bUseS0;
		BOOL bUseS1;
		CDataIO cDataIO;
		_DEV_STATUS(void){
			bOpen = FALSE;
			pcDevice = NULL;
			bUseT0 = FALSE;
			bUseT1 = FALSE;
			bUseS0 = FALSE;
			bUseS1 = FALSE;
		}
	}DEV_STATUS;
	vector<DEV_STATUS*> m_EnumDev;

	BOOL m_bUseLNB;
	UINT m_uiVirtualCount;

protected:
	void FreeSDK();
};
