#include "stdafx.h"
#include "StringUtil.h"

void Format(string& strBuff, const char *format, ...)
{
	va_list params;

	va_start(params, format);
	int iResult;
	char *buff;
	int length = _vscprintf(format, params);
	buff = new char [length + 1];
	iResult = vsprintf_s(buff, length + 1, format, params);
	buff[length] = '\0';
	if (buff != NULL) {
		strBuff = buff;
		delete[] buff;
	}

	va_end(params);
}

void Format(wstring& strBuff, const WCHAR *format, ...)
{
	va_list params;

	va_start(params, format);

	int iResult;
	WCHAR *buff;
	int length = _vscwprintf(format, params);
	buff = new WCHAR [length + 1];
	iResult = vswprintf_s(buff, length + 1, format, params);
	buff[length] = '\0';
	if (buff != NULL) {
		strBuff = buff;
		delete[] buff;
	}

    va_end(params);
}

void Replace(string& strBuff, const string strOld, const string strNew)
{
	string::size_type Pos = 0;
	string::size_type OldPos = 0;

	while ((Pos = strBuff.find(strOld,OldPos)) != string::npos)
	{
		strBuff.replace(Pos,strOld.size(),strNew);
		OldPos = Pos + strNew.size();
	}
}

void Replace(wstring& strBuff, const wstring strOld, const wstring strNew)
{
	string::size_type Pos = 0;
	string::size_type OldPos = 0;

	while ((Pos = strBuff.find(strOld,OldPos)) != string::npos)
	{
		strBuff.replace(Pos,strOld.size(),strNew);
		OldPos = Pos + strNew.size();
	}
}

void WtoA(wstring strIn, string& strOut)
{
	if( strIn.empty() == true ){
		strOut="";
		return ;
	}
	int iLen = 0;
	iLen = WideCharToMultiByte( 932, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL );
	char* pszBuff = new char[iLen+1];
	ZeroMemory(pszBuff, sizeof(char)*(iLen+1));
	WideCharToMultiByte( 932, 0, strIn.c_str(), -1, pszBuff, iLen, NULL, NULL );
	strOut = pszBuff;
	
	delete[] pszBuff;
}

void WtoUTF8(wstring strIn, string& strOut)
{
	if( strIn.empty() == true ){
		strOut="";
		return ;
	}
	int iLen = 0;
	iLen = WideCharToMultiByte( CP_UTF8, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL );
	char* pszBuff = new char[iLen+1];
	ZeroMemory(pszBuff, sizeof(char)*(iLen+1));
	WideCharToMultiByte( CP_UTF8, 0, strIn.c_str(), -1, pszBuff, iLen, NULL, NULL );
	strOut = pszBuff;
	
	delete[] pszBuff;
}

void AtoW(string strIn, wstring& strOut)
{
	if( strIn.empty() == true ){
		strOut=L"";
		return ;
	}
	int iLen = MultiByteToWideChar( 932, 0, strIn.c_str(), -1, NULL, 0 );
	WCHAR* pwszBuff = new WCHAR[iLen+1];
	ZeroMemory(pwszBuff, sizeof(WCHAR)*(iLen+1));
	MultiByteToWideChar( 932, 0, strIn.c_str(), -1, pwszBuff, iLen );

	strOut = pwszBuff;
	
	delete[] pwszBuff;
}

void UTF8toW(string strIn, wstring& strOut)
{
	if( strIn.empty() == true ){
		strOut=L"";
		return ;
	}
	int iLen = MultiByteToWideChar( CP_UTF8, 0, strIn.c_str(), -1, NULL, 0 );
	WCHAR* pwszBuff = new WCHAR[iLen+1];
	ZeroMemory(pwszBuff, sizeof(WCHAR)*(iLen+1));
	MultiByteToWideChar( CP_UTF8, 0, strIn.c_str(), -1, pwszBuff, iLen );

	strOut = pwszBuff;
	
	delete[] pwszBuff;
}

BOOL Separate(string strIn, string strSep, string& strLeft, string& strRight)
{
	if( strIn.empty() == true ){
		strLeft = "";
		strRight = "";
		return FALSE;
	}
	
	strLeft = "";
	strRight = "";
	int iPos = (int)strIn.find(strSep);
	if( iPos == (int)string::npos ){
		strLeft = strIn;
		return FALSE;
	}else if( iPos == 0 ){
		strIn.erase(0, iPos+strSep.length());
		strRight = strIn;
		return TRUE;
	}
	strLeft = strIn.substr(0,iPos);
	strIn.erase(0, iPos+strSep.length());
	strRight = strIn;
	
	return TRUE;
}

