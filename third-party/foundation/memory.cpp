#include "memory.h"
#include "dlmalloc.h"

#include <stdlib.h>
#include <assert.h>
#include <new>
#include <cstdio>

namespace {
	using namespace foundation;

	// Header stored at the beginning of a memory allocation to indicate the
	// size of the allocated data.
	struct Header {
		uint32_t size;
	};

	// If we need to align the memory allocation we pad the header with this
	// value after storing the size. That way we can 
	const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

	// Given a pointer to the header, returns a pointer to the data that follows it.
	inline void *data_pointer(Header *header, uint32_t align) {
		void *p = header + 1;
		return memory::align_forward(p, align);
	}

	// Given a pointer to the data, returns a pointer to the header before it.
	inline Header *header(void *data)
	{
		uint32_t *p = (uint32_t *)data;
		while (p[-1] == HEADER_PAD_VALUE)
			--p;
		return (Header *)p - 1;
	}

	// Stores the size in the header and pads with HEADER_PAD_VALUE up to the
	// data pointer.
	inline void fill(Header *header, void *data, uint32_t size)
	{
		header->size = size;
		uint32_t *p = (uint32_t *)(header + 1);
		while (p < data)
			*p++ = HEADER_PAD_VALUE;
	}

	struct MemoryGlobals {
		static const size_t ALLOCATOR_MEMORY = (2 * sizeof(HeapAllocator)) + sizeof(ScratchAllocator) + sizeof(PageAllocator) + (128 * sizeof(size_t)) + 128;
		char buffer[ALLOCATOR_MEMORY];

		HeapAllocator *static_heap;
		PageAllocator *default_page_allocator;
		HeapAllocator *default_allocator;
		ScratchAllocator *default_scratch_allocator;

		MemoryGlobals() : static_heap(0), default_page_allocator(0), default_allocator(0), default_scratch_allocator(0) {}
	};

	MemoryGlobals _memory_globals;
}

namespace foundation
{

	///
	/// Heap Allocator
	///

	HeapAllocator::HeapAllocator(void *buffer, uint32_t size) 
			: _total_allocated(0) {
		_memory_space = create_mspace_with_base(nullptr, buffer, size, false);
		assert(_memory_space);
	}

	HeapAllocator::HeapAllocator(Allocator &backing_allocator)
			: _total_allocated(0) {
		_memory_space = create_mspace(&backing_allocator, 0, false);
		mspace_track_large_chunks(_memory_space, true);
		assert(_memory_space);
	}

	HeapAllocator::~HeapAllocator() {
		destroy_mspace(_memory_space);
		assert(_total_allocated == 0);
	}

 	void *HeapAllocator::allocate(uint32_t size, uint32_t align) {
 		void *p = mspace_memalign(_memory_space, align, size);
 		auto as = allocated_size(p);
 		_total_allocated += as;
 		return p;
 	}

 	void HeapAllocator::deallocate(void *p) {
 		_total_allocated -= allocated_size(p);
 		mspace_free(_memory_space, p);
 	}

 	uint32_t HeapAllocator::allocated_size(void *p) {
 		return mspace_usable_size(p);
 	}

	uint32_t HeapAllocator::total_allocated() {
		return _total_allocated;
	}

	///
	/// Page Allocator
	///

	// TODO: TR This is currently windows specific, add linux support
	PageAllocator::PageAllocator() 
			: _total_allocated(0) {
	}

	PageAllocator::~PageAllocator() {
		assert(_total_allocated == 0);
	}

 	void *PageAllocator::allocate(uint32_t size, uint32_t align) {
 		void *ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
 		auto allocated = allocated_size(ptr);
 		_total_allocated += allocated;
 		return ptr;
 	}

 	void PageAllocator::deallocate(void *p) {
 		_total_allocated -= allocated_size(p);
 		VirtualFree(p, 0, MEM_RELEASE);
 	}

 	uint32_t PageAllocator::allocated_size(void *p) {
 		MEMORY_BASIC_INFORMATION minfo;
 		VirtualQuery(p, &minfo, sizeof(minfo));
 		return minfo.RegionSize;
 	}

