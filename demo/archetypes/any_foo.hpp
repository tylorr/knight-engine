#ifndef ANY_FOO_
#define ANY_FOO_

#include "type_erasure.h"

struct AnyFoo {
  int print() const;
  void change(int value);
};

#endif
