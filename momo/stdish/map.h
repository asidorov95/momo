/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/map.h

  namespace momo::stdish:
    class map

  This class is similar to `std::map`, but much more efficient in
  memory usage. The implementation is based on a B-tree.

  Deviations from the `std::map`:
  1. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  2. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  3. Type `reference` is not the same as `value_type&`, so
    `for (auto& p : map)` is illegal, but `for (auto p : map)` or
    `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
  4. Functions `begin`, `cbegin`, `rend` and `crend` have logarithmic
    complexity.
  5. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false
    or `ObjectManager<mapped_type>::isNothrowAnywayAssignable` is false,
    functions `erase` can throw exceptions.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.

\**********************************************************/

#pragma once

#include "../TreeMap.h"
#include "node_handle.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessFunc>,
		MemManagerStd<TAllocator>>>
class map
{
private:
	typedef TTreeMap TreeMap;
	typedef typename TreeMap::TreeTraits TreeTraits;
	typedef typename TreeMap::MemManager MemManager;

	typedef typename TreeMap::Iterator TreeMapIterator;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef TLessFunc key_compare;
	typedef TAllocator allocator_type;

	typedef TreeMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	class value_compare
	{
	public:
		bool operator()(const value_type& value1, const value_type& value2) const
		{
			return comp(value1.first, value2.first);
		}

	protected:
		value_compare(const key_compare& keyComp)
			: comp(keyComp)
		{
		}

	protected:
		key_compare comp;
	};

	typedef momo::internal::MapReferenceStd<key_type, mapped_type,
		typename TreeMapIterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef momo::internal::TreeDerivedIterator<TreeMapIterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	typedef internal::map_node_handle<typename TreeMap::ExtractedPair> node_type;
	typedef internal::insert_return_type<iterator, node_type> insert_return_type;

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
		MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator, TreeMapIterator)
	};

	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedPair,
			typename NodeType::MapExtractedPair&)
	};

public:
	map()
	{
	}

	explicit map(const allocator_type& alloc)
		: mTreeMap(TreeTraits(), MemManager(alloc))
	{
	}

	explicit map(const key_compare& lessFunc, const allocator_type& alloc = allocator_type())
		: mTreeMap(TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: map(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: map(lessFunc, alloc)
	{
		insert(first, last);
	}

	map(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: map(values.begin(), values.end(), alloc)
	{
	}

	map(std::initializer_list<value_type> values, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: map(values.begin(), values.end(), lessFunc, alloc)
	{
	}

	map(map&& right) MOMO_NOEXCEPT
		: mTreeMap(std::move(right.mTreeMap))
	{
	}

	map(map&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value)
		: mTreeMap(pvCreateMap(std::move(right), alloc))
	{
	}

	map(const map& right)
		: mTreeMap(right.mTreeMap)
	{
	}

	map(const map& right, const allocator_type& alloc)
		: mTreeMap(right.mTreeMap, MemManager(alloc))
	{
	}

	~map() MOMO_NOEXCEPT
	{
	}

	map& operator=(map&& right)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mTreeMap = pvCreateMap(std::move(right), alloc);
		}
		return *this;
	}

	map& operator=(const map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mTreeMap = TreeMap(right.mTreeMap, MemManager(alloc));
		}
		return *this;
	}

	map& operator=(std::initializer_list<value_type> values)
	{
		TreeMap treeMap(mTreeMap.GetTreeTraits(), MemManager(get_allocator()));
		treeMap.Insert(values.begin(), values.end());
		mTreeMap = std::move(treeMap);
		return *this;
	}

	void swap(map& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mTreeMap.Swap(right.mTreeMap);
	}

	friend void swap(map& left, map& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const MOMO_NOEXCEPT
	{
		return mTreeMap;
	}

	nested_container_type& get_nested_container() MOMO_NOEXCEPT
	{
		return mTreeMap;
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return IteratorProxy(mTreeMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mTreeMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return IteratorProxy(mTreeMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mTreeMap.GetEnd());
	}

	reverse_iterator rbegin() MOMO_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rend() MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(begin());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	const_reverse_iterator crbegin() const MOMO_NOEXCEPT
	{
		return rbegin();
	}

	const_reverse_iterator crend() const MOMO_NOEXCEPT
	{
		return rend();
	}

	key_compare key_comp() const
	{
		return mTreeMap.GetTreeTraits().GetLessFunc();
	}

	value_compare value_comp() const
	{
		struct ValueCompareProxy : public value_compare
		{
			explicit ValueCompareProxy(const key_compare& keyComp)
				: value_compare(keyComp)
			{
			}
		};
		return ValueCompareProxy(key_comp());
	}

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return allocator_type(mTreeMap.GetMemManager().GetCharAllocator());
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mTreeMap.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mTreeMap.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mTreeMap.Clear();
	}

	const_iterator find(const key_type& key) const
	{
		return ConstIteratorProxy(mTreeMap.Find(key));
	}

	iterator find(const key_type& key)
	{
		return IteratorProxy(mTreeMap.Find(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeMap.Find(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	iterator find(const KeyArg& key)
	{
		return IteratorProxy(mTreeMap.Find(key));
	}

	size_type count(const key_type& key) const
	{
		return mTreeMap.HasKey(key) ? 1 : 0;
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	size_type count(const KeyArg& key) const
	{
		return mTreeMap.HasKey(key) ? 1 : 0;
	}

	bool contains(const key_type& key) const
	{
		return mTreeMap.HasKey(key);
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	bool contains(const KeyArg& key) const
	{
		return mTreeMap.HasKey(key);
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return ConstIteratorProxy(mTreeMap.LowerBound(key));
	}

	iterator lower_bound(const key_type& key)
	{
		return IteratorProxy(mTreeMap.LowerBound(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator lower_bound(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeMap.LowerBound(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	iterator lower_bound(const KeyArg& key)
	{
		return IteratorProxy(mTreeMap.LowerBound(key));
	}

	const_iterator upper_bound(const key_type& key) const
	{
		return ConstIteratorProxy(mTreeMap.UpperBound(key));
	}

	iterator upper_bound(const key_type& key)
	{
		return IteratorProxy(mTreeMap.UpperBound(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator upper_bound(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeMap.UpperBound(key));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	iterator upper_bound(const KeyArg& key)
	{
		return IteratorProxy(mTreeMap.UpperBound(key));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return equal_range<key_type, key_compare, void>(key);
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return equal_range<key_type, key_compare, void>(key);
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		const_iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
			return std::pair<const_iterator, const_iterator>(iter, iter);
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	std::pair<iterator, iterator> equal_range(const KeyArg& key)
	{
		iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
			return std::pair<iterator, iterator>(iter, iter);
		return std::pair<iterator, iterator>(iter, std::next(iter));
	}

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, std::pair<iterator, bool>>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(const_iterator hint, Value&& value)

	//std::pair<iterator, bool> insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	std::pair<iterator, bool> insert(value_type&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(std::move(value.second)));
	}

	iterator insert(const_iterator hint, value_type&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(std::move(value.second))).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value,
		std::pair<iterator, bool>>::type
	insert(const std::pair<First, Second>& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second)).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value,
		std::pair<iterator, bool>>::type
	insert(std::pair<First, Second>&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second))).first;
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsert(first, last,
			std::is_same<key_type, typename std::decay<decltype(first->first)>::type>());
	}

	void insert(std::initializer_list<value_type> values)
	{
		mTreeMap.Insert(values.begin(), values.end());
	}

	std::pair<iterator, bool> emplace()
	{
		return pvEmplace(nullptr, std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator hint)
	{
		return pvEmplace(hint, std::tuple<>(), std::tuple<>()).first;
	}

	template<typename ValueArg>
	std::pair<iterator, bool> emplace(ValueArg&& valueArg)
	{
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator hint, ValueArg&& valueArg)
	{
		return insert(hint, std::forward<ValueArg>(valueArg));
	}

	template<typename KeyArg, typename MappedArg>
	std::pair<iterator, bool> emplace(KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace_hint(const_iterator hint, KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg))).first;
	}

	template<typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> emplace(std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(nullptr, std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator hint, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(hint, std::move(keyArgs), std::move(mappedArgs)).first;
	}

	iterator erase(const_iterator where)
	{
		return IteratorProxy(mTreeMap.Remove(ConstIteratorProxy::GetBaseIterator(where)));
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
			return IteratorProxy(mTreeMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(first)));
		}
		size_t count = std::distance(first, last);
		iterator iter = erase(first);
		for (size_t i = 1; i < count; ++i)
			iter = erase(iter);
		return iter;
	}

	size_type erase(const key_type& key)
	{
		return mTreeMap.Remove(key) ? 1 : 0;
	}

	typename TreeMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return mTreeMap[std::move(key)];
	}

	typename TreeMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return mTreeMap[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(hint, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, std::move(key), std::forward<MappedArg>(mappedArg)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, key, std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, key, std::forward<MappedArg>(mappedArg)).first;
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename TreeMap::InsertResult res = mTreeMap.Insert(
			std::move(NodeTypeProxy::GetExtractedPair(node)));
		return { IteratorProxy(res.iterator), res.inserted,
			res.inserted ? node_type() : std::move(node) };
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
		if (node.empty())
			return end();
		std::pair<iterator, bool> res = pvFind(hint, node.key());
		if (!res.second)
			return res.first;
		return IteratorProxy(mTreeMap.Add(IteratorProxy::GetBaseIterator(res.first),
			std::move(NodeTypeProxy::GetExtractedPair(node))));
	}

	node_type extract(const_iterator where)
	{
		return node_type(mTreeMap, ConstIteratorProxy::GetBaseIterator(where));
	}

	node_type extract(const key_type& key)
	{
		const_iterator iter = find(key);
		return (iter != end()) ? extract(iter) : node_type();
	}

	template<typename Map>
	void merge(Map&& map)
	{
		mTreeMap.MergeFrom(map.get_nested_container());
	}

	bool operator==(const map& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	bool operator!=(const map& right) const
	{
		return !(*this == right);
	}

	bool operator<(const map& right) const
	{
		return std::lexicographical_compare(begin(), end(), right.begin(), right.end());
	}

	bool operator>(const map& right) const
	{
		return right < *this;
	}

	bool operator<=(const map& right) const
	{
		return !(right < *this);
	}

	bool operator>=(const map& right) const
	{
		return right <= *this;
	}

private:
	static TreeMap pvCreateMap(map&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mTreeMap);
		TreeMap treeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc));
		treeMap.MergeFrom(right.mTreeMap);
		return treeMap;
	}

	std::pair<iterator, bool> pvFind(std::nullptr_t /*hint*/, const key_type& key)
	{
		iterator resHint = lower_bound(key);
		bool isNewKey = (resHint == end() || mTreeMap.GetTreeTraits().IsLess(key, resHint->first));
		return std::pair<iterator, bool>(resHint, isNewKey);
	}

	std::pair<iterator, bool> pvFind(const_iterator hint, const key_type& key)
	{
		const TreeTraits& treeTraits = mTreeMap.GetTreeTraits();
		if (hint != begin() && !treeTraits.IsLess(std::prev(hint)->first, key))
			return pvFind(nullptr, key);
		if (hint != end() && !treeTraits.IsLess(key, hint->first))
			return pvFind(nullptr, key);
		iterator resHint = IteratorProxy(mTreeMap.MakeMutableIterator(
			ConstIteratorProxy::GetBaseIterator(hint)));
		return std::pair<iterator, bool>(resHint, true);
	}

	template<typename Hint, typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> pvEmplace(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename TreeMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return pvInsert(hint, std::move(keyArgs),
			MappedCreator(mTreeMap.GetMemManager(), std::move(mappedArgs)));
	}

	template<typename Hint, typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		MappedCreator&& mappedCreator)
	{
		MemManager& memManager = mTreeMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type, TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(&keyBuffer);
		bool keyDestroyed = false;
		try
		{
			std::pair<iterator, bool> res = pvFind(hint, *&keyBuffer);
			if (!res.second)
			{
				KeyManager::Destroy(memManager, *&keyBuffer);
				keyDestroyed = true;
				return res;
			}
			auto valueCreator = [&memManager, &keyBuffer, &mappedCreator, &keyDestroyed]
				(key_type* newKey, mapped_type* newMapped)
			{
				KeyManager::Relocate(memManager, *&keyBuffer, newKey);
				keyDestroyed = true;
				try
				{
					std::forward<MappedCreator>(mappedCreator)(newMapped);
				}
				catch (...)
				{
					KeyManager::Destroy(memManager, *newKey);
					throw;
				}
			};
			TreeMapIterator resIter = mTreeMap.AddCrt(
				IteratorProxy::GetBaseIterator(res.first), valueCreator);
			return std::pair<iterator, bool>(IteratorProxy(resIter), true);
		}
		catch (...)
		{
			if (!keyDestroyed)
				KeyManager::Destroy(memManager, *&keyBuffer);
			throw;
		}
	}

	template<typename Hint, typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type,
		typename = typename std::enable_if<std::is_same<key_type, Key>::value>::type>
	std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<RKey>&& key,
		MappedCreator&& mappedCreator)
	{
		std::pair<iterator, bool> res = pvFind(hint,
			static_cast<const key_type&>(std::get<0>(key)));
		if (!res.second)
			return res;
		TreeMapIterator resIter = mTreeMap.AddCrt(IteratorProxy::GetBaseIterator(res.first),
			std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator));
		return std::pair<iterator, bool>(IteratorProxy(resIter), true);
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::true_type /*isKeyType*/)
	{
		mTreeMap.Insert(first, last);
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::false_type /*isKeyType*/)
	{
		for (Iterator iter = first; iter != last; ++iter)
			insert(*iter);
	}

	template<typename Hint, typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(Hint hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = pvEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
			res.first->second = std::forward<MappedArg>(mappedArg);
		return res;
	}

private:
	TreeMap mTreeMap;
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES
template<typename Iterator,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
map(Iterator, Iterator, Allocator = Allocator())
	-> map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type,
		std::less<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		Allocator>;

template<typename Iterator, typename LessFunc,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
map(Iterator, Iterator, LessFunc, Allocator = Allocator())
	-> map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type, LessFunc, Allocator>;

template<typename Key, typename Mapped,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
map(std::initializer_list<std::pair<const Key, Mapped>>, Allocator = Allocator())
	-> map<Key, Mapped, std::less<Key>, Allocator>;

template<typename Key, typename Mapped, typename LessFunc,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
map(std::initializer_list<std::pair<const Key, Mapped>>, LessFunc, Allocator = Allocator())
	-> map<Key, Mapped, LessFunc, Allocator>;
#endif

} // namespace stdish

} // namespace momo
