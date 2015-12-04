#pragma once

#include "buffer_object.h"
#include "array_object.h"

namespace knight {

struct MeshResource {
  BufferObject vbo;
  BufferObject ibo;
  ArrayObject vao;

  MeshResource() :
    vbo{BufferObject::Target::Array},
    ibo{BufferObject::Target::ElementArray},
    vao{} { }
};

namespace mesh_resource {

auto

} // namespace mesh_resource
} // namespace knight
