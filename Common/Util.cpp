#include "stdafx.h"
#include "Util.h"

HANDLE _CreateEvent(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName)
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	 
	memset(&sd,0,sizeof(sd));
	InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	memset(&sa,0,sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=&sd;

	return ::CreateEvent(&sa, bManualReset, bInitialState, lpName);
}

HANDLE _CreateFile( LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	 
	memset(&sd,0,sizeof(sd));
	InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	memset(&sa,0,sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=&sd;

	return ::CreateFile( lpFileName, dwDesiredAccess, dwShareMode, &sa, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
}

HANDLE _CreateMutex( BOOL bInitialOwner, LPCTSTR lpName )
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	 
	memset(&sd,0,sizeof(sd));
	InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	memset(&sa,0,sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=&sd;

	return ::CreateMutex( &sa, bInitialOwner, lpName );
}

HANDLE _CreateFileMapping( HANDLE hFile, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCTSTR lpName)
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	 
	memset(&sd,0,sizeof(sd));
	InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	memset(&sa,0,sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=&sd;

	return ::CreateFileMapping( hFile, &sa, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

HANDLE _CreateNamedPipe( LPCTSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut )
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	 
	memset(&sd,0,sizeof(sd));
	InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	memset(&sa,0,sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=&sd;

	return ::CreateNamedPipe( lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, &sa );
}

BOOL _CreateDirectory( LPCTSTR lpPathName )
{
	BOOL bRet = FALSE;
	if( _tcslen(lpPathName) > 2 ){
		TCHAR szCreatePath[MAX_PATH+1] = _T("");
		szCreatePath[0] = lpPathName[0];
		szCreatePath[1] = lpPathName[1];
		
		for (int i = 2; i < (int)_tcslen(lpPathName); i++) {
			szCreatePath[i] = lpPathName[i];
			if (szCreatePath[i] == '\\') {
				szCreatePath[i+1] = '\0';
				if ( GetFileAttributes(szCreatePath) == 0xFFFFFFFF ) {
					bRet = ::CreateDirectory( szCreatePath, NULL );
				}
			}
		}
		if ( GetFileAttributes(szCreatePath) == 0xFFFFFFFF ) {
			bRet = ::CreateDirectory( szCreatePath, NULL );
		}
	}

	return bRet;
}

HANDLE _CreateFile2( LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE hFile =  ::CreateFile( lpFileName, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	if( hFile == INVALID_HANDLE_VALUE ){
		TCHAR* p = (TCHAR*)_tcsrchr(lpFileName, '\\');
		TCHAR* szDirPath = NULL;
		if( p != NULL ){
			int iSize = (int)(p - lpFileName);
			szDirPath = new TCHAR[iSize+1];
			_tcsncpy_s(szDirPath, iSize+1, lpFileName, iSize);
		}
		if( szDirPath != NULL ){
			_CreateDirectory(szDirPath);
			delete[] szDirPath;
			hFile =  ::CreateFile( lpFileName, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
		}
	}
	return hFile;
}

void _OutputDebugString(const TCHAR *format, ...)
{
	va_list params;

	va_start(params, format);
	int iResult;
	TCHAR *buff;
	int length = _vsctprintf(format, params);
	buff = new TCHAR [length + 1];
	if (buff != NULL) {
		iResult = _vstprintf_s(buff, length + 1, format, params);
		buff[length] = _T('\0');
		OutputDebugString(buff);
		delete[] buff;
	}
	va_end(params);
}
