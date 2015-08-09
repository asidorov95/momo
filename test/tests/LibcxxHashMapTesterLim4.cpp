/**********************************************************\

  tests/LibcxxHashMapTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#undef MOMO_PACK_ALL
#define MOMO_PACK_ALL

#undef MOMO_DEFAULT_MEM_MANAGER
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLim4<>

#define LIBCXX_TEST_BUCKET "lim4"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP
