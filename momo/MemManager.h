/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/MemManager.h

  namespace momo:
    class MemManagerCpp
    class MemManagerC
    class MemManagerWin
    class MemManagerStd
    class MemManagerDefault

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

//class MemManager
//{
//public:
//	static const bool canReallocate = true;
//	static const bool canReallocateInplace = true;
//
//public:
//	MemManager();
//	MemManager(MemManager&& memManager) MOMO_NOEXCEPT;
//	MemManager(const MemManager& memManager);
//	~MemManager() MOMO_NOEXCEPT;
//	MemManager& operator=(const MemManager&) = delete;
//
//	void* Allocate(size_t size);
//	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT;
//	void* Reallocate(void* ptr, size_t size, size_t newSize);
//	bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT;
//};

class MemManagerCpp
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

public:
	MemManagerCpp() MOMO_NOEXCEPT
	{
	}

	MemManagerCpp(MemManagerCpp&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerCpp(const MemManagerCpp& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerCpp() MOMO_NOEXCEPT
	{
	}

	MemManagerCpp& operator=(const MemManagerCpp&) = delete;

	void* Allocate(size_t size)
	{
		assert(size > 0);
		return operator new(size);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		assert(ptr != nullptr && size > 0);
		operator delete(ptr);
	}
};

class MemManagerC
{
public:
	static const bool canReallocate = true;
	static const bool canReallocateInplace = false;

public:
	MemManagerC() MOMO_NOEXCEPT
	{
	}

	MemManagerC(MemManagerC&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerC(const MemManagerC& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerC() MOMO_NOEXCEPT
	{
	}

	MemManagerC& operator=(const MemManagerC&) = delete;

	void* Allocate(size_t size)
	{
		assert(size > 0);
		void* ptr = malloc(size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		return ptr;
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		assert(ptr != nullptr && size > 0);
		free(ptr);
	}

	void* Reallocate(void* ptr, size_t size, size_t newSize)
	{
		assert(ptr != nullptr && size > 0 && newSize > 0);
		if (size == newSize)
			return ptr;
		void* newPtr = realloc(ptr, newSize);
		if (newPtr == nullptr)
			throw std::bad_alloc();
		return newPtr;
	}
};

#ifdef MOMO_USE_MEM_MANAGER_WIN
class MemManagerWin
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = true;

public:
	MemManagerWin() MOMO_NOEXCEPT
	{
	}

	MemManagerWin(MemManagerWin&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerWin(const MemManagerWin& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerWin() MOMO_NOEXCEPT
	{
	}

	MemManagerWin& operator=(const MemManagerWin&) = delete;

	void* Allocate(size_t size)
	{
		assert(size > 0);
		void* ptr = HeapAlloc(GetProcessHeap(), 0, size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		return ptr;
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		assert(ptr != nullptr && size > 0);
		HeapFree(GetProcessHeap(), 0, ptr);
	}

	//void* Reallocate(void* ptr, size_t size, size_t newSize)
	//{
	//	assert(ptr != nullptr && size > 0 && newSize > 0);
	//	if (size == newSize)
	//		return ptr;
	//	void* newPtr = HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
	//	if (newPtr == nullptr)
	//		throw std::bad_alloc();
	//	return newPtr;
	//}

	bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
	{
		assert(ptr != nullptr && size > 0 && newSize > 0);
		if (size == newSize)
			return true;
		void* newPtr = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY,
			ptr, newSize);
		assert(newPtr == ptr || newPtr == nullptr);
		return newPtr == ptr;
	}
};
#endif

template<typename TAllocator,
	bool tUsePtrWrapper = !std::is_nothrow_move_constructible<
		typename std::allocator_traits<TAllocator>::template rebind_alloc<char>>::value>
class MemManagerStd;

template<typename TAllocator>
class MemManagerStd<TAllocator, false>
	: private std::allocator_traits<TAllocator>::template rebind_alloc<char>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> CharAllocator;

	MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<CharAllocator>::value);

	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

public:
	MemManagerStd()
	{
	}

	explicit MemManagerStd(const Allocator& alloc)
		: CharAllocator(alloc)
	{
	}

	MemManagerStd(MemManagerStd&& memManager) MOMO_NOEXCEPT
		: CharAllocator(std::move(memManager._GetCharAllocator()))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: CharAllocator(std::allocator_traits<CharAllocator>
			::select_on_container_copy_construction(memManager._GetCharAllocator()))
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	void* Allocate(size_t size)
	{
		return std::allocator_traits<CharAllocator>::allocate(_GetCharAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		std::allocator_traits<CharAllocator>::deallocate(_GetCharAllocator(), (char*)ptr, size);
	}

	Allocator GetAllocator() const
	{
		return Allocator(_GetCharAllocator());
	}

private:
	const CharAllocator& _GetCharAllocator() const MOMO_NOEXCEPT
	{
		return *this;
	}

	CharAllocator& _GetCharAllocator() MOMO_NOEXCEPT
	{
		return *this;
	}
};

template<typename TAllocator>
class MemManagerStd<TAllocator, true>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> CharAllocator;

	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

public:
	MemManagerStd()
		: mCharAllocator(new CharAllocator)
	{
	}

	explicit MemManagerStd(const Allocator& alloc)
		: mCharAllocator(new CharAllocator(alloc))
	{
	}

	MemManagerStd(MemManagerStd&& memManager) MOMO_NOEXCEPT
		: mCharAllocator(std::move(memManager.mCharAllocator))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: mCharAllocator(new CharAllocator(std::allocator_traits<CharAllocator>
			::select_on_container_copy_construction(*memManager.mCharAllocator)))
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	void* Allocate(size_t size)
	{
		return std::allocator_traits<CharAllocator>::allocate(*mCharAllocator, size);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		std::allocator_traits<CharAllocator>::deallocate(*mCharAllocator, (char*)ptr, size);
	}

	Allocator GetAllocator() const
	{
		return Allocator(*mCharAllocator);
	}

private:
	std::unique_ptr<CharAllocator> mCharAllocator;
};

typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

template<typename TItem>
class MemManagerStd<std::allocator<TItem>, false> : public MemManagerDefault
{
public:
	typedef std::allocator<TItem> Allocator;

public:
	explicit MemManagerStd(const Allocator& /*alloc*/ = Allocator()) MOMO_NOEXCEPT
	{
	}

	MemManagerStd(MemManagerStd&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerStd(const MemManagerStd& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	Allocator GetAllocator() const MOMO_NOEXCEPT
	{
		return Allocator();
	}
};

namespace internal
{
	class MemManagerDummy
	{
	public:
		static const bool canReallocate = false;
		static const bool canReallocateInplace = false;

	public:
		MemManagerDummy() MOMO_NOEXCEPT
		{
		}

		MemManagerDummy(MemManagerDummy&& /*memManager*/) MOMO_NOEXCEPT
		{
		}

		MemManagerDummy(const MemManagerDummy& /*memManager*/) MOMO_NOEXCEPT
		{
		}

		~MemManagerDummy() MOMO_NOEXCEPT
		{
		}

		MemManagerDummy& operator=(const MemManagerDummy&) = delete;

		//void* Allocate(size_t size);

		void Deallocate(void* /*ptr*/, size_t /*size*/) MOMO_NOEXCEPT
		{
			assert(false);
		}
	};

	template<typename TMemManager,
		bool tIsEmpty = std::is_empty<TMemManager>::value>
	class MemManagerWrapper;

	template<typename TMemManager>
	class MemManagerWrapper<TMemManager, false>
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) MOMO_NOEXCEPT
			: mMemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
			: mMemManager(std::move(memManagerWrapper.mMemManager))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() MOMO_NOEXCEPT
		{
		}

		MemManagerWrapper& operator=(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
		{
			if (this != &memManagerWrapper)
			{
				mMemManager.~MemManager();
				new(&mMemManager) MemManager(std::move(memManagerWrapper.mMemManager));
			}
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mMemManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mMemManager;
		}

	private:
		MemManager mMemManager;
	};

	template<typename TMemManager>
	class MemManagerWrapper<TMemManager, true> : protected TMemManager
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) MOMO_NOEXCEPT
			: MemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
			: MemManager(std::move(memManagerWrapper.GetMemManager()))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() MOMO_NOEXCEPT
		{
		}

		MemManagerWrapper& operator=(MemManagerWrapper&& /*memManagerWrapper*/) MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}
	};

	template<typename TMemManager,
		bool tIsMemManagerEmpty = std::is_empty<TMemManager>::value>
	class MemManagerPtr;

	template<typename TMemManager>
	class MemManagerPtr<TMemManager, true> : private TMemManager
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

		static const bool canReallocate = MemManager::canReallocate;
		static const bool canReallocateInplace = MemManager::canReallocateInplace;

	public:
		explicit MemManagerPtr(MemManager& /*memManager*/)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManagerPtr) MOMO_NOEXCEPT
			: MemManager(std::move(memManagerPtr._GetMemManager()))
		{
		}

		MemManagerPtr(const MemManagerPtr& /*memManagerPtr*/)
		{
		}

		~MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		void* Allocate(size_t size)
		{
			return _GetMemManager().Allocate(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			return _GetMemManager().Deallocate(ptr, size);
		}

		void* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return _GetMemManager().Reallocate(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return _GetMemManager().ReallocateInplace(ptr, size, newSize);
		}

	private:
		//const MemManager& _GetMemManager() const MOMO_NOEXCEPT
		//{
		//	return *this;
		//}

		MemManager& _GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}
	};

	template<typename TMemManager>
	class MemManagerPtr<TMemManager, false>
	{
	public:
		typedef TMemManager MemManager;

		static const bool canReallocate = MemManager::canReallocate;
		static const bool canReallocateInplace = MemManager::canReallocateInplace;

	public:
		explicit MemManagerPtr(MemManager& memManager) MOMO_NOEXCEPT
			: mMemManager(memManager)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManagerPtr) MOMO_NOEXCEPT
			: mMemManager(memManagerPtr.mMemManager)
		{
		}

		MemManagerPtr(const MemManagerPtr& memManagerPtr) MOMO_NOEXCEPT
			: mMemManager(memManagerPtr.mMemManager)
		{
		}

		~MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		void* Allocate(size_t size)
		{
			return mMemManager.Allocate(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			return mMemManager.Deallocate(ptr, size);
		}

		void* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mMemManager.Reallocate(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return mMemManager.ReallocateInplace(ptr, size, newSize);
		}

	private:
		MemManager& mMemManager;
	};
}

} // namespace momo
