/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterUnlimP.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET
#ifdef TEST_OLD_HASH_BUCKETS

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketUnlimP.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketUnlimP<>
#define LIBCXX_TEST_BUCKET_NAME "unlimp"

#include "LibcxxHashSetTester.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_SET
