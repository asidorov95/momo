/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleTreeTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_TREE

#undef NDEBUG

#include "../../momo/TreeSet.h"
#include "../../momo/TreeMap.h"
#include "../../momo/stdish/pool_allocator.h"

#include <string>
#include <iostream>
#include <random>
#include <set>

class SimpleTreeTester
{
public:
	static void TestCharAll()
	{
		std::mt19937 mt;

		TestCharTreeNode3<  1, 1, 127>(mt);
		TestCharTreeNode3<  2, 1,  66>(mt);
		TestCharTreeNode3<  3, 1,  32>(mt);
		TestCharTreeNode3<  4, 1,  15>(mt);
		TestCharTreeNode3<  5, 1,   1>(mt);
		TestCharTreeNode3< 10, 1,   3>(mt);
		TestCharTreeNode3<101, 1,   2>(mt);
		TestCharTreeNode3<255, 1,   1>(mt);

		TestCharTreeNode3<  4, 2, 127>(mt);
		TestCharTreeNode3<  5, 2,  66>(mt);
		TestCharTreeNode3<  6, 2,  32>(mt);
		TestCharTreeNode3<  7, 2,  15>(mt);
		TestCharTreeNode3< 14, 3,   1>(mt);
		TestCharTreeNode3< 77, 3,   3>(mt);
		TestCharTreeNode3<121, 3,   2>(mt);
		TestCharTreeNode3<255, 3,   1>(mt);

		TestCharTreeNode3< 37,   7, 127>(mt);
		TestCharTreeNode3< 42,  15,  66>(mt);
		TestCharTreeNode3< 65,  23,  32>(mt);
		TestCharTreeNode3< 77,  30,  15>(mt);
		TestCharTreeNode3< 88,  31,   1>(mt);
		TestCharTreeNode3<104,  33,   3>(mt);
		TestCharTreeNode3<204, 100,   2>(mt);
		TestCharTreeNode3<255, 127,   1>(mt);
	}

	template<size_t maxCapacity, size_t capacityStep, size_t memPoolBlockCount>
	static void TestCharTreeNode3(std::mt19937& mt)
	{
		static const bool useSwap = (maxCapacity + capacityStep) % 2 == 0;
		TestCharTreeNode4<maxCapacity, capacityStep, memPoolBlockCount, useSwap>(mt);
	}

	template<size_t maxCapacity, size_t capacityStep, size_t memPoolBlockCount, bool useSwap>
	static void TestCharTreeNode4(std::mt19937& mt)
	{
		std::cout << "momo::TreeNode<" << maxCapacity << ", " << capacityStep << ", "
			<< memPoolBlockCount << ", " << (useSwap ? "true" : "false") << ">: " << std::flush;

		typedef momo::TreeNode<maxCapacity, capacityStep,
			momo::MemPoolParams<memPoolBlockCount>, useSwap> TreeNode;
		typedef momo::TreeSet<unsigned char, momo::TreeTraits<unsigned char, false, TreeNode>> TreeSet;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = static_cast<unsigned char>(i);

		if (maxCapacity > 1)
		{
			for (size_t i = 0; i <= count; ++i)
			{
				TreeSet set1, set2;
				set1.Insert(array, array + i);
				set2.Insert(array + i, array + count);
				if (i > 0)
					set1.Remove(array[i - 1]);
				if (mt() % 2 == 0)
					std::swap(set1, set2);
				set1.MergeFrom(set2);
				if (i > 0)
					set1.Insert(array[i - 1]);
				assert(set1.GetCount() == count);
				assert(set2.IsEmpty());
				assert(std::equal(set1.GetBegin(), set1.GetEnd(), array));
				set2.Insert(set1.GetBegin(), set1.GetEnd());
				assert(std::equal(set2.GetBegin(), set2.GetEnd(), array));
			}
		}

		{
			std::set<unsigned char, std::less<unsigned char>,
				momo::stdish::unsynchronized_pool_allocator<unsigned char>> sset;
			TreeSet mset;

			std::shuffle(array, array + count, mt);
			for (unsigned char c : array)
			{
				sset.insert(c);
				assert(mset.Insert(c).inserted);
				assert(mset.GetCount() == sset.size());
				assert(std::equal(mset.GetBegin(), mset.GetEnd(), sset.begin()));
			}

			std::shuffle(array, array + count, mt);
			for (unsigned char c : array)
			{
				sset.erase(c);
				assert(mset.Remove(c));
				assert(mset.GetCount() == sset.size());
				assert(std::equal(mset.GetBegin(), mset.GetEnd(), sset.begin()));
			}

			assert(mset.IsEmpty());
			mset.Insert(128);
			assert(mset.GetCount() == 1);
		}

		std::cout << "ok" << std::endl;
	}

