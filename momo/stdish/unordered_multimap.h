/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

  This classes are similar to `std::unordered_multimap`.

  `unordered_multimap` is much more efficient than standard one in
  memory usage. Its implementation is based on hash table with
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
  6. Functions `clear`, `begin`, `cbegin` and iterator increment take
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
	typedef TAllocator allocator_type;

	typedef typename HashTraits::HashFunc hasher;
	typedef typename HashTraits::EqualFunc key_equal;

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
	template<typename KeyArg>
	struct IsValidKeyArg : public HashTraits::template IsValidKeyArg<KeyArg>
	{
	};

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

	unordered_multimap(unordered_multimap&& right) noexcept
		: mHashMultiMap(std::move(right.mHashMultiMap))
	{
	}

	unordered_multimap(unordered_multimap&& right, const allocator_type& alloc)
		noexcept(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value)
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

	~unordered_multimap() noexcept
	{
	}

	unordered_multimap& operator=(unordered_multimap&& right)
		noexcept(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMultiMap = pvCreateMultiMap(std::move(right), alloc);
		}
		return *this;
	}

	unordered_multimap& operator=(const unordered_multimap& right)
	{
		if (this != &right)
		{
			bool propagate = momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMultiMap = HashMultiMap(right.mHashMultiMap, MemManager(alloc));
		}
		return *this;
	}

	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		HashMultiMap hashMultiMap(mHashMultiMap.GetHashTraits(), MemManager(get_allocator()));
		hashMultiMap.Add(values.begin(), values.end());
		mHashMultiMap = std::move(hashMultiMap);
		return *this;
	}

	void swap(unordered_multimap& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashMultiMap.Swap(right.mHashMultiMap);
	}

	friend void swap(unordered_multimap& left, unordered_multimap& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mHashMultiMap;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mHashMultiMap;
	}

	iterator begin() noexcept
	{
		return IteratorProxy(mHashMultiMap.GetBegin());
	}

	const_iterator begin() const noexcept
	{
		return ConstIteratorProxy(mHashMultiMap.GetBegin());
	}

	iterator end() noexcept
	{
		return IteratorProxy(mHashMultiMap.GetEnd());
	}

	const_iterator end() const noexcept
	{
		return ConstIteratorProxy(mHashMultiMap.GetEnd());
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	//float max_load_factor() const noexcept
	//void max_load_factor(float maxLoadFactor)

	hasher hash_function() const
	{
		return mHashMultiMap.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashMultiMap.GetHashTraits().GetEqualFunc();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mHashMultiMap.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mHashMultiMap.GetValueCount();
	}

	MOMO_NODISCARD bool empty() const noexcept
	{
		return size() == 0;
	}

	void clear() noexcept
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

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, const_iterator> find(
		const KeyArg& key) const
	{
		return equal_range(key).first;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, iterator> find(const KeyArg& key)
	{
		return equal_range(key).first;
	}

	const_iterator find(const key_type& key, size_t hashCode) const
	{
		return equal_range(key, hashCode).first;
	}

	iterator find(const key_type& key, size_t hashCode)
	{
		return equal_range(key, hashCode).first;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, const_iterator> find(
		const KeyArg& key, size_t hashCode) const
	{
		return equal_range(key, hashCode).first;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, iterator> find(const KeyArg& key,
		size_t hashCode)
	{
		return equal_range(key, hashCode).first;
	}

	size_type count(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, size_type> count(
		const KeyArg& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	size_type count(const key_type& key, size_t hashCode) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key, hashCode);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, size_type> count(
		const KeyArg& key, size_t hashCode) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key, hashCode);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	bool contains(const key_type& key) const
	{
		return count(key) > 0;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, bool> contains(const KeyArg& key) const
	{
		return count(key) > 0;
	}

	bool contains(const key_type& key, size_t hashCode) const
	{
		return count(key, hashCode) > 0;
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, bool> contains(const KeyArg& key,
		size_t hashCode) const
	{
		return count(key, hashCode) > 0;
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key));
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap, mHashMultiMap.Find(key));
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		std::pair<const_iterator, const_iterator>>
	equal_range(const KeyArg& key) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key));
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, std::pair<iterator, iterator>>
	equal_range(const KeyArg& key)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap, mHashMultiMap.Find(key));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key,
		size_t hashCode) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key, hashCode));
	}

	std::pair<iterator, iterator> equal_range(const key_type& key, size_t hashCode)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key, hashCode));
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		std::pair<const_iterator, const_iterator>>
	equal_range(const KeyArg& key, size_t hashCode) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key, hashCode));
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value, std::pair<iterator, iterator>>
	equal_range(const KeyArg& key, size_t hashCode)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key, hashCode));
	}

	//template<typename Value>
	//momo::internal::EnableIf<std::is_constructible<value_type, Value>::value, iterator>
	//insert(Value&& value)

	//template<typename Value>
	//momo::internal::EnableIf<std::is_constructible<value_type, Value>::value, iterator>
	//insert(const_iterator hint, Value&& value)

	//iterator insert(value_type&& value)

	//iterator insert(const_iterator hint, value_type&& value)

	//iterator insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	iterator insert(std::pair<key_type, mapped_type>&& value)
	{
		return pvEmplace(std::forward_as_tuple(std::move(value.first)),
			std::forward_as_tuple(std::move(value.second)));
	}

	iterator insert(const_iterator, std::pair<key_type, mapped_type>&& value)
	{
		return insert(std::move(value));
	}

	template<typename First, typename Second>
	momo::internal::EnableIf<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>
	insert(const std::pair<First, Second>& value)
	{
		return pvEmplace(std::forward_as_tuple(value.first), std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	momo::internal::EnableIf<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>
	insert(const_iterator, const std::pair<First, Second>& value)
	{
		return insert(value);
	}

	template<typename First, typename Second>
	momo::internal::EnableIf<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>
	insert(std::pair<First, Second>&& value)
	{
		return pvEmplace(std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	momo::internal::EnableIf<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>
	insert(const_iterator, std::pair<First, Second>&& value)
	{
		return insert(std::move(value));
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsert(first, last,
			std::is_same<key_type, typename std::decay<decltype(first->first)>::type>());
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMultiMap.Add(values.begin(), values.end());
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
		if (keyIter->GetCount() == 1)
			return IteratorProxy(mHashMultiMap.MakeIterator(mHashMultiMap.RemoveKey(keyIter)));
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
		size_t count = keyIter->GetCount();
		MOMO_ASSERT(count > 0);
		if (last == ConstIteratorProxy(mHashMultiMap.MakeIterator(keyIter, count)))
			return IteratorProxy(mHashMultiMap.MakeIterator(mHashMultiMap.RemoveKey(keyIter)));
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

	//size_type max_bucket_count() const noexcept
	//size_type bucket_count() const noexcept
	//size_type bucket_size(size_type bucketIndex) const
	//local_iterator begin(size_type bucketIndex)
	//const_local_iterator begin(size_type bucketIndex) const
	//local_iterator end(size_type bucketIndex)
	//const_local_iterator end(size_type bucketIndex) const
	//const_local_iterator cbegin(size_type bucketIndex) const
	//const_local_iterator cend(size_type bucketIndex) const
	//size_type bucket(const key_type& key) const
	//float load_factor() const noexcept

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
			if (ref.GetCount() != keyIterRight->GetCount())
				return false;
			if (!std::is_permutation(ref.GetBegin(), ref.GetEnd(), keyIterRight->GetBegin()))
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

	template<typename Iterator, typename IteratorProxy, typename HashMultiMap, typename KeyIterator>
	static std::pair<Iterator, Iterator> pvEqualRange(HashMultiMap& hashMultiMap,
		KeyIterator keyIter)
	{
		Iterator end = IteratorProxy(hashMultiMap.GetEnd());
		if (!keyIter)
			return { end, end };
		size_t count = keyIter->GetCount();
		if (count == 0)	//?
			return { end, end };
		Iterator first = IteratorProxy(hashMultiMap.MakeIterator(keyIter, 0));
		Iterator last = IteratorProxy(hashMultiMap.MakeIterator(keyIter, count));
		return { first, last };
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
	iterator pvInsert(std::tuple<KeyArgs...>&& keyArgs, MappedCreator&& mappedCreator)
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
			resIter = pvInsert(std::forward_as_tuple(std::move(*&keyBuffer)),
				std::forward<MappedCreator>(mappedCreator));
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
		typename Key = typename std::decay<RKey>::type>
	momo::internal::EnableIf<std::is_same<key_type, Key>::value, iterator>
	pvInsert(std::tuple<RKey>&& key, MappedCreator&& mappedCreator)
	{
		return IteratorProxy(mHashMultiMap.AddCrt(
			std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator)));
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::true_type /*isKeyType*/)
	{
		mHashMultiMap.Add(first, last);
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::false_type /*isKeyType*/)
	{
		for (Iterator iter = first; iter != last; ++iter)
			insert(*iter);
	}

private:
	HashMultiMap mHashMultiMap;
};

template<typename TKey, typename TMapped,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class unordered_multimap_open : public unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>
{
private:
	typedef unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
		momo::HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>> UnorderedMultiMap;

public:
	using UnorderedMultiMap::UnorderedMultiMap;
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap) \
template<typename Iterator, \
	typename Key = std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>, \
	typename Mapped = typename std::iterator_traits<Iterator>::value_type::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(Iterator, Iterator, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, \
	typename Key = std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>, \
	typename Mapped = typename std::iterator_traits<Iterator>::value_type::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, \
	typename Key = std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>, \
	typename Mapped = typename std::iterator_traits<Iterator>::value_type::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<HashFunc&>()(std::declval<const Key&>())), \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(Iterator, Iterator, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, typename EqualFunc, \
	typename Key = std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>, \
	typename Mapped = typename std::iterator_traits<Iterator>::value_type::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<HashFunc&>()(std::declval<const Key&>())), \
	typename = decltype(std::declval<EqualFunc&>()(std::declval<const Key&>(), std::declval<const Key&>()))> \
unordered_multimap(Iterator, Iterator, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashFunc, EqualFunc, Allocator>; \
template<typename Key, typename Mapped, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(std::initializer_list<std::pair<Key, Mapped>>, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Key, typename Mapped, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(std::initializer_list<std::pair<Key, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Key, typename Mapped, typename HashFunc, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<HashFunc&>()(std::declval<const Key&>())), \
	typename = decltype(std::declval<Allocator&>().allocate(size_t{}))> \
unordered_multimap(std::initializer_list<std::pair<Key, Mapped>>, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Key, typename Mapped, typename HashFunc, typename EqualFunc, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = decltype(std::declval<HashFunc&>()(std::declval<const Key&>())), \
	typename = decltype(std::declval<EqualFunc&>()(std::declval<const Key&>(), std::declval<const Key&>()))> \
unordered_multimap(std::initializer_list<std::pair<Key, Mapped>>, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashFunc, EqualFunc, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

#endif

} // namespace stdish

} // namespace momo
