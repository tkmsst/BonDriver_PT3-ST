#ifndef _EX_Buffer_H
#define _EX_Buffer_H

#include "Prefix.h"
#include "EARTH_PT3.h"
#include "EX_Utility.h"
#include "OS_Memory.h"
#include <vector>

namespace EARTH {
namespace EX {
	class Buffer {
	public:
		Buffer(PT::Device *device) : mDevice(device)
		{
			for (uint32 i=0; i<sizeof(mInfo)/sizeof(*mInfo); i++) {
				Info	*info = &mInfo[i];
				info->memory      = NULL;
				info->ptr         = NULL;
				info->size        = 0;
			}
		}

		~Buffer()
		{
			Free();
		}

		status Alloc(uint32 blockSize, uint32 blockCount, bool loop = true)
		{
			status	status;
			
			status = AllocMemory(blockSize, blockCount);
			if (status) return status;

			BuildPageDescriptor(loop);

			status = mDevice->SyncBufferCpu(mInfo[Index_Page].physicals[0].handle);
			if (status) return status;

			return status;
		}

		status Free()
		{
			status	status;
			status = FreeMemory();
			if (status) return status;

			return status;
		}

		status SyncCpu(uint32 blockIndex)
		{
			return mDevice->SyncBufferCpu(mInfo[Index_TS].physicals.at(blockIndex).handle);
		}

		status SyncIo(uint32 blockIndex)
		{
			return mDevice->SyncBufferIo(mInfo[Index_TS].physicals.at(blockIndex).handle);
		}

		void *Ptr(uint32 blockIndex)
		{
			Info	*info = &mInfo[Index_TS];
			uint8	*ptr = static_cast<uint8 *>(info->ptr);
			ptr += info->blockSize * blockIndex;
			return ptr;
		}

		uint64 PageDescriptorAddress()
		{
			_ASSERT(mInfo[Index_Page].physicals[0].bufferInfo);
			uint64	address = mInfo[Index_Page].physicals[0].bufferInfo[0].Address;

			return address;
		}

	protected:
		enum Index {
			Index_TS,		// TS データ領域
			Index_Page,		// ページ記述子領域
			Index_Count
		};

		struct Physical {
			void							*handle;
			const PT::Device::BufferInfo	*bufferInfo;
			uint32							bufferCount;
		};

		struct Info {
			OS::Memory				*memory;
			void					*ptr;
			uint32					size;
			uint32					blockSize;
			uint32					blockCount;
			std::vector<Physical>	physicals;
		};

		PT::Device		*const mDevice;
		Info			mInfo[Index_Count];

		status AllocMemory(uint32 blockSize, uint32 blockCount)
		{
			status	status = PT::STATUS_OK;

			// TS データ領域を確保する
			status = AllocMemory(Index_TS, blockSize , blockCount);
			if (status) return status;

			// ページ記述子領域を確保する
			// 記述子のサイズは 20 バイトなので、4096 バイトの領域に構築できる記述子の数は 204 個である。
			uint32	pageCount = (mInfo[Index_TS].size / OS::Memory::PAGE_SIZE + 203) / 204;
			status = AllocMemory(Index_Page, OS::Memory::PAGE_SIZE * pageCount, 1);
			if (status) return status;

			return status;
		}

		status AllocMemory(Index index, uint32 blockSize, uint32 blockCount)
		{
			status	status = PT::STATUS_OK;

			_ASSERT(index < Index_Count);
			Info	*info = &mInfo[index];

			// メモリを確保する
			uint32	size = blockSize * blockCount;

			_ASSERT(info->memory == NULL);
			info->memory = new OS::Memory(size);
			if (info->memory == NULL) return PT::STATUS_OUT_OF_MEMORY_ERROR;

			uint8	*ptr = static_cast<uint8 *>(info->memory->Ptr());
			info->ptr        = ptr;
			info->size       = size;
			info->blockSize  = blockSize;
			info->blockCount = blockCount;
			if (ptr == NULL) return PT::STATUS_OUT_OF_MEMORY_ERROR;
			
			Physical	block;
			block.handle      = NULL;
			block.bufferInfo  = NULL;
			block.bufferCount = 0;

			// DMA 転送ができるようにブロック毎に物理メモリに固定する
			PT::Device::TransferDirection	direction = (index == Index_TS) ?
				PT::Device::TRANSFER_DIRECTION_WRITE : PT::Device::TRANSFER_DIRECTION_READ;

			uint32	offset = 0;
			for (uint32 i=0; i<blockCount; i++) {
				status = mDevice->LockBuffer(ptr + offset, blockSize, direction, &block.handle);
				if (U::Error(L"Device::LockBuffer()", status)) return status;

				status = mDevice->GetBufferInfo(block.handle, &block.bufferInfo, &block.bufferCount);
				if (U::Error(L"Device::GetBufferInfo()", status)) return status;

				info->physicals.push_back(block);

				offset += blockSize;
			}

			return status;
		}

