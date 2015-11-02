#ifndef _EX_Utility_H
#define _EX_Utility_H

#include "Prefix.h"
#include "EARTH_PT3.h"

namespace EARTH {
namespace EX {
	class U {
	public:
		static bool Error(const wchar_t *function, status status)
		{
			bool error = (status != PT::STATUS_OK);
			if (error) {
				::wprintf(L"ÅöÉGÉâÅ[Ç™î≠ê∂ÇµÇ‹ÇµÇΩÅB%s (0x%08x)\n", function, status);
			}
			return error;
		}

		static sint32 GetNumber(uint32 maxNumber)
		{
			bool	done = false;
			uint32	number = 0;
			do {
				wint_t wint = ::_getwch();
				if (wint == L'\03') ::exit(-1);

				const wchar_t *string = L"0123456789";
				const wchar_t *ptr = ::wcschr(string, wint);
				if (ptr) {
					number = static_cast<uint32>(ptr - string);
					if (number <= maxNumber) {
						::_putch(wint);
						::wprintf(L"\n");
						done = true;
					}
				}
			} while (done == false);

			return number;
		}
	};
}
}

#endif
