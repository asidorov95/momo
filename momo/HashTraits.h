/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashTraits.h

  namespace momo:
    struct HashBucketDefault
    class HashTraits
    class HashTraitsOpen
    class HashTraitsVar
    class HashTraitsStd

\**********************************************************/

#pragma once

#include "details/HashBucketLimP.h"
#include "details/HashBucketLimP1.h"
#include "details/HashBucketLimP4.h"
#include "details/HashBucketUnlimP.h"
#include "details/HashBucketOneIA.h"
#include "details/HashBucketOneI1.h"
#include "details/HashBucketOneI.h"
#include "details/HashBucketOpen1.h"
#include "details/HashBucketOpen2N.h"
#include "details/HashBucketOpenN.h"
#include "details/HashBucketOpenN1.h"
//#include "details/HashBucketLim4.h"

namespace momo
{

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

typedef MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketDefaultOpen;

template<typename TKey,
	typename THashBucket = HashBucketDefault>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = std::false_type;

public:
	HashTraits() MOMO_NOEXCEPT
	{
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::CalcCapacity(bucketCount);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketCountShift(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return HashBucket::logStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		return std::hash<Key>()(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}
};

template<typename TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketDefaultOpen>;

template<typename TKey,
	typename THashBucket = HashBucketDefault>
class HashTraitsVar
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;

	typedef std::function<size_t(size_t)> CalcCapacityFunc;
	typedef std::function<size_t(size_t)> GetBucketCountShiftFunc;

	template<typename KeyArg>
	using IsValidKeyArg = std::false_type;

public:
	explicit HashTraitsVar(const CalcCapacityFunc& calcCapacityFunc = HashBucket::CalcCapacity,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mCalcCapacityFunc(calcCapacityFunc),
		mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
	}

	explicit HashTraitsVar(float maxLoadFactor,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
		HashBucket::CheckMaxLoadFactor(maxLoadFactor);
		mCalcCapacityFunc = [maxLoadFactor] (size_t bucketCount)
			{ return (size_t)((float)bucketCount * maxLoadFactor); };
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return mCalcCapacityFunc(bucketCount);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return mGetBucketCountShiftFunc(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return mLogStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		return std::hash<Key>()(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}

private:
	CalcCapacityFunc mCalcCapacityFunc;
	GetBucketCountShiftFunc mGetBucketCountShiftFunc;
	size_t mLogStartBucketCount;
};

template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THashFunc HashFunc;
	typedef TEqualFunc EqualFunc;
	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = std::false_type;

public:
	explicit HashTraitsStd(size_t startBucketCount = (size_t)1 << HashBucket::logStartBucketCount,
		const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc)
	{
		startBucketCount = std::minmax(startBucketCount, (size_t)8).second;
		mLogStartBucketCount = (uint8_t)internal::UIntMath<size_t>::Log2(startBucketCount - 1) + 1;
		startBucketCount = (size_t)1 << mLogStartBucketCount;
		size_t startCapacity = HashBucket::CalcCapacity(startBucketCount);
		mMaxLoadFactor = (float)startCapacity / (float)startBucketCount;
		HashBucket::CheckMaxLoadFactor(mMaxLoadFactor);
	}

	HashTraitsStd(const HashTraitsStd& hashTraits, float maxLoadFactor)
		: mHashFunc(hashTraits.mHashFunc),
		mEqualFunc(hashTraits.mEqualFunc),
		mLogStartBucketCount(hashTraits.mLogStartBucketCount),
		mMaxLoadFactor(maxLoadFactor)
	{
		HashBucket::CheckMaxLoadFactor(mMaxLoadFactor);
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return (size_t)((float)bucketCount * mMaxLoadFactor);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketCountShift(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return (size_t)mLogStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		return mHashFunc(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return mEqualFunc(key1, key2);
	}

	const HashFunc& GetHashFunc() const MOMO_NOEXCEPT
	{
		return mHashFunc;
	}

	const EqualFunc& GetEqualFunc() const MOMO_NOEXCEPT
	{
		return mEqualFunc;
	}

	float GetMaxLoadFactor() const MOMO_NOEXCEPT
	{
		return mMaxLoadFactor;
	}

private:
	HashFunc mHashFunc;
	EqualFunc mEqualFunc;
	uint8_t mLogStartBucketCount;
	float mMaxLoadFactor;
};

} // namespace momo
