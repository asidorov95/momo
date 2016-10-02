/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLim4.h

  namespace momo:
    struct HashBucketLim4

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tLogMaxCount, size_t tMemPoolBlockCount>
	class BucketLim4
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t logMaxCount = tLogMaxCount;
		MOMO_STATIC_ASSERT(0 < logMaxCount && logMaxCount <= 4);	//?

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef MemPoolUInt32<memPoolBlockCount, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, uint32_t, MemPool::nullPtr> Memory;
		
		static const size_t maxCount = 1 << logMaxCount;

		static const uint32_t stateNull = ((uint32_t)1 << (32 - logMaxCount)) - 1;
		static const uint32_t stateNullWasFull = stateNull - 1;

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount; ++i)
				{
					auto memPoolCreator = [i, &memManager] (MemPool* newMemPool)
					{
						size_t maxTotalBlockCount = (i == 1)
							? ((size_t)1 << (32 - logMaxCount)) - 2
							: ((size_t)1 << (32 - logMaxCount)) / i;
						new(newMemPool) MemPool(i * sizeof(Item), MemManagerPtr(memManager),
							maxTotalBlockCount);
					};
					mMemPools.AddBackNogrowCrt(memPoolCreator);
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			const MemPool& GetMemPool(size_t memPoolIndex) const MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLim4() MOMO_NOEXCEPT
			: mPtrState(stateNull)
		{
		}

		BucketLim4(const BucketLim4&) = delete;

		~BucketLim4() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(_IsEmpty());
		}

		BucketLim4& operator=(const BucketLim4&) = delete;

		ConstBounds GetBounds(const Params& params) const MOMO_NOEXCEPT
		{
			return ConstBounds(_GetItems<const Item>(params), _GetCount());
		}

		Bounds GetBounds(Params& params) MOMO_NOEXCEPT
		{
			return Bounds(_GetItems<Item>(params), _GetCount());
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtrState == stateNull)
				return false;
			if (mPtrState == stateNullWasFull)
				return true;
			return _GetMemPoolIndex() == _GetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!_IsEmpty())
			{
				Item* items = _GetItems<Item>(params);
				ItemTraits::Destroy(items, _GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(_GetPointer());
			}
			mPtrState = stateNull;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (_IsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex =
					_GetMemPoolIndex((mPtrState == stateNull) ? newCount : maxCount);
				MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
				Memory memory(newMemPool);
				Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
				itemCreator(newItems);
				_Set(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				size_t count = _GetCount();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
					Memory memory(newMemPool);
					Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
					MemPool& memPool = params.GetMemPool(memPoolIndex);
					uint32_t ptr = _GetPointer();
					ItemTraits::RelocateCreate(memPool.template GetRealPointer<Item>(ptr),
						newItems, count, itemCreator, newItems + count);
					memPool.Deallocate(ptr);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					Item* items = _GetItems<Item>(params);
					itemCreator(items + count);
					++mPtrState;
					return items + count;
				}
			}
		}

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(_GetPointer());
				mPtrState = (memPoolIndex < _GetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
			}
			else
			{
				--mPtrState;
			}
		}

	private:
		bool _IsEmpty() const MOMO_NOEXCEPT
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void _Set(uint32_t ptr, size_t memPoolIndex, size_t count)
		{
			mPtrState = (uint32_t)(((memPoolIndex - 1) << (32 - logMaxCount))
				+ (size_t)ptr * memPoolIndex + count - 1);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsEmpty());
			return (size_t)(mPtrState >> (32 - logMaxCount)) + 1;
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return 0;
			return internal::UIntMath<size_t>::DivBySmall((size_t)(mPtrState & stateNull),
				_GetMemPoolIndex()).remainder + 1;
		}

		int32_t _GetPointer() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsEmpty());
			return internal::UIntMath<uint32_t>::DivBySmall(mPtrState & stateNull,
				(uint32_t)_GetMemPoolIndex()).quotient;
		}

		template<typename Item, typename Params>
		Item* _GetItems(Params& params) const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return nullptr;
			auto& memPool = params.GetMemPool(_GetMemPoolIndex());
			return memPool.template GetRealPointer<Item>(_GetPointer());
		}

	private:
		uint32_t mPtrState;
	};
}

template<size_t tLogMaxCount = 2,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketLim4 : public internal::HashBucketBase<1 << tLogMaxCount>
{
	static const size_t logMaxCount = tLogMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	using Bucket = internal::BucketLim4<ItemTraits, MemManager,
		logMaxCount, memPoolBlockCount>;
};

} // namespace momo