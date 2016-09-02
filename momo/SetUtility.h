/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/SetUtility.h

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TItem>
	struct SetItemTraits
	{
		typedef TKey Key;
		typedef TItem Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const size_t alignment = ItemManager::alignment;

		template<typename... ItemArgs>
		using Creator = typename ItemManager::template Creator<ItemArgs...>;

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
			return item;
		}

		static void Destroy(Item& item) MOMO_NOEXCEPT
		{
			ItemManager::Destroy(item);
		}

		static void Assign(Item&& srcItem, Item& dstItem)
		{
			dstItem = std::move(srcItem);
		}

		static void AssignNothrowAnyway(Item&& srcItem, Item& dstItem) MOMO_NOEXCEPT
		{
			ItemManager::AssignNothrowAnyway(std::move(srcItem), dstItem);
		}

		static void AssignKey(Key&& srcKey, Item& dstItem)
		{
			MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
			dstItem = std::move(srcKey);
		}

		static void AssignKey(const Key& srcKey, Item& dstItem)
		{
			MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
			dstItem = srcKey;
		}
	};

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion,
		bool tUsePtr = !std::is_nothrow_move_constructible<TContainerTraits>::value
			|| !std::is_nothrow_move_assignable<TContainerTraits>::value
			|| !std::is_empty<TMemManager>::value || tKeepVersion>
	class SetCrew;

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion>
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion, true>
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

		static const bool keepVersion = tKeepVersion;

	private:
		struct Data
		{
			size_t version;
			ContainerTraits containerTraits;
			MemManager memManager;
		};

	public:
		SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
		{
			mData = memManager.template Allocate<Data>(sizeof(Data));
			mData->version = 0;
			try
			{
				new(&mData->containerTraits) ContainerTraits(containerTraits);
			}
			catch (...)
			{
				memManager.Deallocate(mData, sizeof(Data));
				throw;
			}
			new(&mData->memManager) MemManager(std::move(memManager));
		}

		SetCrew(SetCrew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				mData->containerTraits.~ContainerTraits();
				MemManager memManager = std::move(GetMemManager());
				GetMemManager().~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
			}
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		const size_t* GetVersion() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsNull());
			return &mData->version;
		}

		void IncVersion() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsNull());
			++mData->version;
		}

		const ContainerTraits& GetContainerTraits() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsNull());
			return mData->containerTraits;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsNull());
			return mData->memManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsNull());
			return mData->memManager;
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion>
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion, false>
		: private TContainerTraits, private TMemManager
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<ContainerTraits>::value);
		MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<ContainerTraits>::value);
		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

		static const bool keepVersion = tKeepVersion;
		MOMO_STATIC_ASSERT(!keepVersion);

	public:
		SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
			: ContainerTraits(containerTraits),
			MemManager(std::move(memManager))
		{
		}

		SetCrew(SetCrew&& crew) MOMO_NOEXCEPT
			: ContainerTraits(std::move(crew._GetContainerTraits())),
			MemManager(std::move(crew.GetMemManager()))
		{
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() MOMO_NOEXCEPT
		{
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(_GetContainerTraits(), crew._GetContainerTraits());
		}

		const size_t* GetVersion() const MOMO_NOEXCEPT
		{
			return nullptr;
		}

		void IncVersion() MOMO_NOEXCEPT
		{
		}

		const ContainerTraits& GetContainerTraits() const MOMO_NOEXCEPT
		{
			return *this;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}

	private:
		ContainerTraits& _GetContainerTraits() MOMO_NOEXCEPT
		{
			return *this;
		}
	};

	template<typename TItemTraits>
	class SetExtractedItem
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;

	public:
		SetExtractedItem() MOMO_NOEXCEPT
			: mHasItem(false)
		{
		}

		SetExtractedItem(const SetExtractedItem&) = delete;

		~SetExtractedItem() MOMO_NOEXCEPT
		{
			Clear();
		}

		SetExtractedItem& operator=(const SetExtractedItem&) = delete;

		bool IsEmpty() const MOMO_NOEXCEPT
		{
			return !mHasItem;
		}

		void Clear() MOMO_NOEXCEPT
		{
			if (mHasItem)
				ItemTraits::Destroy(GetItem());
			mHasItem = false;
		}

		Item& GetItem() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mHasItem);
			return *&mItemBuffer;
		}

		template<typename ItemCreator>
		void SetItem(const ItemCreator& itemCreator)
		{
			MOMO_ASSERT(!mHasItem);
			itemCreator(&mItemBuffer);
			mHasItem = true;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		bool mHasItem;
	};
}

} // namespace momo
