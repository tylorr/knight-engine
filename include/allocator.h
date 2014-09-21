#include <malloc.h>

class Allocator {
 public:
  virtual void *Allocate(const size_t &size, const size_t &align) = 0;
  virtual void Deallocate(void *ptr) = 0;
  virtual size_t AllocatedSize(void *ptr) = 0;

  template<typename T, typename... Args>
  T *MakeNew(Args&&... args) {
    return new (Allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
  }

  template<typename T>
  void MakeDelete(T *ptr) {
    if (ptr) {
      ptr->~T();
      Deallocate(ptr);
    }
  }
};

class HeapAllocator : public Allocator {
 public:
  HeapAllocator(const char *name, void *base, const size_t &size);
  ~HeapAllocator();

  virtual void *Allocate(const size_t &size, const size_t &align);
  virtual void Deallocate(void *ptr);
  virtual size_t AllocatedSize(void *ptr);

 private:
  const char *name_;
  mspace mspace_;
};
