/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashMapTesterLimP4.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP4<>
#define LIBCXX_TEST_BUCKET_NAME "limp4"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP
