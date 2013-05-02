#ifndef UTILS_H_
#define UTILS_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

void ExitOnGLError(const char *error_message);

#endif // UTILS_H_
