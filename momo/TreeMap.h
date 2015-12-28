/**********************************************************\

  momo/TreeMap.h

  namespace momo:
    struct TreeMapKeyValueTraits
    struct TreeMapSettings
    class TreeMap

\**********************************************************/

#pragma once

#include "TreeSet.h"
#include "MapUtility.h"

namespace momo
{

template<typename TKey, typename TValue>
struct TreeMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue>
{
	typedef TKey Key;
	typedef TValue Value;

	typedef internal::ObjectManager<Key> KeyManager;
	typedef internal::ObjectManager<Value> ValueManager;

	static const bool isKeyNothrowAnywaySwappable = KeyManager::isNothrowAnywaySwappable;
	static const bool isValueNothrowAnywaySwappable = ValueManager::isNothrowAnywaySwappable;

	static void SwapKeysNothrowAnyway(Key& key1, Key& key2) MOMO_NOEXCEPT
	{
		KeyManager::SwapNothrowAnyway(key1, key2);
	}

	static void SwapValuesNothrowAnyway(Value& value1, Value& value2) MOMO_NOEXCEPT
	{
		ValueManager::SwapNothrowAnyway(value1, value2);
	}
};

struct TreeMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	//static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION_VALUE;
};

template<typename TKey, typename TValue,
	typename TTreeTraits = TreeTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = TreeMapKeyValueTraits<TKey, TValue>,
	typename TSettings = TreeMapSettings>
class TreeMap
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef TTreeTraits TreeTraits;
	typedef TMemManager MemManager;
	typedef TKeyValueTraits KeyValueTraits;
	typedef TSettings Settings;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	typedef internal::MapKeyValuePair<KeyValueTraits> KeyValuePair;

	struct TreeSetItemTraits
	{
		typedef typename TreeMap::Key Key;
		typedef KeyValuePair Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const bool isNothrowAnywaySwappable = KeyValueTraits::isKeyNothrowAnywaySwappable
			|| KeyValueTraits::isValueNothrowAnywaySwappable;	//?

		static const size_t alignment = ItemManager::alignment;

		template<typename ItemArg>
		class Creator : public ItemManager::template Creator<ItemArg>
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, Item>::value
				|| std::is_same<ItemArg, const Item&>::value));

		private:
			typedef typename ItemManager::template Creator<ItemArg> BaseCreator;

		public:
			//using BaseCreator::BaseCreator;	// vs2013
			explicit Creator(ItemArg&& itemArg)
				: BaseCreator(std::forward<ItemArg>(itemArg))
			{
			}
		};

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return item.GetKey();
		}

		static void Destroy(Item& item) MOMO_NOEXCEPT
		{
			ItemManager::Destroy(item);
		}

		static void Assign(Item&& srcItem, Item& dstItem)
		{
			KeyValuePair::Assign(std::move(srcItem), dstItem);
		}

		static void SwapNothrowAnyway(Item& item1, Item& item2) MOMO_NOEXCEPT
		{
			KeyValuePair::SwapNothrowAnyway(item1, item2);
		}

		template<typename Iterator, typename ItemCreator>
		static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const ItemCreator& itemCreator, void* pobject)
		{
			KeyValuePair::RelocateCreate(srcBegin, dstBegin, count, itemCreator, pobject);
		}
	};

	struct TreeSetSettings : public momo::TreeSetSettings
	{
		static const CheckMode checkMode = Settings::checkMode;
		//static const bool checkVersion = Settings::checkVersion;
	};

	typedef momo::TreeSet<Key, TreeTraits, MemManager, TreeSetItemTraits, TreeSetSettings> TreeSet;

	typedef typename TreeSet::ConstIterator TreeSetConstIterator;

	typedef internal::MapReference<Key, Value, typename TreeSetConstIterator::Reference> Reference;

	typedef internal::MapValueReferencer<TreeMap> ValueReferencer;

public:
	typedef internal::TreeDerivedIterator<TreeSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef typename ValueReferencer::ValueReferenceRKey ValueReferenceRKey;
	typedef typename ValueReferencer::ValueReferenceCKey ValueReferenceCKey;

