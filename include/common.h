#pragma once

#include <GL/glew.h>

#include <bitset>

constexpr std::size_t operator"" _z(unsigned long long n) { 
  return n; 
}

constexpr unsigned long long operator""_kib(unsigned long long n) {
  return n * 1024;
}

constexpr unsigned long long operator""_mib(unsigned long long n) {
  return n * 1024_kib;
}

constexpr unsigned long long operator""_gib(unsigned long long n) {
  return n * 1024_mib;
}

constexpr unsigned long long operator""_tib(unsigned long long n) {
  return n * 1024_gib;
}

struct GLFWwindow;

namespace knight {

#define KNIGHT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;            \
  TypeName &operator=(const TypeName &) = delete;

#define KNIGHT_DISALLOW_MOVE_AND_ASSIGN(TypeName) \
  TypeName(TypeName &&) = delete;            \
  TypeName &operator=(TypeName &&) = delete;

#define BOOL_STRING(value) (value ? "true" : "false")

#if defined(DEVELOPMENT)
  #define XASSERT(test, msg, ...) do {if (!(test)) error(__LINE__, __FILE__, \
      "\x1b[31mAssertion failed: %s\x1b[0m\n\n" msg, #test, ## __VA_ARGS__);} while (false)

  #define GL_ASSERT(msg, ...) do {                                  \
    auto error_value = glGetError();                                \
    if (error_value != GL_NO_ERROR) {                               \
      printf("\x1b[1m%s:%d:\x1b[0m ", __FILE__, __LINE__);          \
      printf(msg, ## __VA_ARGS__);                                  \
      printf("\n\n");                                               \
      while (error_value != GL_NO_ERROR) {                          \
        printf("\x1b[31m%s\x1b[0m\n", glErrorString(error_value));  \
        error_value = glGetError();                                 \
      }                                                             \
      printf("\n");                                                 \
      stack_trace();                                                \
      abort();                                                      \
    }                                                               \
  } while (false)

  #define GL(line) do { \
    line; \
    XASSERT(glGetError() == GL_NO_ERROR, "OpenGL error"); \
  } while (false)
#else
  #define XASSERT(test, msg, ...) ((void)0)
  #define GL_ASSERT(msg, ...) ((void)0)
  #define GL(line) line
#endif


#define EXPAND(EXPR) std::initializer_list<int>{((EXPR),0)...}

const char *glErrorString(GLenum error);

void stack_trace();

template<typename... Args>
void error(const int &line_number, const char *filename, 
           const char *message, Args... args) {
  // TODO: TR Print stack trace
  printf("\x1b[1m%s:%d:\x1b[0m ", filename, line_number);
  printf(message, args...);
  printf("\n\n");
  stack_trace();
  abort();
}

void *knight_malloc(size_t size);
void knight_free(void *ptr);
void knight_no_memory();

} // namespace knight
