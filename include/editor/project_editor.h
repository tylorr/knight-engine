#pragma once

#include "pointers.h"
#include "vector.h"

#include <google/protobuf/compiler/importer.h>
#include <boost/filesystem/path.hpp>
#include <memory.h>
#include <gsl.h>
#include "assets/proto/meta.pb.h"

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

  proto::ResourceHandle resource_handle;

  DirectoryEntry(foundation::Allocator &allocator, Guid entry_guid,
                 boost::filesystem::path entry_path,
                 DirectoryEntry *parent_entry)
    : guid{entry_guid},
      path{entry_path},
      parent{parent_entry},
      children{allocator} { }
};


class ErrorPrinter : public google::protobuf::compiler::MultiFileErrorCollector {
 public:
  void AddError(const std::string& filename, int line, int column,
                const std::string& message) {
    AddErrorOrWarning(filename, line, column, message, "error", std::cerr);
  }

  void AddWarning(const std::string& filename, int line, int column,
                  const std::string& message) {
    AddErrorOrWarning(filename, line, column, message, "warning", std::clog);
  }

 private:
  void AddErrorOrWarning(
      const std::string& filename, int line, int column,
      const std::string& message, const std::string& type, std::ostream& out) {
    out << filename;

    // Users typically expect 1-based line/column numbers, so we add 1
    // to each here.
    if (line != -1) {
      out << ":" << (line + 1) << ":" << (column + 1);
    }

    if (type == "warning") {
      out << ": warning: " << message << std::endl;
    } else {
      out << ": " << message << std::endl;
    }
  }
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

  const google::protobuf::compiler::Importer &importer() const { return importer_; }

  bool draw();

 private:
  boost::filesystem::path project_path_;
  Pointer<DirectoryEntry> project_root_;
  DirectoryEntry *selected_entry_;

  ErrorPrinter error_printer_;
  google::protobuf::compiler::DiskSourceTree source_tree_;
  google::protobuf::compiler::Importer importer_;
  ComponentEntryMap component_entries_;

  bool draw_entry(DirectoryEntry *entry);
  void draw_entry_selectable(DirectoryEntry *entry, gsl::czstring<> filename);
};

} // namespace editor
} // namespace knight
