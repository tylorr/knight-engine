#include "material.h"
#include "iterators.h"
#include "platform_types.h"
#include "uniform.h"
#include "std_allocator.h"
#include "common.h"

#include <hash.h>
#include <temp_allocator.h>
#include <string_stream.h>
#include <murmur_hash.h>

using namespace foundation;
using namespace foundation::string_stream;

namespace knight {

#define KNIGHT_CREATE_UNIFORM_CASES2(UpperTypeName, LowerTypeName) \
  case GL_ ## UpperTypeName:          uniform = alloc_.make_new<Uniform<LowerTypeName, 1>>(alloc_, *this, name_string); break; \
  case GL_ ## UpperTypeName ## _VEC2: uniform = alloc_.make_new<Uniform<LowerTypeName, 2>>(alloc_, *this, name_string); break; \
  case GL_ ## UpperTypeName ## _VEC3: uniform = alloc_.make_new<Uniform<LowerTypeName, 3>>(alloc_, *this, name_string); break; \
  case GL_ ## UpperTypeName ## _VEC4: uniform = alloc_.make_new<Uniform<LowerTypeName, 4>>(alloc_, *this, name_string); break;

Material::Material(foundation::Allocator &alloc, GLuint program_handle, 
                   uint32_t version, foundation::Array<UniformBase *> uniform_list)
    : program_handle_{program_handle},
      version_{version},
      uniforms_{alloc} {
  for (auto &uniform : uniform_list) {
    auto location = glGetUniformLocation(program_handle, uniform->name());
    uniform->AddMaterial(*this, location);
    hash::set(uniforms_, location, uniform);
  }
}

MaterialManager::MaterialManager(foundation::Allocator &alloc)
    : alloc_{alloc},
      global_uniforms_{alloc},
      shaders_{alloc},
      material_version_{alloc},
      uniforms_{alloc},
      dirty_uniforms_{alloc} { }

MaterialManager::MaterialManager(foundation::Allocator &alloc, Array<const char *> global_uniforms)
    : alloc_{alloc},
      global_uniforms_{global_uniforms},
      shaders_{alloc},
      material_version_{alloc},
      uniforms_{alloc},
      dirty_uniforms_{alloc} { }


MaterialManager::~MaterialManager() {
  for (auto &item : uniforms_) {
    alloc_.make_delete(item.value);
  }

  for (auto &item : shaders_) {
    glDeleteShader(item.value.vertex);
    glDeleteShader(item.value.fragment);
    glDeleteProgram(item.value.program);
  }
  
  hash::clear(shaders_);
  hash::clear(uniforms_);
  hash::clear(material_version_);
}

GLuint MaterialManager::CreateShader(const char *shader_path) {
  TempAllocator4096 temp_allocator;
  Buffer shader_source{temp_allocator};
  File shader_file{shader_path};
  shader_file.Read(shader_source);

  auto shader_id = murmur_hash_64(shader_path, strlen(shader_path), 0u);
  auto shader_handles = hash::get(shaders_, shader_id, ShaderHandles{});
  if (shader_handles.program != 0) {
    return shader_handles.program;
  } else {
    return CreateShaderFromSource(shader_id, c_str(shader_source));
  }
}

GLuint MaterialManager::CreateShaderFromSource(const char *name, const char *shader_source) {
  auto shader_id = murmur_hash_64(name, strlen(name), 0u);
  return CreateShaderFromSource(shader_id, shader_source);
}

GLuint MaterialManager::CreateShaderFromSource(uint64_t shader_id, const char *shader_source) {
  auto getDefine = [](GLenum type) {
    switch (type) {
      case GL_VERTEX_SHADER: return "#define VERTEX\n";
      case GL_FRAGMENT_SHADER: return "#define FRAGMENT\n";
      default: return "";
    }
  };

  auto program_handle = glCreateProgram();

  auto createAndAttachShader = [&](GLenum type) {
    auto shader_handle = GLuint{};
    GL(shader_handle = glCreateShader(type));

    const char *full_source[3] = { "#version 440\n", getDefine(type), shader_source };

    GL(glShaderSource(shader_handle, 3, full_source, nullptr));
    GL(glCompileShader(shader_handle));

    auto result = GLint{};
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
      auto length = GLint{};
      glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &length);

      if (length) {
        char info_log[length];
        glGetShaderInfoLog(shader_handle, length, nullptr, info_log);
        XASSERT(result != GL_FALSE, "Shader compile error:\n%s", info_log);
      }
    }

