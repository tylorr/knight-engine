#include "editor/project_editor.h"

#include "assets/proto/meta.pb.h"
#include "file_util.h"

#include <boost/filesystem/operations.hpp>
#include <imgui.h>
#include <logog.hpp>
#include <temp_allocator.h>

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>

namespace fs = boost::filesystem;
using namespace foundation;
using namespace google::protobuf;

namespace knight {

using namespace string_stream;

namespace editor {

static const char *kTypeUrlPrefix = "type.googleapis.com";
static gsl::czstring<> kMetaFileExtenstion = ".meta";

static std::string get_type_url(const Descriptor* message) {
  return std::string{kTypeUrlPrefix} + "/" + message->full_name();
}

Guid GetOrCreateGuid(fs::path file_path) {
  auto meta_path = file_path;
  meta_path += kMetaFileExtenstion;
  
  TempAllocator512 allocator;
  std::unique_ptr<util::TypeResolver> resolver{
    util::NewTypeResolverForDescriptorPool(
      kTypeUrlPrefix, DescriptorPool::generated_pool())};

  editor::proto::ResourceHandle resource_handle;
  if (fs::exists(meta_path)) {
    Buffer meta_json_buffer{allocator};
    bool success;
    std::tie(meta_json_buffer, success) = 
      file_util::read_file_to_buffer(allocator, meta_path.string().c_str());
    XASSERT(success, "Could not load meta file");

    auto meta_json_str = std::string{meta_json_buffer.begin(), meta_json_buffer.end()};

    std::string meta_binary;
    
    util::JsonToBinaryString(
      resolver.get(), get_type_url(resource_handle.GetDescriptor()), 
      meta_json_str, &meta_binary);

    resource_handle.ParseFromString(meta_binary);
  } else {
    GuidGenerator guid_generator;
    auto guid = guid_generator.newGuid();
    std::stringstream guid_stream;
    guid_stream << guid;

    resource_handle.set_guid(guid_stream.str());

    util::JsonOptions options;
    options.add_whitespace = true;

    std::string meta_json;
    util::BinaryToJsonString(resolver.get(),
                             get_type_url(resource_handle.GetDescriptor()),
                             resource_handle.SerializeAsString(), &meta_json, options);

    file_util::write_buffer_to_file(meta_path.string().c_str(), meta_json);
  }
  
  return resource_handle.guid();
}

ProjectEditor::ProjectEditor(Allocator &allocator, fs::path project_path)
  : project_path_{project_path},
    project_root_{allocate_unique<DirectoryEntry>(
      allocator, allocator, 0, Guid{}, project_path, nullptr)},
    selected_entry_{nullptr} {
  auto current_id = 1;
  auto current_level = 0;
  auto *current_parent = project_root_.get();

  auto asset_itr = fs::recursive_directory_iterator{project_path};
  for (auto &&directory_entry : asset_itr) {

    if (directory_entry.path().extension() == kMetaFileExtenstion)
      continue;

    if (asset_itr.level() > current_level) {
      current_parent = current_parent->children.back().get();
    } else if (asset_itr.level() < current_level) {
      current_parent = current_parent->parent;
    }
    current_level = asset_itr.level();

    auto guid = GetOrCreateGuid(directory_entry);

    auto entry = allocate_unique<DirectoryEntry>(
      allocator, allocator, current_id++, guid, directory_entry, current_parent);
    current_parent->children.push_back(std::move(entry));
  }
}

bool ProjectEditor::Draw() {
  ImGui::Begin("Project");
  auto result = DrawEntry(project_root_.get());
  ImGui::End();
  return result;
}

bool ProjectEditor::DrawEntry(DirectoryEntry *entry) {
  for (auto &&child : entry->children) {
    const auto *filename = child->path.filename().string().c_str();
    if (!child->children.empty()) {
      auto is_open = ImGui::TreeNode(reinterpret_cast<void *>(child->id), "");
      ImGui::SameLine();

      DrawEntrySelectable(child.get(), filename);

      if (is_open) {
        DrawEntry(child.get());
        ImGui::TreePop();
      }
    } else {
      DrawEntrySelectable(child.get(), filename);
    }
  }

  return true;
}

void ProjectEditor::DrawEntrySelectable(DirectoryEntry *entry, gsl::czstring<> filename) {
  bool selected = entry == selected_entry_;
  ImGui::Selectable(filename, &selected);
  if (selected) {
    selected_entry_ = entry;
  }
}

} // namespace editor
} // namespace knight