		status FreeMemory()
		{
			status	status = PT::STATUS_OK;

			for (uint32 i=0; i<sizeof(mInfo)/sizeof(*mInfo); i++) {
				Info	*info = &mInfo[i];
				std::vector<Physical>	&physicals = info->physicals;
				for (uint32 i=0; i<physicals.size(); i++) {
					Physical	*block = &physicals[i];
					if (block->handle) {
						status = mDevice->UnlockBuffer(block->handle);
						if (U::Error(L"Device::UnlockBuffer()", status)) return status;
					}
				}
				physicals.clear();

				if (info->memory) {
					delete_(info->memory);
					info->memory = NULL;
					info->ptr    = NULL;
					info->size   = 0;
				}
			}

			return status;
		}

		// このサンプルでは 4096 バイトにつき 1つの記述子を構築しています。
		// OS::Memory で確保するメモリサイズは 4096 の倍数で、OS::Memory
		// で取得できる仮想アドレスも 4096 バイト境界になっているので、
		// FPGA.txt に記述されている「4GB を跨いだページ領域は 2つに分割する」
		// 処理は考慮する必要がなくなります。
		void BuildPageDescriptor(bool loop)
		{
			void							*pagePtr       = mInfo[Index_Page].ptr;
			std::vector<Physical>			&pagePhysicals = mInfo[Index_Page].physicals;
			const PT::Device::BufferInfo	*pageInfo      = pagePhysicals[0].bufferInfo;

			uint64	pageAddress = pageInfo->Address;
			uint32	pageSize    = pageInfo->Size;
			pageInfo++;

			// ループ用に先頭の物理アドレスを保存しておく
			uint64	loopPageAddress = pageAddress;

			void	*prevPagePtr = NULL;

			std::vector<Physical>	&tsPhysicals = mInfo[Index_TS].physicals;
			for (uint32 i=0; i<tsPhysicals.size(); i++) {
				Physical						*physical = &tsPhysicals[i];
				const PT::Device::BufferInfo	*tsInfo   = physical->bufferInfo;
				uint32							tsCount   = physical->bufferCount;

				for (uint32 j=0; j<tsCount; j++) {
					uint64	tsAddress = tsInfo->Address;
					uint32	tsSize    = tsInfo->Size;
					tsInfo++;

					_ASSERT((tsSize % OS::Memory::PAGE_SIZE) == 0);
					for (uint32 k=0; k<tsSize/OS::Memory::PAGE_SIZE; k++) {
						// 残りが 20 バイト未満かどうかをチェック
						if (pageSize < 20) {
							// 残り部分をスキップ
							pagePtr = static_cast<uint8 *>(pagePtr) + pageSize;

							// アドレスとサイズを更新
							pageAddress = pageInfo->Address;
							pageSize    = pageInfo->Size;
							pageInfo++;
						}

						// 前の記述子があれば、今の記述子とつなげる
						if (prevPagePtr) {
							LinkPageDescriptor(pageAddress, prevPagePtr);
						}

						// 今の記述子ポインタを保存する
						prevPagePtr = pagePtr;

						// 記述子を構築する (nextAddress は現時点ではわからないため 0 にしておく)
						WritePageDescriptor(tsAddress, OS::Memory::PAGE_SIZE, 0, &pagePtr);

						tsAddress += OS::Memory::PAGE_SIZE;

						pageAddress += 20;
						pageSize    -= 20;
					}
				}
			}

			if (prevPagePtr) {
				if (loop) {
					// 末尾から先頭へループさせる
					LinkPageDescriptor(loopPageAddress, prevPagePtr);
				} else {
					// 末尾で DMA 転送を終了させる
					LinkPageDescriptor(1, prevPagePtr);
				}
			}
		}

		void WritePageDescriptor(uint64 address, uint32 size, uint64 nextAddress, void **ptr_)
		{
			_ASSERT(ptr_);
			uint8	*ptr = static_cast<uint8 *>(*ptr_);
			_ASSERT(ptr);

			*reinterpret_cast<uint64 *>(ptr +  0) = address     | 7;	// [2:0] は無視される
			*reinterpret_cast<uint32 *>(ptr +  8) = size        | 7;	// [2:0] は無視される
			*reinterpret_cast<uint64 *>(ptr + 12) = nextAddress | 2;	// [1  ] は無視される

			*ptr_ = ptr + 20;
		}

		void LinkPageDescriptor(uint64 nextAddress, void *ptr_)
		{
			_ASSERT(ptr_);
			uint8	*ptr = static_cast<uint8 *>(ptr_);
			_ASSERT(ptr);

			*reinterpret_cast<uint64 *>(ptr + 12) = nextAddress | 2;	// [1] は無視される
		}
	};
}
}

#endif

