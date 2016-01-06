/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/UserSettings.h

\**********************************************************/

#pragma once

//#define MOMO_USE_SAFE_MAP_BRACKETS
//#define MOMO_USE_UNORDERED_HINT_ITERATORS

//#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) true
#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) \
	(std::is_nothrow_move_constructible<Object>::value || !std::is_copy_constructible<Object>::value)

#define MOMO_MAX_ALIGNMENT (std::alignment_of<std::max_align_t>::value)

#define MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT 32
#define MOMO_DEFAULT_MEM_POOL_CACHED_FREE_BLOCK_COUNT 16

#define MOMO_DEFAULT_CHECK_MODE assertion
#define MOMO_DEFAULT_EXTRA_CHECK_MODE assertion
#define MOMO_CHECK_ITERATOR_VERSION

#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP<>

#define MOMO_DEFAULT_TREE_NODE TreeNode<32, 4, true>

#define MOMO_IS_FAST_COMPARABLE(Key) (std::is_arithmetic<Key>::value || std::is_pointer<Key>::value)

#if defined(_MSC_VER) //defined(_WIN32)
#define MOMO_USE_MEM_MANAGER_WIN
#define MOMO_DEFAULT_MEM_MANAGER MemManagerWin
#elif defined(__linux__)
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC	// linux has fast realloc
#else
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp
#endif

#if !defined(__clang__)
#define MOMO_NULL_UINTPTR ((uintptr_t)(void*)nullptr)
#else
#define MOMO_NULL_UINTPTR ((uintptr_t)0)
#endif

#define MOMO_INVALID_UINTPTR (MOMO_NULL_UINTPTR + 1)

#define MOMO_USE_NOEXCEPT
#define MOMO_USE_TRIVIALLY_COPYABLE

#if defined(_MSC_VER) && _MSC_VER < 1900
#undef MOMO_USE_NOEXCEPT
#endif

#if defined(__GNUC__) && __GNUC__ < 5
#undef MOMO_USE_TRIVIALLY_COPYABLE
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#pragma warning (disable: 4510)	// default constructor could not be generated
#pragma warning (disable: 4512)	// assignment operator could not be generated
#pragma warning (disable: 4610)	// struct can never be instantiated - user defined constructor required
#define _SCL_SECURE_NO_WARNINGS
#endif