#include "file_util.h"
#include "string_util.h"

#include <temp_allocator.h>

#include <windows.h>

using namespace foundation;
using namespace gsl;

namespace knight {

using namespace string_util;
using namespace string_stream;

namespace file_util {

namespace detail {
  uint64_t u_int64_file_time(FILETIME windows_file_time) {
    ULARGE_INTEGER large_file_time;
    large_file_time.LowPart = windows_file_time.dwLowDateTime;
    large_file_time.HighPart = windows_file_time.dwHighDateTime;

    return large_file_time.QuadPart;
  }

  bool list_directory_contents(
      foundation::Allocator &allocator, cwzstring<> directory_path, int depth) { 
    // WIN32_FIND_DATA find_data; 
    // HANDLE find_handle = nullptr; 

    // wchar_t search_path[2048]; 

    // //Specify a file mask. *.* = We want everything! 
    // wsprintf(search_path, L"%s/*.*", directory_path); 

    // find_handle = FindFirstFile(search_path, &find_data);
    // if(find_handle == INVALID_HANDLE_VALUE) { 
    //   wprintf(L"Path not found: [%s]\n", directory_path); 
    //   return false; 
    // } 

    // foundation::TempAllocator128 alloc;

    // do { 
    //   //Find first file will always return "."
    //   //    and ".." as the first two directories. 
    //   if(wcscmp(find_data.cFileName, L".") != 0 &&  
    //      wcscmp(find_data.cFileName, L"..") != 0) { 
    //     //Build up our file path using the passed in 
    //     //  [sDir] and the file/foldername we just found: 
    //     wsprintf(search_path, L"%s/%s", directory_path, find_data.cFileName);

    //     File file{allocator};

    //     //Is the entity a File or Folder? 
    //     if(find_data.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) {
    //       printf("Directory: %s\n", string_stream::c_str(Narrow(alloc, search_path)));
    //       // wprintf(L"Directory: %s\n", search_path); 
    //       ListDirectoryContents(allocator, search_path, depth + 1); //Recursion, I love it! 
    //     } else {
    //       printf("File: %s\n", string_stream::c_str(Narrow(alloc, search_path)));
    //     } 
    //   }
    // } while(FindNextFile(find_handle, &find_data)); //Find the next file. 

    // FindClose(find_handle); //Always, Always, clean things up! 

    return true; 
  }
}

uint64_t get_last_write_time(czstring<> filename) {
  FILETIME last_write_time;

  WIN32_FILE_ATTRIBUTE_DATA data;
  if(GetFileAttributesEx(widen(filename).c_str(), GetFileExInfoStandard, &data)) {
    last_write_time = data.ftLastWriteTime;
  }

  return detail::u_int64_file_time(last_write_time);
}

bool list_directory_contents(foundation::Allocator &allocator, czstring<> directory_path) {
  return detail::list_directory_contents(allocator, widen(directory_path).c_str(), 0);
}

} // namespace file_util

} // namespace knight