    GL(glAttachShader(program_handle, shader_handle));
    return shader_handle;
  };

  auto vertex_handle = createAndAttachShader(GL_VERTEX_SHADER);
  auto fragment_handle = createAndAttachShader(GL_FRAGMENT_SHADER);
  
  auto result = GLint{};
  GL(glLinkProgram(program_handle));
  GL(glGetProgramiv(program_handle, GL_LINK_STATUS, &result));
  if (result == GL_FALSE) {
    auto length = GLint{};
    glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &length);

    if (length) {
      char info_log[length];
      glGetProgramInfoLog(program_handle, length, nullptr, info_log);
      XASSERT(result != GL_FALSE, "Shader program link error:\n%s", info_log);
    }
  }

  hash::set(shaders_, shader_id, ShaderHandles{program_handle, vertex_handle, fragment_handle});

  auto max_uniform_name_length = GLint{};
  glGetProgramiv(program_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);

  auto uniform_count = GLint{};
  glGetProgramiv(program_handle, GL_ACTIVE_UNIFORMS, &uniform_count);

  for (auto i = 0; i < uniform_count; ++i) {
    char name[max_uniform_name_length];
    auto size = GLint{};
    auto type = GLenum{};
    glGetActiveUniform(program_handle, i, max_uniform_name_length, nullptr, 
                       &size, &type, name);

    //TODO: TR Skip uniforms that are in the global_uniform list

    Buffer name_string{alloc_};
    name_string << name;

    UniformBase *uniform = nullptr;
    switch (type) {
      KNIGHT_CREATE_UNIFORM_CASES2(FLOAT, float)
      KNIGHT_CREATE_UNIFORM_CASES2(INT, int)
      KNIGHT_CREATE_UNIFORM_CASES2(BOOL, bool)
      case GL_FLOAT_MAT2: uniform = alloc_.make_new<Uniform<float, 2, 2>>(alloc_, *this, name_string); break;
      case GL_FLOAT_MAT3: uniform = alloc_.make_new<Uniform<float, 3, 3>>(alloc_, *this, name_string); break;
      case GL_FLOAT_MAT4: uniform = alloc_.make_new<Uniform<float, 4, 4>>(alloc_, *this, name_string); break;
      // TODO: TR Handle other unform types (samplers etc.)
      // default: XASSERT(false, "Cannot make a uniform of this type 0x%X", type);
    }

    if (uniform != nullptr) {
      auto version = 0u;
      auto shader_material_hash = murmur_hash_64(&program_handle, sizeof(program_handle), version);
      multi_hash::insert(uniforms_, shader_material_hash, uniform);
    }
  }

  return program_handle;
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(const char *shader_path) {
  return CreateMaterial(CreateShader(shader_path));
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(GLuint program_handle) {
  Array<UniformBase *> material_uniforms{alloc_};

  auto version = 0u;
  auto shader_material_hash = murmur_hash_64(&program_handle, sizeof(program_handle), version);
  multi_hash::get(uniforms_, shader_material_hash, material_uniforms);

  auto material = allocate_shared<Material>(alloc_, alloc_, program_handle, version, material_uniforms);
  material->Bind();
  return material;
}

std::shared_ptr<Material> MaterialManager::CloneMaterial(const std::shared_ptr<Material> &other) {
  auto program_handle = other->program_handle();

  TempAllocator64 temp_alloc;
  Array<UniformBase *> original_uniforms{temp_alloc};
  auto original_hash = murmur_hash_64(&program_handle, sizeof(program_handle), other->version());
  multi_hash::get(uniforms_, original_hash, original_uniforms);

  auto clone_version = hash::get(material_version_, program_handle, 0u) + 1;
  hash::set(material_version_, program_handle, clone_version);

  auto clone_hash = murmur_hash_64(&program_handle, sizeof(program_handle), clone_version);

  Array<UniformBase *> clone_uniforms{alloc_};
  for (auto &uniform : original_uniforms) {
    auto clone = uniform->Clone(alloc_);
    array::push_back(clone_uniforms, clone);
    multi_hash::insert(uniforms_, clone_hash, clone);
  }

  return allocate_shared<Material>(alloc_, alloc_, program_handle, clone_version, clone_uniforms);
}

void MaterialManager::MarkDirty(UniformBase *uniform, const Material &mat, 
                                GLint location) {
  auto program_handle = mat.program_handle();
  auto shader_hash = murmur_hash_64(&program_handle, sizeof(program_handle), mat.version());
  multi_hash::insert(dirty_uniforms_, shader_hash, DirtyUniform{uniform, location});
}

//TODO: TR Handle going from v0 to v1 back to v0 of shader
void MaterialManager::PushUniforms(const Material &mat) {
  auto program_handle = mat.program_handle();
  auto shader_hash = murmur_hash_64(&program_handle, sizeof(program_handle), mat.version());

  TempAllocator64 temp_alloc;
  Array<DirtyUniform> dirty_uniforms_for_mat{temp_alloc};
  multi_hash::get(dirty_uniforms_, shader_hash, dirty_uniforms_for_mat);

  mat.Bind();
  for (auto &dirty_uniform : dirty_uniforms_for_mat) {
    dirty_uniform.uniform->Push(dirty_uniform.location);
  }

  multi_hash::remove_all(dirty_uniforms_, shader_hash);
}

} // namespace knight
