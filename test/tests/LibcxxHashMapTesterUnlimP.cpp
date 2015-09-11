/**********************************************************\

  tests/LibcxxHashMapTesterUnlimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketUnlimP<>

#define LIBCXX_TEST_BUCKET "unlimp"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP
