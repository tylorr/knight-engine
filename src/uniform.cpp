#include "uniform.h"
#include "program.h"

void Uniform::NotifyOwners() {
  int len = owner_.size();
  for (int i = 0; i < len; ++i) {
    owner_[i]->NotifyDirty(this, loc_[i]);
  }
}

template<>
void UniformFloat::UpdateImpl() {
  printf("update float");
}

template<>
void UniformVec2::UpdateImpl() {
  printf("Update float2");
}

template<>
void UniformVec3::UpdateImpl() {
  printf("Update float3");
}

template<>
void UniformVec4::UpdateImpl() {
  printf("Update float4");
}

template<>
void UniformInt::UpdateImpl() {
  
}
