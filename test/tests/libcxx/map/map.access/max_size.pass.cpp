//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// size_type max_size() const;

//#include <map>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    typedef map<int, double> M;
    M m;
    assert(m.max_size() != 0);
    }
#if __cplusplus >= 201103L
    {
    typedef map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
    M m;
    assert(m.max_size() != 0);
    }
#endif
}
