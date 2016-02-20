#pragma once

#include "pointers.h"
#include "vector.h"

#include <boost/filesystem/path.hpp>
#include <memory.h>
#include <gsl.h>
#include <guid.h>

namespace knight {
namespace editor {

class ProjectEditor {
 public:
  ProjectEditor(foundation::Allocator &allocator, boost::filesystem::path project_path);

  bool Draw();

 private:
  struct DirectoryEntry {
    int id;
    boost::filesystem::path path;
    Guid guid;
    DirectoryEntry *parent;
    Vector<Pointer<DirectoryEntry>> children;

    DirectoryEntry(foundation::Allocator &allocator, int entry_id, 
                   Guid entry_guid,
                   boost::filesystem::path entry_path, 
                   DirectoryEntry *parent_entry)
      : id{entry_id},
        path{entry_path},
        guid{entry_guid},
        parent{parent_entry},
        children{allocator} { }
  };

  boost::filesystem::path project_path_;
  Pointer<DirectoryEntry> project_root_;
  DirectoryEntry *selected_entry_;

  bool DrawEntry(DirectoryEntry *entry);
  void DrawEntrySelectable(DirectoryEntry *entry, gsl::czstring<> filename);
};

} // namespace editor
} // namespace knight
