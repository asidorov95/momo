/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashUtility.h

\**********************************************************/

#pragma once

#include "Utility.h"

#define MOMO_MORE_HASH_ITERATOR_OPERATORS(Iterator) \
	Iterator operator++(int) \
	{ \
		Iterator tempIter = *this; \
		++*this; \
		return tempIter; \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT \
	{ \
		return !(*this == iter); \
	} \
	bool operator!() const MOMO_NOEXCEPT \
	{ \
		return *this == ConstIterator(); \
	}

namespace momo
{

namespace internal
{
	template<typename TReference,
		typename TConstReference = typename TReference::ConstReference>
	class HashPointer
	{
	public:
		typedef TReference Reference;
		typedef TConstReference ConstReference;

		typedef HashPointer<ConstReference, ConstReference> ConstPointer;

		typedef const typename std::remove_reference<Reference>::type* RefAddress;

	public:
		//HashPointer() MOMO_NOEXCEPT
		//{
		//}

		explicit HashPointer(Reference ref) MOMO_NOEXCEPT
			: mReference(ref)
		{
		}

		operator ConstPointer() const MOMO_NOEXCEPT
		{
			return ConstPointer(mReference);
		}

		RefAddress operator->() const MOMO_NOEXCEPT
		{
			return std::addressof(mReference);
		}

		Reference operator*() const MOMO_NOEXCEPT
		{
			return mReference;
		}

	private:
		Reference mReference;
	};

	template<typename TBaseIterator, typename TReference,
		typename TConstBaseIterator = typename TBaseIterator::ConstIterator,
		typename TConstReference = typename TReference::ConstReference>
	class HashDerivedIterator
	{
	public:
		typedef TBaseIterator BaseIterator;
		typedef TConstBaseIterator ConstBaseIterator;
		typedef TReference Reference;
		typedef TConstReference ConstReference;

		typedef HashPointer<Reference, ConstReference> Pointer;

		typedef HashDerivedIterator<ConstBaseIterator, ConstReference,
			ConstBaseIterator, ConstReference> ConstIterator;

	public:
		HashDerivedIterator() MOMO_NOEXCEPT
		{
		}

		explicit HashDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mBaseIterator);
		}

		BaseIterator GetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

		HashDerivedIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(Reference(*mBaseIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mBaseIterator == iter.GetBaseIterator();
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashDerivedIterator)

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TIterator>
	class HashInsertResult
	{
	public:
		typedef TIterator Iterator;

	public:
		HashInsertResult(Iterator iter, bool inserted) MOMO_NOEXCEPT
			: iterator(iter),
			inserted(inserted)
		{
		}

	public:
		Iterator iterator;
		bool inserted;
	};

	template<typename TKey, typename TMapped, typename THashMapReference>
	class UnorderedMapReference : public std::pair<const TKey&, TMapped&>
	{
	public:
		typedef TKey Key;
		typedef TMapped Mapped;
		typedef THashMapReference HashMapReference;

		typedef UnorderedMapReference<TKey, const TMapped,
			typename HashMapReference::ConstReference> ConstReference;

	private:
		typedef std::pair<const Key&, Mapped&> RefPair;

	public:
		UnorderedMapReference(const Key& key, Mapped& mapped) MOMO_NOEXCEPT
			: RefPair(key, mapped)
		{
		}

		explicit UnorderedMapReference(HashMapReference ref) MOMO_NOEXCEPT
			: RefPair(ref.key, ref.value)
		{
		}

		operator ConstReference() MOMO_NOEXCEPT
		{
			return ConstReference(this->first, this->second);
		}

		//? ==, !=
	};
}

} // namespace momo

namespace std
{
	template<typename BI, typename R, typename CBI, typename CR>
	struct iterator_traits<momo::internal::HashDerivedIterator<BI, R, CBI, CR>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::HashDerivedIterator<BI, R, CBI, CR>::Pointer pointer;
		typedef typename momo::internal::HashDerivedIterator<BI, R, CBI, CR>::Reference reference;
		typedef reference value_type;	//?
	};
} // namespace std