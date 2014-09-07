#ifndef KNIGHT_COMMON_H_
#define KNIGHT_COMMON_H_

#include <logog.hpp>

#include <bitset>
#include <cstdint>
#include <string>
#include <cstdlib>

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

// TODO: set from cmake
static const uint64_t MAX_COMPONENTS = 64;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

void ExitOnGLError(const std::string &error_message);

template<typename... Args>
void error(const int &line_number, const char *filename, 
           const char *message, Args... args) {
  ERR("%s:%d", filename, line_number);
  ERR(message, args...);
  abort();
}

std::string GetFileContents(const char *filename);

} // namespace knight

#endif // KNIGHT_COMMON_H_
