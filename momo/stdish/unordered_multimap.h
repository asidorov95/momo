/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

  This classes are similar to `std::unordered_multimap`.

  `unordered_multimap` is much more efficient than standard one in
  memory usage. Its implementation is based on hash tables with
  buckets in the form of small arrays.
  `unordered_multimap_open` is based on open addressing hash table.

  Deviations from the `std::unordered_multimap`:
  1. Each of duplicate keys stored only once.
  2. `max_load_factor`, `rehash`, `reserve`, `load_factor` and all
    the functions, associated with buckets or nodes, are not implemented.
  3. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  4. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  5. Type `reference` is not the same as `value_type&`, so
    `for (auto& p : map)` is illegal, but `for (auto p : map)` or
    `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
  6. Functions `begin`, `cbegin` and iterator increment take
    O(bucket_count) time in worst case.
  7. Functions `erase` can throw exceptions thrown by `key_type` and
    `mapped_type` move assignment operators.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.
  But in case of the function `insert`, receiving pair of iterators, it's
  not allowed to pass iterators pointing to the items within the container. 

\**********************************************************/

#pragma once

#include "../HashMultiMap.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename THashMultiMap = HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		MemManagerStd<TAllocator>>>
class unordered_multimap
{
private:
	typedef THashMultiMap HashMultiMap;
	typedef typename HashMultiMap::HashTraits HashTraits;
	typedef typename HashMultiMap::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef THashFunc hasher;
	typedef TEqualFunc key_equal;
	typedef TAllocator allocator_type;

	typedef HashMultiMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	typedef momo::internal::MapReferenceStd<key_type, mapped_type,
		typename HashMultiMap::Iterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef momo::internal::HashDerivedIterator<typename HashMultiMap::Iterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	//node_type;

	//local_iterator;
	//const_local_iterator;

private:
	struct ConstIteratorProxy : public const_iterator
	{
		typedef const_iterator ConstIterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator,
			typename ConstIterator::BaseIterator)
	};

	struct IteratorProxy : public iterator
	{
		typedef iterator Iterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	unordered_multimap()
	{
	}

	explicit unordered_multimap(const allocator_type& alloc)
		: mHashMultiMap(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_multimap(size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_multimap(std::initializer_list<value_type> values,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, equalFunc, alloc)
	{
	}

	unordered_multimap(unordered_multimap&& right) MOMO_NOEXCEPT
		: mHashMultiMap(std::move(right.mHashMultiMap))
	{
	}

	unordered_multimap(unordered_multimap&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF(momo::internal::IsAlwaysEqualAllocator<allocator_type>::value)
		: mHashMultiMap(pvCreateMultiMap(std::move(right), alloc))
	{
	}

	unordered_multimap(const unordered_multimap& right)
		: mHashMultiMap(right.mHashMultiMap)
	{
	}

	unordered_multimap(const unordered_multimap& right, const allocator_type& alloc)
		: mHashMultiMap(right.mHashMultiMap, MemManager(alloc))
	{
	}

	~unordered_multimap() MOMO_NOEXCEPT
	{
	}

	unordered_multimap& operator=(unordered_multimap&& right)
		MOMO_NOEXCEPT_IF(momo::internal::IsAlwaysEqualAllocator<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashMultiMap = pvCreateMultiMap(std::move(right), alloc);
		}
		return *this;
	}

	unordered_multimap& operator=(const unordered_multimap& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashMultiMap = HashMultiMap(right.mHashMultiMap, MemManager(alloc));
		}
		return *this;
	}

	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(unordered_multimap& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashMultiMap.Swap(right.mHashMultiMap);
	}

	friend void swap(unordered_multimap& left, unordered_multimap& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const MOMO_NOEXCEPT
	{
		return mHashMultiMap;
	}

	nested_container_type& get_nested_container() MOMO_NOEXCEPT
	{
		return mHashMultiMap;
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return IteratorProxy(mHashMultiMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mHashMultiMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return IteratorProxy(mHashMultiMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mHashMultiMap.GetEnd());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	//float max_load_factor() const MOMO_NOEXCEPT
	//void max_load_factor(float maxLoadFactor)

	hasher hash_function() const
	{
		return mHashMultiMap.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashMultiMap.GetHashTraits().GetEqualFunc();
	}

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return allocator_type(mHashMultiMap.GetMemManager().GetCharAllocator());
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mHashMultiMap.GetValueCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return size() == 0;
	}

	void clear() MOMO_NOEXCEPT
	{
		mHashMultiMap.Clear();
	}

	//void rehash(size_type bucketCount)
	//void reserve(size_type count)

	const_iterator find(const key_type& key) const
	{
		return equal_range(key).first;
	}

	iterator find(const key_type& key)
	{
		return equal_range(key).first;
	}

	size_type count(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->values.GetCount() : 0;
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		if (!keyIter)
			return std::pair<const_iterator, const_iterator>(end(), end());
		size_t count = keyIter->values.GetCount();
		if (count == 0)	//?
			return std::pair<const_iterator, const_iterator>(end(), end());
		const_iterator first = ConstIteratorProxy(mHashMultiMap.MakeIterator(keyIter, 0));
		const_iterator last = ConstIteratorProxy(std::next(mHashMultiMap.MakeIterator(keyIter, count - 1)));
		return std::pair<const_iterator, const_iterator>(first, last);
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		typename HashMultiMap::KeyIterator keyIter = mHashMultiMap.Find(key);
		if (!keyIter)
			return std::pair<iterator, iterator>(end(), end());
		size_t count = keyIter->values.GetCount();
		if (count == 0)	//?
			return std::pair<iterator, iterator>(end(), end());
		iterator first = IteratorProxy(mHashMultiMap.MakeIterator(keyIter, 0));
		iterator last = IteratorProxy(std::next(mHashMultiMap.MakeIterator(keyIter, count - 1)));
		return std::pair<iterator, iterator>(first, last);
	}

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(const_iterator hint, Value&& value)

	//iterator insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	iterator insert(value_type&& value)
	{
		return pvEmplace(std::forward_as_tuple(value.first),
			std::forward_as_tuple(std::move(value.second)));
	}

	iterator insert(const_iterator, value_type&& value)
	{
		return insert(std::move(value));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const std::pair<First, Second>& value)
	{
		return pvEmplace(std::forward_as_tuple(value.first), std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const_iterator, const std::pair<First, Second>& value)
	{
		return insert(value);
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(std::pair<First, Second>&& value)
	{
		return pvEmplace(std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(const_iterator, std::pair<First, Second>&& value)
	{
		return insert(std::move(value));
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		for (Iterator iter = first; iter != last; ++iter)
			insert(*iter);
	}

	void insert(std::initializer_list<value_type> values)
	{
		insert(values.begin(), values.end());
	}

	iterator emplace()
	{
		return pvEmplace(std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator)
	{
		return emplace();
	}

	template<typename ValueArg>
	iterator emplace(ValueArg&& valueArg)
	{
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator, ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace(KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace_hint(const_iterator, KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return emplace(std::forward<KeyArg>(keyArg), std::forward<MappedArg>(mappedArg));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace(std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(std::move(keyArgs), std::move(mappedArgs));
	}

	iterator erase(const_iterator where)
	{
		typename HashMultiMap::ConstIterator iter = ConstIteratorProxy::GetBaseIterator(where);
		typename HashMultiMap::ConstKeyIterator keyIter = iter.GetKeyIterator();
		if (keyIter->values.GetCount() == 1)
			return IteratorProxy(mHashMultiMap.RemoveKey(keyIter));
		else
			return IteratorProxy(mHashMultiMap.Remove(iter));
	}

	iterator erase(iterator where)
	{
		return erase(static_cast<const_iterator>(where));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (first == last)
		{
			return IteratorProxy(mHashMultiMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(first)));
		}
		if (std::next(first) == last)
			return erase(first);
		typename HashMultiMap::ConstKeyIterator keyIter =
			ConstIteratorProxy::GetBaseIterator(first).GetKeyIterator();
		size_t count = keyIter->values.GetCount();
		MOMO_ASSERT(count > 0);
		if (ConstIteratorProxy(std::next(mHashMultiMap.MakeIterator(keyIter, count - 1))) == last)
			return IteratorProxy(mHashMultiMap.RemoveKey(keyIter));
		throw std::invalid_argument("invalid unordered_multimap erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashMultiMap.RemoveKey(key);
	}

	//iterator insert(node_type&& node)
	//iterator insert(const_iterator, node_type&& node)
	//node_type extract(const_iterator where)
	//node_type extract(const key_type& key)
	//void merge(...)

	//size_type max_bucket_count() const MOMO_NOEXCEPT
	//size_type bucket_count() const MOMO_NOEXCEPT
	//size_type bucket_size(size_type bucketIndex) const
	//local_iterator begin(size_type bucketIndex)
	//const_local_iterator begin(size_type bucketIndex) const
	//local_iterator end(size_type bucketIndex)
	//const_local_iterator end(size_type bucketIndex) const
	//const_local_iterator cbegin(size_type bucketIndex) const
	//const_local_iterator cend(size_type bucketIndex) const
	//size_type bucket(const key_type& key) const
	//float load_factor() const MOMO_NOEXCEPT

	bool operator==(const unordered_multimap& right) const
	{
		if (mHashMultiMap.GetKeyCount() != right.mHashMultiMap.GetKeyCount())
			return false;
		if (mHashMultiMap.GetValueCount() != right.mHashMultiMap.GetValueCount())
			return false;
		for (typename HashMultiMap::ConstKeyIterator::Reference ref : mHashMultiMap.GetKeyBounds())
		{
			typename HashMultiMap::ConstKeyIterator keyIterRight = right.mHashMultiMap.Find(ref.key);
			if (!keyIterRight)
				return false;
			typename HashMultiMap::ConstValueBounds values = ref.values;
			typename HashMultiMap::ConstValueBounds valuesRight = keyIterRight->values;
			if (values.GetCount() != valuesRight.GetCount())
				return false;
			if (!std::is_permutation(values.GetBegin(), values.GetEnd(), valuesRight.GetBegin()))
				return false;
		}
		return true;
	}

	bool operator!=(const unordered_multimap& right) const
	{
		return !(*this == right);
	}

private:
	static HashMultiMap pvCreateMultiMap(unordered_multimap&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mHashMultiMap);
		HashMultiMap hashMultiMap(right.mHashMultiMap.GetHashTraits(), MemManager(alloc));
		for (reference ref : right)
			hashMultiMap.Add(ref.first, std::move(ref.second));
		right.clear();
		return hashMultiMap;
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator pvEmplace(std::tuple<KeyArgs...>&& keyArgs, std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename HashMultiMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return pvInsert(std::move(keyArgs),
			MappedCreator(mHashMultiMap.GetMemManager(), std::move(mappedArgs)));
	}

	template<typename... KeyArgs, typename MappedCreator>
	iterator pvInsert(std::tuple<KeyArgs...>&& keyArgs, const MappedCreator& mappedCreator)
	{
		MemManager& memManager = mHashMultiMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type, HashMultiMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(&keyBuffer);
		iterator resIter;
		try
		{
			resIter = pvInsert(std::forward_as_tuple(std::move(*&keyBuffer)), mappedCreator);
		}
		catch (...)
		{
			KeyManager::Destroy(memManager, *&keyBuffer);
			throw;
		}
		KeyManager::Destroy(memManager, *&keyBuffer);
		return resIter;
	}

	template<typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type,
		typename = typename std::enable_if<std::is_same<key_type, Key>::value>::type>
	iterator pvInsert(std::tuple<RKey>&& key, const MappedCreator& mappedCreator)
	{
		return IteratorProxy(mHashMultiMap.AddCrt(
			std::forward<RKey>(std::get<0>(key)), mappedCreator));
	}

private:
	HashMultiMap mHashMultiMap;
};

template<typename TKey, typename TMapped,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap_open = unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketDefaultOpen>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
