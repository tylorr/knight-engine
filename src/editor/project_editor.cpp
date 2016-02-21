#include "editor/project_editor.h"

#include "meta_generated.h"
#include "file_util.h"

#include <boost/filesystem/operations.hpp>
#include <imgui.h>
#include <logog.hpp>
#include <flatbuffers/idl.h>
#include <temp_allocator.h>

namespace fs = boost::filesystem;
using namespace foundation;

namespace knight {

using namespace string_stream;

namespace editor {

Guid GetOrCreateGuid(fs::path file_path) {
  auto meta_path = file_path;
  meta_path += ".meta";
  
  flatbuffers::Parser parser;
  TempAllocator512 allocator;

  // TODO: Cache loading of this file
  Buffer meta_schema{allocator};
  bool success;
  std::tie(meta_schema, success) = 
    file_util::read_file_to_buffer(allocator, 
      "../assets/schema/editor/meta.fbs");
  XASSERT(success, "Could not load meta schema");
  parser.Parse(c_str(meta_schema));

  Guid guid;
  if (fs::exists(meta_path)) {
    Buffer meta_buffer{allocator};
    std::tie(meta_buffer, success) = 
      file_util::read_file_to_buffer(allocator, meta_path.string().c_str());
    XASSERT(success, "Could not load meta file");
    parser.Parse(c_str(meta_buffer));

    auto resource_handle = 
      schema::GetResourceHandle(parser.builder_.GetBufferPointer());
    guid = Guid{resource_handle->guid()->str()};
  } else {
    GuidGenerator guid_generator;
    guid = guid_generator.newGuid();
    std::stringstream guid_stream;
    guid_stream << guid;

    flatbuffers::FlatBufferBuilder builder;
    auto guid_location = builder.CreateString(guid_stream.str());

    auto resource_handle = schema::CreateResourceHandle(builder, guid_location);
    schema::FinishResourceHandleBuffer(builder, resource_handle);

    std::string json;
    GenerateText(parser, builder.GetBufferPointer(), &json);

    file_util::write_buffer_to_file(meta_path.string().c_str(), json);
  }
  
  return guid;
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

    if (directory_entry.path().extension() == ".meta")
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
