/**********************************************************\

  momo/details/ArrayBucket.h

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"
#include "../Array.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxFastCount, size_t tMemPoolBlockCount, typename TArraySettings>
	class ArrayBucket
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef TArraySettings ArraySettings;
		typedef typename ItemTraits::Item Item;

		static const size_t maxFastCount = tMaxFastCount;
		MOMO_STATIC_ASSERT(0 < maxFastCount && maxFastCount < 16);

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		struct ArrayItemTraits
		{
			typedef typename ItemTraits::Item Item;

			template<typename Arg>
			static void Create(Arg&& arg, void* pitem)
			{
				MOMO_CHECK_TYPE(Item, arg);
				ItemTraits::Create(std::forward<Arg>(arg), pitem);
			}

			static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
			{
				ItemTraits::Destroy(items, count);
			}

			static void Relocate(Item* /*srcItems*/, Item* /*dstItems*/, size_t /*count*/)
			{
				assert(false);	//?
			}

			template<typename ItemCreator>
			static void RelocateAddBack(Item* srcItems, Item* dstItems, size_t srcCount,
				const ItemCreator& itemCreator)
			{
				ItemTraits::RelocateAddBack(srcItems, dstItems, srcCount, itemCreator);
			}
		};

		typedef momo::Array<Item, MemManagerPtr, ArrayItemTraits, ArraySettings> Array;

		static const size_t arrayAlignment = std::alignment_of<Array>::value;

		typedef momo::MemPool<MemPoolParamsVarSize<ItemTraits::alignment, memPoolBlockCount>,
			MemManagerPtr> FastMemPool;
		typedef momo::MemPool<MemPoolParamsVarSize<arrayAlignment, memPoolBlockCount>,
			MemManagerPtr> ArrayMemPool;

		typedef BucketMemory<FastMemPool, unsigned char*> FastMemory;
		typedef BucketMemory<ArrayMemPool, unsigned char*> ArrayMemory;

	public:
		class Params
		{
		private:
			typedef momo::Array<FastMemPool, MemManagerDummy, momo::ArrayItemTraits<FastMemPool>,
				momo::ArraySettings<maxFastCount>> FastMemPools;

		public:
			Params(MemManager& memManager)
				: mArrayMemPool(typename ArrayMemPool::Params(sizeof(Array) + arrayAlignment),
					MemManagerPtr(memManager))
			{
				for (size_t i = 1; i <= maxFastCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mFastMemPools.AddBackNogrow(FastMemPool(typename FastMemPool::Params(blockSize),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			FastMemPool& GetFastMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex > 0);
				return mFastMemPools[memPoolIndex - 1];
			}

			ArrayMemPool& GetArrayMemPool() MOMO_NOEXCEPT
			{
				return mArrayMemPool;
			}

		private:
			FastMemPools mFastMemPools;
			ArrayMemPool mArrayMemPool;
		};

	public:
		ArrayBucket() MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
		}

		ArrayBucket(ArrayBucket&& bucket) MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
			Swap(bucket);
		}

		ArrayBucket(const ArrayBucket& bucket) = delete;

		ArrayBucket(Params& params, const ArrayBucket& bucket)
		{
			ConstBounds bounds = bucket.GetBounds();
			size_t count = bounds.GetCount();
			if (count == 0)
			{
				mPtr = nullptr;
			}
			else if (count <= maxFastCount)
			{
				size_t memPoolIndex = _GetFastMemPoolIndex(count);
				FastMemory memory(params.GetFastMemPool(memPoolIndex));
				Item* items = _GetFastItems(memory.GetPointer());
				size_t index = 0;
				try
				{
					for (; index < count; ++index)
						ItemTraits::Create(bounds[index], items + index);
				}
				catch (...)
				{
					ItemTraits::Destroy(items, index);
					throw;
				}
				_Set(memory.Extract(), _MakeState(memPoolIndex, count));
			}
			else
			{
				ArrayMemPool& arrayMemPool = params.GetArrayMemPool();
				ArrayMemory memory(arrayMemPool);
				new(&_GetArray(memory.GetPointer())) Array(bounds.GetBegin(),
					bounds.GetEnd(), MemManagerPtr(arrayMemPool.GetMemManager()));
				_Set(memory.Extract(), (unsigned char)0);
			}
		}

		~ArrayBucket() MOMO_NOEXCEPT
		{
			assert(mPtr == nullptr);
		}

		ArrayBucket& operator=(ArrayBucket&& bucket) MOMO_NOEXCEPT
		{
			ArrayBucket(std::move(bucket)).Swap(*this);
			return *this;
		}

		ArrayBucket& operator=(const ArrayBucket& bucket) = delete;

		void Swap(ArrayBucket& bucket) MOMO_NOEXCEPT
		{
			std::swap(mPtr, bucket.mPtr);
		}

		ConstBounds GetBounds() const MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		Bounds GetBounds() MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return;
			size_t memPoolIndex = _GetMemPoolIndex();
			if (memPoolIndex > 0)
			{
				ItemTraits::Destroy(_GetFastItems(), _GetFastCount());
				params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
			}
			else
			{
				_GetArray().~Array();
				params.GetArrayMemPool().Deallocate(mPtr);
			}
			mPtr = nullptr;
		}

		template<typename ItemCreator>
		void AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (mPtr == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetFastMemPoolIndex(newCount);
				FastMemory memory(params.GetFastMemPool(newMemPoolIndex));
				itemCreator(_GetFastItems(memory.GetPointer()));
				_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				if (memPoolIndex > 0)
				{
					size_t count = _GetFastCount();
					assert(count <= memPoolIndex);
					if (count == memPoolIndex)
					{
						size_t newCount = count + 1;
						Item* items = _GetFastItems();
						if (newCount <= maxFastCount)
						{
							size_t newMemPoolIndex = _GetFastMemPoolIndex(newCount);
							FastMemory memory(params.GetFastMemPool(newMemPoolIndex));
							ItemTraits::RelocateAddBack(items,
								_GetFastItems(memory.GetPointer()), count, itemCreator);
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
						}
						else
						{
							ArrayMemPool& arrayMemPool = params.GetArrayMemPool();
							ArrayMemory memory(arrayMemPool);
							Array array = Array::CreateCap(maxFastCount * 2,
								MemManagerPtr(arrayMemPool.GetMemManager()));
							ItemTraits::RelocateAddBack(items, array.GetItems(),
								count, itemCreator);
							array.SetCountCrt(newCount, [] (void* /*pitem*/) { });
							new(&_GetArray(memory.GetPointer())) Array(std::move(array));
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							_Set(memory.Extract(), (unsigned char)0);
						}
					}
					else
					{
						itemCreator(_GetFastItems() + count);
						++*mPtr;
					}
				}
				else
				{
					_GetArray().AddBackCrt(itemCreator);
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = GetBounds().GetCount();
			assert(count > 0);
			if (count == 1)
				return Clear(params);
			if (_GetMemPoolIndex() > 0)
			{
				ItemTraits::Destroy(_GetFastItems() + count - 1, 1);
				--*mPtr;
			}
			else
			{
				_GetArray().RemoveBack();
			}
		}

	private:
		void _Set(unsigned char* ptr, unsigned char state) MOMO_NOEXCEPT
		{
			assert(ptr != nullptr);
			mPtr = ptr;
			*mPtr = state;
		}

		static unsigned char _MakeState(size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			return (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetFastMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxFastCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(mPtr != nullptr);
			return (size_t)(*mPtr >> 4);
		}

		size_t _GetFastCount() const MOMO_NOEXCEPT
		{
			assert(_GetMemPoolIndex() > 0);
			return (size_t)(*mPtr & 15);
		}

		Item* _GetFastItems() const MOMO_NOEXCEPT
		{
			assert(_GetMemPoolIndex() > 0);
			return _GetFastItems(mPtr);
		}

		static Item* _GetFastItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return (Item*)(ptr + ItemTraits::alignment);
		}

		Array& _GetArray() const MOMO_NOEXCEPT
		{
			assert(_GetMemPoolIndex() == 0);
			return _GetArray(mPtr);
		}

		static Array& _GetArray(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return *(Array*)(ptr + arrayAlignment);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
			{
				return Bounds(nullptr, nullptr);
			}
			else if (_GetMemPoolIndex() > 0)
			{
				return Bounds(_GetFastItems(), _GetFastCount());
			}
			else
			{
				Array& array = _GetArray();
				return Bounds(array.GetItems(), array.GetCount());
			}
		}

	private:
		unsigned char* mPtr;
	};
}

} // namespace momo