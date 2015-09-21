#pragma once

#include "types.h"
#include "memory_types.h"

#include <utility>

namespace foundation
{
	/// Base class for memory allocators.
	///
	/// Note: Regardless of which allocator is used, prefer to allocate memory in larger chunks
	/// instead of in many small allocations. This helps with data locality, fragmentation,
	/// memory usage tracking, etc.
	class Allocator
	{
	public:
		/// Default alignment for memory allocations.
		static const uint32_t DEFAULT_ALIGN = 4;

		Allocator() {}
		virtual ~Allocator() {}
		
		/// Allocates the specified amount of memory aligned to the specified alignment.
		virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;

		/// Frees an allocation previously made with allocate().
		virtual void deallocate(void *p) = 0;

		static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;

		/// Returns the amount of usable memory allocated at p. p must be a pointer
		/// returned by allocate() that has not yet been deallocated. The value returned
		/// will be at least the size specified to allocate(), but it can be bigger.
		/// (The allocator may round up the allocation to fit into a set of predefined
		/// slot sizes.)
		///
		/// Not all allocators support tracking the size of individual allocations.
		/// An allocator that doesn't suppor it will return SIZE_NOT_TRACKED.
		virtual uint32_t allocated_size(void *p) = 0;

		/// Returns the total amount of memory allocated by this allocator. Note that the 
		/// size returned can be bigger than the size of all individual allocations made,
		/// because the allocator may keep additional structures.
		///
		/// If the allocator doesn't track memory, this function returns SIZE_NOT_TRACKED.
		virtual uint32_t total_allocated() = 0;

		/// Returns base address of an allocation or nullptr if not supported
		virtual void *allocation_base(void *p) = 0;

		/// Creates a new object of type T using the allocator to allocate the memory.
		template<typename T, typename... Args>
		T *make_new(Args&&... args);

		template<typename T>
		T *make_array(uint32_t count);

		/// Frees an object allocated with MAKE_NEW.
		template<typename T>
		void make_delete(T *ptr);

		template<typename T>
		void make_array_delete(T *array, uint32_t count);

	private:
		/// Allocators cannot be copied.
	    Allocator(const Allocator& other) = delete;
	    Allocator& operator=(const Allocator& other) = delete;
	};
	
	template<typename T, typename... Args>
	T *Allocator::make_new(Args&&... args) {
		return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
	}

	template<typename T>
	T *Allocator::make_array(uint32_t count) {
		auto arr = (T *)allocate(sizeof(T) * count, alignof(T));

		for (auto i = 0u; i < count; ++i) {
			new (arr + i) T{};
		}

		return arr;
	}
	
	template<typename T>
	void Allocator::make_delete(T *ptr) {
		if (ptr != nullptr) {
			ptr->~T();
			deallocate(ptr);
		}
	}

	template<typename T>
	void make_array_delete(T *array, uint32_t count) {
		if (array != nullptr) {
			for (auto i = 0u; i < count; ++i) {
				array[i]->~T();
			}
		}
		deallocate(array);
	}

	class HeapAllocator : public Allocator {
	 public:
	 	HeapAllocator(void *buffer, uint32_t size);
	 	HeapAllocator(Allocator &backing_allocator);
	 	~HeapAllocator();

	 	virtual void *allocate(uint32_t size, uint32_t align);
	 	virtual void deallocate(void *p);

	 	virtual uint32_t allocated_size(void *p);
		virtual uint32_t total_allocated();

		virtual void *allocation_base(void *p) { return nullptr; }

	 private:
	 	void *_memory_space;
	 	uint32_t _total_allocated;
	};

	class PageAllocator : public Allocator {
	 public:
	 	PageAllocator();
	 	~PageAllocator();

	 	virtual void *allocate(uint32_t size, uint32_t align);
	 	virtual void deallocate(void *p);

	 	virtual uint32_t allocated_size(void *p);
		virtual uint32_t total_allocated();

		virtual void *allocation_base(void *p);

