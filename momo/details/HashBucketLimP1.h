/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLimP1.h

  namespace momo:
    struct HashBucketLimP1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams, size_t tAlignment>
	class BucketLimP1
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t alignment = tAlignment;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

	public:
		class Params
		{
		public:
			static const bool skipFirstMemPool =
				(maxCount > 1 && ItemTraits::alignment == sizeof(Item));	//?

		private:
			typedef ArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

			static const size_t minMemPoolIndex = skipFirstMemPool ? 2 : 1;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = minMemPoolIndex; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex >= minMemPoolIndex);
				return mMemPools[memPoolIndex - minMemPoolIndex];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP1() MOMO_NOEXCEPT
		{
			_Set(nullptr, _GetMemPoolIndex(1), 0);
		}

		BucketLimP1(const BucketLimP1&) = delete;

		~BucketLimP1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(_GetItems() == nullptr);
		}

		BucketLimP1& operator=(const BucketLimP1&) = delete;

		ConstBounds GetBounds(const Params& /*params*/) const MOMO_NOEXCEPT
		{
			return ConstBounds(_GetItems(), _GetCount());
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(_GetItems(), _GetCount());
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return _GetMemPoolIndex() == _GetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = _GetItems();
			if (items != nullptr)
			{
				ItemTraits::Destroy(items, _GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(items);
			}
			_Set(nullptr, _GetMemPoolIndex(1), 0);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			Item* items = _GetItems();
			if (items == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex();
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
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
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* newItems = memory.GetPointer();
					ItemTraits::RelocateCreate(items, newItems, count,
						itemCreator, newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					itemCreator(items + count);
					++mState;
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
				params.GetMemPool(memPoolIndex).Deallocate(_GetItems());
				if (memPoolIndex != _GetMemPoolIndex(maxCount))
					memPoolIndex = _GetMemPoolIndex(1);
				_Set(nullptr, memPoolIndex, 0);
			}
			else
			{
				--mState;
			}
		}

	private:
		void _Set(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			*&mItemPtrBuffer = items;
			mState = (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			if (Params::skipFirstMemPool && count == 1)
				return 2;
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(mState >> 4);
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & 15);
		}

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			return *&mItemPtrBuffer;
		}

	private:
		ObjectBuffer<Item*, alignment> mItemPtrBuffer;
		unsigned char mState;
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>,
	size_t tAlignment = MOMO_ALIGNMENT_OF(void*)>
struct HashBucketLimP1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t alignment = tAlignment;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Bucket = internal::BucketLimP1<ItemTraits, maxCount, MemPoolParams, alignment>;
};

} // namespace momo
