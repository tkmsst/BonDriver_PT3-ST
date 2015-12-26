#pragma once

#include "../Common/Util.h"
#include "../Common/StringUtil.h"

typedef struct _SPACE_DATA{
	wstring wszName;
	DWORD dwSpace;
	//=オペレーターの処理
	_SPACE_DATA(void){
		wszName = L"";
		dwSpace = 0;
	};
	~_SPACE_DATA(void){
	}
	_SPACE_DATA & operator= (const _SPACE_DATA & o) {
		wszName = o.wszName;
		dwSpace = o.dwSpace;
		return *this;
	}
}SPACE_DATA;

typedef struct _CH_DATA{
	wstring wszName;
	DWORD dwSpace;
	DWORD dwCh;
	DWORD dwPT1Ch;
	DWORD dwTSID;
	//=オペレーターの処理
	_CH_DATA(void){
		wszName = L"";
		dwSpace = 0;
		dwCh = 0;
		dwPT1Ch = 0;
		dwTSID = 0;
	};
	~_CH_DATA(void){
	}
	_CH_DATA & operator= (const _CH_DATA & o) {
		wszName = o.wszName;
		dwSpace = o.dwSpace;
		dwCh = o.dwCh;
		dwPT1Ch = o.dwPT1Ch;
		dwTSID = o.dwTSID;
		return *this;
	}
}CH_DATA;

class CParseChSet
{
public:
	map<DWORD, SPACE_DATA> spaceMap;
	map<DWORD, CH_DATA> chMap;

public:
	CParseChSet(void);
	~CParseChSet(void);

	//ChSet.txtの読み込みを行う
	//戻り値：
	// TRUE（成功）、FALSE（失敗）
	//引数：
	// file_path		[IN]ChSet.txtのフルパス
	BOOL ParseText(
		LPCWSTR filePath
		);

protected:
	wstring filePath;

protected:
	BOOL Parse1Line(string parseLine, SPACE_DATA* info );
	BOOL Parse1Line(string parseLine, CH_DATA* chInfo );
};
