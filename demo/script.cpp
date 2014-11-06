#include <logog.hpp>
#include <memory.h>

#include <cstdio>

extern "C" {
  void Init();
  //void Update(double);
}

void Init() {
  printf("Initialized script!!!\n");
}

void Update(double dt) {
}
