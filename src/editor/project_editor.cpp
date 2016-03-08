#include "editor/project_editor.h"


#include "assets/proto/component.pb.h"
#include "file_util.h"

#include <boost/filesystem/operations.hpp>
#include <imgui.h>
#include <logog.hpp>
#include <temp_allocator.h>

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

namespace fs = boost::filesystem;
using namespace foundation;

namespace protobuf = google::protobuf;
namespace compiler = google::protobuf::compiler;
namespace util = google::protobuf::util;

namespace knight {

using namespace string_stream;

namespace editor {

static const char *kTypeUrlPrefix = "type.googleapis.com";
constexpr gsl::czstring<> kMetaFileExtenstion = ".meta";

static std::string get_type_url(const protobuf::Descriptor* message) {
  return std::string{kTypeUrlPrefix} + "/" + message->full_name();
}

void write_resource_handle(util::TypeResolver *type_resolver, 
                           fs::path file_path, 
                           const proto::ResourceHandle &resource_handle) {
  auto meta_path = file_path;
  meta_path += kMetaFileExtenstion;

  util::JsonOptions options;
  options.add_whitespace = true;

  std::string meta_json;
  util::BinaryToJsonString(type_resolver,
                           get_type_url(resource_handle.GetDescriptor()),
                           resource_handle.SerializeAsString(), &meta_json, options);

  file_util::write_buffer_to_file(meta_path.string().c_str(), meta_json);
}

proto::ResourceHandle load_resource_handle(util::TypeResolver *type_resolver, fs::path file_path) {
  auto meta_path = file_path;
  meta_path += kMetaFileExtenstion;

  TempAllocator512 allocator;
  proto::ResourceHandle resource_handle;
  if (fs::exists(meta_path)) {
    Buffer meta_json_buffer{allocator};
    bool success;
    std::tie(meta_json_buffer, success) =
      file_util::read_file_to_buffer(allocator, meta_path.string().c_str());
    XASSERT(success, "Could not load meta file");

    auto meta_json_str = std::string{meta_json_buffer.begin(), meta_json_buffer.end()};

    std::string meta_binary;

    util::JsonToBinaryString(
      type_resolver, get_type_url(resource_handle.GetDescriptor()),
      meta_json_str, &meta_binary);

    resource_handle.ParseFromString(meta_binary);
  } else {
    GuidGenerator guid_generator;
    auto guid = guid_generator.newGuid();
    std::stringstream guid_stream;
    guid_stream << guid;

    resource_handle.set_guid(guid_stream.str());

    write_resource_handle(type_resolver, file_path, resource_handle);
  }

  return resource_handle;
}

ProjectEditor::ProjectEditor(Allocator &allocator, fs::path project_path)
  : project_path_{project_path},
    project_root_{allocator, Guid{}, project_path, nullptr},
    selected_entry_{nullptr},
    error_printer_{},
    source_tree_{},
    importer_{&source_tree_, &error_printer_},
    component_entries_{} {

  type_resolver_.reset(
    util::NewTypeResolverForDescriptorPool(
      kTypeUrlPrefix, protobuf::DescriptorPool::generated_pool()));

  source_tree_.MapPath("", "..");
  source_tree_.MapPath("", "../third-party/protobuf/src");

  auto current_level = 0;
  auto *current_parent = &project_root_;

  protobuf::DynamicMessageFactory message_factory;

  auto project_iterator = fs::recursive_directory_iterator{project_path};
  for (auto &&fs_entry : project_iterator) {
    auto extension = fs_entry.path().extension();
    if  (extension == kMetaFileExtenstion) {
      continue;
    }

    auto type = EntryType::None;

    auto resource_handle = load_resource_handle(type_resolver_.get(), fs_entry);

    if (project_iterator.level() > current_level) {
      current_parent = current_parent->children.back().get();
    } else if (project_iterator.level() < current_level) {
      current_parent = current_parent->parent;
    }
    current_level = project_iterator.level();

    auto entry = allocate_unique<DirectoryEntry>(allocator,
      allocator, resource_handle.guid(), fs_entry, current_parent);

    if (extension == ".proto") {
      auto relative_path = fs::relative(fs_entry, "..");
      auto *file_descriptor = importer_.Import(relative_path.generic_string());

      if (file_descriptor != nullptr) {
        for (auto i = 0; i < file_descriptor->message_type_count(); ++i) {
          auto *message_descriptor = file_descriptor->message_type(i);

          if (message_descriptor->options().GetExtension(knight::proto::component)) {
            auto full_name = message_descriptor->full_name();

            type = EntryType::ComponentSchema;
            component_entries_[full_name] = entry.get();

            auto &defaults = *resource_handle.mutable_defaults();
            if (defaults.find(full_name) == defaults.end()) {
              auto *factorory_component = message_factory.GetPrototype(message_descriptor);
              defaults[full_name].PackFrom(*factorory_component);
            }
          }
        }
      }
    }

    // TODO: Handle this better
    entry->type = type;
    entry->resource_handle = resource_handle;

    current_parent->children.push_back(std::move(entry));
  }
}

bool ProjectEditor::draw() {
  ImGui::Begin("Project");
  auto result = draw_entry(&project_root_);
  ImGui::End();
  return result;
}

bool ProjectEditor::draw_entry(DirectoryEntry *entry) {
  for (auto &&child : entry->children) {
    auto filename = child->path.filename().string();
    if (!child->children.empty()) {
      auto is_open = ImGui::TreeNode(child.get(), "");
      ImGui::SameLine();

      draw_entry_selectable(child.get(), filename.c_str());

      if (is_open) {
        draw_entry(child.get());
        ImGui::TreePop();
      }
    } else {
      draw_entry_selectable(child.get(), filename.c_str());
    }
  }

  return true;
}

void ProjectEditor::draw_entry_selectable(DirectoryEntry *entry, gsl::czstring<> filename) {
  bool selected = entry == selected_entry_;
  ImGui::Selectable(filename, &selected);
  if (selected) {
    selected_entry_ = entry;
  }
}

void ProjectEditor::save_directory_entry(DirectoryEntry &entry) {
  if (entry.dirty) {
    write_resource_handle(type_resolver_.get(), entry.path, entry.resource_handle);
    entry.dirty = false;
  }

  for (auto &&child : entry.children) {
    save_directory_entry(*child);
  }
}

void ProjectEditor::save() {
  save_directory_entry(project_root_);
}

} // namespace editor
} // namespace knight
