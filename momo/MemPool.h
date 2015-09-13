/**********************************************************\

  momo/MemPool.h

  namespace momo:
    struct MemPoolConst
    class MemPoolParams
    class MemPoolParamsVarSize
    struct MemPoolSettings
    class MemPool

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "Array.h"

namespace momo
{

struct MemPoolConst
{
	static const size_t defaultBlockCount = MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT;
};

template<size_t tBlockSize,
	size_t tBlockAlignment = MOMO_MAX_ALIGNMENT,	//?
	size_t tBlockCount = MemPoolConst::defaultBlockCount>
class MemPoolParams
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

	static const size_t blockSize = (blockCount == 1)
		? ((tBlockSize > 0) ? tBlockSize : 1)
		: ((tBlockSize <= blockAlignment)
			? 2 * blockAlignment
			: ((tBlockSize - 1) / blockAlignment + 1) * blockAlignment);
};

template<size_t tBlockAlignment = MOMO_MAX_ALIGNMENT,
	size_t tBlockCount = MemPoolConst::defaultBlockCount>
class MemPoolParamsVarSize
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

public:
	explicit MemPoolParamsVarSize(size_t blockSize) MOMO_NOEXCEPT
	{
		this->blockSize = (blockCount == 1)
			? ((blockSize > 0) ? blockSize : 1)
			: ((blockSize <= blockAlignment)
				? 2 * blockAlignment
				: ((blockSize - 1) / blockAlignment + 1) * blockAlignment);
	}

protected:
	size_t blockSize;
};

template<size_t tCachedFreeBlockCount = 16>
struct MemPoolSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;
};

template<typename TParams = MemPoolParamsVarSize<>,
	typename TMemManager = MemManagerDefault,
	typename TSettings = MemPoolSettings<>>
class MemPool : private TParams, private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

#ifdef MOMO_USE_TRIVIALLY_COPIABLE
	MOMO_STATIC_ASSERT(std::is_trivially_copyable<Params>::value);
#else
	MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<Params>::value);
	MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<Params>::value);
#endif

private:
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

	typedef Array<void*, internal::MemManagerDummy, ArrayItemTraits<void*>,
		ArraySettings<Settings::cachedFreeBlockCount>> CachedFreeBlocks;

	typedef internal::UIntMath<size_t> SMath;
	typedef internal::UIntMath<uintptr_t> PMath;

	struct BufferChars
	{
		signed char firstFreeBlockIndex;
		signed char freeBlockCount;
	};

	struct BufferPointers
	{
		uintptr_t prevBuffer;
		uintptr_t nextBuffer;
		uintptr_t begin;
	};

	static const uintptr_t nullPtr = (uintptr_t)nullptr;

	static const size_t maxAlignment = std::alignment_of<std::max_align_t>::value;
	MOMO_STATIC_ASSERT((maxAlignment & (maxAlignment - 1)) == 0);

public:
	explicit MemPool(const Params& params = Params(), MemManager&& memManager = MemManager())
		: Params(params),
		MemManagerWrapper(std::move(memManager)),
		mBufferHead(nullPtr),
		mAllocCount(0)
	{
		_CheckParams();
	}

	MemPool(MemPool&& memPool) MOMO_NOEXCEPT
		: Params(std::move(memPool._GetParams())),
		MemManagerWrapper(std::move(memPool._GetMemManagerWrapper())),
		mBufferHead(memPool.mBufferHead),
		mAllocCount(memPool.mAllocCount),
		mCachedFreeBlocks(std::move(memPool.mCachedFreeBlocks))
	{
		memPool.mBufferHead = nullPtr;
		memPool.mAllocCount = 0;
	}

	MemPool& operator=(MemPool&& memPool) MOMO_NOEXCEPT
	{
		MemPool(std::move(memPool)).Swap(*this);
		return *this;
	}

	void Swap(MemPool& memPool) MOMO_NOEXCEPT
	{
		std::swap(_GetMemManagerWrapper(), memPool._GetMemManagerWrapper());
		std::swap(_GetParams(), memPool._GetParams());
		std::swap(mBufferHead, memPool.mBufferHead);
		std::swap(mAllocCount, memPool.mAllocCount);
		mCachedFreeBlocks.Swap(memPool.mCachedFreeBlocks);
	}

	MOMO_FRIEND_SWAP(MemPool)

	~MemPool() MOMO_NOEXCEPT
	{
		MOMO_EXTRA_CHECK(mAllocCount == 0);
		if (Settings::cachedFreeBlockCount > 0)
			_FlushDeallocate();
		MOMO_EXTRA_CHECK(mBufferHead == nullPtr);
	}

	size_t GetBlockSize() const MOMO_NOEXCEPT
	{
		return Params::blockSize;
	}

	size_t GetBlockAlignment() const MOMO_NOEXCEPT
	{
		return Params::blockAlignment;
	}

	size_t GetBlockCount() const MOMO_NOEXCEPT
	{
		return Params::blockCount;
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	void* Allocate()
	{
		void* pblock;
		if (Settings::cachedFreeBlockCount > 0 && mCachedFreeBlocks.GetCount() > 0)
		{
			pblock = mCachedFreeBlocks.GetBackItem();
			mCachedFreeBlocks.RemoveBack();
		}
		else
		{
			if (Params::blockCount > 1)
				pblock = (void*)_NewBlock();
			else if (maxAlignment % Params::blockAlignment == 0)
				pblock = GetMemManager().Allocate(Params::blockSize);
			else
				pblock = (void*)_NewBlock1();
		}
		++mAllocCount;
		return pblock;
	}

	void Deallocate(void* pblock) MOMO_NOEXCEPT
	{
		assert(pblock != nullptr);
		assert(mAllocCount > 0);
		if (Settings::cachedFreeBlockCount > 0)
		{
			if (mCachedFreeBlocks.GetCount() == Settings::cachedFreeBlockCount)
				_FlushDeallocate();
			mCachedFreeBlocks.AddBackNogrow(pblock);
		}
		else
		{
			_DeleteBlock(pblock);
		}
		--mAllocCount;
	}

private:
	Params& _GetParams() MOMO_NOEXCEPT
	{
		return *this;
	}

	const MemManagerWrapper& _GetMemManagerWrapper() const MOMO_NOEXCEPT
	{
		return *this;
	}

	MemManagerWrapper& _GetMemManagerWrapper() MOMO_NOEXCEPT
	{
		return *this;
	}

	void _CheckParams() const
	{
		MOMO_CHECK(0 < Params::blockCount && Params::blockCount < 128);
		MOMO_CHECK(0 < Params::blockAlignment && Params::blockAlignment <= 1024);
		MOMO_CHECK(Params::blockSize > 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize % Params::blockAlignment == 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize / Params::blockAlignment >= 2);
		size_t maxBlockSize =
			(SIZE_MAX - 2 - 3 * sizeof(void*) - 4 * Params::blockAlignment) / Params::blockCount;
		if (Params::blockSize > maxBlockSize)
			throw std::length_error("momo::MemPool length error");
	}

	void _FlushDeallocate() MOMO_NOEXCEPT
	{
		for (void* pblock : mCachedFreeBlocks)
			_DeleteBlock(pblock);
		mCachedFreeBlocks.Clear();
	}

	void _DeleteBlock(void* pblock) MOMO_NOEXCEPT
	{
		if (Params::blockCount > 1)
			_DeleteBlock((uintptr_t)pblock);
		else if (maxAlignment % Params::blockAlignment == 0)
			GetMemManager().Deallocate(pblock, Params::blockSize);
		else
			_DeleteBlock1((uintptr_t)pblock);
	}

	uintptr_t _NewBlock1()
	{
		uintptr_t begin = (uintptr_t)GetMemManager().Allocate(_GetBufferSize1());
		uintptr_t block = PMath::Ceil(begin, (uintptr_t)Params::blockAlignment);
		_GetBufferBegin1(block) = begin;
		return block;
	}

	void _DeleteBlock1(uintptr_t block) MOMO_NOEXCEPT
	{
		uintptr_t begin = _GetBufferBegin1(block);
		GetMemManager().Deallocate((void*)begin, _GetBufferSize1());
	}

	size_t _GetBufferSize1() const MOMO_NOEXCEPT
	{
		size_t bufferUsefulSize = Params::blockSize + Params::blockAlignment
			- SMath::GCD(maxAlignment, Params::blockAlignment);
		size_t bufferSize = SMath::Ceil(bufferUsefulSize, sizeof(void*)) + sizeof(void*);
		return bufferSize;
	}

	uintptr_t& _GetBufferBegin1(uintptr_t block) MOMO_NOEXCEPT
	{
		return *(uintptr_t*)PMath::Ceil(block + Params::blockSize, (uintptr_t)sizeof(void*));
	}

	uintptr_t _NewBlock()
	{
		if (mBufferHead == nullPtr)
			mBufferHead = _NewBuffer();
		uintptr_t block;
		size_t freeBlockCount = (size_t)_NewBlock(mBufferHead, block);
		if (freeBlockCount == 0)
			_RemoveBuffer(mBufferHead, false);
		return block;
	}

	void _DeleteBlock(uintptr_t block) MOMO_NOEXCEPT
	{
		uintptr_t buffer = _GetBuffer(block);
		size_t freeBlockCount = (size_t)_DeleteBlock(buffer, block);
		if (freeBlockCount == 1)
		{
			BufferPointers& pointers = _GetBufferPointers(buffer);
			pointers.prevBuffer = nullPtr;
			pointers.nextBuffer = mBufferHead;
			if (mBufferHead != nullPtr)
				_GetBufferPointers(mBufferHead).prevBuffer = buffer;
			mBufferHead = buffer;
		}
		if (freeBlockCount == Params::blockCount)
			_RemoveBuffer(buffer, true);
	}

	uintptr_t _GetBuffer(uintptr_t block) const MOMO_NOEXCEPT
	{
		assert(block % Params::blockAlignment == 0);
		uintptr_t buffer = PMath::Ceil(block, (uintptr_t)(Params::blockSize * Params::blockCount))
			+ (block % Params::blockSize);
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
			buffer -= Params::blockSize * Params::blockCount + Params::blockAlignment;
		return buffer;
	}

	signed char _NewBlock(uintptr_t buffer, uintptr_t& block) MOMO_NOEXCEPT
	{
		BufferChars& chars = _GetBufferChars(buffer);
		block = _GetBlock(buffer, chars.firstFreeBlockIndex);
		chars.firstFreeBlockIndex = _GetNextFreeBlockIndex(block);
		--chars.freeBlockCount;
		return chars.freeBlockCount;
	}

	signed char _DeleteBlock(uintptr_t buffer, uintptr_t block) MOMO_NOEXCEPT
	{
		BufferChars& chars = _GetBufferChars(buffer);
		_GetNextFreeBlockIndex(block) = chars.firstFreeBlockIndex;
		chars.firstFreeBlockIndex = _GetBlockIndex(buffer, block);
		++chars.freeBlockCount;
		return chars.freeBlockCount;
	}

	signed char& _GetNextFreeBlockIndex(uintptr_t block) MOMO_NOEXCEPT
	{
		return *(signed char*)block;
	}

	uintptr_t _GetBlock(uintptr_t buffer, signed char index) const MOMO_NOEXCEPT
	{
		if (index < 0)
			return buffer - (size_t)(-index) * Params::blockSize;
		else
			return buffer + (size_t)index * Params::blockSize + Params::blockAlignment;
	}

	signed char _GetBlockIndex(uintptr_t buffer, uintptr_t block) const MOMO_NOEXCEPT
	{
		if (block < buffer)
			return -(signed char)((buffer - block) / Params::blockSize);
		else
			return (signed char)((block - buffer) / Params::blockSize);
	}

	uintptr_t _NewBuffer()
	{
		uintptr_t begin = (uintptr_t)GetMemManager().Allocate(_GetBufferSize());
		uintptr_t block = PMath::Ceil(begin, (uintptr_t)Params::blockAlignment);
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
			block += Params::blockAlignment;
		if ((block + Params::blockAlignment) % Params::blockSize == 0)
			block += Params::blockAlignment;
		if (((block / Params::blockSize) % Params::blockCount) == 0)
			block += Params::blockAlignment;
		uintptr_t buffer = _GetBuffer(block);
		signed char index = _GetBlockIndex(buffer, block);
		_GetFirstBlockIndex(buffer) = index;
		BufferChars& chars = _GetBufferChars(buffer);
		chars.firstFreeBlockIndex = index;
		chars.freeBlockCount = (signed char)Params::blockCount;
		BufferPointers& pointers = _GetBufferPointers(buffer);
		pointers.prevBuffer = nullPtr;
		pointers.nextBuffer = nullPtr;
		pointers.begin = begin;
		for (size_t i = 1; i < Params::blockCount; ++i)
		{
			++index;
			_GetNextFreeBlockIndex(block) = index;
			block = _GetBlock(buffer, index);
		}
		_GetNextFreeBlockIndex(block) = (signed char)(-128);
		return buffer;
	}

	void _RemoveBuffer(uintptr_t buffer, bool deallocate) MOMO_NOEXCEPT
	{
		BufferPointers& pointers = _GetBufferPointers(buffer);
		if (pointers.prevBuffer != nullPtr)
			_GetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
		if (pointers.nextBuffer != nullPtr)
			_GetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
		if (mBufferHead == buffer)
			mBufferHead = pointers.nextBuffer;
		if (deallocate)
			GetMemManager().Deallocate((void*)pointers.begin, _GetBufferSize());
	}

	size_t _GetBufferSize() const MOMO_NOEXCEPT
	{
		size_t bufferUsefulSize = Params::blockCount * Params::blockSize
			+ (3 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment;
		if ((Params::blockAlignment & (Params::blockAlignment - 1)) == 0)
			bufferUsefulSize -= std::minmax((size_t)maxAlignment, (size_t)Params::blockAlignment).first;	// gcc & llvm
		else
			bufferUsefulSize -= SMath::GCD(maxAlignment, Params::blockAlignment);
		size_t bufferSize = SMath::Ceil(bufferUsefulSize, sizeof(void*)) + 3 * sizeof(void*)
			+ ((Params::blockAlignment <= 2) ? 2 : 0);
		return bufferSize;
	}

	signed char& _GetFirstBlockIndex(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return *(signed char*)buffer;
	}

	BufferChars& _GetBufferChars(uintptr_t buffer) MOMO_NOEXCEPT
	{
		if (Params::blockAlignment > 2)
			return *(BufferChars*)(buffer + 1);
		else
			return *(BufferChars*)(&_GetBufferPointers(buffer) + 1);
	}

	BufferPointers& _GetBufferPointers(uintptr_t buffer) MOMO_NOEXCEPT
	{
		size_t offset = Params::blockCount - (size_t)(-_GetFirstBlockIndex(buffer));
		return *(BufferPointers*)PMath::Ceil(buffer + Params::blockAlignment
			+ Params::blockSize * offset, (uintptr_t)sizeof(void*));
	}

private:
	MOMO_DISABLE_COPY_CONSTRUCTOR(MemPool);
	MOMO_DISABLE_COPY_OPERATOR(MemPool);

private:
	uintptr_t mBufferHead;
	size_t mAllocCount;
	CachedFreeBlocks mCachedFreeBlocks;
};

namespace internal
{
	template<size_t tBlockCount, typename TMemManager>
	class MemPoolUInt32
	{
	public:
		typedef TMemManager MemManager;

		static const size_t blockCount = tBlockCount;
		MOMO_STATIC_ASSERT(blockCount > 0);

		static const size_t minBlockSize = sizeof(uint32_t);
		static const size_t maxBlockSize = SIZE_MAX / blockCount;
		MOMO_STATIC_ASSERT(minBlockSize <= maxBlockSize);

		static const uint32_t nullPtr = UINT32_MAX;

	private:
		typedef Array<void*, MemManager> Buffers;

	public:
		MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mBuffers(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxBufferCount(maxTotalBlockCount / blockCount),
			mBlockSize(blockSize),
			mAllocCount(0)
		{
			assert(maxTotalBlockCount < (size_t)UINT32_MAX);
			if (mBlockSize < minBlockSize)
				mBlockSize = minBlockSize;
			if (mBlockSize > maxBlockSize)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
		}

		~MemPoolUInt32() MOMO_NOEXCEPT
		{
			assert(mAllocCount == 0);
			_Clear();
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		const void* GetRealPointer(uint32_t ptr) const MOMO_NOEXCEPT
		{
			return _GetRealPointer(ptr);
		}

		void* GetRealPointer(uint32_t ptr) MOMO_NOEXCEPT
		{
			return _GetRealPointer(ptr);
		}

		uint32_t Allocate()
		{
			if (mBlockHead == nullPtr)
				_NewBuffer();
			uint32_t ptr = mBlockHead;
			mBlockHead = *(uint32_t*)GetRealPointer(mBlockHead);
			++mAllocCount;
			return ptr;
		}

		void Deallocate(uint32_t ptr) MOMO_NOEXCEPT
		{
			assert(ptr != nullPtr);
			assert(mAllocCount > 0);
			*(uint32_t*)GetRealPointer(ptr) = mBlockHead;
			mBlockHead = ptr;
			--mAllocCount;
			if (mAllocCount == 0)
				_Shrink();
		}

	private:
		void* _GetRealPointer(uint32_t ptr) const MOMO_NOEXCEPT
		{
			assert(ptr != nullPtr);
			char* buffer = (char*)mBuffers[ptr / blockCount];
			void* realPtr = buffer + (size_t)(ptr % blockCount) * mBlockSize;
			return realPtr;
		}

		void _NewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
			mBuffers.Reserve(bufferCount + 1);
			char* buffer = (char*)GetMemManager().Allocate(_GetBufferSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				void* realPtr = buffer + mBlockSize * i;
				*(uint32_t*)realPtr = (i + 1 < blockCount)
					? (uint32_t)(bufferCount * blockCount + i + 1) : nullPtr;
			}
			mBlockHead = (uint32_t)(bufferCount * blockCount);
			mBuffers.AddBackNogrow(buffer);
		}

		void _Shrink() MOMO_NOEXCEPT
		{
			if (mBuffers.GetCount() > 2)
			{
				_Clear();
				mBlockHead = nullPtr;
				mBuffers.Clear(true);
			}
		}

		void _Clear() MOMO_NOEXCEPT
		{
			MemManager& memManager = GetMemManager();
			size_t bufferSize = _GetBufferSize();
			for (void* buffer : mBuffers)
				memManager.Deallocate(buffer, bufferSize);
		}

		size_t _GetBufferSize() const MOMO_NOEXCEPT
		{
			return blockCount * mBlockSize;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(MemPoolUInt32);
		MOMO_DISABLE_COPY_OPERATOR(MemPoolUInt32);

	private:
		Buffers mBuffers;
		uint32_t mBlockHead;
		size_t mMaxBufferCount;
		size_t mBlockSize;
		size_t mAllocCount;
	};
}

} // namespace momo
