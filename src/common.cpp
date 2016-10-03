#include "common.h"

#include <memory.h>

#include <iostream>

#include <windows.h>
#include <dbghelp.h>

namespace knight {

void stack_trace() {
  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();
  
  CONTEXT context;
  memset(&context, 0, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext(&context);
  
  SymInitialize(process, NULL, TRUE);
  
  DWORD image;
  STACKFRAME64 stackframe;
  ZeroMemory(&stackframe, sizeof(STACKFRAME64));
  
#ifdef _M_IX86
  image = IMAGE_FILE_MACHINE_I386;
  stackframe.AddrPC.Offset = context.Eip;
  stackframe.AddrPC.Mode = AddrModeFlat;
  stackframe.AddrFrame.Offset = context.Ebp;
  stackframe.AddrFrame.Mode = AddrModeFlat;
  stackframe.AddrStack.Offset = context.Esp;
  stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
  image = IMAGE_FILE_MACHINE_AMD64;
  stackframe.AddrPC.Offset = context.Rip;
  stackframe.AddrPC.Mode = AddrModeFlat;
  stackframe.AddrFrame.Offset = context.Rsp;
  stackframe.AddrFrame.Mode = AddrModeFlat;
  stackframe.AddrStack.Offset = context.Rsp;
  stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
  image = IMAGE_FILE_MACHINE_IA64;
  stackframe.AddrPC.Offset = context.StIIP;
  stackframe.AddrPC.Mode = AddrModeFlat;
  stackframe.AddrFrame.Offset = context.IntSp;
  stackframe.AddrFrame.Mode = AddrModeFlat;
  stackframe.AddrBStore.Offset = context.RsBSP;
  stackframe.AddrBStore.Mode = AddrModeFlat;
  stackframe.AddrStack.Offset = context.IntSp;
  stackframe.AddrStack.Mode = AddrModeFlat;
#endif

  StackWalk64(
      image, process, thread,
      &stackframe, &context, NULL, 
      SymFunctionTableAccess64, SymGetModuleBase64, NULL);

  for (size_t i = 0; i < 25; i++) {
    
    BOOL result = StackWalk64(
      image, process, thread,
      &stackframe, &context, NULL, 
      SymFunctionTableAccess64, SymGetModuleBase64, NULL);
    
    if (!result) { break; }
    
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;
    
    DWORD64 displacement = 0;

    std::cout << i << ": ";

    if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {
      // IMAGEHLP_MODULE64 moduleInfo;
      // memset(&moduleInfo, 0, sizeof(IMAGEHLP_MODULE64));
      // moduleInfo.SizeOfStruct = sizeof(moduleInfo);

      // if (SymGetModuleInfo64(process, symbol->ModBase, &moduleInfo)) {
      //   std::cout << moduleInfo.ModuleName << ": ";
      // }
      std::cout << symbol->Name << " ";// << " + 0x" << std::hex << displacement;

      IMAGEHLP_LINE64 line;
      DWORD line_displacement;
      SymSetOptions(SYMOPT_LOAD_LINES);
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      
      if (SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, &line_displacement, &line)) {
        std::cout << "at " << line.FileName << ":" << line.LineNumber;
      }
    } else {
      std::cout << "???";
    }

    std::cout << std::endl;
  }
  
  SymCleanup(process);
}

void *knight_malloc(size_t size) {
  auto &allocator = foundation::memory_globals::default_allocator();
  return allocator.allocate(size);
}

void knight_free(void *ptr) {
  auto &allocator = foundation::memory_globals::default_allocator();
  allocator.deallocate(ptr);
}

void knight_no_memory() {
  XASSERT(false, "Udp packet failed to allocate");
}

} // namespace knight
