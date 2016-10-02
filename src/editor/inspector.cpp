#include "editor/inspector.h"

#include "editor/project_editor.h"

#include <logog.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>

using namespace google::protobuf;

namespace knight {
namespace editor {

namespace {

  // TODO: Find a way to work with proto::vec3 object instead of using reflection
  bool draw_vec3(const FieldDescriptor &field_descriptor, Message &message) {
    auto &descriptor = *message.GetDescriptor();
    auto *x_desc = descriptor.FindFieldByName("x");
    auto *y_desc = descriptor.FindFieldByName("y");
    auto *z_desc = descriptor.FindFieldByName("z");

    auto &reflection = *message.GetReflection();

    float vec_array[] = {
      reflection.GetFloat(message, x_desc),
      reflection.GetFloat(message, y_desc),
      reflection.GetFloat(message, z_desc)
    };
    auto changed = ImGui::DragFloat3(field_descriptor.name().c_str(), vec_array, 0.03f);

    if (changed) {
      reflection.SetFloat(&message, x_desc, vec_array[0]);
      reflection.SetFloat(&message, y_desc, vec_array[1]);
      reflection.SetFloat(&message, z_desc, vec_array[2]);
    }

    return changed;
  }

  bool draw_message_field(const FieldDescriptor &field_descriptor, Message &message) {
    auto &descriptor = *message.GetDescriptor();

    if (descriptor.full_name() == "knight.proto.vec3") {
      return draw_vec3(field_descriptor, message);
    }

    return false;
  }

  bool draw_component(Message &component_messsage) {
    auto &descriptor = *component_messsage.GetDescriptor();
    auto &reflection = *component_messsage.GetReflection();
    ImGui::Text("%s", descriptor.name().c_str());

    auto changed = false;
    for (auto i = 0; i < descriptor.field_count(); ++i) {
      auto &field_descriptor = *descriptor.field(i);

      switch (field_descriptor.type()) {
        case FieldDescriptor::TYPE_MESSAGE:
          auto *field_message = reflection.MutableMessage(&component_messsage, &field_descriptor);
          changed |= draw_message_field(field_descriptor, *field_message);
      }
    }

    return changed;
  }
} // namespace

Inspector::Inspector(ProjectEditor &project_editor)
  : project_editor_{project_editor},
    message_factory_{} {
  message_factory_.SetDelegateToGeneratedFactory(true);
}

void Inspector::draw() {
  // ImGui::Begin("Inspector");
  // auto *selected_entry = project_editor_.selected_entry();
  // if (selected_entry != nullptr && selected_entry->type == editor::EntryType::ComponentSchema) {

  //   const auto &pool = *google::protobuf::DescriptorPool::generated_pool();
  //   for (auto &&item : *selected_entry->resource_handle.mutable_defaults()) {
  //     auto &type_name = item.first;
  //     auto &default_any = item.second;

  //     auto *descriptor = pool.FindMessageTypeByName(type_name);
  //     if (descriptor == nullptr) continue;

  //     auto *factory_component = message_factory_.GetPrototype(descriptor);

  //     // TODO: Cache these so I don't have to allocate every frame
  //     std::unique_ptr<Message> component{factory_component->New()};

  //     default_any.UnpackTo(component.get());
  //     if (draw_component(*component)) {
  //       default_any.PackFrom(*component);
  //       selected_entry->dirty = true;
  //     }
  //   }
  // }
  // ImGui::End();
}


} // namespace editor
} // namespace knight
