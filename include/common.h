#pragma once

#include <bitset>
#include <cstdint>
#include <string>
#include <cstdlib>
#include <limits>
#include <cstdio>

namespace knight {

#define KNIGHT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;            \
  TypeName &operator=(const TypeName &) = delete;

#define KNIGHT_DISALLOW_MOVE_AND_ASSIGN(TypeName) \
  TypeName(TypeName &&) = delete;            \
  TypeName &operator=(TypeName &&) = delete;

#if defined(DEVELOPMENT)
  #define XASSERT(test, msg, ...) do {if (!(test)) error(__LINE__, __FILE__, \
      "Assertion failed: %s\n\n" msg, #test,  __VA_ARGS__);} while (0)
#else
  #define XASSERT(test, msg, ...) ((void)0)
#endif

#define BOOL_STRING(value) (value ? "true" : "false")

template<typename T>
union ID {
  static constexpr uint64_t max() noexcept { 
    return std::numeric_limits<uint64_t>::max(); 
  }
  
  uint64_t id;
  struct {
    uint32_t index;
    uint32_t version;
  };

  ID() : id(0) { }
  ID(const uint64_t &val) : id(val) { }
  ID(uint64_t &&val) : id(val) { }

  operator uint64_t() const { return id; }
  ID &operator=(const uint64_t &val) {
    id = val;
    return *this;
  }

  bool operator==(const ID &other) const {
    return id == other.id;
  }
};

// TODO: set from cmake
static const uint64_t MAX_COMPONENTS = 64;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

void ExitOnGLError(const std::string &error_message);

template<typename... Args>
void error(const int &line_number, const char *filename, 
           const char *message, Args... args) {
  printf("%s:%d", filename, line_number);
  printf(message, args...);
  exit(1);
}

std::string GetFileContents(const char *filename);

} // namespace knight
