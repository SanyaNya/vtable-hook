#pragma once

#include <cstdint>

#ifdef _WIN32 //Windows
#include <Windows.h>
#else //POSIX
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace vthk::details{
namespace {

#ifndef _WIN32
const std::uintptr_t pagesize = sysconf(_SC_PAGE_SIZE);
#endif

inline int unprotect_page(void* addr_in_page)
{
#ifdef _WIN32
  MEMORY_BASIC_INFORMATION mbi;
  VirtualQuery(static_cast<LPCVOID>(addr_in_page), &mbi, sizeof(mbi));
  VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);
  return mbi.Protect;
#else
  void* region = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(addr_in_page) & ~(pagesize-1));
  mprotect(region, pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);
  return PROT_READ|PROT_EXEC;
#endif
}

inline void protect_page(void* addr_in_page, int orig_protection)
{
#ifdef WIN32
  MEMORY_BASIC_INFORMATION mbi;
  VirtualQuery(static_cast<LPCVOID>(addr_in_page), &mbi, sizeof(mbi));
  VirtualProtect(mbi.BaseAddress, mbi.RegionSize, orig_protection, &mbi.Protect);
#else
  void* region = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(addr_in_page) & ~(pagesize-1));
  mprotect(region, pagesize, orig_protection);
#endif
}

template<typename T>
inline void write_to_protected_mem(T* addr, T value)
{
  int orig_protection = unprotect_page(static_cast<void*>(addr));

  *addr = value;

  protect_page(static_cast<void*>(addr), orig_protection);
}

}} //namespace vthk::details::anonymous
