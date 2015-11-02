#ifndef __UTIL_H__
#define __UTIL_H__

// MFC‚ÅŽg‚¤Žž—p
//#define _MFC
#ifdef _MFC
#ifdef _DEBUG
#undef new
#endif
#include <string>
#include <map>
#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#else
#include <string>
#include <map>
#include <vector>
#endif
using namespace std;
#include <TCHAR.h>

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

HANDLE _CreateEvent(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName );
HANDLE _CreateFile( LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
HANDLE _CreateMutex( BOOL bInitialOwner, LPCTSTR lpName );
HANDLE _CreateFileMapping( HANDLE hFile, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCTSTR lpName );
HANDLE _CreateNamedPipe( LPCTSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut );
BOOL _CreateDirectory( LPCTSTR lpPathName );
HANDLE _CreateFile2( LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );

void _OutputDebugString(const TCHAR *pOutputString, ...);
#endif
