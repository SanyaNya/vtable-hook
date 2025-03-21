#pragma once

#include <cstdint>
#include <system_error>

#ifdef _WIN32 //Windows
#include <Windows.h>
#else //POSIX
#include <cerrno>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace vthk::details{
namespace {

#ifndef _WIN32
const std::uintptr_t pagesize = sysconf(_SC_PAGE_SIZE);
#endif

inline int set_vtable_page_write_protection(void* addr_in_page, int orig_protection)
{
#ifdef _WIN32
  MEMORY_BASIC_INFORMATION mbi;
  if(!VirtualQuery(static_cast<LPCVOID>(addr_in_page), &mbi, sizeof(mbi)))
    throw std::system_error(::GetLastError(), std::system_category());

  if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, orig_protection ? orig_protection : PAGE_READWRITE, &mbi.Protect))
    throw std::system_error(::GetLastError(), std::system_category());

  return mbi.Protect;
#else
  void* region = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(addr_in_page) & ~(pagesize-1));
  if(mprotect(region, pagesize, orig_protection ? orig_protection : PROT_READ|PROT_WRITE|PROT_EXEC) == -1)
    throw std::system_error(errno, std::system_category());
  return PROT_READ|PROT_EXEC;
#endif
}

template<typename T>
inline void write_to_protected_mem(T* addr, T value)
{
  int orig_protection = set_vtable_page_write_protection(static_cast<void*>(addr), 0);

  *addr = value;

  set_vtable_page_write_protection(static_cast<void*>(addr), orig_protection);
}

}} //namespace vthk::details::anonymous