	uint32_t PageAllocator::total_allocated() {
		return _total_allocated;
	}

	void *PageAllocator::allocation_base(void *p) {
		MEMORY_BASIC_INFORMATION minfo;
 		VirtualQuery(p, &minfo, sizeof(minfo));
 		return minfo.AllocationBase;
	}

	///
	/// Scratch Allocator
	///
	
	ScratchAllocator::ScratchAllocator(Allocator &backing, uint32_t size) : _backing(backing) {
		_begin = (char *)_backing.allocate(size);
		_end = _begin + size;
		_allocate = _begin;
		_free = _begin;
	}

	ScratchAllocator::~ScratchAllocator() {
		assert(_free == _allocate);
		_backing.deallocate(_begin);
	}

	bool ScratchAllocator::in_use(void *p)
	{
		if (_free == _allocate)
			return false;
		if (_allocate > _free)
			return p >= _free && p < _allocate;
		return p >= _free || p < _allocate;
	}

	void *ScratchAllocator::allocate(uint32_t size, uint32_t align) {
		assert(align % 4 == 0);
		size = ((size + 3)/4)*4;

		char *p = _allocate;
		Header *h = (Header *)p;
		char *data = (char *)data_pointer(h, align);
		p = data + size;

		// Reached the end of the buffer, wrap around to the beginning.
		if (p > _end) {
			h->size = (_end - (char *)h) | 0x80000000u;
			
			p = _begin;
			h = (Header *)p;
			data = (char *)data_pointer(h, align);
			p = data + size;
		}
		
		// If the buffer is exhausted use the backing allocator instead.
		if (in_use(p))
			return _backing.allocate(size, align);

		fill(h, data, p - (char *)h);
		_allocate = p;
		return data;
	}

	void ScratchAllocator::deallocate(void *p) {
		if (!p)
			return;

		if (p < _begin || p >= _end) {
			_backing.deallocate(p);
			return;
		}

		// Mark this slot as free
		Header *h = header(p);
		assert((h->size & 0x80000000u) == 0);
		h->size = h->size | 0x80000000u;

		// Advance the free pointer past all free slots.
		while (_free != _allocate) {
			Header *h = (Header *)_free;
			if ((h->size & 0x80000000u) == 0)
				break;

			_free += h->size & 0x7fffffffu;
			if (_free == _end)
				_free = _begin;
		}
	}

	uint32_t ScratchAllocator::allocated_size(void *p) {
		Header *h = header(p);
		return h->size - ((char *)p - (char *)h);
	}

	uint32_t ScratchAllocator::total_allocated() {
		return _end - _begin;
	}

	void *ScratchAllocator::allocation_base(void *p) { return nullptr; }

	///
	/// Memory globals
	///

	namespace memory_globals
	{
		void init(uint32_t temporary_memory) {
			char *buffer = _memory_globals.buffer;

			auto static_heap = new (buffer) HeapAllocator(buffer + sizeof(HeapAllocator), _memory_globals.ALLOCATOR_MEMORY - sizeof(HeapAllocator));
			_memory_globals.static_heap = static_heap;

			_memory_globals.default_page_allocator = static_heap->make_new<PageAllocator>();
			_memory_globals.default_allocator = static_heap->make_new<HeapAllocator>(*_memory_globals.default_page_allocator);
			_memory_globals.default_scratch_allocator = static_heap->make_new<ScratchAllocator>(*_memory_globals.default_allocator, temporary_memory);
		}

		Allocator &default_allocator() {
			return *_memory_globals.default_allocator;
		}

		Allocator &default_page_allocator() {
			return *_memory_globals.default_page_allocator;
		}

		Allocator &default_scratch_allocator() {
			return *_memory_globals.default_scratch_allocator;
		}

		void shutdown() {
			_memory_globals.static_heap->make_delete(_memory_globals.default_scratch_allocator);
			_memory_globals.static_heap->make_delete(_memory_globals.default_allocator);
			_memory_globals.static_heap->make_delete(_memory_globals.default_page_allocator);
			_memory_globals.static_heap->~HeapAllocator();

			_memory_globals = MemoryGlobals();
		}
	}
}
