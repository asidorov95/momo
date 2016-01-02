/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/GoogleHashTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_GOOGLE_HASH

#include "../../momo/stdish/unordered_map.h"
#include "../../momo/stdish/unordered_set.h"

#define sparse_hash_map momo::stdish::unordered_map
#define sparse_hash_set momo::stdish::unordered_set
#define dense_hash_map momo::stdish::unordered_map_open
#define dense_hash_set momo::stdish::unordered_set_open

#include "google/hashtable_test.h"

#undef sparse_hash_map
#undef sparse_hash_set
#undef dense_hash_map
#undef dense_hash_set

#endif // TEST_GOOGLE_HASH
