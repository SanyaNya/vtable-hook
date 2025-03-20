#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <bit>
#include <memory>
#include <type_traits>
#include "details/write_to_protected_mem.hpp"

#if defined(_MSC_VER) //MSVC ABI
  #include "details/MSVC_VFuncPtr.hpp"
#else //Itanium ABI
  #include "details/Itanium_VFuncPtr.hpp"
#endif

namespace vthk
{

template<typename T, typename R, typename... Args>
#if defined(_MSC_VER) //MSVC ABI
inline auto hook(const T* instance, R(T::*vfuncptr)(Args...), R(__fastcall *hook)(T*, Args...)) -> R(__fastcall *)(T*, Args...)
#else //Itanium ABI
inline auto hook(const T* instance, R(T::*vfuncptr)(Args...), R(*hook)(T*, Args...)) -> R(*)(T*, Args...)
#endif
{
  static_assert(std::is_polymorphic_v<T>, "Class does not have virtual functions");
  details::VFuncPtr vfp(vfuncptr);

  std::uintptr_t* vtable;
  std::memcpy(&vtable, reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(instance) + vfp.this_adj), sizeof(vtable));

  std::uintptr_t orig_vfuncptr = vtable[vfp.vtable_slot];

  details::write_to_protected_mem(&vtable[vfp.vtable_slot], std::bit_cast<std::uintptr_t>(hook));

#if defined(_MSC_VER) //MSVC ABI
  return std::bit_cast<R(__fastcall *)(T*, Args...)>(orig_vfuncptr);
#else
  return std::bit_cast<R(*)(T*, Args...)>(orig_vfuncptr);
#endif
}

} //namespace vthk