	 private:
	 	uint32_t _total_allocated;
	};

	/// An allocator used to allocate temporary "scratch" memory. The allocator
	/// uses a fixed size ring buffer to services the requests.
	///
	/// Memory is always always allocated linearly. An allocation pointer is
	/// advanced through the buffer as memory is allocated and wraps around at
	/// the end of the buffer. Similarly, a free pointer is advanced as memory
	/// is freed.
	///
	/// It is important that the scratch allocator is only used for short-lived
	/// memory allocations. A long lived allocator will lock the "free" pointer
	/// and prevent the "allocate" pointer from proceeding past it, which means
	/// the ring buffer can't be used.
	/// 
	/// If the ring buffer is exhausted, the scratch allocator will use its backing
	/// allocator to allocate memory instead.
	class ScratchAllocator : public Allocator {
	 public:

	 	/// Creates a ScratchAllocator. The allocator will use the backing
	 	/// allocator to create the ring buffer and to service any requests
	 	/// that don't fit in the ring buffer.
	 	///
	 	/// size specifies the size of the ring buffer.
		ScratchAllocator(Allocator &backing, uint32_t size);
		~ScratchAllocator();

		virtual void *allocate(uint32_t size, uint32_t align);
		virtual void deallocate(void *p);

		virtual uint32_t allocated_size(void *p);
		virtual uint32_t total_allocated();

		virtual void *allocation_base(void *p);

   private:
		Allocator &_backing;
		
		char *_begin, *_end;
		char *_allocate, *_free;

		bool in_use(void *p);
	};

	/// Functions for accessing global memory data.
	namespace memory_globals {
		/// Initializes the global memory allocators. scratch_buffer_size is the size of the
		/// memory buffer used by the scratch allocators.
		void init(uint32_t scratch_buffer_size = 4*1024*1024);

		/// Returns a default memory allocator that can be used for most allocations.
		///
		/// You need to call init() for this allocator to be available.
		Allocator &default_allocator();

		/// Allocator that uses the systems specific page allocations
		Allocator &default_page_allocator();

		/// Returns a "scratch" allocator that can be used for temporary short-lived memory
		/// allocations. The scratch allocator uses a ring buffer of size scratch_buffer_size
		/// to service the allocations.
		///
		/// If there is not enough memory in the buffer to match requests for scratch
		/// memory, memory from the default_allocator will be returned instead.
		Allocator &default_scratch_allocator();

		/// Shuts down the global memory allocators created by init().
		void shutdown();
	}

	namespace memory {
		inline void *align_forward(void *p, uint32_t align);
		inline void *pointer_add(void *p, uint32_t bytes);
		inline const void *pointer_add(const void *p, uint32_t bytes);
		inline void *pointer_sub(void *p, uint32_t bytes);
		inline const void *pointer_sub(const void *p, uint32_t bytes);
	}

	// ---------------------------------------------------------------
	// Inline function implementations
	// ---------------------------------------------------------------

	// Aligns p to the specified alignment by moving it forward if necessary
	// and returns the result.
	inline void *memory::align_forward(void *p, uint32_t align)
	{
		uintptr_t pi = uintptr_t(p);
		const uint32_t mod = pi % align;
		if (mod)
			pi += (align - mod);
		return (void *)pi;
	}

	/// Returns the result of advancing p by the specified number of bytes
	inline void *memory::pointer_add(void *p, uint32_t bytes)
	{
		return (void*)((char *)p + bytes);
	}

	inline const void *memory::pointer_add(const void *p, uint32_t bytes)
	{
		return (const void*)((const char *)p + bytes);
	}

	/// Returns the result of moving p back by the specified number of bytes
	inline void *memory::pointer_sub(void *p, uint32_t bytes)
	{
		return (void*)((char *)p - bytes);
	}

	inline const void *memory::pointer_sub(const void *p, uint32_t bytes)
	{
		return (const void*)((const char *)p - bytes);
	}
}
