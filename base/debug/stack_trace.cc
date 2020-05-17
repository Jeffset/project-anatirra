// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/debug/stack_trace.hpp"

#include <cxxabi.h>
#include <execinfo.h>
#include <link.h>
#include <memory>

namespace base::debug {

StackTrace::StackTrace() noexcept {
  void* trace_elems[MAX_TRACES];
  int count = ::backtrace(trace_elems, MAX_TRACES);
  std::unique_ptr<Dl_info[]> dlinfo{new Dl_info[count]};
  oss_ << "Stacktrace:\n";
  for (int i = 0; i < count; ++i) {
    oss_ << '#' << i << ' ';
    ::dladdr(trace_elems[i], &dlinfo[i]);
    if (auto* name = dlinfo[i].dli_sname) {
      int demangle_status;
      char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &demangle_status);
      if (demangle_status != 0) {
        // failed to demangle, use raw name.
        oss_ << name;
      } else {
        // demangling succeded, use it.
        oss_ << demangled;
        ::free(demangled);
      }
    } else {
      oss_ << "<unknown>";
    }
    if (auto* addr = dlinfo[i].dli_saddr) {
      // Write address if present
      oss_ << " (" << std::hex << addr << ')';
    }
    if (auto* obj_name = dlinfo[i].dli_fname) {
      // Write shared object path if present.
      oss_ << " [" << obj_name << ']';
    }
    if (auto* obj_addr = dlinfo[i].dli_fbase) {
      // Write shared object address if present.
      oss_ << " (" << obj_addr << ')';
    }
    oss_ << '\n';
  }
}

std::string StackTrace::to_string() const noexcept {
  return to_string("");
}

std::string StackTrace::to_string(std::string_view preambula) const noexcept {
  return std::string{preambula} + " <- " + oss_.str();
}

}  // namespace base::debug
