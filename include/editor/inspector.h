#pragma once

#include <google/protobuf/dynamic_message.h>

namespace knight {
namespace editor {

class ProjectEditor;

class Inspector {
 public:
  Inspector(ProjectEditor &project_editor);

  void draw();

 private:
  ProjectEditor &project_editor_;
  google::protobuf::DynamicMessageFactory message_factory_;
};

}
}
