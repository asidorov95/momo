//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <cstddef>
#include <new>

template <class T, std::size_t N>
class stack_allocator
{
    char buf_[sizeof(T)*N];
    char* ptr_;
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U> struct rebind {typedef stack_allocator<U, N * sizeof(T) / sizeof(U) + 1> other;};

    stack_allocator() : ptr_(buf_) {}

private:
    stack_allocator(const stack_allocator&);// = delete;
    stack_allocator& operator=(const stack_allocator&);// = delete;

public:
    pointer allocate(size_type n, const void* = 0)
    {
        if (n > N - (ptr_ - buf_) / sizeof(value_type)) {
#ifndef _LIBCPP_NO_EXCEPTIONS
            throw std::bad_alloc();
#else
            std::terminate();
#endif
        }
        pointer r = (T*)ptr_;
        ptr_ += n * sizeof(T);
        return r;
    }
    void deallocate(pointer p, size_type n)
    {
        if ((char*)(p + n) == ptr_)
            ptr_ = (char*)p;
    }

    size_type max_size() const {return N;}
};

template <class T, std::size_t N>
inline
void
swap(stack_allocator<T, N>& x, stack_allocator<T, N>& y) {}

#if defined(_MSC_VER) && _MSC_VER < 1800
namespace std
{
	template <class T, std::size_t N>
	struct is_nothrow_move_constructible<stack_allocator<T, N>> : public std::false_type
	{
	};
}
#endif

#endif  // STACK_ALLOCATOR_H