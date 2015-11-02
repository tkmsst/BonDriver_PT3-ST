#include "StdAfx.h"
#include "ParseChSet.h"

CParseChSet::CParseChSet(void)
{
}

CParseChSet::~CParseChSet(void)
{
}

BOOL CParseChSet::ParseText(LPCWSTR filePath)
{
	if( filePath == NULL ){
		return FALSE;
	}

	this->spaceMap.clear();
	this->chMap.clear();
	this->filePath = filePath;

	HANDLE hFile = _CreateFile2( filePath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE ){
		return FALSE;
	}
	DWORD dwFileSize = GetFileSize( hFile, NULL );
	if( dwFileSize == 0 ){
		CloseHandle(hFile);
		return TRUE;
	}
	char* pszBuff = new char[dwFileSize+1];
	if( pszBuff == NULL ){
		CloseHandle(hFile);
		return FALSE;
	}
	ZeroMemory(pszBuff,dwFileSize+1);
	DWORD dwRead=0;
	ReadFile( hFile, pszBuff, dwFileSize, &dwRead, NULL );

	string strRead = pszBuff;

	CloseHandle(hFile);
	SAFE_DELETE_ARRAY(pszBuff)

	string parseLine="";
	size_t iIndex = 0;
	size_t iFind = 0;
	while( iFind != string::npos ){
		iFind = strRead.find("\r\n", iIndex);
		if( iFind == (int)string::npos ){
			parseLine = strRead.substr(iIndex);
			//strRead.clear();
		}else{
			parseLine = strRead.substr(iIndex,iFind-iIndex);
			//strRead.erase( 0, iIndex+2 );
			iIndex = iFind + 2;
		}
		//先頭；はコメント行
		if( parseLine.find(";") != 0 ){
			//空行？
			if( parseLine.find("\t") != string::npos ){
				if( parseLine.find("$") == 0 ){
					//チューナー空間
					SPACE_DATA item;
					if( Parse1Line(parseLine, &item) == TRUE ){
						this->spaceMap.insert( pair<DWORD, SPACE_DATA>(item.dwSpace,item) );
					}
				}else{
					//チャンネル
					CH_DATA item;
					if( Parse1Line(parseLine, &item) == TRUE ){
						DWORD iKey = (item.dwSpace<<16) | item.dwCh;
						this->chMap.insert( pair<DWORD, CH_DATA>(iKey,item) );
					}
				}
			}
		}
	}

	return TRUE;
}

BOOL CParseChSet::Parse1Line(string parseLine, SPACE_DATA* info )
{
	if( parseLine.empty() == true || info == NULL ){
		return FALSE;
	}
	Replace(parseLine, "$", "");
	string strBuff="";

	Separate( parseLine, "\t", strBuff, parseLine);

	//Ch名
	AtoW(strBuff, info->wszName);

	Separate( parseLine, "\t", strBuff, parseLine);

	//Space
	info->dwSpace = atoi(strBuff.c_str());

	return TRUE;
}

BOOL CParseChSet::Parse1Line(string parseLine, CH_DATA* chInfo )
{
	if( parseLine.empty() == true || chInfo == NULL ){
		return FALSE;
	}
	string strBuff="";

	Separate( parseLine, "\t", strBuff, parseLine);

	//Ch名
	AtoW(strBuff, chInfo->wszName);

	Separate( parseLine, "\t", strBuff, parseLine);

	//Space
	chInfo->dwSpace = atoi(strBuff.c_str());

	Separate( parseLine, "\t", strBuff, parseLine);

	//Ch
	chInfo->dwCh = atoi(strBuff.c_str());

	Separate( parseLine, "\t", strBuff, parseLine);

	//PTxのチャンネル
	chInfo->dwPT1Ch = atoi(strBuff.c_str());

	Separate( parseLine, "\t", strBuff, parseLine);

	//TSID
	chInfo->dwTSID = (WORD)atoi(strBuff.c_str());

	return TRUE;
}
