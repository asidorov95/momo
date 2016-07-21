/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataRow.h

\**********************************************************/

#pragma once

#include "Array.h"

#include <atomic>	//?

namespace momo
{

namespace experimental
{

namespace internal
{
	template<typename TColumnList>
	class DataRow
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

	public:
		DataRow(Raw* raw, const ColumnList* columnList, std::atomic<Raw*>* freeRaws) noexcept
			: mRaw(raw),
			mColumnList(columnList),
			mFreeRaws(freeRaws)
		{
		}

		DataRow(DataRow&& row) noexcept
			: mRaw(row.mRaw),
			mColumnList(row.mColumnList),
			mFreeRaws(row.mFreeRaws)
		{
			row.mRaw = nullptr;
			row.mFreeRaws = nullptr;
		}

		DataRow(const DataRow&) = delete;

		~DataRow() noexcept
		{
			if (mRaw != nullptr)
			{
				mColumnList->DestroyRaw(mRaw);
				while (true)
				{
					Raw* headRaw = *mFreeRaws;
					*reinterpret_cast<Raw**>(mRaw) = headRaw;
					if (mFreeRaws->compare_exchange_weak(headRaw, mRaw))
						break;
				}
			}
		}

		DataRow& operator=(DataRow&& row) noexcept
		{
			DataRow(std::move(row)).Swap(*this);
			return *this;
		}

		DataRow& operator=(const DataRow&) = delete;

		void Swap(DataRow& row) noexcept
		{
			std::swap(mRaw, row.mRaw);
			std::swap(mColumnList, row.mColumnList);
			std::swap(mFreeRaws, row.mFreeRaws);
		}

		template<typename Type>
		const Type& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<Type>(mRaw, offset);
		}

		template<typename Type>
		Type& GetByOffset(size_t offset)
		{
			return mColumnList->template GetByOffset<Type>(mRaw, offset);
		}

		template<typename Type>
		const Type& GetByColumn(const Column<Type>& column) const
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		template<typename Type>
		Type& GetByColumn(const Column<Type>& column)
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		template<typename Type>
		const Type& operator[](const Column<Type>& column) const
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		template<typename Type>
		Type& operator[](const Column<Type>& column)
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		template<typename Type, typename... Args>
		void Fill(const Column<Type>& column, const Type& item, Args&&... args)
		{
			GetByColumn(column) = item;
			Fill(std::forward<Args>(args)...);
		}

		void Fill() noexcept
		{
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* GetRaw() noexcept
		{
			return mRaw;
		}

		Raw* ExtractRaw() noexcept
		{
			Raw* raw = mRaw;
			mRaw = nullptr;
			return raw;
		}

		const Raw* operator->() const noexcept
		{
			return mRaw;
		}

		Raw* operator->() noexcept
		{
			return mRaw;
		}

	private:
		Raw* mRaw;
		const ColumnList* mColumnList;
		std::atomic<Raw*>* mFreeRaws;
	};

	template<typename TColumnList>
	class DataRowRef
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		template<typename Type>
		class ItemRef
		{
		public:
			ItemRef(Type& item, bool readOnly) noexcept
				: mItem(item),
				mReadOnly(readOnly)
			{
			}

			ItemRef(ItemRef&& itemRef) noexcept
				: mItem(itemRef.mItem),
				mReadOnly(itemRef.mReadOnly)
			{
			}

			ItemRef(const ItemRef&) = delete;

			~ItemRef() noexcept
			{
			}

			ItemRef& operator=(ItemRef&& itemRef)
			{
				return _Assign(std::move(itemRef.mItem));
			}

			ItemRef& operator=(const ItemRef& itemRef)
			{
				return _Assign(static_cast<const Type&>(itemRef.mItem));
			}

			ItemRef& operator=(Type&& item)
			{
				return _Assign(std::move(item));
			}

			ItemRef& operator=(const Type& item)
			{
				return _Assign(item);
			}

			operator const Type&() const noexcept
			{
				return mItem;
			}

			const Type& Get() const noexcept
			{
				return mItem;
			}

		private:
			template<typename Arg>
			ItemRef& _Assign(Arg&& arg)
			{
				if (mReadOnly)
					throw std::runtime_error("Item is read only");
				mItem = std::forward<Arg>(arg);
				return *this;
			}

		private:
			Type& mItem;
			bool mReadOnly;
		};

	public:
		DataRowRef(Raw* raw, const ColumnList* columnList, const bool* offsetMarks) noexcept
			: mRaw(raw),
			mColumnList(columnList),
			mOffsetMarks(offsetMarks)
		{
		}

		template<typename Type>
		ItemRef<Type> GetByOffset(size_t offset) const
		{
			return ItemRef<Type>(mColumnList->template GetByOffset<Type>(mRaw, offset),
				mOffsetMarks[offset]);
		}

		template<typename Type>
		ItemRef<Type> GetByColumn(const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->template GetOffset<Type>(column));
		}

		template<typename Type>
		ItemRef<Type> operator[](const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->template GetOffset<Type>(column));
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
		{
			return mRaw;
		}

	private:
		Raw* mRaw;
		const ColumnList* mColumnList;
		const bool* mOffsetMarks;
	};

	template<typename TColumnList>
	class DataConstRowRef
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

	public:
		DataConstRowRef(const Raw* raw, const ColumnList* columnList) noexcept
			: mRaw(raw),
			mColumnList(columnList)
		{
		}

		DataConstRowRef(DataRowRef<ColumnList> rowRef) noexcept
			: DataConstRowRef(rowRef.GetRaw(), &rowRef.GetColumnList())
		{
		}

		DataConstRowRef(const DataRow<ColumnList>& row) noexcept
			: DataConstRowRef(row.GetRaw(), &row.GetColumnList())
		{
		}

		template<typename Type>
		const Type& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<Type>(mRaw, offset);
		}

		template<typename Type>
		const Type& GetByColumn(const Column<Type>& column) const
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		template<typename Type>
		const Type& operator[](const Column<Type>& column) const
		{
			return mColumnList->GetByColumn(mRaw, column);
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
		{
			return mRaw;
		}

	private:
		const Raw* mRaw;
		const ColumnList* mColumnList;
	};

	template<typename TColumn, typename TMemManager>
	class DataConstSelection
	{
	public:
		typedef TColumn ColumnList;
		typedef TMemManager MemManager;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef DataRowRef<ColumnList> RowRef;

		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*>, ArraySettings<2>> Raws;

	public:
		DataConstSelection(Raws&& raws, const ColumnList* columnList) noexcept
			: mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

		size_t GetCount() const noexcept
		{
			return mRaws.GetCount();
		}

		const RowRef operator[](size_t index) const
		{
			return RowRef(mRaws[index], mColumnList);
		}

	private:
		const ColumnList* mColumnList;
		Raws mRaws;
	};
}

} // namespace experimental

} // namespace momo
