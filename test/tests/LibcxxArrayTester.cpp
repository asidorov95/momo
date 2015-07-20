/**********************************************************\

  tests/LibcxxArrayTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_ARRAY

#undef NDEBUG

#include "../../momo/Utility.h"

#include "LibcxxTester.h"

#include "../../momo/stdish/vector.h"
#include "../../momo/SegmentedArray.h"

#ifdef MOMO_USE_TYPE_ALIASES

template<typename TValue>
struct LibcxxSegmentedArrayItemTraits : public momo::SegmentedArrayItemTraits<TValue>
{
	static const bool isNothrowMoveConstructible =
		momo::internal::ObjectManager<TValue>::isNothrowMoveConstructible;
};

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCPP_TEST_STACK_ALLOCATOR

#define LIBCXX_TEST_PREFIX "libcxx_test_array_0"
namespace libcxx_test_array_0
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::Array<TValue, momo::MemManagerStd<TAllocator>, momo::ArrayItemTraits<TValue>,
		momo::ArraySettings<0, false, momo::CheckMode::exception>>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_5"
namespace libcxx_test_array_5
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::Array<TValue, momo::MemManagerStd<TAllocator>, momo::ArrayItemTraits<TValue>,
		momo::ArraySettings<5, false, momo::CheckMode::exception>>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

#undef LIBCPP_TEST_STACK_ALLOCATOR

#define LIBCXX_TEST_SEGMENTED_ARRAY

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt"
namespace libcxx_test_segmented_array_sqrt
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>, LibcxxSegmentedArrayItemTraits<TValue>,
		momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 3, momo::CheckMode::exception>>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst"
namespace libcxx_test_segmented_array_cnst
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>, LibcxxSegmentedArrayItemTraits<TValue>,
		momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::cnst, 0, momo::CheckMode::exception>>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#undef LIBCXX_TEST_SEGMENTED_ARRAY

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#else

#define LIBCXX_TEST_PREFIX "libcxx_test_vector"
namespace libcxx_test_vector
{
using momo::stdish::vector;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#endif

#endif // TEST_LIBCXX_ARRAY