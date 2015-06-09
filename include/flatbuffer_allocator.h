#pragma once

#include "flatbuffers/flatbuffers.h"

namespace foundation {

class FlatBufferAllocator : public flatbuffers::simple_allocator {
 public:
  FlatBufferAllocator(foundation::Allocator &allocator) 
      : allocator_(allocator) { }

  uint8_t *allocate(size_t size) const override {
    return static_cast<uint8_t *>(allocator_.allocate(size * sizeof(uint8_t)));
  }

  void deallocate(uint8_t *p) const override {
    allocator_.deallocate(p);
  }

 private:
  foundation::Allocator &allocator_;
};

}