public:
	explicit TreeMap(const TreeTraits& treeTraits = TreeTraits(),
		MemManager&& memManager = MemManager())
		: mTreeSet(treeTraits, std::move(memManager))
	{
	}

	TreeMap(std::initializer_list<std::pair<Key, Value>> keyValuePairs,
		const TreeTraits& treeTraits = TreeTraits(), MemManager&& memManager = MemManager())
		: TreeMap(treeTraits, std::move(memManager))
	{
		Insert(keyValuePairs);
	}

	TreeMap(TreeMap&& treeMap) MOMO_NOEXCEPT
		: mTreeSet(std::move(treeMap.mTreeSet))
	{
	}

	TreeMap(const TreeMap& treeMap)
		: mTreeSet(treeMap.mTreeSet)
	{
	}

	~TreeMap() MOMO_NOEXCEPT
	{
	}

	TreeMap& operator=(TreeMap&& treeMap) MOMO_NOEXCEPT
	{
		TreeMap(std::move(treeMap)).Swap(*this);
		return *this;
	}

	TreeMap& operator=(const TreeMap& treeMap)
	{
		if (this != &treeMap)
			TreeMap(treeMap).Swap(*this);
		return *this;
	}

	void Swap(TreeMap& treeMap) MOMO_NOEXCEPT
	{
		mTreeSet.Swap(treeMap.mTreeSet);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return ConstIterator(mTreeSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(mTreeSet.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator(mTreeSet.GetEnd());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator(mTreeSet.GetEnd());
	}

	MOMO_FRIEND_SWAP(TreeMap)
	MOMO_FRIENDS_BEGIN_END(const TreeMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(TreeMap&, Iterator)

	const TreeTraits& GetTreeTraits() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetTreeTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mTreeSet.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetCount();
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mTreeSet.IsEmpty();
	}

	void Clear() MOMO_NOEXCEPT
	{
		mTreeSet.Clear();
	}

	ConstIterator LowerBound(const Key& key) const
	{
		return ConstIterator(mTreeSet.LowerBound(key));
	}

	Iterator LowerBound(const Key& key)
	{
		return Iterator(mTreeSet.LowerBound(key));
	}

	ConstIterator UpperBound(const Key& key) const
	{
		return ConstIterator(mTreeSet.UpperBound(key));
	}

	Iterator UpperBound(const Key& key)
	{
		return Iterator(mTreeSet.UpperBound(key));
	}

	ConstIterator Find(const Key& key) const
	{
		return ConstIterator(mTreeSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return Iterator(mTreeSet.Find(key));
	}

	bool HasKey(const Key& key) const
	{
		return mTreeSet.HasKey(key);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(Key&& key, const ValueCreator& valueCreator)
	{
		return _Insert(std::move(key), valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return _Insert(std::move(key),
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(Key&& key, Value&& value)
	{
		return InsertVar(std::move(key), std::move(value));
	}

	InsertResult Insert(Key&& key, const Value& value)
	{
		return InsertVar(std::move(key), value);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(const Key& key, const ValueCreator& valueCreator)
	{
		return _Insert(key, valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return _Insert(key, ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(const Key& key, Value&& value)
	{
		return InsertVar(key, std::move(value));
	}

	InsertResult Insert(const Key& key, const Value& value)
	{
		return InsertVar(key, value);
	}

	template<typename Iterator>
	size_t InsertKV(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		auto insertFunc = [this] (Iterator iter)
			{ return InsertVar(iter->key, iter->value); };
		return _Insert(begin, end, insertFunc);
	}

	template<typename Iterator>
	size_t InsertFS(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		auto insertFunc = [this] (Iterator iter)
			{ return InsertVar(iter->first, iter->second); };
		return _Insert(begin, end, insertFunc);
	}

	size_t Insert(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		return InsertFS(keyValuePairs.begin(), keyValuePairs.end());
	}

	template<typename ValueCreator>
	Iterator AddCrt(ConstIterator iter, Key&& key, const ValueCreator& valueCreator)
	{
		return _Add(iter, std::move(key), valueCreator);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, Key&& key, ValueArgs&&... valueArgs)
	{
		return _Add(iter, std::move(key),
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, Key&& key, Value&& value)
	{
		return AddVar(iter, std::move(key), std::move(value));
	}

	Iterator Add(ConstIterator iter, Key&& key, const Value& value)
	{
		return AddVar(iter, std::move(key), value);
	}

	template<typename ValueCreator>
	Iterator AddCrt(ConstIterator iter, const Key& key, const ValueCreator& valueCreator)
	{
		return _Add(iter, key, valueCreator);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, const Key& key, ValueArgs&&... valueArgs)
	{
		return _Add(iter, key, ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, const Key& key, Value&& value)
	{
		return AddVar(iter, key, std::move(value));
	}

	Iterator Add(ConstIterator iter, const Key& key, const Value& value)
	{
		return AddVar(iter, key, value);
	}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	ValueReferenceRKey operator[](Key&& key)
	{
		Iterator iter = LowerBound((const Key&)key);
		return ValueReferenceRKey(*this, iter,
			_IsEqual(iter, (const Key&)key) ? nullptr : std::addressof(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Iterator iter = LowerBound(key);
		return ValueReferenceCKey(*this, iter,
			_IsEqual(iter, key) ? nullptr : std::addressof(key));
	}
#else
	ValueReferenceRKey operator[](Key&& key)
	{
		return _Insert(std::move(key), ValueCreator<>()).iterator->value;
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		return _Insert(key, ValueCreator<>()).iterator->value;
	}
#endif

	// (!KeyManager::isNothrowAnywayMoveAssignable
	// && !ValueManager::isNothrowAnywayMoveAssignable) -> basic exception safety
	Iterator Remove(ConstIterator iter)
	{
		return Iterator(mTreeSet.Remove(iter.GetBaseIterator()));
	}

	// (!KeyManager::isNothrowAnywayMoveAssignable
	// && !ValueManager::isNothrowAnywayMoveAssignable) -> basic exception safety
	bool Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

private:
	bool _IsEqual(ConstIterator iter, const Key& key) const MOMO_NOEXCEPT
	{
		return iter != GetEnd() && !GetTreeTraits().IsLess(key, iter->key);
	}

	template<typename RKey, typename ValueCreator>
	InsertResult _Insert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = LowerBound((const Key&)key);
		if (_IsEqual(iter, (const Key&)key))
			return InsertResult(iter, false);
		iter = _Add(iter, std::forward<RKey>(key), valueCreator);
		return InsertResult(iter, true);
	}

	template<typename Iterator, typename InsertFunc>
	size_t _Insert(Iterator begin, Iterator end, InsertFunc insertFunc)
	{
		size_t count = 0;
		for (Iterator iter = begin; iter != end; ++iter)
			count += insertFunc(iter).inserted ? 1 : 0;
		return count;
	}

	template<typename RKey, typename ValueCreator>
	Iterator _Add(ConstIterator iter, RKey&& key, const ValueCreator& valueCreator)
	{
		auto pairCreator = [&key, &valueCreator] (void* ppair)
			{ new(ppair) KeyValuePair(std::forward<RKey>(key), valueCreator); };
		return Iterator(mTreeSet.AddCrt(iter.GetBaseIterator(), pairCreator));
	}

private:
	TreeSet mTreeSet;
};

} // namespace momo