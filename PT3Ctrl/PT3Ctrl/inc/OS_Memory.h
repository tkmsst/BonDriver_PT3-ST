#ifndef _OS_Memory_H
#define _OS_Memory_H

#include "Prefix.h"
#include <Windows.h>

namespace EARTH {
namespace OS {
	class Memory {
	public:
		enum {
			PAGE_SIZE = (4)*1024
		};

		Memory(size_t size)
		{
			_ASSERT((size % PAGE_SIZE) == 0);
			mPtr = ::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
		}

		~Memory()
		{
			if (mPtr) {
				BOOL	b = ::VirtualFree(mPtr, 0, MEM_RELEASE);
				if (b) {
					mPtr = NULL;
				}
				_ASSERT(b);
			}
		}

		void *Ptr() const
		{
			return mPtr;
		}

	protected:
		void	*mPtr;
	};
}
}

#endif