	static void TestStrAll()
	{
		std::cout << "momo::TreeSet: " << std::flush;
		TestStrTreeSet();
		std::cout << "ok" << std::endl;

		std::cout << "momo::TreeMap: " << std::flush;
		TestStrTreeMap();
		std::cout << "ok" << std::endl;
	}

	static void TestStrTreeSet()
	{
		typedef momo::TreeSet<std::string> TreeSet;
		std::string s1 = "s1";
		TreeSet set = { s1, "s2" };
		set.Insert("s3");
		set = set;
		set = std::move(set);
		assert(set.GetCount() == 3);
		assert(set.ContainsKey("s2"));
		TreeSet::ConstIterator iter = set.Find("s1");
		assert(*iter == "s1");
		auto es = set.Extract(iter);
		assert(es.GetItem() == "s1");
		set.Remove(set.Add(set.GetLowerBound(s1), std::move(es)), es);
		assert(es.GetItem() == s1);
		set.Insert(std::move(es));
		assert(es.IsEmpty());
		set.Remove(set.Find("s1"));
		iter = set.Find("s1");
		assert(iter == set.GetEnd());
		set.Insert(&s1, &s1 + 1);
		set.ResetKey(set.GetLowerBound("s1"), s1);
		set.ResetKey(set.GetUpperBound(s1), "s2");
		set.Remove("s2");
		for (const std::string& s : set)
			assert(s == "s1" || s == "s3");
		set.Clear();
		assert(set.IsEmpty());
	}

	static void TestStrTreeMap()
	{
		typedef momo::TreeMap<std::string, std::string> TreeMap;
		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";
		std::string s5 = "s5";
		TreeMap map = { {"s1", "s1"}, {"s2", s2} };
		map.Insert(s3, "s3");
		map.Insert(s4, s4);
		map[s5] = "s5";
		assert(static_cast<std::string>(map["s5"]) == s5);
		map["s6"] = "s6";
		map.ResetKey(map.GetLowerBound("s1"), s1);
		map.ResetKey(map.GetUpperBound(s1), "s2");
		map = map;
		map = std::move(map);
		assert(map.GetCount() == 6);
		assert(map.ContainsKey(s2));
		TreeMap::ConstIterator iter1 = map.Find(s1);
		assert(iter1->key == s1 && iter1->value == s1);
		map.Remove(s1);
		TreeMap::Iterator iter2 = map.Find("s5");
		assert(iter2->key == s5 && iter2->value == s5);
		auto ep = map.Extract(iter2);
		assert(ep.GetKey() == s5 && ep.GetValue() == s5);
		map.Remove(map.Add(map.GetLowerBound(s5), std::move(ep)), ep);
		assert(ep.GetKey() == s5 && ep.GetValue() == s5);
		map.Insert(std::move(ep));
		assert(ep.IsEmpty());
		map.Remove(map.Find("s5"));
		map.Remove(s3);
		map.Remove("s4");
		std::pair<std::string, std::string> pair("s4", s4);
		map.Insert(&pair, &pair + 1);
		map.Insert(map.Find(s2), std::next(map.Find(s2)));	//?
		assert(map.GetCount() == 3);
		map.Remove(s4);
		for (auto ref : map)
			assert(ref.value == "s2" || ref.value == "s6");
		for (auto ref : static_cast<const TreeMap&>(map))
			assert(ref.value == "s2" || ref.value == "s6");
		assert(map.GetCount() == 2);
		map.Clear();
		assert(map.IsEmpty());
	}
};

static int testSimpleTree = (SimpleTreeTester::TestStrAll(), SimpleTreeTester::TestCharAll(), 0);

#endif // TEST_SIMPLE_TREE
