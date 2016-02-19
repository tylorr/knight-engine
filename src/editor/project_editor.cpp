#include "editor/project_editor.h"

#include <boost/filesystem/operations.hpp>
#include <imgui.h>

namespace fs = boost::filesystem;
using foundation::Allocator;

namespace knight {
namespace editor {

ProjectEditor::ProjectEditor(Allocator &allocator, fs::path project_path)
  : project_path_{project_path},
    project_root_{allocate_unique<DirectoryEntry>(allocator, allocator, 0, project_path, nullptr)},
    selected_entry_{nullptr} {
  auto current_id = 1;
  auto current_level = 0;
  auto *current_parent = project_root_.get();

  auto asset_itr = fs::recursive_directory_iterator{project_path};
  for (auto &&directory_entry : asset_itr) {
    if (asset_itr.level() > current_level) {
      current_parent = current_parent->children.back().get();
    } else if (asset_itr.level() < current_level) {
      current_parent = current_parent->parent;
    }
    current_level = asset_itr.level();

    auto entry = allocate_unique<DirectoryEntry>(
      allocator, allocator, current_id++, directory_entry, current_parent);
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
