//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// size_type bucket_size(size_type n) const

#ifdef _LIBCPP_DEBUG
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_set<int, LibcppIntHash> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        assert(c.bucket_size(0) == 0);
        assert(c.bucket_size(1) == 1);
        assert(c.bucket_size(2) == 1);
        assert(c.bucket_size(3) == 1);
        assert(c.bucket_size(4) == 1);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        assert(c.bucket_size(0) == 0);
        assert(c.bucket_size(1) == 1);
        assert(c.bucket_size(2) == 1);
        assert(c.bucket_size(3) == 1);
        assert(c.bucket_size(4) == 1);
    }
#endif
#if _LIBCPP_DEBUG_LEVEL >= 1
    {
        typedef unordered_set<int> C;
        C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        LIBCPP_CATCH(c.bucket_size(3));
#else
        LIBCPP_CATCH(c.bucket_size(33));
#endif
        //assert(false);
    }
#endif
}
