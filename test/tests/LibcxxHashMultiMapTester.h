/**********************************************************\

  tests/LibcxxHashMultiMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../momo/stdish/unordered_multimap.h"

namespace
{

#ifdef MOMO_USE_TYPE_ALIASES

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_multimap_" LIBCXX_TEST_BUCKET
template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap = momo::stdish::unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	momo::HashMultiMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		momo::MemManagerStd<TAllocator>, momo::HashMultiMapKeyValueTraits<TKey, TMapped>,
		momo::HashMultiMapSettings<momo::CheckMode::exception>>>;
#include "LibcxxUnorderedMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#else

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_multimap_" LIBCXX_TEST_BUCKET
using momo::stdish::unordered_multimap;
#include "LibcxxUnorderedMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

#endif

} // namespace