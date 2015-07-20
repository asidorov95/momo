/**********************************************************\

  tests/LibcxxTester.h

\**********************************************************/

#pragma once

#include <iostream>
#include <cfloat>
#include <string>

#ifndef MOMO_USE_NOEXCEPT
#define _LIBCPP_HAS_NO_NOEXCEPT
#endif

#ifndef MOMO_USE_DELETE_FUNCS
#define _LIBCPP_HAS_NO_DELETED_FUNCTIONS
#endif

//#ifndef MOMO_USE_INIT_LISTS
#if defined(_MSC_VER) && _MSC_VER < 1900
#define _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
#endif

#ifndef MOMO_USE_VARIADIC_TEMPLATES
#define _LIBCPP_HAS_NO_VARIADICS
#endif

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO
//#define LIBCPP_TEST_MIN_ALLOCATOR

#include "libcxx/support/MoveOnly.h"
#include "libcxx/support/Copyable.h"
#include "libcxx/support/NotConstructible.h"
#include "libcxx/support/DefaultOnly.h"
#include "libcxx/support/Emplaceable.h"

//#include "libcxx/support/min_allocator.h"
#include "libcxx/support/stack_allocator.h"
#include "libcxx/support/test_allocator.h"
#include "libcxx/support/test_iterators.h"
#include "libcxx/support/test_compare.h"
#include "libcxx/support/test_hash.h"

struct LibcppIntHash
{
	typedef int argument_type;

	size_t operator()(int key) const //MOMO_NOEXCEPT
	{
		return (size_t)key;
	}
};

#define LIBCPP_CATCH(expr) try { (void)(expr); assert(false); } catch (...) {}

#define LIBCXX_TEST_BEGIN(name) \
	namespace name { \
	void main(); \
	int TestLibcxx() \
	{ \
		std::cout << LIBCXX_TEST_PREFIX << "_" << #name << ": "; \
		main(); \
		std::cout << "ok" << std::endl; \
		return 0; \
	} \
	static int testLibcxx = (TestLibcxx(), 0);

#define LIBCXX_TEST_END }