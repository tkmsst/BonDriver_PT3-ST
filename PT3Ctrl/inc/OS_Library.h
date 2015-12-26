#ifndef _OS_Library_H
#define _OS_Library_H

#include "Prefix.h"
#include "EARTH_PT3.h"
#include <windows.h>

namespace EARTH {
namespace OS {
	class Library {
	public:
		Library()
		{
			::wsprintf(mPath, L"");
			mModule = NULL;
			mFunction = NULL;

			#if PUBLIC_LEVEL <= 1
				::wsprintf(mPath, L"SDK_EARTHSOFT_PT3.dll");
			#elif PUBLIC_LEVEL == 2 && !defined(_DEBUG)
				::wsprintf(mPath, L"Update_PT3_20120704.dll");
			#else
				const wchar_t *bit;
				#if !defined(_WIN64)
					bit = L"Win32";
				#else
					bit = L"x64";
				#endif

				const wchar_t	*configuration;
				#if defined(_DEBUG)
					configuration = L"Debug";
				#else
					configuration = L"Release";
				#endif

				::wsprintf(mPath, L"..\\..\\..\\..\\SDK\\Output\\%s\\%s\\SDK.dll", bit, configuration);
			#endif

			_ASSERT(mModule == NULL);
			mModule = ::LoadLibrary(mPath);
			if (mModule == NULL) return;

			mFunction = reinterpret_cast<PT::Bus::NewBusFunction>(::GetProcAddress(mModule, "_"));
			if (mFunction == NULL) return;
		}

		~Library()
		{
			if (mModule) {
				BOOL	b = ::FreeLibrary(mModule);
				if (b) {
					mModule = NULL;
				}
			}

			mFunction = NULL;
		}

		const wchar_t *Path() const
		{
			return mPath;
		}

		PT::Bus::NewBusFunction Function() const
		{
			return mFunction;
		}

	protected:
		wchar_t						mPath[MAX_PATH];
		HMODULE						mModule;
		PT::Bus::NewBusFunction		mFunction;
	};
}
}

#endif
