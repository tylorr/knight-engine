#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <utility>
#include <queue>

#include "gc.h"

class Uniform;

class Program {
 public:
  void ExtractShaderUniforms();

  void NotifyDirty(Uniform *, unsigned int);

  void UpdateProgram();
  
 private:
  typedef std::pair<Uniform *, unsigned int> UniformLocPair;

  void UpdateUniform(Uniform *);

  unsigned int program_handle_;

  std::queue<UniformLocPair> dirty_uniforms_;

};

#endif // PROGRAM_H_
