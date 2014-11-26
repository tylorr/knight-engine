#pragma once

#include <bitset>
#include <cstdint>
#include <string>
#include <cstdlib>
#include <limits>
#include <cstdio>

constexpr std::size_t operator"" _z(unsigned long long n) { 
  return n; 
}

namespace knight {

#define KNIGHT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;            \
  TypeName &operator=(const TypeName &) = delete;

#define KNIGHT_DISALLOW_MOVE_AND_ASSIGN(TypeName) \
  TypeName(TypeName &&) = delete;            \
  TypeName &operator=(TypeName &&) = delete;

#define BOOL_STRING(value) (value ? "true" : "false")

template<typename Owner, typename T, size_t index_bits, size_t version_bits>
union ID {
  typedef T type;
  
  T id;
  struct {
    T index   : index_bits;
    T version : version_bits;
  };

  ID() : id(0) { }
  ID(const T &val) : id(val) { }
  ID(T &&val) : id(val) { }

  operator T() const { return id; }
  ID &operator=(const T &val) {
    id = val;
    return *this;
  }

  bool operator==(const ID &other) const {
    return id == other.id;
  }
};

template<typename Owner>
struct ID32 { typedef ID<Owner, uint32_t, 16, 16> ID; };

template<typename Owner>
struct ID64 { typedef ID<Owner, uint64_t, 32, 32> ID; };

// TODO: set from cmake
static const uint64_t kMaxComponents = 64;

typedef std::bitset<kMaxComponents> ComponentMask;

void ExitOnGLError(const std::string &error_message);

#if defined(DEVELOPMENT)
  #define XASSERT(test, msg, ...) do {if (!(test)) error(__LINE__, __FILE__, \
      "Assertion failed: %s\n\n" msg, #test, ##  __VA_ARGS__);} while (0)
#else
  #define XASSERT(test, msg, ...) ((void)0)
#endif

template<typename... Args>
void error(const int &line_number, const char *filename, 
           const char *message, Args... args) {
  // TODO: TR Print stack trace
  printf("%s:%d\n", filename, line_number);
  printf(message, args...);
  printf("\n");
  abort();
}

std::string GetFileContents(const char *filename);

void *knight_malloc(size_t size);
void knight_free(void *ptr);
void knight_no_memory();

} // namespace knight
