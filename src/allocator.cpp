#include "allocator.h"

#include "common.h"

#include <logog.hpp>

HeapAllocator::HeapAllocator(const char *name, void *base, const size_t &size)
    : name_(name) {
  mspace_ = create_mspace_with_base(base, size, 0);
  XASSERT(mspace_ != nullptr, "Size too small for heap allocator(%s): %zu", name, size);
}

HeapAllocator::~HeapAllocator() {
  size_t freed = destroy_mspace(mspace_);
  DBUG("Freed: %zu", freed);
}

void *HeapAllocator::Allocate(const size_t &size, const size_t &align) {
  return mspace_memalign(mspace_, size, align);
}

void HeapAllocator::Deallocate(void *ptr) {
  mspace_free(mspace_, ptr);
}

size_t HeapAllocator::AllocatedSize(void *ptr) {
  return mspace_usable_size(ptr);
}