BOOL Separate(wstring strIn, wstring strSep, wstring& strLeft, wstring& strRight)
{
	if( strIn.empty() == true ){
		strLeft = L"";
		strRight = L"";
		return FALSE;
	}
	
	strLeft = L"";
	strRight = L"";
	int iPos = (int)strIn.find(strSep);
	if( iPos == (int)string::npos ){
		strLeft = strIn;
		return FALSE;
	}else if( iPos == 0 ){
		strIn.erase(0, iPos+strSep.length());
		strRight = strIn;
		return TRUE;
	}
	strLeft = strIn.substr(0,iPos);
	strIn.erase(0, iPos+strSep.length());
	strRight = strIn;
	
	return TRUE;
}

void ChkFolderPath(string& strPath)
{
	if( strPath.empty() == true ){
		return ;
	}
	string strChk = "\\";
	if( strChk.compare(strPath.substr(strPath.length()-1, 1)) == 0 ){
		strPath.erase(strPath.length()-1, 1);
	}
}

void ChkFolderPath(wstring& strPath)
{
	if( strPath.empty() == true ){
		return ;
	}
	wstring strChk = L"\\";
	if( strChk.compare(strPath.substr(strPath.length()-1, 1)) == 0 ){
		strPath.erase(strPath.length()-1, 1);
	}
}

void ChkFileName(string& strPath)
{
	if( strPath.empty() == true ){
		return ;
	}
	Replace(strPath, "\\","Åè");
	Replace(strPath, "/","Å^");
	Replace(strPath, ":","ÅF");
	Replace(strPath, "*","Åñ");
	Replace(strPath, "?","ÅH");
	Replace(strPath, "\"","Åh");
	Replace(strPath, "<","ÅÉ");
	Replace(strPath, ">","ÅÑ");
	Replace(strPath, "|","Åb");
}

void ChkFileName(wstring& strPath)
{
	if( strPath.empty() == true ){
		return ;
	}
	Replace(strPath, L"\\",L"Åè");
	Replace(strPath, L"/",L"Å^");
	Replace(strPath, L":",L"ÅF");
	Replace(strPath, L"*",L"Åñ");
	Replace(strPath, L"?",L"ÅH");
	Replace(strPath, L"\"",L"Åh");
	Replace(strPath, L"<",L"ÅÉ");
	Replace(strPath, L">",L"ÅÑ");
	Replace(strPath, L"|",L"Åb");
}

int CompareNoCase(string str1, string str2)
{
	DWORD dwSize1 = (DWORD)str1.length()+1;
	DWORD dwSize2 = (DWORD)str2.length()+1;

	char* szBuff1 = new char[dwSize1];
	char* szBuff2 = new char[dwSize2];

	ZeroMemory(szBuff1, sizeof(char)*dwSize1);
	ZeroMemory(szBuff2, sizeof(char)*dwSize2);

	strcpy_s(szBuff1, dwSize1, str1.c_str());
	strcpy_s(szBuff2, dwSize2, str2.c_str());

	_strlwr_s(szBuff1, dwSize1);
	_strlwr_s(szBuff2, dwSize2);

	string strBuff1 = szBuff1;
	string strBuff2 = szBuff2;

	delete[] szBuff1;
	delete[] szBuff2;

	return strBuff1.compare(strBuff2);
}

int CompareNoCase(wstring str1, wstring str2)
{
	DWORD dwSize1 = (DWORD)str1.length()+1;
	DWORD dwSize2 = (DWORD)str2.length()+1;

	WCHAR* szBuff1 = new WCHAR[dwSize1];
	WCHAR* szBuff2 = new WCHAR[dwSize2];

	ZeroMemory(szBuff1, sizeof(WCHAR)*dwSize1);
	ZeroMemory(szBuff2, sizeof(WCHAR)*dwSize2);

	wcscpy_s(szBuff1, dwSize1, str1.c_str());
	wcscpy_s(szBuff2, dwSize2, str2.c_str());

	_wcslwr_s(szBuff1, dwSize1);
	_wcslwr_s(szBuff2, dwSize2);

	wstring strBuff1 = szBuff1;
	wstring strBuff2 = szBuff2;

	delete[] szBuff1;
	delete[] szBuff2;

	return strBuff1.compare(strBuff2);
}
