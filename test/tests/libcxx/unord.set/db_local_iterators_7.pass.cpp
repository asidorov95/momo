//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// Increment local_iterator past end.

#if _LIBCPP_DEBUG >= 1

//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

//#include <unordered_set>
//#include <cassert>
//#include <iterator>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
    typedef int T;
    typedef unordered_set<T> C;
    C c(1);
    C::local_iterator i = c.begin(0);
    ++i;
    LIBCPP_CATCH(++i);
    //assert(false);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef unordered_set<T, min_allocator<T>> C;
    C c(1);
    C::local_iterator i = c.begin(0);
    ++i;
    ++i;
    assert(false);
    }
#endif

}

#else

void main()
{
}

#endif
