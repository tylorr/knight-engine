#pragma once

#include "pointers.h"
#include "vector.h"

#include <boost/filesystem/path.hpp>
#include <memory.h>
#include <gsl.h>
#include <guid.h>

#include <map>
#include <string>

namespace knight {
namespace editor {

enum class EntryType {
  None = 0,
  ComponentSchema = 1,
};

struct DirectoryEntry {
  Guid guid;
  EntryType type;
  boost::filesystem::path path;
  DirectoryEntry *parent;
  Vector<Pointer<DirectoryEntry>> children;

  bool dirty;

  DirectoryEntry(foundation::Allocator &allocator, Guid entry_guid,
                 boost::filesystem::path entry_path,
                 DirectoryEntry *parent_entry)
    : guid{entry_guid},
      path{entry_path},
      parent{parent_entry},
      children{allocator} { }
};

class ProjectEditor {
 public:
  using ComponentEntryMap = std::map<std::string, DirectoryEntry *>;

  ProjectEditor(foundation::Allocator &allocator, boost::filesystem::path project_path);

  const DirectoryEntry *selected_entry() const { return selected_entry_; }
  DirectoryEntry *selected_entry() { return selected_entry_; }

  const ComponentEntryMap &component_entries() const {
    return component_entries_;
  }

  bool draw();
  void save();

 private:
  boost::filesystem::path project_path_;
  DirectoryEntry project_root_;
  DirectoryEntry *selected_entry_;
  ComponentEntryMap component_entries_;

  bool draw_entry(DirectoryEntry *entry);
  void draw_entry_selectable(DirectoryEntry *entry, gsl::czstring<> filename);

  void save_directory_entry(DirectoryEntry &entry);
};

} // namespace editor
} // namespace knight
